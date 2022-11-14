#ifndef SOFTEQ_COMMON_HTTP_CONNECTION_H
#define SOFTEQ_COMMON_HTTP_CONNECTION_H

/*!
 \file
 \brief Definition of class of HTTP connection
 */

#include <common/net/http/http_session.hh>

#include <string>

namespace softeq
{
namespace common
{
namespace net
{
namespace http
{
enum class Method
{
    GET,
    POST,
    PUT,
    OPTIONS,
    DELETE,
};

class IHttpConnection
{
public:
    virtual ~IHttpConnection() = default;

    /*!
       Returns value of HTTP-header in the request
       \param[in] name Name of header to return
       \return String value of header
     */
    virtual std::string header(const std::string &name) const = 0;
    /*!
       Method check a header exists in the request
       \param[in] name Name of header to check
       \return true of false
     */
    virtual bool requestHasHeader(const std::string &name) const = 0;
    /*!
       Method check a header exists in the response
       \param[in] name Name of header to check
       \return true of false
     */
    virtual bool responseHasHeader(const std::string &name) const = 0;
    /*!
       Install HTTP-header into response
       \param[in] name Name of header to install
       \param[in] content Value of header to install
    */
    virtual void setResponseHeader(const std::string &name, const std::string &content) = 0;
    /*!
        Method to remove header from response
        \param[in] name Name of header to remove
    */
    virtual void removeResponseHeader(const std::string &name) = 0;
    /*!
       Method to retrive GET-request (i.e. URI + query string)
       \return String, containing request
    */
    virtual std::string get() const = 0;
    /*!
       Method to retrieve body of POST/PUT requests
       \return String, containing body of POST/PUT request
    */
    virtual std::string body() const = 0;
    /*!
       Method to retrieve URI-part of request
       \return String, containing request URI
    */
    virtual std::string path() const = 0;
    /*!
       Method to retrieve value of the variable from query string in GET-request (and in POST request as well).
       \param[in] name Name of the variable to return
       \return String value of required variable
    */
    virtual std::string field(const std::string &name) const = 0;
    /*!
       Method to check existance of field from query string in GET-request (and in POST request as well).
       \param[in] name Name of the variable to return
       \return String value of required variable
    */
    virtual bool hasField(const std::string &name) const = 0;

    /*!
       Method to retrive cookie attribute value from the current request
       \param[in] key Attirbute name
       \return String, containing attribute value
    */
    virtual std::string cookie(const std::string &key) const = 0;
    virtual bool hasCookie(const std::string &key) const = 0;
    virtual bool setCookie(const std::string &key, const std::string &value) = 0;
    //    virtual void removeCookie(const std::string &key) const = 0;
    /*
            virtual std::string footer(const std::string& key) const = 0;
            virtual bool hasFooter(const std::string& key) const = 0;
            virtual void setFooter(const std::string& key, const std::string& value) const = 0;
    */

    /*!
        Set the http error code and error message
        \param[in] error Numerical code of http error
        \param[in] error_message Error message
      */
    virtual void setError(int error, const std::string &error_message) = 0;
    /*!
       Method to set code of error, if occurred during last operation
       \param[in] error Numerical code of error
     */
    virtual void setError(int error) = 0;
    /*!
       Method to retrieve value of installed error code
       \return Value of error code
    */
    virtual int error() const = 0;
    /*!
       Method to send raw string toward client-side, implements semantic of stream-insertion operator
       \param[in] output String, which needed to send
       \return Reference to current instance of HttpConnection
    */
    virtual IHttpConnection &operator<<(const std::string &output) = 0;
    /*!
      Method to send file to client
      \param[in] filepath Path to file, which needed to send
    */
    virtual void sendFile(const std::string &filepath) = 0;
    /*!
      Method to get inforation about connected client
      \return description of the client
    */
    virtual std::string clientDescription() const = 0;
    /*!
     * Method to install instance of class, which implements HttpSession
     * \param[in] session Weak Pointer to instance of IHttpSession-implementaion
     */
    virtual void attachSession(HttpSession::SPtr session) = 0;
    virtual HttpSession::WPtr session() const = 0;
    /*!
     * Method to remove instaled instance of IHttpSession-implementer
     */
    virtual void detachSession() = 0;

    /*!
      Method to get a type of HTTP request
      \return HTTP method type
     */
    virtual Method method() const = 0;
};

} // namespace http
} // namespace net
} // namespace common
} // namespace softeq

#endif // SOFTEQ_COMMON_HTTP_CONNECTION_H
