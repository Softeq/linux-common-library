#ifndef SOFTEQ_COMMON_SIGNAL_MGR_H_
#define SOFTEQ_COMMON_SIGNAL_MGR_H_

#include <functional>
#include <map>
#include <mutex>
#include <csignal>

namespace softeq
{
namespace common
{

class SignalManager final
{
public:
    SignalManager(const SignalManager &) = delete;
    SignalManager &operator=(const SignalManager &) = delete;

    static SignalManager &instance();

    using SignalAction = std::function<void(int)>;

    bool RegisterHandler(int signum, SignalAction handler, bool dont_override = false);
    bool ClearHandler(int signum);
    bool MaskSignal(int signum);
    bool SetDefault(int signum);

private:
    SignalManager() = default;
    static void GlobalHandler(int signum);

    struct SignalHandler
    {
        struct sigaction oldAction;
        SignalAction handler;
    };
    std::mutex _mutex;
    std::map<int, SignalHandler> _handlers;
};

} // namespace common
} // namespace softeq

#endif // SOFTEQ_COMMON_SIGNAL_MGR_H_
