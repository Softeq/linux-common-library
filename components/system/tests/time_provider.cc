#include <chrono>
#include <ctime>
#include <exception>
#include <functional>
#include <gtest/gtest.h>

#include <memory>
#include <mutex>
#include <ostream>
#include <stdexcept>
#include <stdlib.h>
#include <sys/time.h>
#include <future>
#include <thread>

#include <system/testtimeprovider.hh>

using namespace softeq::common::system;
namespace
{
struct TestResult
{
    std::time_t duration;
    TimeProvider::State state;
};
} // namespace

TestResult waitCase(std::reference_wrapper<TestTimeProvider> timeProvider,
                    std::reference_wrapper<std::promise<void>> readyPromise)
{
    TestResult res;
    std::mutex m;
    std::unique_lock<std::mutex> lock(m);

    res.duration = std::time(nullptr);
    readyPromise.get().set_value();
    res.state = timeProvider.get().wait(lock);
    res.duration = std::time(nullptr) - res.duration;
    return res;
}

TEST(TimeProvider, Singleton)
{
    TimeProvider::SPtr tp = TimeProvider::instance();
    EXPECT_TRUE(tp);
    EXPECT_NO_THROW(std::dynamic_pointer_cast<DefTimeProvider>(tp));
    TimeProvider::instance(std::make_shared<TestTimeProvider>());
    EXPECT_NO_THROW(std::dynamic_pointer_cast<TestTimeProvider>(TimeProvider::instance()));
}

TEST(TimeProvider, Wait)
{
    TestTimeProvider timeProvider;
    std::promise<void> readyPromise;
    std::shared_future<void> readyFuture(readyPromise.get_future());
    TestResult res = {};

    std::future<TestResult> f1 =
        std::async(std::launch::async, waitCase, std::ref(timeProvider), std::ref(readyPromise));
    readyFuture.wait();                                         // wait until thread is ready
    std::this_thread::sleep_for(std::chrono::milliseconds(10)); // to be sure cond_var.wait realy start

    timeProvider.interruptWait();
    res = f1.get();
    EXPECT_EQ(res.state, TimeProvider::State::Interrupt);
    ASSERT_EQ(res.duration, 0);
}

TEST(TimeProvider, MultiTimeProviders)
{
    TestTimeProvider timeProvider1;
    TestTimeProvider timeProvider2;
    std::promise<void> readyPromise1;
    std::promise<void> readyPromise2;
    std::future<void> readyFuture1(readyPromise1.get_future());
    std::future<void> readyFuture2(readyPromise2.get_future());
    TestResult res = {};
    timeProvider1.setTime(0);
    std::future<TestResult> f1 =
        std::async(std::launch::async, waitCase, std::ref(timeProvider1), std::ref(readyPromise1));
    std::future<TestResult> f2 =
        std::async(std::launch::async, waitCase, std::ref(timeProvider2), std::ref(readyPromise2));
    readyFuture1.wait();
    readyFuture2.wait();
    // wait until thread is ready
    // to be sure cond_var.wait realy start
    std::this_thread::sleep_for(std::chrono::milliseconds(10));

    timeProvider1.interruptWait();
    res = f1.get();
    EXPECT_EQ(res.state, TimeProvider::State::Interrupt);
    EXPECT_FALSE(f1.valid());
    ASSERT_EQ(res.duration, 0);
    EXPECT_TRUE(f2.valid());
    // Interrupt it because f2 will be destructed first and will wait
    timeProvider2.interruptWait();
    res = f2.get();
    EXPECT_EQ(res.state, TimeProvider::State::Interrupt);
    ASSERT_EQ(res.duration, 0);
}

TEST(TimeProvider, Destroy)
{
    std::future<TestResult> f1;
    {
        TestTimeProvider timeProvider;
        std::promise<void> readyPromise;
        std::shared_future<void> readyFuture(readyPromise.get_future());
        f1 = std::async(std::launch::async, waitCase, std::ref(timeProvider), std::ref(readyPromise));
        readyFuture.wait();                                         // wait until thread is ready
        std::this_thread::sleep_for(std::chrono::milliseconds(10)); // to be sure cond_var.wait realy start
    }
    TestResult res = f1.get();
    EXPECT_EQ(res.state, TimeProvider::State::Interrupt);
    ASSERT_EQ(res.duration, 0);
}

