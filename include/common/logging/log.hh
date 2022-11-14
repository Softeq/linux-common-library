#ifndef SOFTEQ_COMMON_LOG_H_
#define SOFTEQ_COMMON_LOG_H_

#include <common/logging/logger_interface.hh>

#include <cstdarg>
#include <string>
#include <map>

namespace softeq
{
namespace common
{
namespace logging
{
/*!
  \brief Logger.
*/
class Log final
{
public:
    Log(Log &&) = delete;
    ~Log();
    static Log &get();
    /*!
        Writes the C string pointed by format to the output with domain prefic
        \param[in] level Log level
        \param[in] domain String prefic
        \param[in] format C string that contains a format string that follows the same specifications
         as format in printf (see  http://www.cplusplus.com/reference/cstdio/printf/ for details)
      */
    void Message(LogLevel level, const char *domain, const char *format, ...);
    /*!
        Print system error
        \param[in] domain String prefic
        \param[in] format C string that contains a format string that follows the same specifications
         as format in printf (see  http://www.cplusplus.com/reference/cstdio/printf/ for details)
      */
    void SystemError(const char *domain, const char *format, ...);
    /*!
        Writes the C string pointed by format to the output  with domain prefic, replacing any format specifier,
        but using the elements in the variable argument list identified by arg instead of additional function arguments.
        \param[in] level Log level
        \param[in] domain String prefic
        \param[in] format C string that contains a format string that follows the same specifications
         as format in printf (see  http://www.cplusplus.com/reference/cstdio/printf/ for details)
        \param[in] args A value identifying a variable arguments list initialized with va_start
         va_list is a special type defined in \<cstdarg\>.
      */
    void MessageV(LogLevel level, const char *domain, const char *format, va_list args);
    /*!
       Set log level
        \param[in] level Log level
    */
    void level(LogLevel level);
    LogLevel level()
    {
        return _level;
    };
    /*!
       Set iface
        \param[in] iface Pointer to LoggerInterface class
    */
    void set(LoggerInterface::UPtr &&iface) noexcept
    {
        _logger = std::move(iface);
    }

private:
    Log();
    std::string identity_;

    LoggerInterface::UPtr _logger;
    LogLevel _level = LogLevel::DEBUG;
    bool _raise_sigtrap_on_error = false;
    bool _raise_sigtrap_on_warning = false;
    std::map<std::string, LogLevel> _domain_filter; // filtering of different domain messages by severity level
};

inline Log &log()
{
    return Log::get();
}

} // namespace logging
} // namespace common
} // namespace softeq

#define LOGT(domain, ...) softeq::common::logging::log().Message(softeq::common::logging::LogLevel::TRACE, domain, __VA_ARGS__)
#define LOGD(domain, ...) softeq::common::logging::log().Message(softeq::common::logging::LogLevel::DEBUG, domain, __VA_ARGS__)
#define LOGI(domain, ...) softeq::common::logging::log().Message(softeq::common::logging::LogLevel::INFO, domain, __VA_ARGS__)
#define LOGW(domain, ...) softeq::common::logging::log().Message(softeq::common::logging::LogLevel::WARNING, domain, __VA_ARGS__)
#define LOGE(domain, ...) softeq::common::logging::log().Message(softeq::common::logging::LogLevel::ERROR, domain, __VA_ARGS__)
#define LOGC(domain, ...) softeq::common::logging::log().Message(softeq::common::logging::LogLevel::CRITICAL, domain, __VA_ARGS__)
#define LOGF(domain, ...) softeq::common::logging::log().Message(softeq::common::logging::LogLevel::FATAL, domain, __VA_ARGS__)
#define LOGSYS(domain, ...) softeq::common::logging::log().SystemError(domain, __VA_ARGS__)

#endif // SOFTEQ_COMMON_LOG_H_
