#ifndef COMMON_TESTS_TEST_TIME_PROVIDER_HH_
#define COMMON_TESTS_TEST_TIME_PROVIDER_HH_

#include <common/system/time_provider.hh>
#include <atomic>

class TestTimeProvider : public softeq::common::system::TimeProvider
{
    std::atomic<std::int64_t> _timeOffset{0};

public:
    void setTime(int second, int minute, int hour, int dayOfMonth, int month, int year)
    {
        std::tm t{};
        t.tm_sec = second;
        t.tm_min = minute;
        t.tm_hour = hour;
        t.tm_mday = dayOfMonth;
        t.tm_mon = month - 1;
        t.tm_year = year - 1900;
        t.tm_isdst = -1;
        setTime(mktime(&t));
    }

    void setTime(std::time_t time)
    {
        setTimeOffset(time - std::time(nullptr));
    }

    void setTimeOffset(std::int64_t timeOffset)
    {
        std::time_t oldTime = now();
        _timeOffset = timeOffset;
        onTimeChanged(oldTime);
        // chaging the time is not offen and instantly
        // to have posibility to call it continuesly do sleep
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    void sleep(std::time_t duration) override
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));

        _timeOffset += duration;
        _state = State::Wait;
        _condVar.notify_all();
    }

    time_t now() const override
    {
        return std::time(nullptr) + _timeOffset;
    }
};

#endif
