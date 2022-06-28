#include "softeq/common/exceptions.hh"

#include <cstring>
#include <string>

#include <errno.h>
#include <string.h>

namespace
{
constexpr int cBuffSize = 256;
} // namespace

using namespace softeq::common;

std::string SystemError::getErrorMessage() const
{
    std::string errMsg(cBuffSize, '\0');

#if ((_POSIX_C_SOURCE >= 200112L || _XOPEN_SOURCE >= 600) && !_GNU_SOURCE)
    // XSI-compliant version of strerror_r
    int res = strerror_r(errno, &errMsg[0], errMsg.size());
    if (res != 0)
    {
        errMsg = "Unknown error: " + std::to_string(errno);
    }
#else
    // GNU-specific version of strerror_r
    const char *errBuff = strerror_r(errno, &errMsg[0], errMsg.size());
    if (errBuff != &errMsg[0])
    {
        errMsg = std::string(errBuff, std::strlen(errBuff));
    }
    else
    {
        errMsg.resize(errMsg.find('\0'));
    }
#endif

    return errMsg;
}