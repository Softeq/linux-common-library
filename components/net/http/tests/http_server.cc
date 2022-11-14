#include <ctime>
#include <gtest/gtest.h>

#include <common/net/http/http_connection.hh>
#include <common/net/http/http_server.hh>
#include <common/net/curl_helper/curl_helper.hh>
#include <common/stdutils/timeutils.hh>
#include <common/logging/log.hh>
#include <system/testtimeprovider.hh>

#include <memory>
#include <stdexcept>
#include <string>

#include <random>
#include <fstream>
#include <future>
#include <chrono>
#include <utility>

#include <sys/stat.h>
#include <uuid/uuid.h>

#include <ifaddrs.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netdb.h>
#include <net/if.h>

using namespace softeq::common;
using namespace softeq::common::net::http;
using namespace softeq::common::net::curl;

namespace
{
const char *LOG_DOMAIN = "HttpServerTest";
const std::string firstIpAddr{"127.0.0.1"};
std::string secondIpAddr;

const std::string rootFileDirName{"/tmp"};
const std::string endpointRoot{"/"};
const std::string endpointUpload{"/upload"};
const std::string endpointClient{"/client"};
const std::string endpointDownloadFile{"/download_file.txt"};
const std::string endpointCreateSession{"/create_session"};
const std::string endpointSessionData{"/session_data"};
const std::string endpointCreateSessionWithSleep{"/create_session_with_sleep"};

const std::string headerXSession{"X-Session"};
const std::string headerXSessionExpiry{"X-Session-Expiry"};
} // namespace

std::string execCmdInTerminal(const char *cmd);

class TestHttpDispatcher final : public IHttpConnectionDispatcher
{
public:
    struct TestSession : public HttpSession
    {
        std::string data;
        virtual void onExpire()
        {
            LOGI(LOG_DOMAIN, "The session %d has expired", id());
        }
    };

    bool handle(IHttpConnection &connection) override
    {
        if (connection.path() == endpointRoot)
        {
            // do nothing
            return true;
        }
        if (connection.path() == endpointUpload)
        {
            _uploadedContent = connection.body();
            return true;
        }
        if (connection.path() == endpointClient)
        {
            connection << connection.clientDescription();
            return true;
        }
        if (connection.path() == endpointDownloadFile)
        {
            std::string fileName{rootFileDirName + connection.path()};
            connection.sendFile(fileName);
            return true;
        }
        if (connection.path() == endpointCreateSession)
        {
            auto session(std::make_shared<TestSession>());
            session->data = connection.body();
            session->lastActivity();
            connection.attachSession(session);
            return true;
        }
        if (connection.path() == endpointSessionData)
        {
            // it will throw exception in the bad case
            std::shared_ptr<TestSession> session = std::dynamic_pointer_cast<TestSession>(connection.session().lock());
            if (!session)
            {
                connection.setError(HttpStatusCode::STATUS_NOT_FOUND);
                return false;
            }
            connection.setResponseHeader("Content-type", "text/plain");
            connection << session->data;
            return true;
        }
        if (connection.path() == endpointCreateSessionWithSleep)
        {
            auto session(std::make_shared<TestSession>());
            session->data = connection.body();
            session->lastActivity();
            connection.attachSession(session);

            // sleep for 1 sec to have a time to do something async
            std::this_thread::sleep_for(std::chrono::milliseconds(1500));
            return true;
        }

        LOGE(LOG_DOMAIN, "TestHttpDispatcher: unknown endpoint");
        return false;
    }

    std::string uploadedContent()
    {
        return _uploadedContent;
    }

private:
    std::string _uploadedContent;
};

class HttpServerTest : public testing::Test
{
protected:
    HttpServerTest()
        : _timeProvider(system::TimeProvider::instance(std::make_shared<TestTimeProvider>()))
        , _server(new HttpServer(_serverSettings, _dispatcher))
    {
        secondIpAddr = getRealIpV4();
    }

    void SetUp() override
    {
        _serverSettings.port = 8080;
        _serverSettings.enableSecure = false;

        ASSERT_TRUE(_server->start());
    }

    void TearDown() override
    {
        // clean up test file
        {
            remove(_testFileName.c_str());
            errno = 0;
        }
        _server->stop();
    }

