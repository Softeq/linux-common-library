#include "softeq/common/sd_bus.hh"

#include <systemd/sd-bus.h>
#include <unistd.h>

#include "softeq/common/log.hh"
#include "softeq/common/scope_guard.hh"

using namespace softeq::common;

namespace
{
const char *LOG_DOMAIN = "SdBus";

int methodGetHandler(sd_bus_message *message, void *userData, sd_bus_error *retError)
{
    const char *key = nullptr;
    int r;

    r = sd_bus_message_read(message, "s", &key);
    if (r < 0)
    {
        LOGE(LOG_DOMAIN, "Failed to parse parameters: %s", strerror(-r));
        return r;
    }
    return static_cast<SdBus *>(userData)->ipcServerInterface(message, key, SdBusTypes::kUndefined, false);
}

int methodSetHandler(sd_bus_message *message, void *userData, sd_bus_error *retError)
{
    const char *key = nullptr;
    SdBusTypes typeVal = SdBusTypes::kUndefined;
    int r;

    // Read the parameters
    r = sd_bus_message_read(message, "s", &key);
    if (r < 0)
    {
        LOGE(LOG_DOMAIN, "Failed to parse key parameter: %s", strerror(-r));
        return r;
    }
    r = sd_bus_message_read(message, "y", &typeVal);
    if (r < 0)
    {
        LOGE(LOG_DOMAIN, "Failed to parse typeVal parameter: %s", strerror(-r));
        return r;
    }
    return static_cast<SdBus *>(userData)->ipcServerInterface(message, key, typeVal, true);
}

const sd_bus_vtable rilserviceVtable[] = {
    // clang-format off
    SD_BUS_VTABLE_START(0),
	SD_BUS_METHOD("get_object", "s", "yv", methodGetHandler, SD_BUS_VTABLE_UNPRIVILEGED),
    SD_BUS_METHOD("set_object", "syv", "", methodSetHandler, SD_BUS_VTABLE_UNPRIVILEGED),
	SD_BUS_VTABLE_END
    // clang-format on
};

template <class Type>
int callSdbusMessageRead(sd_bus_message *message, const char *typeChar, softeq::common::Any &val)
{
    Type v;
    int r = sd_bus_message_read(message, "v", typeChar, &v);
    val = v;
    return r;
}

} // namespace

SdBus::SdBus(const std::string &name, const std::string &path, bool actAsClient)
    : _sdbusName(name)
    , _sdbusPath(path)
    , _isClient(actAsClient)
    , _active(true)
{
    if (!openBus())
    {
        throw std::runtime_error("Unable to open sd_bus");
    }
    if (!_isClient)
    {
        if (initServer() == EXIT_FAILURE)
        {
            throw std::runtime_error("Unable to set dbus service up");
        }
        _thread = std::thread(&SdBus::serverWorker, this);
    }
}

SdBus::~SdBus()
{
    if (!_isClient)
    {
        _active = false;
        try
        {
            if (_thread.joinable())
            {
                _thread.join();
            }
        }
        catch (const std::runtime_error &error)
        {
            LOGE(LOG_DOMAIN, "Unable to join to the serverWorker thread %s", error.what());
        }
    }
    closeBus();

    if (_slot != nullptr)
    {
        sd_bus_slot_unref(_slot);
    }
}

SdBusTypes SdBus::getAnyType(Any &val)
{
    if (val.type() == typeid(int64_t))
    {
        return SdBusTypes::kInt64;
    }
    else if (val.type() == typeid(uint64_t))
    {
        return SdBusTypes::kUInt64;
    }
    else if (val.type() == typeid(int32_t))
    {
        return SdBusTypes::kInt32;
    }
    else if (val.type() == typeid(uint32_t))
    {
        return SdBusTypes::kUInt32;
    }
    else if (val.type() == typeid(int16_t))
    {
        return SdBusTypes::kInt16;
    }
    else if (val.type() == typeid(uint16_t))
    {
        return SdBusTypes::kUInt16;
    }
    else if (val.type() == typeid(int8_t))
    {
        return SdBusTypes::kInt8;
    }
    else if (val.type() == typeid(uint8_t))
    {
        return SdBusTypes::kUInt8;
    }
    else if (val.type() == typeid(bool))
    {
        return SdBusTypes::kBool;
    }
    else if (val.type() == typeid(std::string))
    {
        return SdBusTypes::kString;
    }
    return SdBusTypes::kUndefined;
}

