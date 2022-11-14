#ifndef SOFTEQ_COMMON_SERVICE_H_
#define SOFTEQ_COMMON_SERVICE_H_

#include <csignal>
#include <mutex>
#include <condition_variable>
#include <ctime>

namespace softeq
{
namespace common
{
namespace system
{
class SystemService;

/*!
  \brief Service dispatcher interface

  The user must implement the interface. System signals are translated to
  the corresponding methods calls basing on service implementation
*/
class ServiceDispatcher
{
    friend class SystemService;

public:
    using UPtr = std::unique_ptr<ServiceDispatcher>;

    /*!
      Constructs the dispatcher
      \param[in] service The reference to the service
     */
    explicit ServiceDispatcher(SystemService &service);
    virtual ~ServiceDispatcher() = default;

    /*!
      Retrieves current heartbeat period
      \return heartbeat period in milliseconds
    */
    long heartbeatPeriod() const noexcept;

    /*!
      Sets new heartbeat period in milliseconds
    */
    void setHeartbeatPeriod(long valueMs) noexcept;

private:
    /*!
      Performs the last stage of the servide initialization, and enter the main loop.
      Executed after fork (in case of SysVInit).
      \return A success of the start. May throw exception
    */
    virtual bool onStart() = 0;
    /*!
      Performs the deinitialization of the service
      \return A success of the stop. May throw exception
    */
    virtual bool onStop() = 0;
    /*!
      Performs the reconfiguration of the service
      \return true if the signal was handled, false otherwise.
    */
    virtual bool onConfigure() = 0;

    /*!
      Indicates about an active state. Can be used for small work.
      less that PULSE time
    */
    // GCOVR_EXCL_START
    virtual void heartbeat() noexcept
    {
    }
    // GCOVR_EXCL_STOP
protected:
    /*!
      Returns the reference to current service instance
      \return service reference
    */
    SystemService &service() noexcept;

private:
    SystemService &_host;

    // Keeps a period in milliseconds when the heartbeat is called,
    // initially sets to the value of DEFAULT_PULSE_TIME, allows to adjust it from custom implementation
    long _heartbeatPeriodMs;
};

/*!
  \brief Base service implementation where the service is a program
  that runs in the background outside the interactive control of system users.
*/
class SystemService
{
    SystemService(const SystemService &) = delete;
    SystemService &operator=(const SystemService &) = delete;
    SystemService(SystemService &&) = delete;
    SystemService &operator=(SystemService &&) = delete;

public:
    SystemService();
    virtual ~SystemService();

    /*!
      Starts the service. This is a blocking call
      \param[in] service Service unique ptr
      \returns int return the code which is set by setError
    */
    virtual int run(ServiceDispatcher::UPtr service);

    /*!
      Sends the SIGNAL to itself.
      \param[in] signum Signal
    */
    void signal(int signum);

    /*!
      Terminates the service. Sends SIGINT
    */
    void terminate();

    /*!
      Returns uptime of the service
      \return the number of seconds from the start
    */
    std::time_t uptime() const;

    /*!
      Shows that the service is in working state
      \return state value
    */
    bool isActive() const;

    /*!
      Shows that the service is still running but could be not active
      \return state value
    */
    bool isRunning() const;

    /*!
      Sets custom error code that will be returned by applicaiton
    */
    void setError(int code);

protected:
    /*!
      Registers handler for particular signals
    */
    void registerSignalsHandler();

    int _exitCode = EXIT_SUCCESS;

private:
    void handleSignal(int signum);
    void finalActions();

    ServiceDispatcher::UPtr _impl = nullptr;
    std::sig_atomic_t _running = false;
    std::sig_atomic_t _active = false;

    std::mutex _mutex;
    std::condition_variable _condVar;

    std::time_t _startTime = 0;
};

#ifdef WITH_SYSTEMD
// TODO: Need description about SystemD
class SystemdService final : public SystemService
{
public:
    explicit SystemdService(bool isNotifyType = false);

    int run(std::unique_ptr<ServiceDispatcher> service) override;

private:
    bool _isNotifyType;
};
#endif

// TODO: Need description about SysVInit
class SysVService final : public SystemService
{
public:
    SysVService();

    int run(std::unique_ptr<ServiceDispatcher> service) override;

private:
    std::string _procName;
    std::string _pidFile;

    bool daemonize();
    bool forkAndTerminateParent();
};

} // namespace system
} // namespace common
} // namespace softeq

#endif // SOFTEQ_COMMON_SERVICE_H_
