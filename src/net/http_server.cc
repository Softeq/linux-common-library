#include "softeq/common/net/http_server.hh"

#include <stdexcept>

#include "http_server_impl.hh"

namespace softeq
{
namespace common
{
namespace net
{
HttpServer::HttpServer(const settings_t &settings, IHttpConnectionDispatcher &dispatcher)
    : _impl(new HttpServerImpl(settings, dispatcher))
{
}

HttpServer::~HttpServer()
{
}

bool HttpServer::start()
{
    return _impl->start();
}

void HttpServer::stop()
{
    _impl->stop();
}

} // namespace net
} // namespace common
} // namespace softeq
