#include <atomic>
#include <chrono>
#include <future>
#include <thread>
#include <functional>
#include <gtest/gtest.h>
#include <fstream>

#include "softeq/common/service.hh"
#include "softeq/common/fsutils.hh"
#include "softeq/common/log.hh"

#define NAME(x) #x

using namespace softeq::common;

class SimpleService : public ServiceDispatcher
{
public:
    enum RunMode
    {
        SuccessfulRun,
        FailureOnStart,
        FailureOnStop,
        CrashOnStart,
        SegFaultOnStart,
        CrashOnStop,
        CrashOnConfigure,
        CrashOnHeartbeat
    };

    explicit SimpleService(SystemService &svc, RunMode mode)
        : ServiceDispatcher(svc)
        , _mode(mode)
    {
    }

    bool onStart() override
    {
        return _runmap[_mode].onStart();
    }

    bool onStop() override
    {
        return _runmap[_mode].onStop();
    }

    bool onConfigure() override
    {
        return _runmap[_mode].onConfig();
    }

    void heartbeat() noexcept override
    {
        _runmap[_mode].heartbeat();
    }

private:
    using HeartbeatHandler = std::function<void(void)>;
    using InitHandler = std::function<bool(void)>;
    using DeinitHandler = std::function<bool(void)>;
    using ConfigHandler = std::function<bool(void)>;

    struct Handlers
    {
        InitHandler onStart;
        DeinitHandler onStop;
        ConfigHandler onConfig;
        HeartbeatHandler heartbeat;
    };
    using RunMap = std::map<RunMode, Handlers>;

    // clang-format off
    RunMap _runmap = {
        {
            RunMode::SuccessfulRun,
            {
                [this]() -> bool {
                    setHeartbeatPeriod(100); // adjust the period to speed up the test
                    return true;
                },
                []() -> bool { return true; },
                nullptr,
                [this]() -> void {
                    static unsigned long n = 5;
                    static std::atomic<int> hbCounter{0};

                    if (++hbCounter > 1)
                    {// another async call occured, test failed
                        service().setError(hbCounter);
                        service().terminate();
                    }
                    else if (--n == 0)
                    {// n-1 heartbeat handlers have been called, test finished
                        EXPECT_EQ(service().uptime(), 1);
                        service().terminate();
                    }
                    else
                    {// heartbeat handler's body (longs k=n-i heartbeat periods)
                        std::this_thread::sleep_for(std::chrono::milliseconds(n*(heartbeatPeriod()-1)));
                    }
                    --hbCounter;
                }
            }
        },
        {
            RunMode::FailureOnStart,
            {
                [this]() -> bool {
                    service().setError(-1);
                    return false;
                },
                nullptr,
                nullptr,
                nullptr
            }
        },
        {
            RunMode::FailureOnStop,
            {
                []() -> bool { return true; },
                [this]() -> bool {
                    service().setError(SIGINT);
                    return false;
                },
                nullptr,
                [this]() -> void { service().terminate(); }
            }
        },
        {
            RunMode::CrashOnStart,
            {
                []() -> bool { throw std::runtime_error("Runtime error on service start"); },
                nullptr,
                nullptr
            }
        },
        {
            RunMode::SegFaultOnStart,
            {
                [this]() -> bool {
                    service().signal(SIGSEGV);
                    return false;
                },
                nullptr,
                nullptr,
                nullptr
            }
        },
        {
            RunMode::CrashOnStop,
            {
                [this]() -> bool {
                    service().signal(SIGHUP);
                    return true;
                },
                []() -> bool { throw std::runtime_error("Runtime error on service stop"); },
                []() -> bool { return true; },
                [this]() -> void { service().signal(SIGTERM); }
            }
        },
        {
            RunMode::CrashOnConfigure,
            {
                [this]() -> bool {
                    service().signal(SIGHUP);
                    return true;
                },
                []() -> bool { return true; },
                []() -> bool { throw std::runtime_error("Runtime error on service reconfigration"); },
                [this]() -> void { service().signal(SIGTERM); }
            }
        },
        {
            RunMode::CrashOnHeartbeat,
            {
                []() -> bool { return true; },
                []() -> bool { return true; },
                nullptr,
                [this]() -> void { throw std::runtime_error("Uncaught error on service heartbeat"); }
            }
        }
    };
    // clang-format on