bool SdBus::openBus()
{
    // we can try to connect via system bus
    int r =
#ifdef USE_SYSTEMBUS
        sd_bus_open_system(&_bus);
#else
        sd_bus_open_user(&_bus);
#endif
    if (r < 0)
    {
        LOGE(LOG_DOMAIN, "sd_bus_open_system Failed connect to the System Bus: %s", strerror(-r));
        if (_bus != nullptr)
        {
            sd_bus_unref(_bus);
        }
        return false;
    }
    return true;
}

void SdBus::closeBus()
{
    if (_bus != nullptr)
    {
        sd_bus_unref(_bus);
    }
}

void SdBus::freeBusResources(sd_bus_error *error, sd_bus_message *message)
{
    sd_bus_error_free(error);
    sd_bus_message_unref(message);
}

int SdBus::initServer()
{
    int r;

    // Install the object
    LOGI(LOG_DOMAIN, "Register %s => %s", sdbusName().c_str(), sdbusPath().c_str());
    r = sd_bus_add_object_vtable(_bus, &_slot, sdbusPath().c_str(), sdbusName().c_str(), rilserviceVtable, nullptr);
    if (r < 0)
    {
        LOGE(LOG_DOMAIN, "Failed to issue method call: %s", strerror(-r));
        return EXIT_FAILURE;
    }
    sd_bus_slot_set_userdata(_slot, static_cast<void *>(this));

    // Take a well-known service name so that clients can find us
    r = sd_bus_request_name(_bus, sdbusName().c_str(), 0);
    if (r < 0)
    {
        LOGE(LOG_DOMAIN, "Failed to acquire service name: %s", strerror(-r));
        return EXIT_FAILURE;
    }

    return r < 0 ? EXIT_FAILURE : EXIT_SUCCESS;
}

int SdBus::serverWorker()
{
    int r;

    do
    {
        // process requests
        r = sd_bus_process(_bus, NULL);
        if (r < 0)
        {
            LOGE(LOG_DOMAIN, "Failed to process bus: %s", strerror(-r));
            return EXIT_FAILURE;
        }
        // we processed a request, try to process another one, right-away
        if (r > 0)
        {
            continue;
        }

        do
        {
            // wait for the next request to process
            r = sd_bus_wait(_bus, (uint64_t)1000000);
        } while (r == 0 && _active);

        if (r < 0)
        {
            LOGE(LOG_DOMAIN, "Failed to wait on bus: %s", strerror(-r));
            return EXIT_FAILURE;
        }
    } while (_active);
    return r < 0 ? EXIT_FAILURE : EXIT_SUCCESS;
}

bool SdBus::propertiesManipulations(const std::string &key, Any &val, bool mode)
{
    std::unique_lock<std::mutex> lock(_mutex);
    try
    {
        auto it = _properties.find(key);
        if (it != _properties.end())
        {
            if (mode)
            {
                Any oldValue = it->second;
                it->second = val;
                onPropertyChanged(key, oldValue, val);
            }
            else
            {
                val = it->second;
            }
            return true;
        }
        else
        {
            if (!mode)
            {
                return false;
            }
            _properties.emplace(key, val);
        }
    }
    catch (std::runtime_error &error)
    {
        // TODO error handler
        return false;
    }
    return true;
}

