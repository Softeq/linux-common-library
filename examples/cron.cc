#include <common/logging/log.hh>
#include <common/system/cron.hh>

#include <iostream>
#include <chrono>
#include <thread>
#include <csignal>

using namespace softeq::common::system;
using namespace softeq::common::logging;

namespace
{
const char *const LOG_DOMAIN = "ExamplesCron";

bool cronCallback(const std::string &data)
{
    LOGI(LOG_DOMAIN, "executing job with param: %s", data.c_str());
    return true;
}

bool global_isActive = true;

void catch_function(int signo)
{
    LOGI(LOG_DOMAIN, "Interactive attention signal caught. %d", signo);
    global_isActive = false;
}
} // namespace

int main()
{
    softeq::common::logging::log().level(LogLevel::TRACE);

    Cron::UPtr cron(CronFactory::create());
    cron->addJob("Each minute", "* * * * * *", [] { return cronCallback("1"); });
    cron->addJob("Each 5 minutes", "*/5 * * * * *", [] { return cronCallback("2"); });
    cron->addJob("Each working day at 19:00 (Go home)", "0 19 * * 1-6 *", [] { return cronCallback("3"); });
    cron->addJob("Happy new year", "0 0 1 1 * *", [] { return cronCallback("4"); });
    cron->addJob("Each odd minute", "1-59/2 * * * * *", [] { return cronCallback("5"); });
    cron->addJob("Each even minute", "0-59/2 * * * * *", [] { return cronCallback("6"); });
    cron->addJob("Weekly status call", "0 13 * * * 5", [] { return cronCallback("7"); });
    cron->addJob("Each 23 minute of odd hour at work (9:23, 11:23...17:23)", "23 9-18/2 * * * *",
                 [] { return cronCallback("8"); });
    cron->start();

    if (signal(SIGINT, catch_function) == SIG_ERR)
    {
        LOGE(LOG_DOMAIN, "An error occurred while setting a signal handler");
        return EXIT_FAILURE;
    }

    while (global_isActive)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }

    LOGI(LOG_DOMAIN, "Exiting");

    return EXIT_SUCCESS;
}
