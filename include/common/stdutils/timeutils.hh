#ifndef SOFTEQ_COMMON_TIMEUTILS_H_
#define SOFTEQ_COMMON_TIMEUTILS_H_

#include <ctime>
#include <string>

/*!
  \brief List of functions for manipulate with time.
*/
namespace softeq
{
namespace common
{
namespace stdutils
{
/*!
  Function converts timestamp as ISO8601 string representation "%FT%T%z" to timestamp
  \return Time as timestamp format (see: time_t)
  \param[in] str - timestamp as ISO8601 string: "%FT%T%z"
*/
time_t string_to_timestamp(const std::string &str);

/*!
   Function converts timestamp to string representation ISO8601: "%FT%T%z"
   \return Time as string with ISO8601 format "%FT%T%z"
   \param[in] time - timestamp
*/
std::string timestamp_to_string(time_t time);

// Whether the year is leap.
bool is_leap_year(int year);

// Days per month. January index is 1, December is 12.
int days_per_month(int month, int year = 0);

// The month must be in [1..12].
int week_day(int day, int month, int year);

/// Make time_t from the given date and time in UTC/GMT.
time_t make_time(int year, int month, int day, int hour, int minute, int second);

uint64_t get_monotonic_ns();

} // namespace stdutils
} // namespace common
} // namespace softeq

#endif // SOFTEQ_COMMON_TIMEUTILS_H_