    std::string getRealIpV4()
    {
        struct ifaddrs *ifaddr, *ifa;
        char host[NI_MAXHOST] = {};

        if (getifaddrs(&ifaddr) == -1)
        {
            throw std::runtime_error("getifaddrs");
        }

        for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next)
        {
            if (ifa->ifa_addr == NULL)
            {
                continue;
            }
            if (ifa->ifa_addr->sa_family == AF_INET && (ifa->ifa_flags & IFF_LOOPBACK) == 0)
            {
                break;
            }
        }
        if (ifa)
        {
            if (getnameinfo(ifa->ifa_addr, sizeof(struct sockaddr_in), host, NI_MAXHOST, NULL, 0, NI_NUMERICHOST) != 0)
            {
                throw std::runtime_error("getnameinfo() failed");
            }
        }

        freeifaddrs(ifaddr);
        return host;
    }

    std::string getRandomString(int length)
    {
        static const char alphanum[] = "0123456789"
                                       "!@#$%^&*"
                                       "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                                       "abcdefghijklmnopqrstuvwxyz";
        std::string s;
        for (int i = 0; i < length; ++i)
        {
            s += alphanum[rand() % (sizeof(alphanum) - 1)];
        }
        return s;
    }

    std::string createTestFile(int randomDataLength)
    {
        std::string data = getRandomString(randomDataLength);
        _testFileName = {rootFileDirName + endpointDownloadFile};
        std::ofstream file(_testFileName);
        file << data;
        file.close();

        return data;
    }

    void checkClientInfo(std::string serverIp)
    {
        CurlHelper curl(serverIp + ":8080" + endpointClient);
        ASSERT_TRUE(curl.doGet());
        EXPECT_EQ(curl.responseCode(), HttpStatusCode::STATUS_OK);
        ASSERT_TRUE(curl.responseText() == serverIp);
    }

    void checkDownloadedData(std::string serverIp, std::string data, bool shouldBeDownloadedSuccessfull = true)
    {
        CurlHelper curl(serverIp + ":8080" + endpointDownloadFile);
        if (!shouldBeDownloadedSuccessfull)
        {
            ASSERT_FALSE(curl.doGet());
            return;
        }
        ASSERT_TRUE(curl.doGet());

        EXPECT_EQ(curl.responseCode(), HttpStatusCode::STATUS_OK);
        ASSERT_TRUE(curl.responseText() == data);
    }

    std::string _testFileName = "";
    IHttpServer::settings_t _serverSettings;
    TestHttpDispatcher _dispatcher;
    std::shared_ptr<system::TimeProvider> _timeProvider;
    std::unique_ptr<HttpServer> _server;

    const std::string _serverUrl{"http://localhost:8080"};
    const std::string _endpointUpload{"/upload"};
};

TEST(HttpServerTestBase, StartStop)
{
    IHttpServer::settings_t settings;
    TestHttpDispatcher dispatcher;
    settings.port = 8080;
    EXPECT_NO_FATAL_FAILURE({ HttpServer _server(settings, dispatcher); });

    EXPECT_NO_FATAL_FAILURE({
        HttpServer _server(settings, dispatcher);
        _server.start();
    });

    EXPECT_NO_FATAL_FAILURE({
        HttpServer _server(settings, dispatcher);
        _server.start();
        _server.stop();
        _server.start();
    });

    EXPECT_NO_FATAL_FAILURE({
        HttpServer _server(settings, dispatcher);
        _server.start();
        _server.stop();
        _server.stop();
    });
}

TEST_F(HttpServerTest, StartOnFirstIpAddress)
{
    _server->stop();
    _serverSettings.address = firstIpAddr;
    ASSERT_TRUE(_server->start());
    std::string data = createTestFile(5);
    checkDownloadedData(firstIpAddr, data, true);
    checkDownloadedData(secondIpAddr, data, false);
}

TEST_F(HttpServerTest, ClientData)
{
    _server->stop();
    _serverSettings.address.clear();
    ASSERT_TRUE(_server->start());
    checkClientInfo(firstIpAddr);
}

TEST_F(HttpServerTest, StartOnSecondIpAddress)
{
    _server->stop();
    _serverSettings.address = secondIpAddr;
    ASSERT_TRUE(_server->start());
    std::string data = createTestFile(5);
    checkDownloadedData(firstIpAddr, data, false);
    checkDownloadedData(secondIpAddr, data, true);
}

TEST_F(HttpServerTest, StartOnBothIpAddress)
{
    _server->stop();
    _serverSettings.address.clear();
    ASSERT_TRUE(_server->start());
    std::string data = createTestFile(5);
    checkDownloadedData(firstIpAddr, data, true);
    checkDownloadedData(secondIpAddr, data, true);
}

TEST_F(HttpServerTest, StarUseNotAvailableSecureKeys)
{
    _server->stop();
    _serverSettings.enableSecure = true;
    ASSERT_FALSE(_server->start());
}

