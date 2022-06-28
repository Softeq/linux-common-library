#include <softeq/common/log.hh>
#include <cstring>
#include <csignal>
#include <thread>
#include <sstream>
#include <iomanip>
#include "console_logger.hh"

using namespace softeq::common;

Log::Log()
    : _logger(new ConsoleLogger())
{
    try
    {
        char *filter = getenv("SC_LOG_FILTER");

        if (filter)
        {
            std::stringstream stream_filter(filter);
            std::string single_filter;
            while (getline(stream_filter, single_filter, ','))
            {
                size_t delim_pos = single_filter.find(':');
                if (delim_pos == single_filter.npos)
                {
                    _level = logLevelFromString(single_filter);
                }
                else
                {
                    _domain_filter[single_filter.substr(0, delim_pos)] =
                        logLevelFromString(single_filter.substr(delim_pos + 1));
                }
            }
        }
    }
    catch (const std::exception &e)
    {
        std::cerr << "Logger filter error: " << e.what() << "\nfix your SC_LOG_FILTER envvar\n";
    }
    _raise_sigtrap_on_error = (getenv("SC_DEBUG_ON_ERROR") != NULL);
    _raise_sigtrap_on_warning = (getenv("SC_DEBUG_ON_WARNING") != NULL);
}

Log::~Log()
{
}

Log &Log::get()
{
    static Log logger;
    return logger;
}

void Log::SystemError(const char *domain, const char *format, ...)
{
    va_list args;
    static char sysmesg[2048];
    snprintf(sysmesg, sizeof(sysmesg), "System error (%s), %s", strerror(errno), format);
    va_start(args, format);
    MessageV(LogLevel::CRITICAL, domain, sysmesg, args);
    va_end(args);
}

void Log::Message(LogLevel level, const char *domain, const char *format, ...)
{
    va_list args;
    va_start(args, format);
    MessageV(level, domain, format, args);
    va_end(args);
}

void Log::MessageV(LogLevel level, const char *domain, const char *format, va_list args)
{
    LogLevel severity_level = _level;
    if (domain != NULL)
    {
        const auto it = _domain_filter.find(domain);
        if (it != _domain_filter.end())
            severity_level = it->second;
    }
    else
    {
        domain = "";
    }

    if (level <= severity_level)
    {
        static char buf[2048];
        vsnprintf(buf, sizeof(buf), format, args);

        LogContext logContext(domain, std::this_thread::get_id());
        _logger->log(level, logContext, buf);
    }

    if ((_raise_sigtrap_on_error && level == LogLevel::ERROR) ||
        (_raise_sigtrap_on_warning && level == LogLevel::WARNING))
    {
        raise(SIGTRAP);
    }
}

void Log::level(LogLevel level)
{
    _level = level;
}
