#ifndef SOFTEQ_COMMON_HTTP_SERVER_H
#define SOFTEQ_COMMON_HTTP_SERVER_H

/*!
 \file
 \brief Definition of class of HTTP server
 */

#include <memory>
#include <string>
#include <common/net/http/http_connection.hh>

namespace softeq
{
namespace common
{
namespace net
{
namespace http
{
enum HttpStatusCode
{
    STATUS_OK = 200,
    STATUS_NO_CONTENT = 204,
    STATUS_BAD_REQUEST = 400,
    STATUS_UNAUTHORIZED = 401,
    STATUS_FORBIDDEN = 403,
    STATUS_NOT_FOUND = 404,
    STATUS_NOT_ALLOWED = 405,
    STATUS_NOT_ACCEPTABLE = 406,
    STATUS_TOO_MANY_REQUESTS = 429,
    STATUS_INTERNAL_ERROR = 500,
    STATUS_SERVICE_UNAVAILABLE = 503,
    STATUS_NOT_IMPLEMENTED = 501,
    STATUS_GATEWAY_TIMEOUT = 504,
};

struct IHttpConnectionDispatcher
{
    virtual ~IHttpConnectionDispatcher() = default;

    /*!
       Abstract method which must be implemented on end-developer side
       \param[in,out] connection provides input request and accepts response at the same manner
       \return        true if request was handled by dispatcher
    */
    virtual bool handle(IHttpConnection &connection) = 0;
};

class IHttpServer
{
public:
    virtual ~IHttpServer() = default;

    /*!
        Starts http server
        \return  true if server is started successfully
    */
    virtual bool start() = 0;

    /*!
        Stops http server
    */
    virtual void stop() = 0;

    struct settings_t
    {
        /*!
          Address to bind the server
        */
        std::string address;
        /*!
          Port for http/https connection
        */
        uint16_t port{80};

        /*!
          Should the server use HTTPS
        */
        bool enableSecure{false};

        /*!
          Path to the key file
        */
        std::string keyFilePath;

        /*!
          Path to the certificate file
        */
        std::string certFilePath;
    };
};

class HttpServerImpl;

class HttpServer : IHttpServer
{
public:
    using UPtr = std::unique_ptr<HttpServer>;

    explicit HttpServer(const settings_t &settings, IHttpConnectionDispatcher &dispatcher);
    ~HttpServer() override;

    /*!
        Starts http server
        \return  true if server is started successfully
    */
    bool start() override;

    /*!
        Stops http server
    */
    void stop() override;

private:
    std::unique_ptr<HttpServerImpl> _impl;
};

} // namespace http
} // namespace net
} // namespace common
} // namespace softeq

#endif // SOFTEQ_COMMON_HTTP_SERVER_H
