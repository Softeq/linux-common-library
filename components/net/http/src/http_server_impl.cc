#include "http_server_impl.hh"
#include "http_connection_impl.hh"

#include <common/system/cron.hh>
#include <common/system/fsutils.hh>
#include <common/system/time_provider.hh>
#include <common/logging/log.hh>
#include <common/stdutils/optional.hh>
#include <common/stdutils/timeutils.hh>
#include <common/stdutils/stdutils.hh>

#include <algorithm>
#include <cassert>
#include <cstring>
#include <memory>

#include <fcntl.h>
#include <cinttypes>
#include <sys/stat.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <microhttpd.h>

namespace
{
const std::map<std::string, softeq::common::net::http::Method> methodTypeFromString{
    {MHD_HTTP_METHOD_GET, softeq::common::net::http::Method::GET},
    {MHD_HTTP_METHOD_POST, softeq::common::net::http::Method::POST},
    {MHD_HTTP_METHOD_PUT, softeq::common::net::http::Method::PUT},
    {MHD_HTTP_METHOD_DELETE, softeq::common::net::http::Method::DELETE},
    {MHD_HTTP_METHOD_OPTIONS, softeq::common::net::http::Method::OPTIONS}};

sockaddr_in createAddressFor(const std::string &ipAddress, uint16_t port)
{
    sockaddr_in addressToUse;
    addressToUse.sin_family = AF_INET;
    addressToUse.sin_port = htons(port);
    inet_pton(AF_INET, ipAddress.c_str(), &addressToUse.sin_addr);

    return addressToUse;
}

} // namespace

