#pragma once
#include "utils.hh"

#include <common/system/cron.hh>
#include <common/net/http/http_server.hh>

#include <atomic>
#include <cassert>
#include <list>

namespace softeq
{
namespace common
{
namespace net
{
namespace http
{
class HttpConnectionImpl;

class HttpServerImpl final
{
public:
    HttpServerImpl(const IHttpServer::settings_t &settings, IHttpConnectionDispatcher &dispatcher);

    ~HttpServerImpl();

    bool start();

    void stop();

    void incConnectionCounter()
    {
        _connectionsCounter++;
    }
    void decConnectionCounter()
    {
        _connectionsCounter--;
    }

private:
    bool handle(IHttpConnection &connection);

    static void mhdLogger(void *cls, const char *fm, va_list ap);

    void waitUntilRequestsCompleted();

    void processSession(HttpConnectionImpl &connection);
    int pruneSessionsOnExpiration();

    bool startServerHttp();

    static ssize_t fileReader(void *cls, uint64_t pos, char *buf, size_t max);

    static void fileFreeCallback(void *cls);

    static bool parseRange(const std::string &str, Range &range);

    static MHD_Response *createResponseFromFilerange(const std::string &filename, const Range &file_range,
                                                     const int filesize);

    int handleHttpRequest(MHD_Connection *mhd_conn, HttpConnectionImpl &http_conn);

    static void mhdPostProcess(HttpConnectionImpl *http_conn, const char *data, size_t data_size);

    static void mhdRequestCompleted(void *cls, MHD_Connection *conn, void **con_cls, enum MHD_RequestTerminationCode);

    static int mhdEventHandler(void *cls, MHD_Connection *connection, const char *url, const char *method,
                               const char *version, const char *upload_data, size_t *upload_data_size,
                               void **conCls) noexcept;

    const IHttpServer::settings_t &_settings;
    IHttpConnectionDispatcher &_dispatcher;
    struct MHD_Daemon *_server{nullptr};
    std::atomic<bool> _goingToStop{false};
    std::atomic<unsigned> _connectionsCounter{0};

    std::unique_ptr<char[]> _keyBuffer;
    std::unique_ptr<char[]> _certBuffer;
    std::list<HttpSession::SPtr> _sessions;
    softeq::common::system::Cron::UPtr _cron;
};

} // namespace http
} // namespace net
} // namespace common
} // namespace softeq
