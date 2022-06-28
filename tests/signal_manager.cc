#include "../src/signal_manager.hh"

#include <gtest/gtest.h>
#include <iostream>
#include <pthread.h>
#include <signal.h>
#include <string.h>
#include <sstream>

using namespace softeq::common;

namespace
{

class HandlersInfo final
{
public:

    HandlersInfo(const HandlersInfo &) = delete;
    HandlersInfo &operator=(const HandlersInfo &) = delete;

    static HandlersInfo &instance()
    {
        static HandlersInfo handlersInfo;
        return handlersInfo;
    }

    int getReceivedSignal()
    {
        int result;
        result = _receivedSignal;
        _receivedSignal = 0;
        return result;
    }

    void setReceivedSignal(int value)
    {
        _receivedSignal = value;
    }

private:
    HandlersInfo() = default;
    int _receivedSignal;
};

void threadHandler(int sigNum)
{
    ASSERT_TRUE((sigNum == SIGUSR1) || (sigNum = SIGUSR2));
    HandlersInfo &handlersInfo = HandlersInfo::instance();
    handlersInfo.setReceivedSignal(sigNum);
}
}   /* namespace */

TEST(SigManager, SigManager)
{
    SignalManager &signalManager = SignalManager::instance();
    HandlersInfo &handlersInfo = HandlersInfo::instance();
    ASSERT_TRUE(signalManager.RegisterHandler(SIGUSR1,
                                              [this](int signum) { threadHandler(signum); },
                                              true));
    int time = 0;
    int gettedSignum = 0;
    raise(SIGUSR1);
    while(!(gettedSignum = handlersInfo.getReceivedSignal()) && ((++time) < 3))
    {
        sleep(1);
    }
    ASSERT_LT(time, 3);
    ASSERT_EQ(gettedSignum, SIGUSR1);

    signalManager.RegisterHandler(SIGUSR2,
                                  [this](int signum) { threadHandler(signum); },
                                  true);
    time = 0;
    raise(SIGUSR2);
    while(!(gettedSignum = handlersInfo.getReceivedSignal()) && ((++time) < 3))
    {
        sleep(1);
    }
    ASSERT_LT(time, 3);
    ASSERT_EQ(gettedSignum, SIGUSR2);

    ASSERT_TRUE(signalManager.MaskSignal(SIGUSR1));
    raise(SIGUSR1);
    sleep(0.1);
    ASSERT_FALSE(handlersInfo.getReceivedSignal());

    ASSERT_FALSE(signalManager.RegisterHandler(SIGUSR1,
                                               [this](int signum) { threadHandler(signum); },
                                               true));
    ASSERT_FALSE(signalManager.SetDefault(SIGUSR1));
    ASSERT_TRUE(signalManager.RegisterHandler(SIGUSR1,
                                              [this](int signum) { threadHandler(signum); },
                                              true));
    time = 0;
    raise(SIGUSR1);
    while(!(gettedSignum = handlersInfo.getReceivedSignal()) && ((++time) < 3))
    {
        sleep(1);
    }
    ASSERT_LT(time, 3);
    ASSERT_EQ(gettedSignum, SIGUSR1);

    ASSERT_FALSE(signalManager.RegisterHandler(-20,
                                               [this](int signum) { threadHandler(signum); },
                                               true));
    ASSERT_TRUE(signalManager.SetDefault(SIGUSR1));
    ASSERT_FALSE(signalManager.ClearHandler(SIGUSR1));
    ASSERT_TRUE(signalManager.ClearHandler(SIGUSR2));
}
