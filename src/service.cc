#include "softeq/common/fsutils.hh"
#include "softeq/common/service.hh"
#include "signal_manager.hh"
#include "softeq/common/log.hh"
#include "softeq/common/scope_guard.hh"
#include "softeq/common/settings.hh"

#include <cstring>
#include <execinfo.h>
#include <fcntl.h>
#include <fstream>
#include <future>
#include <sys/stat.h>
#include <sys/types.h>
#include <thread>
#include <unistd.h>

#ifdef WITH_SYSTEMD
#include <systemd/sd-daemon.h>
#endif

#ifdef HAS_GCOV
#ifdef __cplusplus
extern "C" void __gcov_flush();
#endif
#endif

using namespace softeq::common;

namespace
{
const char *const LOG_DOMAIN = "SystemService";
const long DEFAULT_PULSE_TIME = 500L;
const long MIN_PULSE_TIME = 1L;

int createPidFile(const char *procname, const char *pidfile)
{
    int fd;
    int ret = -1;

    fsutils::Path pidpath(pidfile);
    if (pidpath.hasParent())
    {
        fsutils::Path parent_path = pidpath.parent();
        std::error_code ec;
        if (!parent_path.exist() && !fsutils::mkdirs(parent_path))
        {
            LOGE(LOG_DOMAIN, "Failed to create path %s (%s)", std::string(parent_path).c_str(),
                                                            strerror(errno));
            return -EPERM;
        }
    }

    fd = open(pidfile, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
    if (fd == -1)
    {
        ret = -errno;
        LOGE(LOG_DOMAIN, "Could not open PID file %s (%s)", pidfile, strerror(-ret));
        return ret;
    }

    int flags = fcntl(fd, F_GETFD); /* Fetch flags */
    if (flags == -1)
    {
        ret = -errno;
        LOGE(LOG_DOMAIN, "Could not get flags for PID file %s (%s)", pidfile, strerror(-ret));
        return ret;
    }

    flags |= FD_CLOEXEC; /* Turn on FD_CLOEXEC */

    if (fcntl(fd, F_SETFD, flags) == -1) /* Update flags */
    {
        ret = -errno;
        LOGE(LOG_DOMAIN, "Could not set flags for PID file %s (%s)", pidfile, strerror(-ret));
        return ret;
    }

    struct flock fl;

    fl.l_type = F_WRLCK;
    fl.l_whence = SEEK_SET;
    fl.l_start = 0;
    fl.l_len = 0;

    if (fcntl(fd, F_SETLK, &fl) == -1)
    {
        ret = -errno;
        if (errno == EAGAIN || errno == EACCES)
        {
            LOGE(LOG_DOMAIN, "PID file '%s' is locked. Probably '%s' is already running", pidfile, procname);
        }
        else
        {
            LOGE(LOG_DOMAIN, "Unable to lock PID file '%s' (%s)", pidfile, strerror(-ret));
        }
        return ret;
    }

    if (ftruncate(fd, 0) == -1)
    {
        ret = -errno;
        LOGE(LOG_DOMAIN, "Could not truncate PID file '%s' (%s)", pidfile, strerror(-ret));
        return ret;
    }

    std::string output = std::to_string(getpid()) + '\n';
    if (write(fd, output.c_str(), output.size()) != static_cast<ssize_t>(output.size()))
    {
        LOGE(LOG_DOMAIN, "Writing to PID file '%s'", pidfile);
        return -EIO;
    }

    return fd;
}

[[ noreturn ]] void terminateHandler()
{
    std::exception_ptr exptr{std::current_exception()};
    if (exptr != nullptr)
    {
        try
        {
            std::rethrow_exception(exptr);
        }
        catch (std::exception &ex)
        {
            LOGC(LOG_DOMAIN, "Terminated due to exception: %s", ex.what());
        }
        catch (...)
        {
            LOGF(LOG_DOMAIN, "Terminated due to non std exception");
        }
    }
    else
    {
        LOGF(LOG_DOMAIN, "Terminated due to unknown reason");
    }

#ifdef NDEBUG
    exit(EXIT_FAILURE);
#else
    int nptrs;

    constexpr int cSize = 100;
    std::array<void*, cSize> buffer;
    char **strings;

    nptrs = backtrace(&buffer[0], cSize);

    strings = backtrace_symbols(&buffer[0], nptrs);
    if (strings != nullptr)
    {
        for (int j = 0; j < nptrs; j++)
        {
            LOGD(LOG_DOMAIN, "#%d %s", j + 1, strings[j]);
        }
    }
    else
    {
        LOGE(LOG_DOMAIN, "Cannot obtain backtrace (%s)", strerror(errno));
    }

    free(strings);
    /* Dump coverage data in real-time, because after std::abort
     * coverage reports are not produced */
#ifdef HAS_GCOV
    __gcov_flush();
#endif
    /* It will dump the core to investigate */
    std::abort();
#endif
}

} // namespace

// class ServiceDispatcher
ServiceDispatcher::ServiceDispatcher(SystemService &service)
    : _host(service)
{
    _heartbeatPeriodMs = DEFAULT_PULSE_TIME;
}

long ServiceDispatcher::heartbeatPeriod() const noexcept
{
    return _heartbeatPeriodMs;
}

void ServiceDispatcher::setHeartbeatPeriod(long valueMs) noexcept
{
    if (valueMs < MIN_PULSE_TIME)
    {
        valueMs = MIN_PULSE_TIME;
    }

    _heartbeatPeriodMs = valueMs;
}

SystemService &ServiceDispatcher::service() noexcept
{
    return _host;
}

// class SystemService
//
SystemService::SystemService()
{
    std::set_terminate(terminateHandler);
    registerSignalsHandler();
}

SystemService::~SystemService()
{
    terminate();
    LOGD(LOG_DOMAIN, "The service has been destroyed");
}

int SystemService::run(std::unique_ptr<ServiceDispatcher> service)
{
    _impl = std::move(service);
    _startTime = std::time(nullptr);
    _running = true;
    _active = true;
    bool result;

    LOGI(LOG_DOMAIN, "Initialization of the service");
    try
    {
        result = _impl->onStart();
    }
    catch (const std::exception &ex)
    {
        LOGE(LOG_DOMAIN, "Exception onStart %s", ex.what());
        result = false;
    }

    if (!result)
    {
        LOGE(LOG_DOMAIN, "The start has failed");
        _running = false;
        _active = false;
        if (_exitCode == EXIT_SUCCESS)
        {
            _exitCode = EXIT_FAILURE;
        }
        return _exitCode;
    }

    std::future<void> beat;
    while (_active)
    {
        std::chrono::steady_clock::time_point pulsePassed =
            std::chrono::steady_clock::now() + std::chrono::milliseconds(_impl->_heartbeatPeriodMs);

        if (beat.valid() && beat.wait_until(pulsePassed) != std::future_status::ready)
        {
            LOGW(LOG_DOMAIN, "Heart beats too fast for your body");
        }
        else
        {
            std::unique_lock<std::mutex> lock(_mutex);
            if (_running)
            {
                beat = std::async(std::launch::async, [this] { _impl->heartbeat(); });
            }
            _condVar.wait_until(lock, pulsePassed);
        }
    }

    if (beat.valid())
    {
        LOGI(LOG_DOMAIN, "Wait for the last heartbeat");
        beat.wait();
    }
    LOGI(LOG_DOMAIN, "Destructing of the service");

    try
    {
        result = _impl->onStop();
    }
    catch (const std::exception &ex)
    {
        LOGE(LOG_DOMAIN, "Exception onStop %s", ex.what());
        result = false;
    }

    if (!result)
    {
        LOGE(LOG_DOMAIN, "The stop has failed");
        if (_exitCode == EXIT_SUCCESS)
        {
            _exitCode = EXIT_FAILURE;
        }
    }

    _impl = nullptr;
    _running = false;
    _startTime = 0;

    LOGI(LOG_DOMAIN, "Exiting...");
    return _exitCode;
}

void SystemService::signal(int signum)
{
    raise(signum);
}

void SystemService::terminate()
{
    signal(SIGINT);
}

std::time_t SystemService::uptime() const
{
    return _startTime ? std::time(nullptr) - _startTime : 0;
}

bool SystemService::isActive() const
{
    return _active;
}

bool SystemService::isRunning() const
{
    return _running;
}

void SystemService::setError(int code)
{
    _exitCode = code;
}

void SystemService::registerSignalsHandler()
{
    SignalManager &signal_manager = SignalManager::instance();
    signal_manager.RegisterHandler(
        SIGINT, [this](int signum) { handleSignal(signum); },
        /* overwrite = */ false);
    signal_manager.RegisterHandler(
        SIGTERM, [this](int signum) { handleSignal(signum); },
        /* overwrite = */ false);
    signal_manager.RegisterHandler(
        SIGHUP, [this](int signum) { handleSignal(signum); },
        /* overwrite = */ false);
    signal_manager.RegisterHandler(
        SIGSEGV, [this](int signum) { handleSignal(signum); },
        /* overwrite = */ false);
}

void SystemService::handleSignal(int signum)
{
    switch (signum)
    {
    case SIGINT:
    case SIGTERM:
        finalActions();
        break;

    case SIGHUP:
        _running = false;
        bool result;
        try
        {
            result = _impl->onConfigure();
        }
        catch (const std::exception &ex)
        {
            LOGE(LOG_DOMAIN, "Exception onConfigure %s", ex.what());
            result = false;
        }
        LOGE(LOG_DOMAIN, "Reconfiguration reasult: %s", result ? "success" : "failure");

        _running = true;
        break;

    case SIGSEGV:
        finalActions();
        LOGE(LOG_DOMAIN, "Segmentation fault detected: %d - %s", signum, strsignal(signum));
        exit(signum);

    default:
        LOGW(LOG_DOMAIN, "Uncaught signal: %d - %s", signum, strsignal(signum));
        break;
    }
}

void SystemService::finalActions()
{
    std::lock_guard<std::mutex> lock(_mutex);
    _active = false;
    _condVar.notify_one();
}

#ifdef WITH_SYSTEMD
SystemdService::SystemdService(bool isNotifyType)
    : SystemService()
    , _isNotifyType(isNotifyType)
{
}

int SystemdService::run(std::unique_ptr<ServiceDispatcher> service)
{
    if (_isNotifyType)
    {
        sd_notify(0, "READY=1");
    }
    SystemService::run(std::move(service));

    if (_isNotifyType)
    {
        sd_notify(0, "STOPPING=1");
    }
    return _exitCode;
}
#endif

SysVService::SysVService()
    : SystemService()
{
    std::ifstream("/proc/self/comm") >> _procName;
    _pidFile = "/var/run/user/" + std::to_string(getuid()) + "/" + _procName + ".pid";
}

int SysVService::run(std::unique_ptr<ServiceDispatcher> service)
{
    if (!daemonize())
    {
        LOGE(LOG_DOMAIN, "Failed to daemonize");
        return EXIT_FAILURE;
    }

    /* Register new signals handler after daemonization */
    registerSignalsHandler();
    scope_guard cleanup;

    int fd = createPidFile(_procName.c_str(), _pidFile.c_str());
    if (fd < 0)
    {
        return fd; // fd contains the error code
    }

    cleanup += [fd, this]() {
        close(fd);
        unlink(_pidFile.c_str());
    };

    SystemService::run(std::move(service));

    return _exitCode;
}

// SysV daemonize following the scheme from https://www.man7.org/linux/man-pages/man7/daemon.7.html
bool SysVService::daemonize()
{
    // Close all open file descriptors
    for (long fd = sysconf(_SC_OPEN_MAX); fd > 0; fd--)
    {
        close(static_cast<int>(fd));
    }

    // Reset all signal handlers to default
    SignalManager &signal_manager = SignalManager::instance();
    for (size_t i = 0; i < _NSIG; i++)
    {
        signal_manager.SetDefault(i);
    }
    // Reset the signal mask
    sigset_t sigMask;
    sigemptyset(&sigMask);
    sigprocmask(SIG_SETMASK, &sigMask, nullptr);

    if (!forkAndTerminateParent())
        return false;

    // detach from any terminal and create an independent session
    if (setsid() < 0)
    {
        return false;
    }
    signal_manager.MaskSignal(SIGCHLD);

    if (!forkAndTerminateParent())
    {
        return false;
    }

    // Change the working directory to the root directory
    if (chdir("/"))
    {
        return false;
    }

    // connect /dev/null to standard input, output, and error
    stdin = fopen("/dev/null", "r");
    stdout = fopen("/dev/null", "w+");
    stderr = fopen("/dev/null", "w+");

    return true;
}

bool SysVService::forkAndTerminateParent()
{
    pid_t pid = fork();

    if (pid < 0)
    {
        return false;
    }
    // Terminate the parent
    if (pid > 0)
    {
        exit(EXIT_SUCCESS);
    }
    return true;
}
