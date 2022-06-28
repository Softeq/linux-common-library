#include <gtest/gtest.h>
#include <chrono>
#include <thread>

#include <softeq/common/cron.hh>

using namespace softeq::common;

inline const time_t minutesInSec(const int mins) { return (mins * 60); }
inline const time_t hoursInSec(const int hours) { return minutesInSec(hours * 60); }
inline const time_t daysInSec(const int days) { return hoursInSec(days * 24); }

TEST(Cron, MinuteFormat)
{
    ICron::UPtr cron(CronFactory::create());
    const ICron::JobId failed_result = ICron::kInvalidJobId;
    ICron::JobId result = failed_result;

    result = cron->addJob("Wrong format", "60 * * * * *", true, [&] { return true; });
    EXPECT_EQ(result, failed_result);
    result = cron->addJob("Wrong format", "*/60 * * * * *", true, [&] { return true; });
    EXPECT_EQ(result, failed_result);
    result = cron->addJob("Wrong format", "0-60 * * * * *", true, [&] { return true; });
    EXPECT_EQ(result, failed_result);
    result = cron->addJob("Wrong format", "1000 * * * * *", true, [&] { return true; });
    EXPECT_EQ(result, failed_result);
    result = cron->addJob("Wrong format", "-1 * * * * *", true, [&] { return true; });
    EXPECT_EQ(result, failed_result);
    result = cron->addJob("Wrong format", "a * * * * *", true, [&] { return true; });
    EXPECT_EQ(result, failed_result);
}

TEST(Cron, HourFormat)
{
    ICron::UPtr cron(CronFactory::create());
    const ICron::JobId failed_result = ICron::kInvalidJobId;
    ICron::JobId result = failed_result;

    result = cron->addJob("Wrong format", "* 24  * * * *", true, [&] { return true; });
    EXPECT_EQ(result, failed_result);
    result = cron->addJob("Wrong format", "* */24 * * * *", true, [&] { return true; });
    EXPECT_EQ(result, failed_result);
    result = cron->addJob("Wrong format", "* 0-24  * * * *", true, [&] { return true; });
    EXPECT_EQ(result, failed_result);
    result = cron->addJob("Wrong format", "* 1000 * * * *", true, [&] { return true; });
    EXPECT_EQ(result, failed_result);
    result = cron->addJob("Wrong format", "* -1 * * * *", true, [&] { return true; });
    EXPECT_EQ(result, failed_result);
    result = cron->addJob("Wrong format", "* a * * * *", true, [&] { return true; });
    EXPECT_EQ(result, failed_result);
}

TEST(Cron, DayFormat)
{
    ICron::UPtr cron(CronFactory::create());
    const ICron::JobId failed_result = ICron::kInvalidJobId;
    ICron::JobId result = failed_result;

    result = cron->addJob("Wrong format", "* * 32 * * *", true, [&] { return true; });
    EXPECT_EQ(result, failed_result);
    result = cron->addJob("Wrong format", "* * */32 * * *", true, [&] { return true; });
    EXPECT_EQ(result, failed_result);
    result = cron->addJob("Wrong format", "* * 0-32 * * *", true, [&] { return true; });
    EXPECT_EQ(result, failed_result);
    result = cron->addJob("Wrong format", "* * 0 * * *", true, [&] { return true; });
    EXPECT_EQ(result, failed_result);
    result = cron->addJob("Wrong format", "* * 1000 * * *", true, [&] { return true; });
    EXPECT_EQ(result, failed_result);
    result = cron->addJob("Wrong format", "* * -1 * * *", true, [&] { return true; });
    EXPECT_EQ(result, failed_result);
    result = cron->addJob("Wrong format", "* * a * * *", true, [&] { return true; });
    EXPECT_EQ(result, failed_result);
}

