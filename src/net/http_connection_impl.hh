#pragma once

#include <atomic>
#include <map>
#include <string>
#include <sstream>

#include "softeq/common/net/http_connection.hh"

struct MHD_Connection;

namespace softeq
{
namespace common
{
namespace net
{
class HttpServerImpl;

class HttpConnectionImpl : public IHttpConnection
{
public:
    using HttpHeader = std::pair<std::string, std::string>;
    using HttpHeaders = std::map<std::string, std::string>;

    HttpConnectionImpl(struct MHD_Connection *connection, const std::string &url, Method method,
                       const std::string &body, HttpServerImpl &owner);
    ~HttpConnectionImpl() override;

    std::string clientDescription() const override final;

    std::string header(const std::string &name) const override;

    bool requestHasHeader(const std::string &name) const override;

    bool responseHasHeader(const std::string &name) const override;

    void setResponseHeader(const std::string &name, const std::string &content) override;

    void removeResponseHeader(const std::string &name) override;

    std::string cookie(const std::string &key) const override;

    bool hasCookie(const std::string &key) const override;

    bool setCookie(const std::string &key, const std::string &value) override;

    // TODO: have to be refactored to MHD_response* getResponce() where all cases will be processed
    HttpHeaders responseHeaders() const;

    void setError(int error, const std::string &error_message) override;

    void setError(int error) override;

    int error() const override;

    std::string get() const override;

    std::string body() const override;

    std::string path() const override;

    std::string field(const std::string &name) const override;

    bool hasField(const std::string &name) const override;

    IHttpConnection &operator<<(const std::string &output) override;

    void sendFile(const std::string &filepath) override;

    std::string streamName() const;

    std::string strResponse() const;

    void appendBodyData(const char *data, std::size_t data_size);

    void attachSession(HttpSession::SPtr session) final override;

    HttpSession::WPtr session() const override;

    void detachSession() final override;

    Method method() const override;

private:
    const int cSessionLifeTimeMin = 5;
    const int cSecInMin = 60;

    struct MHD_Connection *_connection;
    std::string _url;
    std::string _body;
    HttpServerImpl &_owner;
    int _error = 0;
    std::string _streamName;
    std::stringstream _strResponse;
    HttpHeaders _responseHeaders;
    Method _method;

    HttpSession::SPtr _session;
};

inline void HttpConnectionImpl::setError(int error)
{
    _error = error;
}

inline int HttpConnectionImpl::error() const
{
    return _error;
}

inline std::string HttpConnectionImpl::get() const
{
    return _url;
}

inline std::string HttpConnectionImpl::body() const
{
    return _body;
}

inline void HttpConnectionImpl::setResponseHeader(const std::string &name, const std::string &content)
{
    _responseHeaders[name] = content;
}

// TODO: have to be refactored to MHD_response* getResponce() where all cases will be processed
inline HttpConnectionImpl::HttpHeaders HttpConnectionImpl::responseHeaders() const
{
    return _responseHeaders;
}

inline std::string HttpConnectionImpl::streamName() const
{
    return _streamName;
}

inline std::string HttpConnectionImpl::strResponse() const
{
    return _strResponse.str();
}

inline void HttpConnectionImpl::appendBodyData(const char *data, std::size_t data_size)
{
    _body.append(data, data_size);
}

inline void HttpConnectionImpl::detachSession()
{
    _session.reset();
}

inline Method HttpConnectionImpl::method() const
{
    return _method;
}

} // namespace net
} // namespace common
} // namespace softeq
