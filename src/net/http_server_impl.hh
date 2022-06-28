#pragma once

#include <atomic>
#include <cassert>
#include <list>

#include <softeq/common/cron.hh>
#include "softeq/common/net/http_server.hh"
#include "softeq/common/net/http_session.hh"

#include "utils.hh"

namespace softeq
{
namespace common
{
namespace net
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

    void processSessions(HttpConnectionImpl &connection);
    int checkSessionsExpiration();

    bool startServerHttp();
#ifdef HTTPS_ENABLED
    bool startServerHttps();
#endif
    static ssize_t fileReader(void *cls, uint64_t pos, char *buf, size_t max);

    static void fileFreeCallback(void *cls);

    static bool parseRange(const std::string &str, Range &range);

    static MHD_Response *createResponseFromFilerange(const std::string &filename,
                                                     const Range &file_range,
                                                     const int filesize);

    int handleHttpRequest(MHD_Connection *mhd_conn, HttpConnectionImpl &http_conn);

    static void mhdPostProcess(HttpConnectionImpl *http_conn, const char *data, size_t data_size);

    static void mhdRequestCompleted(void *cls, MHD_Connection *conn, void **con_cls, enum MHD_RequestTerminationCode);

    static int mhdEventHandler(void *cls, MHD_Connection *connection, const char *url, const char *method,
                               const char *version, const char *upload_data, size_t *upload_data_size,
                               void **conCls) noexcept;

    const IHttpServer::settings_t &_settings;
    IHttpConnectionDispatcher &_dispatcher;
    struct MHD_Daemon *_server;
    std::atomic<bool> _goingToStop;
    std::atomic<unsigned> _connectionsCounter;

    std::unique_ptr<char[]> _keyBuffer;
    std::unique_ptr<char[]> _certBuffer;
    std::list<HttpSession::SPtr> _sessions;
    int _hostId;
    ICron::UPtr _cron;
};

} // namespace net
} // namespace common
} // namespace softeq
