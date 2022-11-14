#include <ctime>
#include <gtest/gtest.h>

#include <common/system/cron.hh>

#include <chrono>
#include <memory>
#include <thread>
#include <mutex>
#include <atomic>
#include <condition_variable>

#include <system/testtimeprovider.hh>

using namespace softeq::common::system;

inline time_t secondsInMinutes(const int mins)
{
    return (mins * 60);
}

inline time_t secondsInHours(const int hours)
{
    return secondsInMinutes(hours * 60);
}

inline time_t secondsInDays(const int days)
{
    return secondsInHours(days * 24);
}

bool runTwoTasks(const std::string &timespec1, const std::string &timespec2);
bool runTestWithDefTimeProvider(const std::string &timespec, time_t timeout);

TEST(Cron, MinuteFormat)
{
    Cron::UPtr cron(CronFactory::create());
    const Cron::JobId failed_result = Cron::invalidJobId;
    Cron::JobId result = failed_result;

    result = cron->addJob("Wrong format", "60 * * * * *", [&] { return true; });
    EXPECT_EQ(result, failed_result);
    result = cron->addJob("Wrong format", "*/60 * * * * *", [&] { return true; });
    EXPECT_EQ(result, failed_result);
    result = cron->addJob("Wrong format", "0-60 * * * * *", [&] { return true; });
    EXPECT_EQ(result, failed_result);
    result = cron->addJob("Wrong format", "1000 * * * * *", [&] { return true; });
    EXPECT_EQ(result, failed_result);
    result = cron->addJob("Wrong format", "-1 * * * * *", [&] { return true; });
    EXPECT_EQ(result, failed_result);
    result = cron->addJob("Wrong format", "a * * * * *", [&] { return true; });
    EXPECT_EQ(result, failed_result);
    // TODO check internal size
}

TEST(Cron, HourFormat)
{
    Cron::UPtr cron(CronFactory::create());
    const Cron::JobId failed_result = Cron::invalidJobId;
    Cron::JobId result = failed_result;

    result = cron->addJob("Wrong format", "* 24  * * * *", [&] { return true; });
    EXPECT_EQ(result, failed_result);
    result = cron->addJob("Wrong format", "* */24 * * * *", [&] { return true; });
    EXPECT_EQ(result, failed_result);
    result = cron->addJob("Wrong format", "* 0-24  * * * *", [&] { return true; });
    EXPECT_EQ(result, failed_result);
    result = cron->addJob("Wrong format", "* 1000 * * * *", [&] { return true; });
    EXPECT_EQ(result, failed_result);
    result = cron->addJob("Wrong format", "* -1 * * * *", [&] { return true; });
    EXPECT_EQ(result, failed_result);
    result = cron->addJob("Wrong format", "* a * * * *", [&] { return true; });
    EXPECT_EQ(result, failed_result);
}

TEST(Cron, DayFormat)
{
    Cron::UPtr cron(CronFactory::create());
    const Cron::JobId failed_result = Cron::invalidJobId;
    Cron::JobId result = failed_result;

    result = cron->addJob("Wrong format", "* * 32 * * *", [&] { return true; });
    EXPECT_EQ(result, failed_result);
    result = cron->addJob("Wrong format", "* * */32 * * *", [&] { return true; });
    EXPECT_EQ(result, failed_result);
    result = cron->addJob("Wrong format", "* * 0-32 * * *", [&] { return true; });
    EXPECT_EQ(result, failed_result);
    result = cron->addJob("Wrong format", "* * 0 * * *", [&] { return true; });
    EXPECT_EQ(result, failed_result);
    result = cron->addJob("Wrong format", "* * 1000 * * *", [&] { return true; });
    EXPECT_EQ(result, failed_result);
    result = cron->addJob("Wrong format", "* * -1 * * *", [&] { return true; });
    EXPECT_EQ(result, failed_result);
    result = cron->addJob("Wrong format", "* * a * * *", [&] { return true; });
    EXPECT_EQ(result, failed_result);
}

TEST(Cron, MonthFormat)
{
    Cron::UPtr cron(CronFactory::create());
    const Cron::JobId failed_result = Cron::invalidJobId;
    Cron::JobId result = failed_result;

    result = cron->addJob("Wrong format", "* * * 13 * *", [&] { return true; });
    EXPECT_EQ(result, failed_result);
    result = cron->addJob("Wrong format", "* * * */13 * *", [&] { return true; });
    EXPECT_EQ(result, failed_result);
    result = cron->addJob("Wrong format", "* * * 0-13 * *", [&] { return true; });
    EXPECT_EQ(result, failed_result);
    result = cron->addJob("Wrong format", "* * * 0 * *", [&] { return true; });
    EXPECT_EQ(result, failed_result);
    result = cron->addJob("Wrong format", "* * * 1000 * *", [&] { return true; });
    EXPECT_EQ(result, failed_result);
    result = cron->addJob("Wrong format", "* * * -1 * *", [&] { return true; });
    EXPECT_EQ(result, failed_result);
    result = cron->addJob("Wrong format", "* * * a * *", [&] { return true; });
    EXPECT_EQ(result, failed_result);
}