TestResult waitForCase(std::reference_wrapper<TestTimeProvider> timeProvider, std::time_t timeToWait,
                       std::reference_wrapper<std::promise<void>> readyPromise)
{
    TestResult res;
    std::mutex m;
    std::unique_lock<std::mutex> lock(m);
    res.duration = std::time(nullptr);
    readyPromise.get().set_value();
    res.state = timeProvider.get().waitFor(lock, timeToWait);
    res.duration = std::time(nullptr) - res.duration;
    return res;
}

TEST(TimeProvider, WaitForTimeout)
{
    TestTimeProvider timeProvider;
    std::promise<void> readyPromise;
    TestResult res = {};
    auto timeoutDurationSec = 1;
    res = waitForCase(timeProvider, timeoutDurationSec, readyPromise);
    EXPECT_EQ(res.duration, timeoutDurationSec);
    EXPECT_EQ(res.state, TimeProvider::State::Wait);
}

TEST(TimeProvider, WaitForInterrupt)
{
    TestTimeProvider timeProvider;
    std::promise<void> readyPromise;
    std::shared_future<void> readyFuture(readyPromise.get_future());
    TestResult res = {};

    std::future<TestResult> f1 =
        std::async(std::launch::async, waitForCase, std::ref(timeProvider), 10, std::ref(readyPromise));
    readyFuture.wait();                                         // wait until thread is ready
    std::this_thread::sleep_for(std::chrono::milliseconds(10)); // to be sure cond_var.wait realy start

    timeProvider.interruptWait();
    res = f1.get();
    EXPECT_EQ(res.state, TimeProvider::State::Interrupt);
    ASSERT_EQ(res.duration, 0);
}

TEST(TimeProvider, WaitForTimechangeTimeout)
{
    TestTimeProvider timeProvider;
    std::promise<void> readyPromise;
    std::shared_future<void> readyFuture(readyPromise.get_future());
    TestResult res = {};

    std::time_t current = timeProvider.now();

    std::future<TestResult> f1 =
        std::async(std::launch::async, waitForCase, std::ref(timeProvider), 10, std::ref(readyPromise));
    readyFuture.wait();                                         // wait until thread is ready
    std::this_thread::sleep_for(std::chrono::milliseconds(10)); // to be sure cond_var.wait realy start
    timeProvider.setTime(current + 3);
    EXPECT_TRUE(f1.valid()) << "wait still waiting";
    timeProvider.setTime(current + 6);
    EXPECT_TRUE(f1.valid()) << "wait still waiting";
    timeProvider.setTime(current + 9);
    EXPECT_TRUE(f1.valid()) << "wait still waiting";
    res = f1.get();

    EXPECT_EQ(res.state, TimeProvider::State::Wait);
    EXPECT_LE(res.duration, 1);
}

TEST(TimeProvider, WaitForTimechange)
{
    TestTimeProvider timeProvider;
    std::promise<void> readyPromise;
    std::shared_future<void> readyFuture(readyPromise.get_future());
    TestResult res = {};
    std::future<TestResult> f1 =
        std::async(std::launch::async, waitForCase, std::ref(timeProvider), 10, std::ref(readyPromise));
    readyFuture.wait();                                         // wait until thread is ready
    std::this_thread::sleep_for(std::chrono::milliseconds(10)); // to be sure cond_var.wait realy start
    timeProvider.setTimeOffset(11);
    res = f1.get();
    EXPECT_EQ(res.state, TimeProvider::State::TimeChange);
    EXPECT_LE(res.duration, 1);
}

