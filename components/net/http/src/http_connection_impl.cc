#include "http_connection_impl.hh"
#include "http_server_impl.hh"
#include "system/time_provider.hh"
#include "utils.hh"

#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <microhttpd.h>

namespace softeq
{
namespace common
{
namespace net
{
namespace http
{
/// Implementation of HttpConnectionImpl
HttpConnectionImpl::HttpConnectionImpl(MHD_Connection *connection, const std::string &url, Method method,
                                       const std::string &body, HttpServerImpl &owner)
    : _connection(connection)
    , _url(url)
    , _body(body)
    , _owner(owner)
    , _method(method)
{
    _owner.incConnectionCounter();
}

HttpConnectionImpl::~HttpConnectionImpl()
{
    _owner.decConnectionCounter();
}

// TODO: At the moment clinet description contains the client address only
std::string HttpConnectionImpl::clientDescription() const
{
    const MHD_ConnectionInfo *info = MHD_get_connection_info(_connection, MHD_CONNECTION_INFO_CLIENT_ADDRESS);
    struct sockaddr *res = info->client_addr;
    std::string result;
    switch (res->sa_family)
    {
    case AF_INET:
    {
        char s[INET_ADDRSTRLEN] = {'\0'};
        struct sockaddr_in *addr_in = (struct sockaddr_in *)res;
        inet_ntop(AF_INET, &(addr_in->sin_addr), s, INET_ADDRSTRLEN);
        result = s;
        break;
    }
    case AF_INET6:
    {
        char s[INET6_ADDRSTRLEN] = {'\0'};
        struct sockaddr_in6 *addr_in6 = (struct sockaddr_in6 *)res;
        inet_ntop(AF_INET6, &(addr_in6->sin6_addr), s, INET6_ADDRSTRLEN);
        result = s;
        break;
    }
    default:
        break;
    }

    return result;
}

std::string HttpConnectionImpl::header(const std::string &name) const
{
    const char *value = MHD_lookup_connection_value(_connection, MHD_HEADER_KIND, name.c_str());
    return value ? value : "";
}

bool HttpConnectionImpl::requestHasHeader(const std::string &name) const
{
    return MHD_lookup_connection_value(_connection, MHD_HEADER_KIND, name.c_str()) != nullptr ? true : false;
}

bool HttpConnectionImpl::responseHasHeader(const std::string &name) const
{
    return _responseHeaders.find(name) != _responseHeaders.end();
}

std::string HttpConnectionImpl::cookie(const std::string &key) const
{
    const char *value = MHD_lookup_connection_value(_connection, MHD_COOKIE_KIND, key.c_str());
    return value ? value : "";
}

bool HttpConnectionImpl::hasCookie(const std::string &key) const
{
    ParamsMap params;
    return MHD_get_connection_values(_connection, MHD_COOKIE_KIND, &MHD_getParamsIter, &params) &&
           params.find(key) != params.end();
}

bool HttpConnectionImpl::setCookie(const std::string &key, const std::string &value)
{
    int res = MHD_set_connection_value(_connection, MHD_COOKIE_KIND, key.c_str(), value.c_str());

    return (res == MHD_YES) ? true : false;
}

void HttpConnectionImpl::removeResponseHeader(const std::string &name)
{
    for (auto it = _responseHeaders.begin(); it != _responseHeaders.end();)
    {
        if (it->first == name)
        {
            // erase returns the iterator following the removed elements
            it = _responseHeaders.erase(it);
        }
        else
        {
            ++it;
        }
    }
}

void HttpConnectionImpl::setError(int error, const std::string &error_message)
{
    setError(error);
    if (!error_message.empty())
        (*this) << error_message;
}

std::string HttpConnectionImpl::path() const
{
    std::size_t pos = _url.find('?');
    return ((pos != std::string::npos) ? _url.substr(0, pos) : _url);
}

std::string HttpConnectionImpl::field(const std::string &name) const
{
    const char *value = MHD_lookup_connection_value(_connection, MHD_GET_ARGUMENT_KIND, name.c_str());
    return value ? value : "";
}

bool HttpConnectionImpl::hasField(const std::string &name) const
{
    ParamsMap params;
    return MHD_get_connection_values(_connection, MHD_GET_ARGUMENT_KIND, &MHD_getParamsIter, &params) &&
           params.find(name) != params.end();
}

IHttpConnection &HttpConnectionImpl::operator<<(const std::string &output)
{
    _streamName.clear();
    _strResponse << output;
    return *this;
}

void HttpConnectionImpl::sendFile(const std::string &filepath)
{
    _strResponse.clear();
    _streamName = filepath;
}

void HttpConnectionImpl::attachSession(HttpSession::SPtr session)
{
    session->extendExpiration(system::TimeProvider::instance()->now() + cSessionLifeTimeMin * cSecInMin);
    _session = session;
}

HttpSession::WPtr HttpConnectionImpl::session() const
{
    if (_session)
    {
        _session->touch();
    }
    return _session;
}

} // namespace http
} // namespace net
} // namespace common
} // namespace softeq
