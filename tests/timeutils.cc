#include <chrono>
#include <gtest/gtest.h>
#include <stdint.h>
#include <thread>

#include "softeq/common/timeutils.hh"

using namespace softeq::common;

TEST(TimeUtils, GetMonotonicNs)
{
	constexpr uint64_t periodNs  = 100000000ULL;
	constexpr uint64_t timeError = 10000000ULL;

	uint64_t startTime = time::get_monotonic_ns();
	std::this_thread::sleep_for(std::chrono::nanoseconds(periodNs));
	uint64_t timeDelta = time::get_monotonic_ns() - startTime;

	ASSERT_LE(timeDelta, periodNs + timeError);
	ASSERT_GE(timeDelta, periodNs - timeError);
}


TEST(TimeUtils, Timestamp2String)
{
	time_t time = 0;
	std::string timestamp = time::timestamp_to_string(time);
	ASSERT_TRUE(timestamp == "1970-01-01T03:00:00+0300");

	time = 60;
	timestamp = time::timestamp_to_string(time);
	ASSERT_TRUE(timestamp == "1970-01-01T03:01:00+0300");

	time = 915148800ULL;
	timestamp = time::timestamp_to_string(time);
	ASSERT_TRUE(timestamp == "1999-01-01T02:00:00+0200");

	time = 946684800ULL;
	timestamp = time::timestamp_to_string(time);
	ASSERT_TRUE(timestamp == "2000-01-01T02:00:00+0200");

	time = 1612314306ULL;
	timestamp = time::timestamp_to_string(time);
	ASSERT_TRUE(timestamp == "2021-02-03T04:05:06+0300");
}

TEST(TimeUtils, String2Timestamp)
{
	time_t time = time::string_to_timestamp("1970-01-01T03:00:00+0300");
	ASSERT_EQ(time, 0);

	time = time::string_to_timestamp("1970-01-01T03:01:00+0300");
	ASSERT_EQ(time, 60);

	time = time::string_to_timestamp("1999-01-01T02:00:00+0200");
	ASSERT_EQ(time, 915148800ULL);

	time = time::string_to_timestamp("2000-01-01T02:00:00+0200");
	ASSERT_EQ(time, 946684800ULL);

	time = time::string_to_timestamp("2021-02-03T04:05:06+0300");
	ASSERT_EQ(time, 1612314306ULL);
}

TEST(TimeUtils, DaysPerMonth)
{
	ASSERT_EQ(31, time::days_per_month(0, 2020));

	ASSERT_EQ(31, time::days_per_month(1, 2020));
	ASSERT_EQ(29, time::days_per_month(2, 2020));

	ASSERT_EQ(30, time::days_per_month(4, 2021));
	ASSERT_EQ(28, time::days_per_month(2, 2021));

	ASSERT_EQ(31, time::days_per_month(3, 2021));
	ASSERT_EQ(31, time::days_per_month(5, 2021));

	ASSERT_EQ(30, time::days_per_month(6, 2021));

	ASSERT_EQ(31, time::days_per_month(7, 2021));

	ASSERT_EQ(31, time::days_per_month(8, 2021));

	ASSERT_EQ(30, time::days_per_month(9, 2021));

	ASSERT_EQ(31, time::days_per_month(10, 2021));

	ASSERT_EQ(30, time::days_per_month(11, 2021));

	ASSERT_EQ(31, time::days_per_month(12, 2021));
}

TEST(TimeUtils, MakeTime)
{
	ASSERT_EQ(0, time::make_time(1970, 1, 1, 0, 0, 0));

	ASSERT_EQ(949550706ULL, time::make_time(2000, 2, 3, 4, 5, 6));

	ASSERT_EQ(1582979696ULL, time::make_time(2020, 2, 29, 12, 34, 56));

	ASSERT_EQ(1584324246ULL, time::make_time(2020, 3, 16, 2, 4, 6));

	ASSERT_EQ(1614556799ULL, time::make_time(2021, 2, 28, 23, 59, 59));

	ASSERT_EQ(1617602828ULL, time::make_time(2021, 4, 5, 6, 7, 8));
}

TEST(TimeUtils, WeekDay)
{
	ASSERT_EQ(4, time::week_day(0, 1, 2021));

	ASSERT_EQ(0, time::week_day(0, 2, 2021));

	ASSERT_EQ(3, time::week_day(31, 3, 2021));

	ASSERT_EQ(5, time::week_day(31, 12, 2021));
}

TEST(TimeUtils, IsLeapYear)
{
	ASSERT_EQ(false, time::is_leap_year(3));
	ASSERT_EQ(false, time::is_leap_year(2003));

	ASSERT_EQ(true, time::is_leap_year(4));
	ASSERT_EQ(true, time::is_leap_year(2004));

	ASSERT_EQ(false, time::is_leap_year(15));
	ASSERT_EQ(false, time::is_leap_year(2015));

	ASSERT_EQ(false, time::is_leap_year(25));
	ASSERT_EQ(false, time::is_leap_year(2025));

	ASSERT_EQ(false, time::is_leap_year(26));
	ASSERT_EQ(false, time::is_leap_year(2026));
}



