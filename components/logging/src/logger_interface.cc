#include "logger_interface.hh"

#include <map>
#include <stdexcept>

using namespace softeq::common::logging;

namespace
{
const std::map<LogLevel, char> LOGLEVEL_TO_CHAR = {
    // clang-format off
    {LogLevel::NONE, '\0'},
    {LogLevel::FATAL, 'F'},
	{LogLevel::CRITICAL, 'C'},
	{LogLevel::ERROR, 'E'},
    {LogLevel::WARNING, 'W'},
	{LogLevel::INFO, 'I'},
	{LogLevel::DEBUG, 'D'},
	{LogLevel::TRACE, 'T'},
    // clang-format on
};
const std::map<std::string, LogLevel> STR_TO_LOGLEVEL = {
    // clang-format off
	{"none", LogLevel::NONE},
	{"fatal", LogLevel::FATAL},
	{"critical", LogLevel::CRITICAL},
	{"error", LogLevel::ERROR},
	{"warning", LogLevel::WARNING},
	{"info", LogLevel::INFO},
	{"debug", LogLevel::DEBUG},
	{"trace", LogLevel::TRACE},
    // clang-format on
};

const std::map<int, LogLevel> NUM_TO_LOGLEVEL = {
    // clang-format off
	{0, LogLevel::NONE},
	{1, LogLevel::FATAL},
	{2, LogLevel::CRITICAL},
	{3, LogLevel::ERROR},
	{4, LogLevel::WARNING},
	{5, LogLevel::INFO},
	{6, LogLevel::DEBUG},
	{7, LogLevel::TRACE},
    // clang-format on
};
} // namespace

namespace softeq
{
namespace common
{
namespace logging
{

LogLevel logLevelFromString(const std::string &str)
{
    auto it = STR_TO_LOGLEVEL.find(str);
    if (it != STR_TO_LOGLEVEL.end())
        return it->second;
    auto it2 = NUM_TO_LOGLEVEL.find(std::stoi(str));
    if (it2 != NUM_TO_LOGLEVEL.end())
        return it2->second;

    throw std::logic_error("invalid value of loglevel");
}

char logLevelToChar(LogLevel level)
{
    return LOGLEVEL_TO_CHAR.at(level);
}

} // namespace logging
} // namespace common
} // namespace softeq