namespace softeq
{
namespace common
{
namespace net
{
namespace http
{
/// Implementation of HttpServerImpl
HttpServerImpl::HttpServerImpl(const IHttpServer::settings_t &settings, IHttpConnectionDispatcher &dispatcher)
    : _settings(settings)
    , _dispatcher(dispatcher)
    , _cron(common::system::CronFactory::create())
{
    _cron->addJob("Check HTTP sessions expiration", "* * * * * *",
                  std::bind(&HttpServerImpl::pruneSessionsOnExpiration, this));
}

HttpServerImpl::~HttpServerImpl()
{
    stop();
}

bool HttpServerImpl::handle(IHttpConnection &connection)
{
    if (_goingToStop)
    {
        connection.setError(HttpStatusCode::STATUS_SERVICE_UNAVAILABLE, "The web server is about to shutdown.");
        return false;
    }
    bool result = _dispatcher.handle(connection);
    if (!connection.error())
    {
        connection.setError(HttpStatusCode::STATUS_OK);
    }
    return result;
}

void HttpServerImpl::waitUntilRequestsCompleted()
{
    static const char *const message = "Wait until existing requests are being handled...";
    constexpr int sleep_us = 100 * 1000;
    constexpr int max_attempts = 10;

    int attempt = 0;
    while (0 < _connectionsCounter)
    {
        usleep(sleep_us);
        if (max_attempts <= ++attempt)
        {
            attempt = 0;
            LOGD(LOG_DOMAIN, message);
        }
    }
}

bool HttpServerImpl::start()
{
    if (_settings.enableSecure && MHD_is_feature_supported(MHD_FEATURE_TLS) == MHD_NO)
    {
        LOGE(LOG_DOMAIN, "HTTPS is not available");
    }

    if (!startServerHttp())
    {
        LOGE(LOG_DOMAIN, "Couldn't start web server!");
        return false;
    }

    _cron->start();

    _goingToStop = false;
    LOGI(LOG_DOMAIN, "Web server started on port %d", _settings.port);
    return true;
}

void HttpServerImpl::stop()
{
    if (!_server)
    {
        return;
    }

    _cron->stop();
    LOGD(LOG_DOMAIN, "Stop accepting new requests.");
    _goingToStop = true;
    MHD_socket fd = MHD_quiesce_daemon(_server);

    waitUntilRequestsCompleted();

    LOGD(LOG_DOMAIN, "Stopping of web server...");
    MHD_stop_daemon(_server);

    if (fd != MHD_INVALID_SOCKET)
    {
        LOGD(LOG_DOMAIN, "Closing socket");
        close(fd);
    }
    else
    {
        LOGE(LOG_DOMAIN, "MHD socket is invalid, nothing to close");
    }
    LOGI(LOG_DOMAIN, "Web server stopped.");
    _server = nullptr;
}

bool HttpServerImpl::startServerHttp()
{
    assert(_server == nullptr);

    sockaddr_in forcedAddress;
    if (!(_settings.address.empty()))
    {
        forcedAddress = createAddressFor(_settings.address, _settings.port);
    }
    sockaddr_in *forcedAddressPtr = (_settings.address.empty()) ? nullptr : &forcedAddress;

    if (_settings.enableSecure)
    {
        if (!common::system::readBinaryFileIntoBuffer(_settings.certFilePath, _certBuffer))
        {
            LOGE(LOG_DOMAIN, "Couldn't load certificate from file %s.", _settings.certFilePath.c_str());
        }

        if (!common::system::readBinaryFileIntoBuffer(_settings.keyFilePath, _keyBuffer))
        {
            LOGE(LOG_DOMAIN, "Couldn't load key from file %s.", _settings.keyFilePath.c_str());
        }

        // clang-format off
        _server = MHD_start_daemon(
            MHD_USE_THREAD_PER_CONNECTION | MHD_USE_INTERNAL_POLLING_THREAD | MHD_USE_DEBUG |
            MHD_USE_ITC | MHD_USE_TLS,
            _settings.port, nullptr, nullptr, &mhdEventHandler, this,
            MHD_OPTION_EXTERNAL_LOGGER        , mhdLogger, nullptr,
            MHD_OPTION_NOTIFY_COMPLETED       , mhdRequestCompleted, nullptr,
            MHD_OPTION_CONNECTION_LIMIT       , cHttpDaemonConnectionsLimit,
            MHD_OPTION_PER_IP_CONNECTION_LIMIT, cHttpDaemonConnectionsPerIPLimit,
            MHD_OPTION_LISTENING_ADDRESS_REUSE, cHttpDaemonAddressReuse,
            MHD_OPTION_SOCK_ADDR              , forcedAddressPtr,
            MHD_OPTION_HTTPS_MEM_KEY          , _keyBuffer.get(),
            MHD_OPTION_HTTPS_MEM_CERT         , _certBuffer.get(),
            MHD_OPTION_END);
        // clang-format on
    }
    else
    {
        // clang-format off
        _server = MHD_start_daemon(
            MHD_USE_THREAD_PER_CONNECTION | MHD_USE_INTERNAL_POLLING_THREAD | MHD_USE_DEBUG |
            MHD_USE_ITC,
            _settings.port, nullptr, nullptr, &mhdEventHandler, this,
            MHD_OPTION_EXTERNAL_LOGGER        , mhdLogger, nullptr,
            MHD_OPTION_NOTIFY_COMPLETED       , mhdRequestCompleted, nullptr,
            MHD_OPTION_CONNECTION_LIMIT       , cHttpDaemonConnectionsLimit,
            MHD_OPTION_PER_IP_CONNECTION_LIMIT, cHttpDaemonConnectionsPerIPLimit,
            MHD_OPTION_LISTENING_ADDRESS_REUSE, cHttpDaemonAddressReuse,
            MHD_OPTION_SOCK_ADDR              , forcedAddressPtr,
            MHD_OPTION_END);
        // clang-format on
    }

    return _server != nullptr;
}

MHD_Response *HttpServerImpl::createResponseFromFilerange(const std::string &filename, const Range &file_range,
                                                          const int filesize)
{
    LOGT(LOG_DOMAIN, "Send portion of file content in range: %d-%d/%d", file_range._pos_begin, file_range._pos_end,
         filesize);

    MHD_Response *response = nullptr;
    FILE *file = ::fopen(filename.c_str(), "rb");
    if (file != nullptr)
    {
        FileReaderContext *fr_ctx = new FileReaderContext(file, file_range._pos_begin);
        // maximal size to read is equal to length of range, the same for size of block to read
        response = MHD_create_response_from_callback(file_range.length(), file_range.length(), &fileReader, fr_ctx,
                                                     &fileFreeCallback);
        if (!response)
            delete fr_ctx;
    }
    else
    {
        LOGE(LOG_DOMAIN, "Can't open file to read: %s", filename.c_str());
    }
    return response;
}

int HttpServerImpl::handleHttpRequest(MHD_Connection *mhdConn, HttpConnectionImpl &httpConn)
{
    // TODO: need RAII here
    MHD_Response *response = nullptr;

#ifndef NDEBUG
    /* Set response headers */
    // disabled to prevent DOS attack
    httpConn.setResponseHeader("Access-Control-Allow-Origin", "*");
#endif
    if (handle(httpConn) && httpConn.error() == MHD_HTTP_OK)
    {
        if (auto s = httpConn.session().lock())
        {
            auto iter = std::find_if(_sessions.begin(), _sessions.end(),
                                     [&](HttpSession::SPtr &session) { return session->id() == s->id(); });

            if (iter == _sessions.end())
            {
                _sessions.push_back(s);
            }
            httpConn.setResponseHeader("X-Session", s->id());
            httpConn.setResponseHeader("X-Session-Expiry",
                                       softeq::common::stdutils::timestamp_to_string(s->expiration()));
        }

        if (!httpConn.streamName().empty())
        {
            std::string filename(httpConn.streamName());
            struct stat buf;

            if (stat(filename.c_str(), &buf) == 0)
            {
                LOGD(LOG_DOMAIN, "Send file: %s", filename.c_str());
                LOGT(LOG_DOMAIN, "File size %" PRId64 "", static_cast<int64_t>(buf.st_size));

                // TODO: what if file size is zero
                Range file_range(0, buf.st_size > 0 ? (int)buf.st_size - 1
                                                    : 0); // _end is a number of the last byte (not file size)
                std::string range_header_val = httpConn.header("Range");
                LOGD(LOG_DOMAIN, "Requested range (as string): %s", range_header_val.c_str());
                if (!range_header_val.empty())
                {
                    Range range;
                    if (ParseRange(range_header_val, range))
                    {
                        LOGD(LOG_DOMAIN, "Requested range (parsed): %d-%d", range._pos_begin, range._pos_end);

                        if (range._pos_begin < buf.st_size) // TODO: if file size is zero
                        {
                            if (range._pos_begin > file_range._pos_begin)
                                file_range._pos_begin = range._pos_begin;
                            if (range._pos_end <= file_range._pos_end)
                                file_range._pos_end = range._pos_end;

                            response = createResponseFromFilerange(filename, file_range, (int)buf.st_size);

                            if (response)
                                httpConn.setError(MHD_HTTP_PARTIAL_CONTENT);
                            else
                                httpConn.setError(MHD_HTTP_INTERNAL_SERVER_ERROR); /* Couldn't open file */

                            std::string range_header_val2 = softeq::common::stdutils::string_format(
                                "bytes %d-%d/%d", file_range._pos_begin,
                                (file_range._pos_end > 0 ? file_range._pos_end : 0), buf.st_size);

                            MHD_add_response_header(response, "Content-Range", range_header_val2.c_str());
                            MHD_add_response_header(response, "Content-Length",
                                                    std::to_string(file_range.length()).c_str());
                        }
                        else
                        {
                            // Not satisfiable range
                            LOGE(LOG_DOMAIN, "Required range %d-%d is invalid for file %s", range._pos_begin,
                                 range._pos_end, filename.c_str());
                            httpConn.setError(MHD_HTTP_RANGE_NOT_SATISFIABLE);
                        }
                    }
                    else
                    {
                        LOGE(LOG_DOMAIN, "Couldn't parse Range header.");
                        httpConn.setError(MHD_HTTP_RANGE_NOT_SATISFIABLE);
                    }
                }
                else
                {
                    int fd = ::open(filename.c_str(), O_RDONLY);
                    if (fd != -1)
                        response = MHD_create_response_from_fd(buf.st_size, fd);

                    if (response == nullptr)
                    {
                        // file exist but not accessible for read
                        LOGE(LOG_DOMAIN, "Couldn't open file to send '%s'", filename.c_str());
                        httpConn.setError(MHD_HTTP_NOT_ACCEPTABLE);
                    }
                }
            }
            else
            {
                LOGE(LOG_DOMAIN, "File to send '%s' wasn't found", filename.c_str());
                httpConn.setError(MHD_HTTP_NOT_FOUND);
            }
        }
        else
        {
            /* Execution of command was successful and don't needed to send any file */
            std::string content(httpConn.strResponse());
            LOGT(LOG_DOMAIN, "Output HTTP content: %s", content.c_str());
            response = MHD_create_response_from_buffer(content.length(), const_cast<char *>(content.c_str()),
                                                       MHD_RESPMEM_MUST_COPY);
        }
    }

    if (httpConn.error() != MHD_HTTP_OK && !response)
    {
        std::string content(httpConn.strResponse());
        LOGT(LOG_DOMAIN, "Output HTTP content: %s", content.c_str());
        response = MHD_create_response_from_buffer(content.length(), const_cast<char *>(content.c_str()),
                                                   MHD_RESPMEM_MUST_COPY);
    }

    for (const std::pair<const std::string, std::string> &header : httpConn.responseHeaders())
    {
        MHD_add_response_header(response, header.first.c_str(), header.second.c_str());
    }

    int ret = MHD_queue_response(mhdConn, httpConn.error(), response);
    MHD_destroy_response(response);

    return ret;
}

int HttpServerImpl::mhdEventHandler(void *cls, MHD_Connection *connection, const char *url, const char *method,
                                    const char *version, const char *uploadData, size_t *uploadDataSize,
                                    void **conCls) noexcept
{
    assert(cls);
    (void)version;

    HttpServerImpl *httpServer = static_cast<HttpServerImpl *>(cls);
    if (httpServer->_goingToStop)
    {
        std::string content("The server is going to shut down");

        MHD_Response *response = MHD_create_response_from_buffer(content.length(), const_cast<char *>(content.c_str()),
                                                                 MHD_RESPMEM_MUST_COPY);
        MHD_queue_response(connection, HttpStatusCode::STATUS_SERVICE_UNAVAILABLE, response);
        MHD_destroy_response(response);

        return MHD_NO;
    }

    HttpConnectionImpl *httpConn = static_cast<HttpConnectionImpl *>(*conCls);
    Method methodType;

    try
    {
        methodType = methodTypeFromString.at(method);
    }
    catch (const std::out_of_range &)
    {
        LOGE(LOG_DOMAIN, "Unsupported method: \'%s\' of http-request. URL: %s", method, url);
        return MHD_NO;
    }

    if (httpConn == nullptr)
    {
        LOGI(LOG_DOMAIN, "Got HTTP request (%s): %s", method, url);
        /* std::bad_alloc is possible */

        if (methodType == Method::OPTIONS)
        {
            MHD_Response *response = MHD_create_response_from_buffer(0, nullptr, MHD_RESPMEM_PERSISTENT);
#ifndef NDEBUG
            MHD_add_response_header(response, "Access-Control-Allow-Origin", "*");
#endif
            MHD_add_response_header(response, "Access-Control-Allow-Methods", "GET, POST, PUT, OPTIONS, DELETE");
            MHD_add_response_header(response, "Access-Control-Allow-Headers",
                                    "authorization,cache-control,content-type,pragma,signature");
            MHD_add_response_header(response, "Access-Control-Allow-Credentials", "true");
            MHD_add_response_header(response, "Content-Type", "text/plain");
            MHD_add_response_header(response, "Content-Length", "0");
            MHD_queue_response(connection, MHD_HTTP_OK, response);
            MHD_destroy_response(response);
        }
        else
        {
            httpConn = new HttpConnectionImpl(connection, url, methodType, std::string(uploadData, *uploadDataSize),
                                              *httpServer);
            *conCls = httpConn;

            httpServer->processSession(*httpConn);
        }

        return MHD_YES;
    }

    // By this moment the cookie has been sent and
    // there is no need to send it once again
    //    http_connection->removeHeader("Set-Cookie");

    if (*uploadDataSize != 0)
    {
        mhdPostProcess(httpConn, uploadData, *uploadDataSize);
        *uploadDataSize = 0;
        return MHD_YES;
    }
    else
    {
        return httpServer->handleHttpRequest(connection, *httpConn);
    }
}

void HttpServerImpl::mhdPostProcess(HttpConnectionImpl *http_conn, const char *data, size_t data_size)
{
    assert(http_conn);

    http_conn->appendBodyData(data, data_size);
}

void HttpServerImpl::processSession(HttpConnectionImpl &connection)
{
    if (!connection.requestHasHeader("X-Session"))
    {
        return;
    }

    std::string sessionId = connection.header("X-Session");

    auto iter = std::find_if(_sessions.begin(), _sessions.end(),
                             [&](HttpSession::SPtr &session) { return session->id() == sessionId; });

    if (iter == _sessions.end())
    {
        LOGD(LOG_DOMAIN, "The session '%s' does not exist", sessionId.c_str());
        return;
    }

    connection.attachSession(*iter);
}

int HttpServerImpl::pruneSessionsOnExpiration()
{
    std::time_t currentTime = system::TimeProvider::instance()->now();

    _sessions.remove_if([currentTime](HttpSession::WPtr session) {
        if (auto s = session.lock())
        {
            std::time_t sessionExpiration = s->expiration();
            if (sessionExpiration <= currentTime)
            {
                s->onExpire();
                return true;
            }
        }
        return false;
    });
    return 0;
}

ssize_t HttpServerImpl::fileReader(void *cls, uint64_t pos, char *buf, size_t max)
{
    FileReaderContext *ctx = static_cast<FileReaderContext *>(cls);

    (void)::fseek(ctx->file(), ctx->offset() + pos, SEEK_SET);
    size_t rd_count = ::fread(buf, 1, max, ctx->file());

    LOGT(LOG_DOMAIN, "File reader callback has read %u bytes.", static_cast<uint32_t>(rd_count));
    return rd_count;
}

void HttpServerImpl::fileFreeCallback(void *cls)
{
    FileReaderContext *ctx = static_cast<FileReaderContext *>(cls);
    delete ctx;
}

void HttpServerImpl::mhdLogger(void *cls, const char *fm, va_list ap)
{
    (void)cls;
    char buf[1024] = {'\0'};
    ::vsnprintf(buf, sizeof(buf), fm, ap);
    LOGD(LOG_DOMAIN, "MHD-log-message: %s", buf);
}

void HttpServerImpl::mhdRequestCompleted(void *cls, MHD_Connection *conn, void **con_cls,
                                         enum MHD_RequestTerminationCode)
{
    (void)cls;
    (void)conn;
    HttpConnectionImpl *http_conn = static_cast<HttpConnectionImpl *>(*con_cls);
    delete http_conn;
    *con_cls = nullptr;
}

} // namespace http
} // namespace net
} // namespace common
} // namespace softeq
