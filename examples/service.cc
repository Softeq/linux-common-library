#include <common/system/service.hh>
#include <common/logging/log.hh>

namespace
{
const char *const LOG_DOMAIN = "Service";
}
using namespace softeq::common::system;

class MyService : public ServiceDispatcher
{
public:
    explicit MyService(SystemService &service)
        : ServiceDispatcher(service)
    {
        LOGI(LOG_DOMAIN, "The first initialization of the service");
    }

    MyService(const MyService &) = delete;
    MyService(MyService &&) = delete;

    ~MyService() override
    {
        LOGI(LOG_DOMAIN, "Deinitialization of the service");
    }

    bool onStart() override
    {
        LOGI(LOG_DOMAIN, "Begin of the second initialization of the service");
        std::this_thread::sleep_for(std::chrono::seconds(2));
        LOGI(LOG_DOMAIN, "End of the second initialization of the service");
        return true;
    }
    bool onStop() override
    {
        LOGI(LOG_DOMAIN, "Begin of the destructing of the service");
        std::this_thread::sleep_for(std::chrono::seconds(1));
        LOGI(LOG_DOMAIN, "End of the destructing of the service");
        return true;
    }
    bool onConfigure() override
    {
        LOGI(LOG_DOMAIN, "Reconfiguration of the service");
        return true;
    }
    void heartbeat() noexcept override
    {
        LOGI(LOG_DOMAIN, "Begin of the heartbeated service work");
        std::this_thread::sleep_for(std::chrono::seconds(3));
        LOGI(LOG_DOMAIN, "End  of the heartbeated  service work");
    }
};

int main()
{
    SystemService service;

    return service.run(std::unique_ptr<ServiceDispatcher>(new MyService(service)));
}