    RunMode _mode;
};

TEST(Service, SetHeartbeatPeriodTest)
{
    SystemService service;
    SimpleService dispatcher(service, SimpleService::RunMode::FailureOnStart);

    dispatcher.setHeartbeatPeriod(100500);
    EXPECT_EQ(dispatcher.heartbeatPeriod(), 100500);
    dispatcher.setHeartbeatPeriod(0);
    EXPECT_EQ(dispatcher.heartbeatPeriod(), 1);
    dispatcher.setHeartbeatPeriod(-5);
    EXPECT_EQ(dispatcher.heartbeatPeriod(), 1);
}

TEST(Service, UnsuccessfulStart)
{
    SystemdService service;

    int exitCode = service.run(
        std::unique_ptr<ServiceDispatcher>(new SimpleService(service, SimpleService::RunMode::FailureOnStart)));

    EXPECT_EQ(exitCode, -1);
    EXPECT_FALSE(service.isActive());
    EXPECT_FALSE(service.isRunning());

    service.setError(EXIT_SUCCESS);
    EXPECT_NO_THROW(exitCode = service.run(std::unique_ptr<ServiceDispatcher>(
                        new SimpleService(service, SimpleService::RunMode::CrashOnStart))));
    EXPECT_NE(exitCode, EXIT_SUCCESS);
    EXPECT_FALSE(service.isActive());
    EXPECT_FALSE(service.isRunning());

    EXPECT_EXIT(service.run(std::unique_ptr<ServiceDispatcher>(
                new SimpleService(service, SimpleService::RunMode::SegFaultOnStart))),
                ::testing::ExitedWithCode(SIGSEGV),
                "");
}

TEST(Service, UnsuccessfulStop)
{
    SystemdService service;
    int exitCode = service.run(
        std::unique_ptr<ServiceDispatcher>(new SimpleService(service, SimpleService::RunMode::FailureOnStop)));

    EXPECT_EQ(exitCode, SIGINT);
    EXPECT_FALSE(service.isActive());
    EXPECT_FALSE(service.isRunning());

    service.setError(EXIT_SUCCESS);
    EXPECT_NO_THROW(exitCode = service.run(std::unique_ptr<ServiceDispatcher>(
                        new SimpleService(service, SimpleService::RunMode::CrashOnStop))));
    EXPECT_FALSE(exitCode == EXIT_SUCCESS);
    EXPECT_FALSE(service.isActive());
    EXPECT_FALSE(service.isRunning());
}

TEST(Service, HeartBeat)
{
    SystemdService service(true);

    EXPECT_TRUE(service.uptime() == 0);
    int exitCode = service.run(
        std::unique_ptr<ServiceDispatcher>(new SimpleService(service, SimpleService::RunMode::SuccessfulRun)));

    EXPECT_TRUE(exitCode == EXIT_SUCCESS);
    EXPECT_FALSE(service.isActive());
    EXPECT_FALSE(service.isRunning());
    EXPECT_TRUE(service.uptime() == 0);
}

TEST(Service, CrashOnConfigure)
{
    SystemdService service;
    int exitCode;
    EXPECT_TRUE(service.uptime() == 0);
    EXPECT_NO_THROW(exitCode = service.run(std::unique_ptr<ServiceDispatcher>(
                        new SimpleService(service, SimpleService::RunMode::CrashOnConfigure))));
    EXPECT_TRUE(exitCode == EXIT_SUCCESS);
    EXPECT_FALSE(service.isActive());
    EXPECT_FALSE(service.isRunning());
    EXPECT_TRUE(service.uptime() == 0);
}

TEST(Service, TerminateHandler)
{
    SystemdService service(true);

    EXPECT_EXIT(service.run(std::unique_ptr<ServiceDispatcher>(
                new SimpleService(service, SimpleService::RunMode::CrashOnHeartbeat))),
                ::testing::KilledBySignal(SIGABRT),
                "");
}