TEST(Cron, DayOfWeekFormat)
{
    Cron::UPtr cron(CronFactory::create());
    const Cron::JobId failed_result = Cron::invalidJobId;
    Cron::JobId result = failed_result;

    result = cron->addJob("Wrong format", "* * * * 8 *", [&] { return true; });
    EXPECT_EQ(result, failed_result);
    result = cron->addJob("Wrong format", "* * * * */8 *", [&] { return true; });
    EXPECT_EQ(result, failed_result);
    result = cron->addJob("Wrong format", "* * * * 0-8 *", [&] { return true; });
    EXPECT_EQ(result, failed_result);
    result = cron->addJob("Wrong format", "* * * * 1000 *", [&] { return true; });
    EXPECT_EQ(result, failed_result);
    result = cron->addJob("Wrong format", "* * * * -1 *", [&] { return true; });
    EXPECT_EQ(result, failed_result);
    result = cron->addJob("Wrong format", "* * * * a *", [&] { return true; });
    EXPECT_EQ(result, failed_result);
}

TEST(Cron, StartStop)
{
    std::time_t t0, t1;
    Cron::UPtr cron(CronFactory::create());
    t0 = std::time(nullptr);
    // without jobs
    cron->start();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    cron->stop();
    t1 = std::time(nullptr);
    EXPECT_GE(1, t1 - t0) << "Expect it stops fast without jobs";
    // with once job
    cron->addJob("", "* * * * * *", [&] { return false; });
    t0 = std::time(nullptr);
    cron->start();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    cron->stop();
    t1 = std::time(nullptr);
    EXPECT_GE(1, t1 - t0) << "Expect it stops fast with single job";
    // with periodic job
    cron->addJob("", "* * * * * *", [&] { return true; });
    t0 = std::time(nullptr);
    cron->start();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    cron->stop();
    t1 = std::time(nullptr);
    EXPECT_GE(1, t1 - t0) << "Expect it stops fast with multiple jobs";
}
// add after start
// add start stop remove start
TEST(Cron, TimeCalculation)
{
    Cron::UPtr cron(CronFactory::create());

    Cron::JobId minJobId = cron->addJob("", "1 * * * * *", [&] { return true; });
    Cron::JobId hourJobId = cron->addJob("", "1 2 * * * *", [&] { return true; });
    Cron::JobId mdayJobId = cron->addJob("", "1 2 3 * * *", [&] { return true; });
    Cron::JobId wdayJobId = cron->addJob("", "1 2 * * 5 *", [&] { return true; });
    Cron::JobId monthJobId = cron->addJob("", "1 2 3 4 * *", [&] { return true; });
    Cron::JobId yearJobId = cron->addJob("", "1 2 3 4 * 2098", [&] { return true; });
    ASSERT_TRUE(minJobId != Cron::invalidJobId && hourJobId != Cron::invalidJobId && mdayJobId != Cron::invalidJobId &&
                wdayJobId != Cron::invalidJobId && monthJobId != Cron::invalidJobId && yearJobId != Cron::invalidJobId);

    time_t current_time = std::time(nullptr);

    time_t next_execution_interval = cron->jobExecutionTime(minJobId) - current_time;
    ASSERT_GE(next_execution_interval, 0);
    ASSERT_LE(next_execution_interval, secondsInHours(1));
    time_t rawtime = cron->jobExecutionTime(minJobId);
    struct tm *tm = localtime(&rawtime);
    ASSERT_EQ(tm->tm_min, 1);

    next_execution_interval = cron->jobExecutionTime(hourJobId) - current_time;
    ASSERT_GE(next_execution_interval, 0);
    ASSERT_LE(next_execution_interval, secondsInDays(1));
    rawtime = cron->jobExecutionTime(hourJobId);
    tm = localtime(&rawtime);
    ASSERT_EQ(tm->tm_min, 1);
    ASSERT_EQ(tm->tm_hour, 2);

    next_execution_interval = cron->jobExecutionTime(mdayJobId) - current_time;
    ASSERT_GE(next_execution_interval, 0);
    ASSERT_LE(next_execution_interval, secondsInDays(31));
    rawtime = cron->jobExecutionTime(mdayJobId);
    tm = localtime(&rawtime);
    ASSERT_EQ(tm->tm_min, 1);
    ASSERT_EQ(tm->tm_hour, 2);
    ASSERT_EQ(tm->tm_mday, 3);

    next_execution_interval = cron->jobExecutionTime(wdayJobId) - current_time;
    ASSERT_GE(next_execution_interval, 0);
    ASSERT_LE(next_execution_interval, secondsInDays(7));
    rawtime = cron->jobExecutionTime(wdayJobId);
    tm = localtime(&rawtime);
    ASSERT_EQ(tm->tm_min, 1);
    ASSERT_EQ(tm->tm_hour, 2);
    ASSERT_EQ(tm->tm_wday, 5);

    next_execution_interval = cron->jobExecutionTime(monthJobId) - current_time;
    ASSERT_GE(next_execution_interval, 0);
    ASSERT_LE(next_execution_interval, secondsInDays(365));
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

TEST(Cron, DefTimeProvider)
{
    TimeProvider::instance(std::make_shared<DefTimeProvider>());
    Cron::UPtr cron(CronFactory::create());
    int counter = 0;
    std::condition_variable cv;
    std::mutex m;

    cron->addJob("Each minute Job", "* * * * * *", [&counter, &cv] {
        counter++;
        cv.notify_one();
        return true;
    });

    cron->start();

    std::unique_lock<std::mutex> lock(m);
    cv.wait_for(lock, std::chrono::seconds(60), [&counter]() { return counter; });

    EXPECT_EQ(counter, 1) << "Expect job has been performed once";
}

TEST(Cron, CheckSkipTask)
{
    std::shared_ptr<TestTimeProvider> timeProvider = std::make_shared<TestTimeProvider>();
    TimeProvider::instance(timeProvider);
    Cron::UPtr cron(CronFactory::create());

    int counter1 = 0;
    int counter2 = 0;

    Cron::JobId id1 = cron->addJob("Each minute Job1", "* * * * * *", [&counter1, &timeProvider] {
        counter1++;
        timeProvider->sleep(30);
        return true;
    });

    Cron::JobId id2 = cron->addJob("Each minute Job2", "* * * * * *", [&counter2, &timeProvider] {
        counter2++;
        //        timeProvider->sleep(30);
        return true;
    });

    std::time_t time1 = cron->jobExecutionTime(id1);
    std::time_t time2 = cron->jobExecutionTime(id2);
    EXPECT_EQ(time1, time2) << "Jobs must be scheduled for the same time";

    timeProvider->setTime(time1 - 1);

    cron->start();

    std::this_thread::sleep_for(std::chrono::seconds(2));
    EXPECT_EQ(counter1 * counter2, 1) << "Expect that both jobs are performed once but " << counter1 << " and "
                                      << counter2;
}

TEST(Cron, Each4Minute)
{
    //  ASSERT_TRUE(runTwoTasks("*/3 * * * * *", "*/4 * * * * *"));
    std::shared_ptr<TestTimeProvider> timeProvider = std::make_shared<TestTimeProvider>();
    TimeProvider::instance(timeProvider);
    Cron::UPtr cron(CronFactory::create());

    std::condition_variable cv;
    std::mutex m;

    int counter1 = 0;

    Cron::JobId id1 = cron->addJob("Each minute Job1", "*/4 * * * * *", [&counter1, &cv] {
        counter1++;
        cv.notify_one();
        return true;
    });
    cron->start();

    int cycleTimes = 4;
    for (int cycleCounter = 1; cycleCounter < cycleTimes; ++cycleCounter)
    {
        std::unique_lock<std::mutex> lock(m);
        timeProvider->setTime(cron->jobExecutionTime(id1) - 1);
        cv.wait_for(lock, std::chrono::seconds(2));
        EXPECT_EQ(counter1, cycleCounter);
    }
}

TEST(Cron, LeapYear)
{
    std::shared_ptr<TestTimeProvider> timeProvider = std::make_shared<TestTimeProvider>();
    TimeProvider::instance(timeProvider);
    Cron::UPtr cron(CronFactory::create());

    int performed = 0;
    std::condition_variable cv;
    std::mutex m;

    timeProvider->setTime(0);
    cron->addJob("Leap year job", "0 0 29 2 * *", [&] {
        performed++;
        cv.notify_one();
        return true;
    });

    cron->start();

    EXPECT_EQ(performed, 0) << "Expect that a job is not performed on start";

    performed = 0;
    timeProvider->setTime(59, 59, 23, 28, 2, 2020);
    EXPECT_EQ(performed, 0) << "Expect that a job is not performed on time change";

    {
        std::unique_lock<std::mutex> lock(m);
        performed = 0;
        cv.wait_for(lock, std::chrono::seconds(2));
        EXPECT_EQ(performed, 1) << "Expect that a job is performed on time at first time";
    }
    {
        std::unique_lock<std::mutex> lock(m);
        performed = 0;
        timeProvider->setTime(59, 59, 23, 28, 2, 2021);
        cv.wait_for(lock, std::chrono::seconds(2));
        EXPECT_EQ(performed, 0) << "Expect that a job is not at not leap year";
    }
    {
        std::unique_lock<std::mutex> lock(m);
        performed = 0;
        timeProvider->setTime(59, 59, 23, 28, 2, 2024);
        cv.wait_for(lock, std::chrono::seconds(2));
        EXPECT_EQ(performed, 1) << "Expect that a job is performed one time till next leap year";
    }
}
