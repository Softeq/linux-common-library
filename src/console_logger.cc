#include "console_logger.hh"

#include <cstdio>
#include <cstdlib>
#include <iomanip>
#include <sstream>
#include <thread>

using namespace softeq::common;

namespace
{
inline std::string makeThreadId(const std::thread::id &id)
{
    std::stringstream stream;
    stream << "0x" << std::uppercase << std::setfill('0') << std::setw(4) << std::hex << id;
    return stream.str();
}
} // namespace
ConsoleLogger::ConsoleLogger()
{
}

void ConsoleLogger::log(LogLevel level, const LogContext &context, const char *msg)
{
    fprintf(stdout, "%lu %s [%c] %s: %s\n", static_cast<unsigned long>(time(0)),
            makeThreadId(context._threadId).c_str(), logLevelToChar(level), context._name.c_str(), msg);
    fflush(stdout);
}
