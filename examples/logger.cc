#include <iostream>
#include <softeq/common/log.hh>
#include <softeq/common/system_logger.hh>

using namespace softeq::common;

void PrintLogs()
{
    const char* domain = "Test message";

    const char* format = "This is a %s log";
    // LogLevel::TRACE 
    LOGT(domain, format, "trace");
    // LogLevel::DEBUG
    LOGD(domain, format, "debug");
    // LogLevel::INFO
    LOGI(domain, format, "info");
    // LogLevel::WARNING
    LOGW(domain, format, "warning");
    // LogLevel::ERROR
    LOGE(domain, format, "error");
    // LogLevel::CRITICAL
    LOGC(domain, format, "critical");
    // LogLevel::FATAL
    LOGF(domain, format, "fatal");

    // System error
    // The errno variable needs to be initialized, let it be "Permission denied"
    errno = EACCES;
    LOGSYS(domain, "Test: Some error has occurred");
}

int main(int argc, char *argv[])
{
    std::cout << "Start printing" << std::endl;

    // Start with ConsoleLogger as default logger
    for (auto level = (uint32_t)LogLevel::NONE; level <= (uint32_t)LogLevel::TRACE; level++)
    {
        log().level((LogLevel)level);
        PrintLogs();
        std::cout << "Switch log level" << std::endl;
    }

    // Change logger to the syslog
    std::cout << "Logger has been changed, at the moment all the messages will be appeared in syslog" << std::endl;
    log().set(LoggerInterface::UPtr(new softeq::common::SystemLogger("test", 0,0)));
    PrintLogs();
}
