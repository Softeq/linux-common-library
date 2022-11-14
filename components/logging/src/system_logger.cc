#include "system_logger.hh"

#include <syslog.h>

using namespace softeq::common::logging;

SystemLogger::~SystemLogger()
{
    ::closelog();
}

SystemLogger::SystemLogger(const std::string &ident, int options, int facility)
{
    ::setlogmask(LOG_UPTO(LOG_DEBUG));
    const char *prefix;
    if (ident.empty())
        prefix = getenv("LOG_PREFIX");
    else
        prefix = ident.c_str();
    ::openlog((prefix ? prefix : "softeq"), options == 0 ? LOG_CONS | LOG_NDELAY : options,
              facility == 0 ? LOG_USER : facility);
}

void SystemLogger::log(LogLevel level, const LogContext &context, const char *msg)
{
    switch (level)
    {
    case LogLevel::DEBUG:
        syslog(LOG_DEBUG, "[%s] %s", context._name.c_str(), msg);
        break;
    case LogLevel::INFO:
        syslog(LOG_INFO, "[%s] %s", context._name.c_str(), msg);
        break;
    case LogLevel::WARNING:
        syslog(LOG_WARNING, "[%s] %s", context._name.c_str(), msg);
        break;
    case LogLevel::ERROR:
        syslog(LOG_ERR, "[%s] %s", context._name.c_str(), msg);
        break;
    case LogLevel::CRITICAL:
        syslog(LOG_CRIT, "[%s] %s", context._name.c_str(), msg);
        break;
    case LogLevel::FATAL:
        syslog(LOG_ALERT, "[%s] %s", context._name.c_str(), msg);
        break;
    default:
        break;
    }
}