TEST(TimeProvider, Sleep)
{
    TestTimeProvider timeProvider;
    std::promise<void> readyPromise;
    std::shared_future<void> readyFuture(readyPromise.get_future());
    TestResult res = {};
    auto timeToSleep = 10;
    std::atomic_bool cbHappened{false};
    auto fn = [&cbHappened](std::time_t oldTime) {
        std::cout << "Callback happened, Oldtime " << oldTime << std::endl;
        cbHappened = true;
    };
    EXPECT_NO_THROW(timeProvider.subscribe(fn));
    std::future<TestResult> f1 =
        std::async(std::launch::async, waitForCase, std::ref(timeProvider), timeToSleep, std::ref(readyPromise));
    readyFuture.wait();                                         // wait until thread is ready
    std::this_thread::sleep_for(std::chrono::milliseconds(10)); // to be sure cond_var.wait realy start
    auto beginTime = timeProvider.now();
    timeProvider.sleep(timeToSleep);
    auto endTime = timeProvider.now();
    res = f1.get();
    EXPECT_NO_THROW(timeProvider.unsubscribe(fn));
    EXPECT_FALSE(cbHappened) << "Expect time change was not happened";
    EXPECT_EQ(res.state, TimeProvider::State::Wait) << "Expect sleep does not affect state of wait";
    EXPECT_LE(res.duration, 1) << "Ecpect test provider's sleep does not wait";
    EXPECT_EQ(endTime - beginTime, timeToSleep) << "Expect test provider's time changed";
}

TEST(TimeProvider, WaitForSleep)
{
    TestTimeProvider timeProvider;
    std::promise<void> readyPromise;
    std::shared_future<void> readyFuture(readyPromise.get_future());
    TestResult res = {};
    std::future<TestResult> f1 =
        std::async(std::launch::async, waitForCase, std::ref(timeProvider), 10, std::ref(readyPromise));
    readyFuture.wait();                                         // wait until thread is ready
    std::this_thread::sleep_for(std::chrono::milliseconds(10)); // to be sure cond_var.wait realy start
    timeProvider.sleep(11);
    res = f1.get();
    EXPECT_EQ(res.state, TimeProvider::State::Wait);
    EXPECT_LE(res.duration, 1);
}

TestResult waitUntilCase(std::reference_wrapper<TestTimeProvider> timeProvider, std::time_t timeToWaitUntil,
                         std::reference_wrapper<std::promise<void>> readyPromise)
{
    TestResult res;
    std::mutex m;
    std::unique_lock<std::mutex> lock(m);
    res.duration = std::time(nullptr);
    readyPromise.get().set_value();
    res.state = timeProvider.get().waitUntil(lock, timeToWaitUntil);
    res.duration = std::time(nullptr) - res.duration;
    return res;
}

TEST(TimeProvider, WaitUntilPast)
{
    TestTimeProvider timeProvider;
    std::promise<void> readyPromise;
    TestResult res = {};

    res = waitUntilCase(timeProvider, timeProvider.now() - 10, readyPromise);
    EXPECT_EQ(res.state, TimeProvider::State::Wait);
    ASSERT_EQ(res.duration, 0);
}

TEST(TimeProvider, WaitUntilInterrupt)
{
    TestTimeProvider timeProvider;
    std::promise<void> readyPromise;
    std::shared_future<void> readyFuture(readyPromise.get_future());
    TestResult res = {};

    std::future<TestResult> f1 = std::async(std::launch::async, waitUntilCase, std::ref(timeProvider),
                                            timeProvider.now() + 10, std::ref(readyPromise));
    readyFuture.wait();                                         // wait until thread is ready
    std::this_thread::sleep_for(std::chrono::milliseconds(10)); // to be sure cond_var.wait realy start

    timeProvider.interruptWait();
    res = f1.get();
    EXPECT_EQ(res.state, TimeProvider::State::Interrupt);
    ASSERT_EQ(res.duration, 0);
}

TEST(TimeProvider, MultiWaitInterrupt)
{
    TestTimeProvider timeProvider;
    std::promise<void> readyPromise1;
    std::promise<void> readyPromise2;
    std::future<void> readyFuture1(readyPromise1.get_future());
    std::future<void> readyFuture2(readyPromise2.get_future());
    std::future<TestResult> f1 =
        std::async(std::launch::async, waitCase, std::ref(timeProvider), std::ref(readyPromise1));
    std::future<TestResult> f2 =
        std::async(std::launch::async, waitCase, std::ref(timeProvider), std::ref(readyPromise2));
    readyFuture1.wait();
    readyFuture2.wait();
    // wait until thread is ready
    // to be sure cond_var.wait realy start
    std::this_thread::sleep_for(std::chrono::milliseconds(10));

    timeProvider.interruptWait();

    TestResult res2 = f2.get();
    EXPECT_EQ(res2.state, TimeProvider::State::Interrupt);
    ASSERT_LE(res2.duration, 1);
    TestResult res1 = f1.get();
    EXPECT_EQ(res1.state, TimeProvider::State::Interrupt);
    ASSERT_LE(res1.duration, 1);
}

