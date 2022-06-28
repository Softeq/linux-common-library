#ifndef SOFTEQ_COMMON_SYSTEM_LOGGER_H_
#define SOFTEQ_COMMON_SYSTEM_LOGGER_H_

#include "softeq/common/logger_interface.hh"

namespace softeq
{
namespace common
{
/*!
  \brief System logger implementation.
*/
class SystemLogger : public LoggerInterface
{
public:
    SystemLogger(const std::string &ident, int option, int facility);
    ~SystemLogger();

    /*!
        Method for log info
        \param[in] level Log level
        \param[in] context Log context
        \param[in] msg Char pointer to message
      */
    virtual void log(LogLevel level, const LogContext &context, const char *msg);
};

} // namespace common
} // namespace softeq

#endif // SOFTEQ_COMMON_SYSTEM_LOGGER_H_
