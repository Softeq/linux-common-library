#include "timeutils.hh"

#include <array>

namespace
{
const std::array<int, 12> month_days{31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

inline int zero_based_month(int month)
{
    int result = (month - 1) % 12;
    if (result < 0)
        result += 12;

    return result;
}

const char *time_format = "%FT%T%z"; // iso8601
//   const char *time_format = "%a, %d %b %Y %T %z"; //rfc822

} // namespace

namespace softeq
{
namespace common
{
namespace stdutils
{

time_t string_to_timestamp(const std::string &str)
{
    std::tm t = {};
    strptime(str.c_str(), time_format, &t);
    return mktime(&t);
}

std::string timestamp_to_string(time_t time)
{
    char buff[26];
    struct tm tmbuf;
    strftime(buff, sizeof(buff), time_format, localtime_r(&time, &tmbuf));
    return std::string(buff);
}

bool is_leap_year(int year)
{
    return !(year & 3) && (!(year & 15) || (year % 25));
}

int days_per_month(int month, int year)
{
    month = zero_based_month(month);

    int result = month_days[month] + static_cast<int>(1 == month && is_leap_year(year));
    return result;
}

int week_day(int day, int month, int year)
{
    month = zero_based_month(month);

    const std::array<int, 12> shift{0, 3, 2, 5, 0, 3, 5, 1, 4, 6, 2, 4};

    year -= static_cast<int>(month < 2);
    int result = (year + year / 4 - year / 100 + year / 400 + shift[month] + day) % 7;
    return result;
}

time_t make_time(int year, int month, int day, int hour, int minute, int second)
{
    month = zero_based_month(month);

    time_t result = day - 1;
    for (int y = 1970; y < year; ++y)
    {
        result += is_leap_year(y) ? 366 : 365;
    }
    for (int m = 0; m < month; ++m)
    {
        result += month_days[m] + static_cast<int>(1 == m && is_leap_year(year));
    }
    constexpr int minute_seconds = 60;
    constexpr int hour_seconds = 60 * minute_seconds;
    constexpr int day_seconds = 24 * hour_seconds;
    result *= day_seconds;

    result += hour * hour_seconds + minute * minute_seconds + second;
    return result;
}

uint64_t get_monotonic_ns()
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    uint64_t tval = ts.tv_sec;
    tval *= 1000000000;
    tval += ts.tv_nsec;
    return tval;
}

} // namespace stdutils
} // namespace common
} // namespace softeq
