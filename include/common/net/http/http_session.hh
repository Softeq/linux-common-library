#ifndef SOFTEQ_COMMON_HTTP_SESSION_H
#define SOFTEQ_COMMON_HTTP_SESSION_H

/*!
 \file
 \brief Definition of class of HTTP session
 */

#include <ctime>
#include <memory>
#include <string>

namespace softeq
{
namespace common
{
namespace net
{
namespace http
{
/*!
 Session management is used to facilitate secure interactions between a user and some service or application and applies
to a sequence of requests and responses associated with that particular user. When a user has an ongoing session with a
web application, they are submitting requests within their session and often times are providing potentially sensitive
information. The application may retain this information and/or track the status of the user during the session across
multiple requests. More importantly, it is critical that the application has a means of protecting private data
belonging to each unique user, especially within authenticated sessions.

Session tokens serve to identify a user’s session within the HTTP traffic being exchanged between the application and
all of its users. HTTP traffic on its own is stateless, meaning each request is processed independently, even if they
are related to the same session. Thus, session management is crucial for directing these web interactions and these
tokens are vital as they’re passed back and forth between the user and the web application. Each request and response
made will have an associated session token which allows the application to remember distinct information about the
client using it. Session cookies were designed to help manage sessions, however there are several properties of the
cookie that must be configured and implemented correctly to prevent potential compromises.

It should be noted that cookies are not the only means of carrying out a session, it is also possible to include headers
that contain session tokens. Moreover, while session tokens can be embedded within a URL this should not be implemented
as URLs are often logged in various places and cached, increasingly the likelihood of disclosure.
 */
class HttpSession
{
    HttpSession(HttpSession &) = delete;
    HttpSession(HttpSession &&) = delete;
    HttpSession &operator=(HttpSession &) = delete;
    HttpSession &operator=(HttpSession &&) = delete;

public:
    using WPtr = std::weak_ptr<HttpSession>;
    using SPtr = std::shared_ptr<HttpSession>;

    HttpSession();
    virtual ~HttpSession() = default;

    /*!
      Return Id for the current session
      \return id
    */
    std::string id() const;

    /*!
       Refresh last activity time
     */
    void touch();

    /*!
      Return last connection activity time
      \return Connection activity status
    */
    time_t lastActivity() const;

    /*!
      Extend expiration of the session
    */
    void extendExpiration(time_t time);

    /*!
      Return expiration time of the session
      \return Expiration time
    */
    time_t expiration() const;

    virtual void onExpire() = 0;

private:
    time_t _lastActivity{0};
    time_t _expiration{0};
    std::string _id;
};

} // namespace http
} // namespace net
} // namespace common
} // namespace softeq

#endif // SOFTEQ_COMMON_HTTP_SESSION_H