TEST(Cron, MonthFormat)
{
    ICron::UPtr cron(CronFactory::create());
    const ICron::JobId failed_result = ICron::kInvalidJobId;
    ICron::JobId result = failed_result;

    result = cron->addJob("Wrong format", "* * * 13 * *", true, [&] { return true; });
    EXPECT_EQ(result, failed_result);
    result = cron->addJob("Wrong format", "* * * */13 * *", true, [&] { return true; });
    EXPECT_EQ(result, failed_result);
    result = cron->addJob("Wrong format", "* * * 0-13 * *", true, [&] { return true; });
    EXPECT_EQ(result, failed_result);
    result = cron->addJob("Wrong format", "* * * 0 * *", true, [&] { return true; });
    EXPECT_EQ(result, failed_result);
    result = cron->addJob("Wrong format", "* * * 1000 * *", true, [&] { return true; });
    EXPECT_EQ(result, failed_result);
    result = cron->addJob("Wrong format", "* * * -1 * *", true, [&] { return true; });
    EXPECT_EQ(result, failed_result);
    result = cron->addJob("Wrong format", "* * * a * *", true, [&] { return true; });
    EXPECT_EQ(result, failed_result);
}

TEST(Cron, DayOfWeekFormat)
{
    ICron::UPtr cron(CronFactory::create());
    const ICron::JobId failed_result = ICron::kInvalidJobId;
    ICron::JobId result = failed_result;

    result = cron->addJob("Wrong format", "* * * * 8 *", true, [&] { return true; });
    EXPECT_EQ(result, failed_result);
    result = cron->addJob("Wrong format", "* * * * */8 *", true, [&] { return true; });
    EXPECT_EQ(result, failed_result);
    result = cron->addJob("Wrong format", "* * * * 0-8 *", true, [&] { return true; });
    EXPECT_EQ(result, failed_result);
    result = cron->addJob("Wrong format", "* * * * 1000 *", true, [&] { return true; });
    EXPECT_EQ(result, failed_result);
    result = cron->addJob("Wrong format", "* * * * -1 *", true, [&] { return true; });
    EXPECT_EQ(result, failed_result);
    result = cron->addJob("Wrong format", "* * * * a *", true, [&] { return true; });
    EXPECT_EQ(result, failed_result);
}