TEST_F(HttpServerTest, UploadContent)
{
    std::string _uploadContent = getRandomString(8192);
    CurlHelper curl(_serverUrl + _endpointUpload);

    // send data to the server
    ASSERT_TRUE(curl.doPost(_uploadContent));
    // check uploaded data
    ASSERT_TRUE(_dispatcher.uploadedContent() == _uploadContent);
}

TEST_F(HttpServerTest, DownloadFile)
{
    std::string data = createTestFile(16384);
    checkDownloadedData(firstIpAddr, std::move(data), true);
}

TEST_F(HttpServerTest, DownloadFile_FileNotAccessible)
{
    std::string data = createTestFile(5);

    // chmod write only, which not allow to open for read
    EXPECT_EQ(chmod(_testFileName.c_str(), S_IWUSR), 0);

    CurlHelper curl(_serverUrl + endpointDownloadFile);
    curl.setVerbose(true);
    ASSERT_TRUE(curl.doGet());

    // check response
    LOGD(LOG_DOMAIN, "'%s'", curl.responseHeaders().c_str());
    EXPECT_EQ(curl.responseCode(), STATUS_NOT_ACCEPTABLE);
    EXPECT_EQ(curl.responseText().size(), 0);
    ASSERT_TRUE(curl.responseText() == "");
    ASSERT_TRUE(curl.responseHeaders().find("Content-Length: 0") != std::string::npos);
    ASSERT_TRUE(curl.errorString() == "");
}

class CurlHelperExt : public CurlHelper
{
public:
    explicit CurlHelperExt(const std::string &url)
        : CurlHelper(url)
    {
    }
    virtual ~CurlHelperExt(){};

    bool doUnsupportedMethod()
    {
        curl_easy_setopt(_curl_handle, CURLOPT_URL, _url.c_str());
        curl_easy_setopt(_curl_handle, CURLOPT_CUSTOMREQUEST, "FOO_METHOD");
        curl_easy_setopt(_curl_handle, CURLOPT_NOSIGNAL, 1);
        if (_verbose)
        {
            curl_easy_setopt(_curl_handle, CURLOPT_STDERR, stdout);
            curl_easy_setopt(_curl_handle, CURLOPT_VERBOSE, 1L);
        }

        _curlErrorCode = curl_easy_perform(_curl_handle);

        if (_curlErrorCode)
        {
            LOGE(LOG_DOMAIN, "curl_easy_perform failed: %s", curl_easy_strerror(_curlErrorCode));
            return false;
        }

        return true;
    }

    void doDoubleCreateSessionWithSleep(const std::string &_serverUrl)
    {
        std::string id;
        std::time_t expTime;
        CurlHelper req1(_serverUrl + endpointCreateSessionWithSleep);

        // do 1st request
        ASSERT_TRUE(req1.doGet());
        // check request 1 response
        LOGD(LOG_DOMAIN, "'%s'", req1.responseHeaders().c_str());
        EXPECT_EQ(req1.responseCode(), HttpStatusCode::STATUS_OK);
        ASSERT_TRUE(req1.responseText().empty());

        // check request 1 X-Session header
        ASSERT_TRUE(req1.responseHasHeader(headerXSession));
        id = req1.responseHeader(headerXSession);

        // check request 1 X-Session-Expiry header
        ASSERT_TRUE(req1.responseHasHeader(headerXSessionExpiry));
        expTime = stdutils::string_to_timestamp(req1.responseHeader(headerXSessionExpiry));
        ASSERT_TRUE(expTime != -1);

        // do 2d request with X-Session header from 1st
        req1.addRequestHeader(headerXSession, id);

        // set timeout to not waste time when server goes down
        curl_easy_setopt(req1._curl_handle, CURLOPT_TIMEOUT, 1);

        // wait for a fail because in sleep time for the 1st request was send server->stop()
        ASSERT_FALSE(req1.doGet());
    }