TEST(Service, SysVService)
{
    std::string testFileName{"svservice_test"};
    pid_t pid = fork();

    ASSERT_TRUE(pid >= 0) << "Fork error: " << strerror(errno);

    if (pid == 0)
    {
        std::string pathToTest{"/workdir/build/tests/" + testFileName};
        ASSERT_TRUE(execl(pathToTest.c_str(), pathToTest.c_str(), NULL) >= 0)
                    << "Failed to start test executable: " << pathToTest;
    }
    else
    {
        std::time_t startTime = std::time(nullptr);
        std::time_t stopTime = 0;
        std::time_t uptime = 0;
        ulong heartbeatPeriod = 0;
        uint hbCount = 0;
        bool runningStatus = true;
        std::string startFile   = "/tmp/sv_onstart";
        std::string confFile    = "/tmp/sv_onconf";
        std::string stopFile    = "/tmp/sv_onstop";
        std::string hbFile      = "/tmp/sv_hbcount";
        std::string pidFile     = "/var/run/user/" + std::to_string(getuid()) +
                                  "/" + testFileName + ".pid";
        std::ifstream file;

        /* Check .pid file to find out exact start time
         * because after this file creation service starts to count its uptime */
        while(fsutils::exist(pidFile) == false) {
            if((std::time(nullptr) - startTime) > 1)
            {
                FAIL() << "No .pid file after 2 seconds. File path: " << pidFile;
            }
        }
        startTime = std::time(nullptr);

        /* Check PID file */
        std::this_thread::sleep_for(std::chrono::seconds(1));
        file.open(pidFile);
        ASSERT_TRUE(file.is_open()) << "Failed to open " NAME(pidFile) << ": " << pidFile
                                    << "\nError: " << strerror(errno);
        pid = 0;
        file >> pid;
        file.close();
        ASSERT_TRUE(pid > 0) << "Failed to obtain pid from: " << pidFile
                                << "\nError: " << strerror(errno);

        /* Check on start file */
        file.open(startFile);
        ASSERT_TRUE(file.is_open()) << "Failed to open " NAME(startFile) << ": " << startFile
                                    << "\nError: " << strerror(errno);
        file >> heartbeatPeriod;
        file.close();
        EXPECT_EQ(heartbeatPeriod, 500L);

        /* Check on configure file */
        ASSERT_TRUE(kill(pid, SIGHUP) == 0) << "Failed to send SIGHUP to pid: " << pid;
        std::this_thread::sleep_for(std::chrono::seconds(1));
        file.open(confFile);
        ASSERT_TRUE(file.is_open()) << "Failed to open " NAME(confFile) << ": " << confFile
                                    << "\nError: " << strerror(errno);
        file >> runningStatus;
        file.close();
        EXPECT_FALSE(runningStatus) << "During onConfigure() handler isRunning should be false";

        /* Check on stop file */
        ASSERT_TRUE(kill(pid, SIGTERM) == 0) << "Failed to send SIGTERM to pid: " << pid;
        stopTime = std::time(nullptr);
        std::this_thread::sleep_for(std::chrono::seconds(1));
        file.open(stopFile);
        ASSERT_TRUE(file.is_open()) << "Failed to open " NAME(stopFile) << ": " << stopFile
                                    << "\nError: " << strerror(errno);
        file >> uptime;
        file.close();
        EXPECT_EQ(uptime, stopTime - startTime);

        /* Check heartbeat count */
        file.open(hbFile);
        ASSERT_TRUE(file.is_open()) << "Failed to open " NAME(hbFile) << ": " << hbFile
                                    << "\nError: " << strerror(errno);
        file >> hbCount;
        file.close();
        EXPECT_GE(hbCount, static_cast<uint>(uptime * ( 1000.0 / heartbeatPeriod)));

        /* PID file should be deleted now */
        ASSERT_FALSE(fsutils::exist(pidFile)) << NAME(pidFile) << " = " << pidFile;
    }
}
