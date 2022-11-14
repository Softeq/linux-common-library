#ifndef SOFTEQ_COMMON_SD_BUS_H_
#define SOFTEQ_COMMON_SD_BUS_H_

#include <common/stdutils/any.hh>

#include <string>
#include <thread>
#include <mutex>
#include <map>
#include <atomic>

#include <systemd/sd-bus.h>

namespace softeq
{
namespace common
{
namespace system
{

enum class SdBusTypes
{
    kInt64,
    kUInt64,
    kInt32,
    kUInt32,
    kInt16,
    kUInt16,
    kInt8,
    kUInt8,
    kBool,
    kString,
    kUndefined
};

/*!
  \brief The class is the wrapper for SD-Bus <https://www.freedesktop.org/software/systemd/man/sd-bus.html>
*/
class SdBus
{
    SdBus(const SdBus &) = delete;
    SdBus &operator=(const SdBus &) = delete;
    SdBus(SdBus &&) = delete;
    SdBus &operator=(SdBus &&) = delete;

public:
    /*!
      SdBus Constructor
      true if the service construct to work in client mode else false
      throws an exception if connect via system bus was unsuccessful
     */
    explicit SdBus(const std::string &name, const std::string &path, bool actAsClient);
    virtual ~SdBus();

    /*!
       Callback if property has been changed
       \param[in] name Property name
       \param[in] oldValue Previous property value
       \param[in] newValue new property value
    */
    virtual void onPropertyChanged(const std::string &name, const common::stdutils::Any &oldValue, const common::stdutils::Any &newValue)
    {
        (void)name;
        (void)oldValue;
        (void)newValue;
    }

    /*!
       Returns the name of the sdbus object
       \return String value of  sdbus object
    */
    const std::string &sdbusName()
    {
        return _sdbusName;
    }

    /*!
       Returns the path to the sdbus object
       \return String path value
    */
    const std::string &sdbusPath()
    {
        return _sdbusPath;
    }

    /*!
       Client call method to get property
       \param[in] key Key of property
       \return Any object
     */
    common::stdutils::Any property(const std::string &key) noexcept;

    /*!
       Client call for set property
       \param[in] key Key of property
       \param[in] val Any value of property
       \return Bool execution result
     */
    bool setProperty(const std::string &key, common::stdutils::Any &val) noexcept;

    /*!
       IPC server interface for work with properties via sdbus callbacks
       \param[in] m sd_bus_message Message
       \param[in] key Key of property
       \param[in] typeVal SDBus type
       \param[in] isSetOperation bool value
       \return int execution result
     */
    int ipcServerInterface(sd_bus_message *m, const char *key, SdBusTypes typeVal, bool isSetOperation);

private:
    bool openBus();

    void closeBus();

    void freeBusResources(sd_bus_error *error, sd_bus_message *message);

    // get number for specialized type
    static SdBusTypes getAnyType(common::stdutils::Any &anyVal);

    // Server part of class
    int initServer();
    int serverWorker();

    // just get or set a property
    bool propertiesManipulations(const std::string &key, common::stdutils::Any &val, bool isSet);

    const std::string _sdbusName;
    const std::string _sdbusPath;

    // Server part of class
    std::mutex _mutex;

    bool _isClient;
    std::atomic<bool> _active;

    std::map<std::string, common::stdutils::Any> _properties;
    std::thread _thread;

    sd_bus *_bus = nullptr;
    sd_bus_slot *_slot = nullptr;
};

} // namespace system
} // namespace common
} // namespace softeq

#endif // SOFTEQ_COMMON_SD_BUS_H_