TEST(TimeProvider, MultiWaitForTimeout)
{
    TestTimeProvider timeProvider;
    std::promise<void> readyPromise1;
    std::promise<void> readyPromise2;
    std::future<void> readyFuture1(readyPromise1.get_future());
    std::future<void> readyFuture2(readyPromise2.get_future());
    std::future<TestResult> f1 =
        std::async(std::launch::async, waitForCase, std::ref(timeProvider), 10, std::ref(readyPromise1));
    readyFuture1.wait();

    timeProvider.sleep(5);
    std::future<TestResult> f2 =
        std::async(std::launch::async, waitForCase, std::ref(timeProvider), 10, std::ref(readyPromise2));
    readyFuture2.wait();
    // wait until thread is ready
    // to be sure cond_var.wait realy start
    std::this_thread::sleep_for(std::chrono::milliseconds(10));

    timeProvider.setTimeOffset(14);

    TestResult res1 = f1.get();
    EXPECT_EQ(res1.state, TimeProvider::State::TimeChange);
    ASSERT_LE(res1.duration, 1);

    TestResult res2 = f2.get();
    EXPECT_EQ(res2.state, TimeProvider::State::Wait);
    ASSERT_LE(res2.duration, 2);
}

// TEST(TimeProvider, MultiWaitTimeChange)
//{
//}

TEST(TimeProvider, TimeChangeCallbacs)
{
    TestTimeProvider timeProvider;
    std::promise<std::time_t> readyPromise;
    std::future<std::time_t> readyFuture;

    auto fn = [&readyPromise, &readyFuture](std::time_t oldTime) {
        std::cout << "Callback happened" << std::endl;
        readyFuture = readyPromise.get_future();
        readyPromise.set_value(oldTime);
    };
    EXPECT_NO_THROW(timeProvider.subscribe(fn));
    std::time_t current = timeProvider.now();
    timeProvider.setTimeOffset(1);
    EXPECT_EQ(current, readyFuture.get());
    EXPECT_GE(current + 2, timeProvider.now());
    // reset promise
    readyPromise = std::promise<std::time_t>();
    EXPECT_NO_THROW(timeProvider.unsubscribe(fn));
    timeProvider.setTimeOffset(1);
    EXPECT_FALSE(readyFuture.valid());
}

// self test of TestTimeProvider
TEST(TimeProvider, TestTimeProvider)
{
    TestTimeProvider timeProvider;
    std::time_t current = timeProvider.now();
    timeProvider.setTimeOffset(10);
    EXPECT_EQ(timeProvider.now(), current + 10);
    timeProvider.setTime(0);
    EXPECT_EQ(timeProvider.now(), 0);
    timeProvider.setTime(50, 53, 10, 12, 10, 2022);
    EXPECT_EQ(timeProvider.now(), 1665561230);
}

TEST(TimeProvider, DefTimeProvider)
{
    DefTimeProvider timeProvider;
    std::time_t timeToSleep = 1;
    std::time_t current = timeProvider.now();
    timeProvider.sleep(timeToSleep);
    EXPECT_EQ(timeProvider.now() - current, timeToSleep);
}

// TODO: enable after figuring out how to run the test as root
TEST(TimeProvider, DISABLED_SystemTimeChanged)
{
    std::promise<std::time_t> readyPromise;
    std::future<std::time_t> readyFuture(readyPromise.get_future());
    DefTimeProvider timeProvider;
    std::time_t timeToJump = 10;

    auto fn = [&readyPromise](std::time_t oldTime) { readyPromise.set_value(oldTime); };
    EXPECT_NO_THROW(timeProvider.subscribe(fn));
    std::time_t current = timeProvider.now();

    struct timeval tv;
    struct timezone tz;
    gettimeofday(&tv, &tz);
    tv.tv_sec += timeToJump;
    settimeofday(&tv, &tz);

    EXPECT_EQ(timeProvider.now() - current, timeToJump);
    EXPECT_EQ(current, readyFuture.get());
}
