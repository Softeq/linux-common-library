#include "http_session.hh"
#include "system/time_provider.hh"
#include "stdutils/stdutils.hh"

#include <uuid/uuid.h>

namespace softeq
{
namespace common
{
namespace net
{
namespace http
{
HttpSession::HttpSession()
{
    uuid_t nId;

    // ASSUMPTION: uuid is generated as an unique value for each session
    uuid_generate(nId);
    std::string tmp(UUID_STR_LEN, '\0');
    uuid_unparse(nId, &tmp[0]);
    _id.assign(tmp, 0, UUID_STR_LEN - 1);
}

std::string HttpSession::id() const
{
    return _id;
}

void HttpSession::touch()
{
    _lastActivity = system::TimeProvider::instance()->now();
}

time_t HttpSession::lastActivity() const
{
    return _lastActivity;
};

void HttpSession::extendExpiration(time_t time)
{
    _expiration = time;
}

time_t HttpSession::expiration() const
{
    return _expiration;
}

} // namespace http
} // namespace net
} // namespace common
} // namespace softeq