TEST(Cron, TimeCalculation)
{
    ICron::UPtr cron(CronFactory::create());

    ICron::JobId minJobId =   cron->addJob("", "1 * * * * *", true, [&] { return true; });
    ICron::JobId hourJobId =  cron->addJob("", "1 2 * * * *", true, [&] { return true; });
    ICron::JobId mdayJobId =  cron->addJob("", "1 2 3 * * *", true, [&] { return true; });
    ICron::JobId wdayJobId =  cron->addJob("", "1 2 * * 5 *", true, [&] { return true; });
    ICron::JobId monthJobId = cron->addJob("", "1 2 3 4 * *", true, [&] { return true; });
    ICron::JobId yearJobId =  cron->addJob("", "1 2 3 4 * 2098", true, [&] { return true; });
    time_t current_time = std::time(nullptr);

    time_t next_execution_interval = cron->jobExecutionTime(minJobId) - current_time;
    ASSERT_GE(next_execution_interval, 0);
    ASSERT_LE(next_execution_interval, hoursInSec(1));
    time_t rawtime = cron->jobExecutionTime(minJobId);
    struct tm *tm = localtime(&rawtime);
    ASSERT_EQ(tm->tm_min, 1);

    next_execution_interval = cron->jobExecutionTime(hourJobId) - current_time;
    ASSERT_GE(next_execution_interval, 0);
    ASSERT_LE(next_execution_interval, daysInSec(1));
    rawtime = cron->jobExecutionTime(hourJobId);
    tm = localtime(&rawtime);
    ASSERT_EQ(tm->tm_min, 1);
    ASSERT_EQ(tm->tm_hour, 2);

    next_execution_interval = cron->jobExecutionTime(mdayJobId) - current_time;
    ASSERT_GE(next_execution_interval, 0);
    ASSERT_LE(next_execution_interval, daysInSec(31));
    rawtime = cron->jobExecutionTime(mdayJobId);
    tm = localtime(&rawtime);
    ASSERT_EQ(tm->tm_min, 1);
    ASSERT_EQ(tm->tm_hour, 2);
    ASSERT_EQ(tm->tm_mday, 3);

    next_execution_interval = cron->jobExecutionTime(wdayJobId) - current_time;
    ASSERT_GE(next_execution_interval, 0);
    ASSERT_LE(next_execution_interval, daysInSec(7));
    rawtime = cron->jobExecutionTime(wdayJobId);
    tm = localtime(&rawtime);
    ASSERT_EQ(tm->tm_min, 1);
    ASSERT_EQ(tm->tm_hour, 2);
    ASSERT_EQ(tm->tm_wday, 5);

    next_execution_interval = cron->jobExecutionTime(monthJobId) - current_time;
    ASSERT_GE(next_execution_interval, 0);
    ASSERT_LE(next_execution_interval, daysInSec(365));
    rawtime = cron->jobExecutionTime(monthJobId);
    tm = localtime(&rawtime);
    ASSERT_EQ(tm->tm_min, 1);
    ASSERT_EQ(tm->tm_hour, 2);
    ASSERT_EQ(tm->tm_mday, 3);
    ASSERT_EQ(tm->tm_mon, (4 - 1)); // minus 1 because month started from 1 in cron format

    rawtime = cron->jobExecutionTime(yearJobId);
    tm = localtime(&rawtime);
    ASSERT_EQ(tm->tm_min, 1);
    ASSERT_EQ(tm->tm_hour, 2);
    ASSERT_EQ(tm->tm_mday, 3);
    ASSERT_EQ(tm->tm_mon, (4 - 1)); // minus 1 because month started from 1 in cron format
    ASSERT_EQ(tm->tm_year, (2098 - 1900));
}

TEST(Cron, EachMinJobs)
{
    static int job1_call_cntr = 0;
    static int job2_call_cntr = 0;
    static int job3_call_cntr = 0;

    job1_call_cntr = 0;
    job2_call_cntr = 0;
    job3_call_cntr = 0;

    ICron::UPtr cron(CronFactory::create());

    ICron::JobId job1_id = cron->addJob("Each minute Job1", "* * * * * *", true, [&] { job1_call_cntr++; return true; });
    ICron::JobId job2_id = cron->addJob("Each minute Job2", "* * * * * *", true, [&] { job2_call_cntr++; return true; });
    cron->start();

    time_t start_time = std::time(nullptr);

    while (job1_call_cntr == 0)
    {
        ASSERT_LT(std::time(nullptr) - start_time, 62);
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
    cron->addJob("Each minute Job3", "* * * * * *", true, [&] { job3_call_cntr++; return true; });
    cron->removeJob(job1_id);

    ASSERT_EQ(job1_call_cntr, 1);
    ASSERT_EQ(job2_call_cntr, 1);
    ASSERT_EQ(job3_call_cntr, 0);

    std::this_thread::sleep_for(std::chrono::seconds(58));
    ASSERT_EQ(job1_call_cntr, 1);
    ASSERT_EQ(job2_call_cntr, 1);
    ASSERT_EQ(job3_call_cntr, 0);

    std::this_thread::sleep_for(std::chrono::seconds(4));
    ASSERT_EQ(job1_call_cntr, 1);
    ASSERT_EQ(job2_call_cntr, 2);
    ASSERT_EQ(job3_call_cntr, 1);

    cron->removeJob(job2_id);
    std::this_thread::sleep_for(std::chrono::seconds(62));
    ASSERT_EQ(job1_call_cntr, 1);
    ASSERT_EQ(job2_call_cntr, 2);
    ASSERT_EQ(job3_call_cntr, 2);
}
