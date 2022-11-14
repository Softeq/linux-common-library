#include "signal_manager.hh"

#include <common/logging/log.hh>

#include <cerrno>
#include <cstring>

namespace
{
const char LOG_DOMAIN[] = "SignalManager";
}

using namespace softeq::common::system;

SignalManager &SignalManager::instance()
{
    static SignalManager signal_manager;
    return signal_manager;
}

bool SignalManager::registerHandler(int signum, std::function<void(int)> handler, bool overwrite)
{
    std::lock_guard<std::mutex> lk(_mutex);

    struct sigaction newAction;
    struct sigaction oldAction;
    std::memset(&newAction, 0, sizeof newAction);
    std::memset(&oldAction, 0, sizeof oldAction);

    newAction.sa_handler = SignalManager::GlobalHandler;
    sigemptyset(&newAction.sa_mask);
    newAction.sa_flags = 0;

    bool isRegistered = false;
    std::map<int, SignalManager::SignalHandler>::iterator it = _handlers.find(signum);
    if (it != _handlers.end())
    {
        if (overwrite)
        {
            it->second.handler = handler;
        }
        isRegistered = true;
    }
    else
    {
        sigaction(signum, nullptr, &oldAction);
        if (oldAction.sa_handler != SIG_IGN)
        {
            if (sigaction(signum, &newAction, nullptr) == 0)
            {
                _handlers[signum] = SignalHandler{
                    .oldAction = oldAction,
                    .handler = handler,
                };
                isRegistered = true;
            }
            else
            {
                LOGE(LOG_DOMAIN, "Error registering handler for signal %d, errno: %s", signum, strerror(errno));
            }
        }
        else
        {
            LOGI(LOG_DOMAIN, "Signal %d is set to SIG_IGN", signum);
        }
    }
    return isRegistered;
}

bool SignalManager::clearHandler(int signum)
{
    std::lock_guard<std::mutex> lk(_mutex);

    bool isCleared = false;

    std::map<int, SignalManager::SignalHandler>::iterator it = _handlers.find(signum);
    if (it != _handlers.end())
    {
        sigaction(signum, &it->second.oldAction, nullptr);
        _handlers.erase(it);
        isCleared = true;
    }
    return isCleared;
}

bool SignalManager::maskSignal(int signum)
{
    std::lock_guard<std::mutex> lk(_mutex);

    std::map<int, SignalManager::SignalHandler>::iterator it = _handlers.find(signum);
    bool isRegistered = it != _handlers.end();
    if (isRegistered)
    {
        _handlers.erase(it);
    }

    struct sigaction action;
    std::memset(&action, 0, sizeof action);
    action.sa_handler = SIG_IGN;
    sigemptyset(&action.sa_mask);
    sigaction(signum, &action, nullptr);

    return isRegistered;
}

bool SignalManager::setDefault(int signum)
{
    std::lock_guard<std::mutex> lk(_mutex);

    std::map<int, SignalManager::SignalHandler>::iterator it = _handlers.find(signum);
    bool isRegistered = it != _handlers.end();
    if (isRegistered)
    {
        _handlers.erase(it);
    }

    struct sigaction action;
    std::memset(&action, 0, sizeof action);
    action.sa_handler = SIG_DFL;
    sigemptyset(&action.sa_mask);
    sigaction(signum, &action, nullptr);

    return isRegistered;
}

void SignalManager::GlobalHandler(int signum)
{
    SignalManager &signalManager = SignalManager::instance();

    std::lock_guard<std::mutex> lk(signalManager._mutex);

    std::map<int, SignalManager::SignalHandler>::iterator it = signalManager._handlers.find(signum);
    if (it != signalManager._handlers.end())
    {
        it->second.handler(signum);
    }
    else
    {
        LOGE(LOG_DOMAIN, "Signal handler triggered for signal %d, but we're not subscribed", signum);
    }
}
