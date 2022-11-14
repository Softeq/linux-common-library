#include "http_server.hh"

#include "http_server_impl.hh"

#include <stdexcept>

using namespace softeq::common::net::http;

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
