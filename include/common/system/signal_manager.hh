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
namespace system
{

class SignalManager final
{
public:
    SignalManager(const SignalManager &) = delete;
    SignalManager &operator=(const SignalManager &) = delete;

    static SignalManager &instance();

    using SignalAction = std::function<void(int)>;

    bool registerHandler(int signum, SignalAction handler, bool dont_override = false);
    bool clearHandler(int signum);
    bool maskSignal(int signum);
    bool setDefault(int signum);

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

} // namespace system
} // namespace common
} // namespace softeq

#endif // SOFTEQ_COMMON_SIGNAL_MGR_H_