    bool doOptionsMethod()
    {
        curl_easy_setopt(_curl_handle, CURLOPT_URL, _url.c_str());
        curl_easy_setopt(_curl_handle, CURLOPT_CUSTOMREQUEST, "OPTIONS");
        curl_easy_setopt(_curl_handle, CURLOPT_NOSIGNAL, 1);
        if (_verbose)
        {
            curl_easy_setopt(_curl_handle, CURLOPT_STDERR, stdout);
            curl_easy_setopt(_curl_handle, CURLOPT_VERBOSE, 1L);
        }

        _response.host = this;
        _response.bytes.clear();

        curl_easy_setopt(_curl_handle, CURLOPT_WRITEDATA, &_response);
        curl_easy_setopt(_curl_handle, CURLOPT_WRITEFUNCTION, CurlHelper::writeDataCallback);

        _response.headers.clear();
        curl_easy_setopt(_curl_handle, CURLOPT_HEADERDATA, &_response);
        curl_easy_setopt(_curl_handle, CURLOPT_HEADERFUNCTION, CurlHelper::headerDataCallback);

        if (_headerList)
        {
            curl_easy_setopt(_curl_handle, CURLOPT_HTTPHEADER, _headerList);
        }

        // Exec
        _curlErrorCode = curl_easy_perform(_curl_handle);

        if (!_curlErrorCode)
        {
            parseResponseHeaders();
        }

        if (_headerList)
        {
            curl_slist_free_all(_headerList);
            _headerList = nullptr;
        }

        if (_curlErrorCode)
        {
            LOGE(LOG_DOMAIN, "curl_easy_perform failed: %s", curl_easy_strerror(_curlErrorCode));
            return false;
        }

        return true;
    }
};

TEST_F(HttpServerTest, UnsupportedMethodWhileServerDown)
{
    // Test to emulate situation when came 2 requests in a one connection
    // ans server goes down while 1st request is not finished

    // send 2 CreateSession requests with 1sec sleep
    CurlHelperExt curlExt(_serverUrl);
    auto async = std::async(std::launch::async, &CurlHelperExt::doDoubleCreateSessionWithSleep, &curlExt, _serverUrl);

    // wait for be sure that async already started
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    // try to down server at the moment of the 1st CreateSessionWithSleep request
    _server->stop();
}

TEST_F(HttpServerTest, UnsupportedHttpMethod)
{
    CurlHelperExt curl(_serverUrl);

    LOGD(LOG_DOMAIN, "Send Unsupported request");
    // expects that server just closes connection without any return data
    ASSERT_FALSE(curl.doUnsupportedMethod());
}

TEST_F(HttpServerTest, OptionsHttpMethod)
{
    CurlHelperExt curl(_serverUrl);
    ASSERT_TRUE(curl.doOptionsMethod());

    LOGD(LOG_DOMAIN, "%s", curl.responseHeaders().c_str());
    EXPECT_EQ(curl.responseCode(), HttpStatusCode::STATUS_OK);
    EXPECT_EQ(curl.responseText().size(), 0);
    ASSERT_TRUE(curl.responseHasHeader("Access-Control-Allow-Credentials"));
    ASSERT_TRUE(curl.responseHasHeader("Access-Control-Allow-Headers"));
    ASSERT_TRUE(curl.responseHasHeader("Access-Control-Allow-Methods"));
    ASSERT_TRUE(curl.responseHasHeader("Access-Control-Allow-Origin"));
    ASSERT_TRUE(curl.errorString() == "");
}

TEST_F(HttpServerTest, DownloadFileWithRange1_ok_StartDataRange)
{
    std::string data = createTestFile(16384);

    CurlHelper curl(_serverUrl + endpointDownloadFile);
    curl.addRequestHeader("Range", "bytes=0-1023");
    ASSERT_TRUE(curl.doGet());

    // check response
    // LOGD(LOG_DOMAIN, "'%s'", curl.responseHeaders().c_str());
    EXPECT_EQ(curl.responseCode(), 206);
    EXPECT_EQ(curl.responseText().size(), 1024);
    ASSERT_TRUE(curl.responseText() == data.substr(0, 1024 /*len*/));
    ASSERT_TRUE(curl.responseHeaders().find("Content-Length: 1024") != std::string::npos);
    ASSERT_TRUE(curl.responseHeaders().find("Content-Range: bytes 0-1023/16384") != std::string::npos);
    ASSERT_TRUE(curl.errorString() == "");
}

TEST_F(HttpServerTest, DownloadFileWithRange2_ok_EndDataRange)
{
    std::string data{"1234567890"};
    std::string fileName{"/tmp" + endpointDownloadFile};

    std::ofstream file(fileName);
    file << data;
    file.close();

    CurlHelper curl(_serverUrl + endpointDownloadFile);
    curl.addRequestHeader("Range", "bytes=5-9");
    ASSERT_TRUE(curl.doGet());

    // check response
    // LOGD(LOG_DOMAIN, "'%s'", curl.responseHeaders().c_str());
    EXPECT_EQ(curl.responseCode(), 206);
    EXPECT_EQ(curl.responseText().size(), 5);
    ASSERT_TRUE(curl.responseText() == data.substr(5, 5 /*len*/));
    ASSERT_TRUE(curl.responseHeaders().find("Content-Length: 5") != std::string::npos);
    ASSERT_TRUE(curl.responseHeaders().find("Content-Range: bytes 5-9/10") != std::string::npos);
    ASSERT_TRUE(curl.errorString() == "");

    // clean up
    remove(fileName.c_str());
}

