#include <fstream>
#include <cerrno>
#include <cstring>
#include <sys/stat.h>

#include "softeq/common/service.hh"
#include "softeq/common/log.hh"
#include "softeq/common/system_logger.hh"
#include "softeq/common/exceptions.hh"

namespace
{
const char *const LOG_DOMAIN = "SysVServiceTest";
}
using namespace softeq::common;

class SysVServiceTest : public ServiceDispatcher
{
public:
    explicit SysVServiceTest(SystemService &service)
        : ServiceDispatcher(service), _hbCount(0)
    {
        log().set(LoggerInterface::UPtr(new softeq::common::SystemLogger("", 0,0)));
        LOGI(LOG_DOMAIN, "SysVService initialization");
    }

    SysVServiceTest(const SysVServiceTest &) = delete;
    SysVServiceTest(SysVServiceTest &&) = delete;

    ~SysVServiceTest() override
    {
        LOGI(LOG_DOMAIN, "Deinitialization of the SysVService");
    }

    bool onStart() override
    {
        LOGI(LOG_DOMAIN, "Start of the SysVService");

        umask(S_IRWXO);
        std::string fileName{"/tmp/sv_onstart"};
        std::ofstream startFile(fileName);
        if (!startFile)
        {
            LOGE(LOG_DOMAIN, "Failed to open file: %s - %s", fileName.c_str(), std::strerror(errno));
            return false;
        }

        startFile << heartbeatPeriod() << std::endl;
        startFile.close();

        LOGI(LOG_DOMAIN, "Wrote heart beat period = %d ms to %s", heartbeatPeriod(), fileName.c_str());
        return true;
    }

    bool onStop() override
    {
        LOGI(LOG_DOMAIN, "Stop of the SysVService");

        std::string fileName{"/tmp/sv_onstop"};
        std::ofstream stopFile(fileName);
        if (!stopFile)
        {
            LOGE(LOG_DOMAIN, "Failed to open file: %s - %s", fileName.c_str(), std::strerror(errno));
            return false;
        }

        stopFile << service().uptime() << std::endl;
        stopFile.close();

        LOGI(LOG_DOMAIN, "Wrote uptime = %d to %s", service().uptime(), fileName.c_str());
        return true;
    }

    bool onConfigure() override
    {
        LOGI(LOG_DOMAIN, "Reconfiguration of the SysVService");

        std::string fileName{"/tmp/sv_onconf"};
        std::ofstream confFile(fileName);
        if (!confFile)
        {
            LOGE(LOG_DOMAIN, "Failed to open file: %s - %s", fileName.c_str(), std::strerror(errno));
            return false;
        }

        confFile << service().isRunning() << std::endl;
        confFile.close();

        LOGI(LOG_DOMAIN, "Wrote running status = %d to %s", service().isRunning(), fileName.c_str());
        return true;
    }

    void heartbeat() noexcept override
    {
        std::string fileName{"/tmp/sv_hbcount"};
        std::ofstream hbFile(fileName);
        if (!hbFile)
        {
            LOGE(LOG_DOMAIN, "Failed to open file: %s - %s", fileName.c_str(), std::strerror(errno));
            return;
        }

        hbFile << ++_hbCount << std::endl;
        hbFile.close();

        // LOGI(LOG_DOMAIN, "Wrote heart beats count = %u to %s", _hbCount, fileName.c_str());
    }

private:
    uint _hbCount;
};

int main()
{
    SysVService service;

    return service.run(std::unique_ptr<ServiceDispatcher>(new SysVServiceTest(service)));
}