bool SdBus::setProperty(const std::string &key, Any &val) noexcept
{
    scope_guard guard;
    sd_bus_error error = SD_BUS_ERROR_NULL;
    sd_bus_error *errorPtr = &error;
    sd_bus_message *m = nullptr;
    SdBusTypes typeVal;
    int r = 0;

    if (!_isClient)
    {
        propertiesManipulations(key, val, true);
    }
    else
    {
        guard += [=] { freeBusResources(errorPtr, m); };

        // TODO check if bus is opened

        typeVal = SdBus::getAnyType(val);
        switch (typeVal)
        {
        case SdBusTypes::kInt64:
            r = sd_bus_call_method(_bus, sdbusName().c_str(), sdbusPath().c_str(), sdbusName().c_str(), "set_object",
                                   &error, &m, "syv", key.c_str(), typeVal, "x", any_cast<int64_t>(val));
            break;
        case SdBusTypes::kUInt64:
            r = sd_bus_call_method(_bus, sdbusName().c_str(), sdbusPath().c_str(), sdbusName().c_str(), "set_object",
                                   &error, &m, "syv", key.c_str(), typeVal, "t", any_cast<uint64_t>(val));
            break;
        case SdBusTypes::kInt32:
            r = sd_bus_call_method(_bus, sdbusName().c_str(), sdbusPath().c_str(), sdbusName().c_str(), "set_object",
                                   &error, &m, "syv", key.c_str(), typeVal, "i", any_cast<int32_t>(val));
            break;
        case SdBusTypes::kUInt32:
            r = sd_bus_call_method(_bus, sdbusName().c_str(), sdbusPath().c_str(), sdbusName().c_str(), "set_object",
                                   &error, &m, "syv", key.c_str(), typeVal, "u", any_cast<uint32_t>(val));
            break;
        case SdBusTypes::kInt16:
            r = sd_bus_call_method(_bus, sdbusName().c_str(), sdbusPath().c_str(), sdbusName().c_str(), "set_object",
                                   &error, &m, "syv", key.c_str(), typeVal, "n", any_cast<int16_t>(val));
            break;
        case SdBusTypes::kUInt16:
            r = sd_bus_call_method(_bus, sdbusName().c_str(), sdbusPath().c_str(), sdbusName().c_str(), "set_object",
                                   &error, &m, "syv", key.c_str(), typeVal, "q", any_cast<uint16_t>(val));
            break;
        case SdBusTypes::kInt8:
            r = sd_bus_call_method(_bus, sdbusName().c_str(), sdbusPath().c_str(), sdbusName().c_str(), "set_object",
                                   &error, &m, "syv", key.c_str(), typeVal, "y", any_cast<int8_t>(val));
            break;
        case SdBusTypes::kUInt8:
            r = sd_bus_call_method(_bus, sdbusName().c_str(), sdbusPath().c_str(), sdbusName().c_str(), "set_object",
                                   &error, &m, "syv", key.c_str(), typeVal, "y", any_cast<uint8_t>(val));
            break;
        case SdBusTypes::kBool:
            r = sd_bus_call_method(_bus, sdbusName().c_str(), sdbusPath().c_str(), sdbusName().c_str(), "set_object",
                                   &error, &m, "syv", key.c_str(), typeVal, "b", any_cast<bool>(val));
            break;
        case SdBusTypes::kString:
            r = sd_bus_call_method(_bus, sdbusName().c_str(), sdbusPath().c_str(), sdbusName().c_str(), "set_object",
                                   &error, &m, "syv", key.c_str(), typeVal, "s", any_cast<std::string &>(val).c_str());
            break;
        default:
            LOGE(LOG_DOMAIN, "Unsupported type was detected in a %s call", __func__);
            r = -EINVAL;
            break;
        }
        if (r < 0)
        {
            LOGE(LOG_DOMAIN, "Failed to issue sd_bus_call_method, error: %s", error.message);
        }
    }
    return r >= 0;
}

Any SdBus::property(const std::string &key) noexcept
{
    scope_guard guard;
    sd_bus_error error = SD_BUS_ERROR_NULL;
    sd_bus_error *errorPtr = &error;
    sd_bus_message *m = nullptr;
    SdBusTypes typeVal = SdBusTypes::kUndefined;
    Any val;

    guard += [=] { freeBusResources(errorPtr, m); };

    // TODO check if bus is opened

    int r = sd_bus_call_method(_bus, sdbusName().c_str(), sdbusPath().c_str(), sdbusName().c_str(), "get_object",
                               errorPtr, &m, "s", key.c_str());
    if (r < 0)
    {
        LOGE(LOG_DOMAIN, "Failed to issue method call: %s", error.message);
        // just return empty
        return val;
    }
    r = sd_bus_message_read(m, "y", &typeVal);
    if (r < 0)
    {
        LOGE(LOG_DOMAIN, "Failed to parse typeVal response message: %s", strerror(-r));
        // just return empty
        return val;
    }
    switch (typeVal)
    {
    case SdBusTypes::kInt64: // int64
        r = callSdbusMessageRead<int64_t>(m, "x", val);
        break;
    case SdBusTypes::kUInt64: // uint64
        r = callSdbusMessageRead<uint64_t>(m, "t", val);
        break;
    case SdBusTypes::kInt32: // int32
        r = callSdbusMessageRead<int32_t>(m, "i", val);
        break;
    case SdBusTypes::kUInt32: // uint32
        r = callSdbusMessageRead<uint32_t>(m, "u", val);
        break;
    case SdBusTypes::kInt16: // int16
        r = callSdbusMessageRead<int16_t>(m, "n", val);
        break;
    case SdBusTypes::kUInt16: // uint16
        r = callSdbusMessageRead<uint16_t>(m, "q", val);
        break;
    case SdBusTypes::kInt8: // int8
        r = callSdbusMessageRead<int8_t>(m, "y", val);
        break;
    case SdBusTypes::kUInt8: // uint8
        r = callSdbusMessageRead<uint8_t>(m, "y", val);
        break;
    case SdBusTypes::kBool: // boolean
        r = callSdbusMessageRead<bool>(m, "b", val);
        break;
    case SdBusTypes::kString: // string
    {
        const char *v;
        r = sd_bus_message_read(m, "v", "s", &v);
        val = v;
        break;
    }
    default:
        LOGE(LOG_DOMAIN, "Unsupported type was detected in a %s call", __func__);
        break;
    }
    if (r < 0)
    {
        LOGE(LOG_DOMAIN, "Failed to issue sd_bus_message_read, error: %s", error.message);
    }
    return val;
}