TEST_F(HttpServerTest, DownloadFileWithRange2_ok_MiddleDataRange)
{
    std::string data{"1234567890"};
    std::string fileName{"/tmp" + endpointDownloadFile};

    std::ofstream file(fileName);
    file << data;
    file.close();

    CurlHelper curl(_serverUrl + endpointDownloadFile);
    curl.addRequestHeader("Range", "bytes=2-7");
    ASSERT_TRUE(curl.doGet());

    // check response
    // LOGD(LOG_DOMAIN, "'%s'", curl.responseHeaders().c_str());
    EXPECT_EQ(curl.responseCode(), 206);
    EXPECT_EQ(curl.responseText().size(), 6);
    ASSERT_TRUE(curl.responseText() == data.substr(2, 6 /*len*/));
    ASSERT_TRUE(curl.responseHeaders().find("Content-Length: 6") != std::string::npos);
    ASSERT_TRUE(curl.responseHeaders().find("Content-Range: bytes 2-7/10") != std::string::npos);
    ASSERT_TRUE(curl.errorString() == "");

    // clean up
    remove(fileName.c_str());
}

TEST_F(HttpServerTest, DownloadFileWithRange2_ok_1byte)
{
    std::string data = createTestFile(5);

    CurlHelper curl(_serverUrl + endpointDownloadFile);
    curl.addRequestHeader("Range", "bytes=0-0");
    ASSERT_TRUE(curl.doGet());

    // check response
    // LOGD(LOG_DOMAIN, "'%s'", curl.responseHeaders().c_str());
    EXPECT_EQ(curl.responseCode(), 206);
    EXPECT_EQ(curl.responseText().size(), 1);
    ASSERT_TRUE(curl.responseText() == data.substr(0, 1 /*len*/));
    ASSERT_TRUE(curl.responseHeaders().find("Content-Length: 1") != std::string::npos);
    ASSERT_TRUE(curl.responseHeaders().find("Content-Range: bytes 0-0/5") != std::string::npos);
    ASSERT_TRUE(curl.errorString() == "");
}

TEST_F(HttpServerTest, DownloadFileWithRange3_ok_2bytes)
{
    std::string data = createTestFile(5);

    CurlHelper curl(_serverUrl + endpointDownloadFile);
    curl.setVerbose(true);
    curl.addRequestHeader("Range", "bytes=0-1");
    ASSERT_TRUE(curl.doGet());

    // check response
    LOGD(LOG_DOMAIN, "'%s'", curl.responseHeaders().c_str());
    EXPECT_EQ(curl.responseCode(), 206);
    EXPECT_EQ(curl.responseText().size(), 2);
    ASSERT_TRUE(curl.responseText() == data.substr(0, 2 /*len*/));
    ASSERT_TRUE(curl.responseHeaders().find("Content-Length: 2") != std::string::npos);
    ASSERT_TRUE(curl.responseHeaders().find("Content-Range: bytes 0-1/5") != std::string::npos);
    ASSERT_TRUE(curl.errorString() == "");
}

TEST_F(HttpServerTest, DownloadFileWithRange4_NoBytesPrefix)
{
    std::string data = createTestFile(5);

    CurlHelper curl(_serverUrl + endpointDownloadFile);
    curl.setVerbose(true);
    curl.addRequestHeader("Range", "0-1"); // should be 'bytes=0-1'
    ASSERT_TRUE(curl.doGet());

    // check response
    // LOGD(LOG_DOMAIN, "'%s'", curl.responseHeaders().c_str());
    EXPECT_EQ(curl.responseCode(), 416);
    EXPECT_EQ(curl.responseText().size(), 0);
    ASSERT_TRUE(curl.responseText() == "");
    ASSERT_TRUE(curl.responseHeaders().find("Content-Length: 0") != std::string::npos);
    ASSERT_TRUE(curl.errorString() == "");
}

TEST_F(HttpServerTest, DownloadFileWithRange5_StartOffsetMoreFileSize)
{
    std::string data = createTestFile(5);

    CurlHelper curl(_serverUrl + endpointDownloadFile);
    curl.setVerbose(true);
    curl.addRequestHeader("Range", "bytes=100-200");
    ASSERT_TRUE(curl.doGet());

    // check response
    // LOGD(LOG_DOMAIN, "'%s'", curl.responseHeaders().c_str());
    EXPECT_EQ(curl.responseCode(), 416);
    EXPECT_EQ(curl.responseText().size(), 0);
    ASSERT_TRUE(curl.responseText() == "");
    ASSERT_TRUE(curl.responseHeaders().find("Content-Length: 0") != std::string::npos);
    ASSERT_TRUE(curl.errorString() == "");
}

