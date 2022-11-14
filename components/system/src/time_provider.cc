#include "time_provider.hh"

#include <common/logging/log.hh>
#include <common/stdutils/stdutils.hh>
#include <common/stdutils/scope_guard.hh>

#include <condition_variable>
#include <errno.h>
#include <limits.h>
#include <memory>
#include <mutex>
#include <poll.h>
#include <stdio.h>
#include <sys/timerfd.h>
#include <unistd.h>

#include <cstring>

using namespace softeq::common;
using namespace softeq::common::system;

namespace
{
const char *const LOG_DOMAIN = "TimeProvider";
} // namespace

TimeProvider::~TimeProvider()
{
    interruptWait();
}

TimeProvider::SPtr TimeProvider::instance(TimeProvider::SPtr newTP /* = nullptr*/)
{
    static TimeProvider::SPtr tp = std::make_shared<DefTimeProvider>();
    if (newTP)
    {
        tp = newTP;
    }
    return tp;
}

void TimeProvider::sleep(std::time_t duration)
{
    std::mutex m;
    std::unique_lock<std::mutex> lock(m);
    waitFor(lock, duration);
}

TimeProvider::State TimeProvider::wait(std::unique_lock<std::mutex> &lock)
{
    LOGD(LOG_DOMAIN, __func__);
    _state = State::Wait;
    _condVar.wait(lock, [this] { return _state == State::TimeChange || _state == State::Interrupt; });
    return _state;
}

TimeProvider::State TimeProvider::waitFor(std::unique_lock<std::mutex> &lock, std::time_t duration)
{
    LOGD(LOG_DOMAIN, __func__);
    std::time_t newTime, current = newTime = now();
    _state = State::Wait;
    do
    {
        LOGD(LOG_DOMAIN, "loop wait from %ld for %ld at %ld", current, duration, newTime);
        if (_condVar.wait_for(lock, std::chrono::seconds(current + duration - newTime)) == std::cv_status::timeout)
        {
            _state = State::Wait;
        }
        newTime = now();
        LOGD(LOG_DOMAIN, "interrupt wait at %ld with status %s", newTime,
             _state == State::TimeChange ? "TimeChange" : _state == State::Interrupt ? "Interrupt" : "Wait");
    } while (current + duration > newTime && _state != State::Interrupt);

    LOGD(LOG_DOMAIN, "stop wait for %ld at %ld", duration, newTime);
    return _state;
}

TimeProvider::State TimeProvider::waitUntil(std::unique_lock<std::mutex> &lock, std::time_t time)
{
    LOGD(LOG_DOMAIN, __func__);
    std::time_t current = now();
    if (time <= current)
    {
        return _state;
    }
    return waitFor(lock, time - current);
}

void TimeProvider::interruptWait()
{
    LOGD(LOG_DOMAIN, __func__);
    _state = State::Interrupt;
    _condVar.notify_all();
}

void TimeProvider::subscribe(const TimeChangedCb &cb)
{
    std::unique_lock<std::mutex> lock(_handlersMutex);
    _handlers.emplace_back(cb);
}

void TimeProvider::unsubscribe(const TimeChangedCb &cb)
{
    std::unique_lock<std::mutex> lock(_handlersMutex);
    _handlers.remove_if([&cb](const TimeChangedCb &innerCb) { return cb.target_type() == innerCb.target_type(); });
}

void TimeProvider::onTimeChanged(std::time_t oldTime)
{
    LOGD(LOG_DOMAIN, "Time changed from %ld to %ld", oldTime, now());

    _state = State::TimeChange;
    std::unique_lock<std::mutex> lock(_handlersMutex);
    for (TimeChangedCb cb : _handlers)
    {
        cb(oldTime);
    }
    _condVar.notify_all();
}

DefTimeProvider::DefTimeProvider()
{
    _sysTimeMonitorThread = std::thread(&DefTimeProvider::sysTimeChangeMonitor, this);
    LOGD(LOG_DOMAIN, "sysTimeChangeMonitoring thread is started");
}

DefTimeProvider::~DefTimeProvider()
{
    _stopTimeMonitorThread = true;

    if (_sysTimeMonitorThread.joinable())
    {
        _sysTimeMonitorThread.join();
        LOGD(LOG_DOMAIN, "sysTimeChangeMonitoring thread is stopped");
    }
}

time_t DefTimeProvider::now() const
{
    return ::time(nullptr);
}

void DefTimeProvider::sysTimeChangeMonitor()
{
    int fdTimer = timerfd_create(CLOCK_REALTIME, 0);
    stdutils::scope_guard guard([&fdTimer] { ::close(fdTimer); });

    struct itimerspec timerSpec
    {
        .it_interval = {.tv_sec = INT_MAX, .tv_nsec = 0}, .it_value = {.tv_sec = INT_MAX, .tv_nsec = 0 }
    };

    int retSettime = timerfd_settime(fdTimer, TFD_TIMER_ABSTIME | TFD_TIMER_CANCEL_ON_SET, &timerSpec, nullptr);
    if (retSettime < 0)
    {
        throw std::system_error(errno, std::system_category(), "Can't set timerfd time");
    }

    struct pollfd fds
    {
        .fd = fdTimer, .events = POLLIN | POLLERR, .revents = 0,
    };

    constexpr int timeoutMs = 500; // timeout wait until the time change event occurs
    constexpr int fdsNumber = 1;   // timerfd file descriptors number
    constexpr int bufferSize = 10; // buffer size has to be greater than 8 bytes
    std::time_t oldTime = now();

    // GCOVR_EXCL_START
    while (!_stopTimeMonitorThread)
    {
        int retPoll = poll(&fds, fdsNumber, timeoutMs);
        if (retPoll > 0)
        {
            uint8_t buffer[bufferSize];
            int retRead = read(fds.fd, &buffer, bufferSize);
            if (retRead < 0)
            {
                if (errno != ECANCELED)
                {
                    throw std::system_error(errno, std::system_category(), "timerfd read error");
                }

                LOGI(LOG_DOMAIN, "System time has been changed");

                onTimeChanged(oldTime);
            }
            // Value retRead == 0 could be in case TFD_TIMER_CANCEL_ON_SET flag wasn't set in timerfd_settime.
            // Value retRead > 0 is a number of timer expirations. In the case of a 32-bit system, the timer
            // will expire approximately once every 74 years.
        }
        else if (retPoll == 0) // Timeout
        {
            oldTime = now();
        }
        else
        {
            throw std::system_error(errno, std::system_category(), "timerfd polling error");
        }
    }
    // GCOVR_EXCL_STOP
}
