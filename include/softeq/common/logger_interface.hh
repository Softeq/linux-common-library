#ifndef SOFTEQ_COMMON_LOGGER_INTERFACE_H
#define SOFTEQ_COMMON_LOGGER_INTERFACE_H

#include <string>
#include <thread>

namespace softeq
{
namespace common
{
/*!
  \brief The class describes log interface.
*/
enum class LogLevel
{ /**< detail and verbosity of logging level */
  NONE = 0,
  FATAL,
  CRITICAL,
  ERROR,
  WARNING,
  INFO,
  DEBUG,
  TRACE
};

/*!
   Method to convert argument into LogLevel value
   \param[in] str String to convert
   \return Value of log level if conversion successfull, otherwise throws std::logic_error
 */
LogLevel logLevelFromString(const std::string &str);
/*!
   Method to convert log level to char
   \param[in] level Log level
   \return Char value of level
 */
char logLevelToChar(LogLevel level);

struct LogContext final
{
    LogContext(const std::string &name, const std::thread::id &threadId)
        : _name(name)
        , _threadId(threadId)
    {
    }

    std::string _name;
    std::thread::id _threadId;
};

class LoggerInterface
{
public:
    using UPtr = std::unique_ptr<LoggerInterface>;
    virtual ~LoggerInterface() = default;
    /*!
       Abstract log method
       \param[in] level Log level
       \param[in] context Log context S
       \param[in] msg Char pointer to message
    */
    virtual void log(LogLevel level, const LogContext &context, const char *msg) = 0;
};

} // namespace common
} // namespace softeq

#endif // SOFTEQ_COMMON_LOGGER_INTERFACE_H