TEST_F(HttpServerTest, DownloadFileWithRange6_StartOffsetMoreEndOffset)
{
    std::string data = createTestFile(5);

    CurlHelper curl(_serverUrl + endpointDownloadFile);
    curl.setVerbose(true);
    curl.addRequestHeader("Range", "bytes=200-100");
    ASSERT_TRUE(curl.doGet());

    // check response
    // LOGD(LOG_DOMAIN, "'%s'", curl.responseHeaders().c_str());
    EXPECT_EQ(curl.responseCode(), 416);
    EXPECT_EQ(curl.responseText().size(), 0);
    ASSERT_TRUE(curl.responseText() == "");
    ASSERT_TRUE(curl.responseHeaders().find("Content-Length: 0") != std::string::npos);
    ASSERT_TRUE(curl.errorString() == "");
}

TEST_F(HttpServerTest, DownloadFileWithRange7_FileNotFound)
{
    std::string data = getRandomString(1000);
    std::string fileName{"/tmp" + endpointDownloadFile + ".foo"};

    std::ofstream file(fileName);
    file << data;
    file.close();

    CurlHelper curl(_serverUrl + endpointDownloadFile);
    curl.setVerbose(true);
    curl.addRequestHeader("Range", "bytes=0-99");
    ASSERT_TRUE(curl.doGet());

    // check response
    LOGD(LOG_DOMAIN, "'%s'", curl.responseHeaders().c_str());
    EXPECT_EQ(curl.responseCode(), STATUS_NOT_FOUND);
    EXPECT_EQ(curl.responseText().size(), 0);
    ASSERT_TRUE(curl.responseText() == "");
    ASSERT_TRUE(curl.responseHeaders().find("Content-Length: 0") != std::string::npos);
    ASSERT_TRUE(curl.errorString() == "");

    // clean up
    remove(fileName.c_str());
}

TEST_F(HttpServerTest, DownloadFileWithRange8_FileNotAccessible)
{
    std::string data = createTestFile(5);

    // chmod write only, which not allow to open for read
    EXPECT_EQ(chmod(_testFileName.c_str(), S_IWUSR), 0);

    CurlHelper curl(_serverUrl + endpointDownloadFile);
    curl.setVerbose(true);
    curl.addRequestHeader("Range", "bytes=0-1");
    ASSERT_TRUE(curl.doGet());

    // check response
    LOGD(LOG_DOMAIN, "'%s'", curl.responseHeaders().c_str());
    EXPECT_EQ(curl.responseCode(), STATUS_INTERNAL_ERROR);
    EXPECT_EQ(curl.responseText().size(), 0);
    ASSERT_TRUE(curl.responseText().empty());
    ASSERT_TRUE(curl.responseHeaders().find("Content-Length: 0") != std::string::npos);
    ASSERT_TRUE(curl.errorString().empty());
}

TEST_F(HttpServerTest, Session_UpdateExpirationTime)
{
    std::string id1;
    std::string id2;
    std::time_t exp1;
    std::time_t exp2;
    std::time_t timeToSleep = 10;

    {
        LOGD(LOG_DOMAIN, "=============== Request 1 ===============");

        CurlHelper req1(_serverUrl + endpointCreateSession);
        ASSERT_TRUE(req1.doGet());

        // check request 1 response
        LOGD(LOG_DOMAIN, "Response 1\n%s", req1.responseHeaders().c_str());
        EXPECT_EQ(req1.responseCode(), HttpStatusCode::STATUS_OK);
        ASSERT_TRUE(req1.responseText().empty());

        // check request 1 X-Session header
        ASSERT_TRUE(req1.responseHasHeader(headerXSession));
        id1 = req1.responseHeader(headerXSession);

        // check request 1 X-Session-Expiry header
        ASSERT_TRUE(req1.responseHasHeader(headerXSessionExpiry));
        exp1 = stdutils::string_to_timestamp(req1.responseHeader(headerXSessionExpiry));
        ASSERT_TRUE(exp1 != -1);
    }
    _timeProvider->sleep(timeToSleep);

    {
        LOGD(LOG_DOMAIN, "=============== Request 2 ===============");

        CurlHelper req2(_serverUrl);
        req2.addRequestHeader(headerXSession, id1);
        ASSERT_TRUE(req2.doGet());
        LOGD(LOG_DOMAIN, "Response 2\n%s", req2.responseHeaders().c_str());

        // check request 2 response
        EXPECT_EQ(req2.responseCode(), HttpStatusCode::STATUS_OK);
        ASSERT_TRUE(req2.responseText().empty());

        // check request 2 X-Session header
        ASSERT_TRUE(req2.responseHasHeader(headerXSession));
        id2 = req2.responseHeader(headerXSession);

        // check request 2 X-Session-Expiry header
        ASSERT_TRUE(req2.responseHasHeader(headerXSessionExpiry));
        exp2 = stdutils::string_to_timestamp(req2.responseHeader(headerXSessionExpiry));
        ASSERT_TRUE(exp2 != -1);
    }

    ASSERT_TRUE(id1 == id2);
    EXPECT_EQ(exp2 - exp1, timeToSleep);
}

