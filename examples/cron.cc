#include <iostream>
#include <chrono>
#include <thread>
#include <csignal>
#include <softeq/common/log.hh>

#include <softeq/common/cron.hh>

using namespace softeq::common;

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

int main(int argc, char *argv[])
{
    softeq::common::log().level(LogLevel::TRACE);

    ICron::UPtr cron(CronFactory::create());
    cron->addJob("Each minute", "* * * * * *", true, [] { return cronCallback("1"); });
    cron->addJob("Each 5 minutes", "*/5 * * * * *", true, [] { return cronCallback("2"); });
    cron->addJob("Each working day at 19:00 (Go home)", "0 19 * * 1-6 *", true, [] { return cronCallback("3"); });
    cron->addJob("Happy new year", "0 0 1 1 * *", true, [] { return cronCallback("4"); });
    cron->addJob("Each odd minute", "1-59/2 * * * * *", true, [] { return cronCallback("5"); });
    cron->addJob("Each even minute", "0-59/2 * * * * *", true, [] { return cronCallback("6"); });
    cron->addJob("Weekly status call", "0 13 * * * 5", true, [] { return cronCallback("7"); });
    cron->addJob("Each 23 minute of odd hour at work (9:23, 11:23...17:23)", "23 9-18/2 * * * *", true,
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