int SdBus::ipcServerInterface(sd_bus_message *msg, const char *key, SdBusTypes tval, bool isSetOperation)
{
    Any val;
    int r = 0;
    if (isSetOperation)
    {
        switch (tval)
        {
        case SdBusTypes::kInt64:
            r = callSdbusMessageRead<int64_t>(msg, "x", val);
            break;
        case SdBusTypes::kUInt64:
            r = callSdbusMessageRead<uint64_t>(msg, "t", val);
            break;
        case SdBusTypes::kInt32:
            r = callSdbusMessageRead<int32_t>(msg, "i", val);
            break;
        case SdBusTypes::kUInt32:
            r = callSdbusMessageRead<uint32_t>(msg, "u", val);
            break;
        case SdBusTypes::kInt16:
            r = callSdbusMessageRead<int16_t>(msg, "n", val);
            break;
        case SdBusTypes::kUInt16:
            r = callSdbusMessageRead<uint16_t>(msg, "q", val);
            break;
        case SdBusTypes::kInt8:
            r = callSdbusMessageRead<int8_t>(msg, "y", val);
            break;
        case SdBusTypes::kUInt8:
            r = callSdbusMessageRead<uint8_t>(msg, "y", val);
            break;
        case SdBusTypes::kBool:
            r = callSdbusMessageRead<bool>(msg, "b", val);
            break;
        case SdBusTypes::kString:
            r = callSdbusMessageRead<const char *>(msg, "s", val);
            break;
        default:
            break;
        }
        if (r < 0)
        {
            LOGE(LOG_DOMAIN, "Failed to parse typeVal parameter: %s", strerror(-r));
        }
        else
        {
            propertiesManipulations(key, val, true);
        }
        return sd_bus_reply_method_return(msg, "");
    }
    else
    {
        if (propertiesManipulations(key, val, false))
        {
            if (val.hasValue())
            {
                SdBusTypes typeVal = getAnyType(val);
                switch (typeVal)
                {
                case SdBusTypes::kInt64:
                    return sd_bus_reply_method_return(msg, "yv", typeVal, "x", any_cast<int64_t>(val));
                case SdBusTypes::kUInt64:
                    return sd_bus_reply_method_return(msg, "yv", typeVal, "t", any_cast<uint64_t>(val));
                case SdBusTypes::kInt32:
                    return sd_bus_reply_method_return(msg, "yv", typeVal, "i", any_cast<int32_t>(val));
                case SdBusTypes::kUInt32:
                    return sd_bus_reply_method_return(msg, "yv", typeVal, "u", any_cast<uint32_t>(val));
                case SdBusTypes::kInt16:
                    return sd_bus_reply_method_return(msg, "yv", typeVal, "n", any_cast<int16_t>(val));
                case SdBusTypes::kUInt16:
                    return sd_bus_reply_method_return(msg, "yv", typeVal, "q", any_cast<uint16_t>(val));
                case SdBusTypes::kInt8:
                    return sd_bus_reply_method_return(msg, "yv", typeVal, "y", any_cast<int8_t>(val));
                case SdBusTypes::kUInt8:
                    return sd_bus_reply_method_return(msg, "yv", typeVal, "y", any_cast<uint8_t>(val));
                case SdBusTypes::kBool:
                    return sd_bus_reply_method_return(msg, "yv", typeVal, "b", any_cast<bool>(val));
                case SdBusTypes::kString:
                    return sd_bus_reply_method_return(msg, "yv", typeVal, "s", any_cast<std::string &>(val).c_str());
                default:
                    break;
                }
            }
        }
        // property is not present then return type kUndefined
        return sd_bus_reply_method_return(msg, "yv", SdBusTypes::kUndefined, "s", "");
    }
}
