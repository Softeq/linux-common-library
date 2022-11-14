#ifndef SOFTEQ_COMMON_TIME_PROVIDER_H_
#define SOFTEQ_COMMON_TIME_PROVIDER_H_

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <ctime>
#include <functional>
#include <list>
#include <memory>
#include <mutex>
#include <thread>

namespace softeq
{
namespace common
{
namespace system
{
class TimeProvider
{
public:
    enum class State
    {
        Wait,
        TimeChange,
        Interrupt
    };
    /**
     * SPtr is a shared pointer to ITimeProvider
     */
    using SPtr = std::shared_ptr<TimeProvider>;
    /**
     * SPtr is a shared pointer to ITimeProvider
     */
    using WPtr = std::weak_ptr<TimeProvider>;
    /*!
     Callback ptototipe called after the time has been chaged.
     \param old time;
    */
    using TimeChangedCb = std::function<void(std::time_t)>;

    // can return timechange or interrupt
    virtual State wait(std::unique_lock<std::mutex> &lock) final;
    // if time has been changed then it checks new time can continue waiting and return timeout
    // can return timechange or interrupt or timeout
    virtual State waitFor(std::unique_lock<std::mutex> &lock, std::time_t duration) final;
    // can return timechange or interrupt or timeout
    virtual State waitUntil(std::unique_lock<std::mutex> &lock, std::time_t time) final;

    // must not change state of wait
    // default logic is to call waitFor
    virtual void sleep(std::time_t duration);

    void interruptWait();

    /**
     * Get current time.
     * @return Current time
     */
    virtual time_t now() const = 0;
    virtual ~TimeProvider();

    void subscribe(const TimeChangedCb &cb);
    void unsubscribe(const TimeChangedCb &cb);

    // factory method
    static SPtr instance(SPtr def = nullptr);

protected:
    std::condition_variable _condVar;
    std::atomic<State> _state{State::Wait};

private:
    std::mutex _handlersMutex;
    std::list<TimeChangedCb> _handlers;

protected:
    virtual void onTimeChanged(std::time_t time) final;
};

/*!
 *  DefTimeProvider is a descendant of TimeProvider implementing
 *  - getting system time
 *  - detection of system time change
 */
class DefTimeProvider : public TimeProvider
{
public:
    explicit DefTimeProvider();
    ~DefTimeProvider() override;

    /*!
     * \brief Returns current system time value
     * \return curret system time
     */
    time_t now() const override;

protected:
    /*!
     * \brief Monitors system time change
     */
    virtual void sysTimeChangeMonitor();

private:
    std::thread _sysTimeMonitorThread;
    std::atomic<bool> _stopTimeMonitorThread{false};
};

} // namespace system
} // namespace common
} // namespace softeq

#endif // SOFTEQ_COMMON_TIME_PROVIDER_H_