TEST_F(HttpServerTest, Session_ExpireSession)
{
    std::string id1;
    std::string id2;
    std::time_t exp1;
    //    std::time_t exp2;
    std::time_t current = _timeProvider->now();

    {
        LOGD(LOG_DOMAIN, "=============== Request 1 ===============");

        CurlHelper req1(_serverUrl + endpointCreateSession);
        ASSERT_TRUE(req1.doGet());

        // check request 1 response
        EXPECT_EQ(req1.responseCode(), HttpStatusCode::STATUS_OK);
        ASSERT_TRUE(req1.responseText().empty());

        // check request 1 X-Session header
        ASSERT_TRUE(req1.responseHasHeader(headerXSession));
        id1 = req1.responseHeader(headerXSession);

        // check request 1 X-Session-Expiry header
        ASSERT_TRUE(req1.responseHasHeader(headerXSessionExpiry));
        exp1 = stdutils::string_to_timestamp(req1.responseHeader(headerXSessionExpiry));
        ASSERT_TRUE(exp1 != -1);
    }

    _timeProvider->sleep(exp1 - current);

    {
        LOGD(LOG_DOMAIN, "=============== Request 2 ===============");

        CurlHelper req2(_serverUrl);
        req2.addRequestHeader(headerXSession, id1);
        ASSERT_TRUE(req2.doGet());
        LOGD(LOG_DOMAIN, "'%s'", req2.responseHeaders().c_str());

        // check request 2 response
        EXPECT_EQ(req2.responseCode(), HttpStatusCode::STATUS_OK);
        ASSERT_TRUE(req2.responseText().empty());

        // check request 2 X-Session header
        EXPECT_FALSE(req2.responseHasHeader(headerXSession));
        id2 = req2.responseHeader(headerXSession);

        // check request 2 X-Session-Expiry header
        EXPECT_FALSE(req2.responseHasHeader(headerXSessionExpiry));
        // exp2 = stdutils::string_to_timestamp(req2.responseHeader(headerXSessionExpiry));
    }
}

TEST_F(HttpServerTest, Session_CreateNewSessionFromPreviousOne)
{
    std::string id1;
    std::string id2;
    time_t exp1;
    time_t exp2;

    {
        LOGD(LOG_DOMAIN, "=============== Request 1 ===============");

        // Send request to create a new Session
        CurlHelper req1(_serverUrl + endpointCreateSession);
        req1.setVerbose(true);
        ASSERT_TRUE(req1.doGet());

        // check request 1 response
        EXPECT_EQ(req1.responseCode(), HttpStatusCode::STATUS_OK);
        ASSERT_TRUE(req1.responseText().empty());
        ASSERT_TRUE(req1.errorString().empty());

        // check request 1 X-Session header
        ASSERT_TRUE(req1.responseHasHeader(headerXSession));
        id1 = req1.responseHeader(headerXSession);

        // check request 1 X-Session-Expiry header
        ASSERT_TRUE(req1.responseHasHeader(headerXSessionExpiry));
        exp1 = stdutils::string_to_timestamp(req1.responseHeader(headerXSessionExpiry));
        ASSERT_TRUE(exp1 != -1);
    }
    _timeProvider->sleep(10);

    {
        LOGD(LOG_DOMAIN, "=============== Request 2 ===============");

        // Send request to create a new Session with the header of the previous Session
        CurlHelper req2(_serverUrl + endpointCreateSession);
        req2.setVerbose(true);
        req2.addRequestHeader(headerXSession, id1);
        ASSERT_TRUE(req2.doGet());

        // check request 2 response
        LOGD(LOG_DOMAIN, "'%s'", req2.responseHeaders().c_str());
        EXPECT_EQ(req2.responseCode(), HttpStatusCode::STATUS_OK);
        ASSERT_TRUE(req2.responseText().empty());

        // check request 2 X-Session header
        ASSERT_TRUE(req2.responseHasHeader(headerXSession));
        id2 = req2.responseHeader(headerXSession);

        // check request 2 X-Session-Expiry header
        ASSERT_TRUE(req2.responseHasHeader(headerXSessionExpiry));
        exp2 = stdutils::string_to_timestamp(req2.responseHeader(headerXSessionExpiry));
        ASSERT_TRUE(exp2 != -1);
    }
    {
        LOGD(LOG_DOMAIN, "=============== Request 3 ===============");

        // Send request to restore first session
        CurlHelper req3(_serverUrl);
        req3.setVerbose(true);
        req3.addRequestHeader(headerXSession, id1);
        ASSERT_TRUE(req3.doGet());

        // check request 3 response
        LOGD(LOG_DOMAIN, "'%s'", req3.responseHeaders().c_str());
        EXPECT_EQ(req3.responseCode(), HttpStatusCode::STATUS_OK);
        ASSERT_TRUE(req3.responseText().empty());

        // check request 3 X-Session header
        ASSERT_TRUE(req3.responseHasHeader(headerXSession));
        EXPECT_EQ(id1, req3.responseHeader(headerXSession)) << "First restored";

        // check request 3 X-Session-Expiry header
        ASSERT_TRUE(req3.responseHasHeader(headerXSessionExpiry));
        exp1 = stdutils::string_to_timestamp(req3.responseHeader(headerXSessionExpiry));
        ASSERT_TRUE(exp2 != -1);
    }

    // uuid of second session must be different
    ASSERT_TRUE(id1 != id2) << "The session is new";
    EXPECT_EQ(exp1, exp2) << "Both are touched";
}

