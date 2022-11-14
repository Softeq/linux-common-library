#pragma once

#include "logger_interface.hh"

namespace softeq
{
namespace common
{
namespace logging
{

class ConsoleLogger : public LoggerInterface
{
public:
    ConsoleLogger();
    virtual void log(LogLevel level, const LogContext &context, const char *msg);
};

} // namespace logging
} // namespace common
} // namespace softeq
