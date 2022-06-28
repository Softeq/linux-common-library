#pragma once

#include "softeq/common/logger_interface.hh"

namespace softeq
{
namespace common
{

class ConsoleLogger : public LoggerInterface
{
public:
    ConsoleLogger();
    virtual void log(LogLevel level, const LogContext &context, const char *msg);
};

} // namespace common
} // namespace softeq