TEST_F(HttpServerTest, Session_SessionData)
{
    std::string id1;
    std::string id2;
    std::time_t exp1;
    std::time_t exp2;
    std::string sessionData{"session_internal_data"};

    {
        LOGD(LOG_DOMAIN, "=============== Request 1 ===============");

        // Send request to create a new Session with some session data
        CurlHelper req1(_serverUrl + endpointCreateSession);
        req1.setVerbose(true);
        req1.addRequestHeader("Content-Type", "text/plain");
        ASSERT_TRUE(req1.doPost(sessionData));

        // check request 1 response
        EXPECT_EQ(req1.responseCode(), HttpStatusCode::STATUS_OK);
        ASSERT_TRUE(req1.responseText().empty());

        // check request 1 X-Session header
        ASSERT_TRUE(req1.responseHasHeader(headerXSession));
        id1 = req1.responseHeader(headerXSession);

        // check request 1 X-Session-Expiry header
        exp1 = stdutils::string_to_timestamp(req1.responseHeader(headerXSessionExpiry));
        ASSERT_TRUE(exp1 != -1);
    }
    {
        LOGD(LOG_DOMAIN, "=============== Request 2 ===============");

        // Send request to create a new Session with the header of the previous Session
        CurlHelper req2(_serverUrl + endpointSessionData);
        req2.setVerbose(true);

        req2.addRequestHeader(headerXSession, id1);
        ASSERT_TRUE(req2.doGet());

        // check request 2 response
        EXPECT_EQ(req2.responseCode(), HttpStatusCode::STATUS_OK);
        ASSERT_TRUE(req2.responseText() == sessionData);

        // check request 2 X-Session header
        ASSERT_TRUE(req2.responseHasHeader(headerXSession));
        id2 = req2.responseHeader(headerXSession);

        // check request 2 X-Session-Expiry header
        exp2 = stdutils::string_to_timestamp(req2.responseHeader(headerXSessionExpiry));
        ASSERT_TRUE(exp2 != -1);
    }

    ASSERT_TRUE(id1 == id2);
    EXPECT_EQ(exp1, exp2);
}

TEST_F(HttpServerTest, Session_IgnoreWrongSession)
{
    CurlHelper curl(_serverUrl);
    curl.setVerbose(true);
    curl.addRequestHeader(headerXSession, "11111111-1111-1111-1111-111111111111");
    ASSERT_TRUE(curl.doGet());

    // check response
    EXPECT_EQ(curl.responseCode(), HttpStatusCode::STATUS_OK);
    ASSERT_TRUE(curl.responseText().empty());
    ASSERT_FALSE(curl.responseHasHeader(headerXSession));
    ASSERT_FALSE(curl.responseHasHeader(headerXSessionExpiry));
}

std::string execCmdInTerminal(const char *cmd)
{
    std::array<char, 128> buffer;
    std::string result;
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd, "r"), pclose);
    if (!pipe)
    {
        throw std::runtime_error("popen() failed!");
    }
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr)
    {
        result += buffer.data();
    }
    return result;
}
