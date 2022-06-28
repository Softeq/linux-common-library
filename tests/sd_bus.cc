#include <gtest/gtest.h>
#include <string>
#include <softeq/common/sd_bus.hh>

using namespace softeq::common;

template <typename Type>
void setCheckProperties(SdBus &server, SdBus &client, Type property)
{
    Any val = property;
    std::string keyName{typeid(Type).name()};
    server.setProperty(keyName, val);
    EXPECT_EQ(any_cast<Type>(client.property(keyName)), any_cast<Type>(val));
}

template <typename Type>
void resetCheckProperties(SdBus &server, SdBus &client, Type property)
{
    Any val = property;
    Any init = 0;
    std::string keyName{typeid(Type).name()};
    server.setProperty(keyName, init);
    client.setProperty(keyName, val);
    EXPECT_EQ(any_cast<Type>(client.property(keyName)), any_cast<Type>(val));
}

TEST(Sdbus, Ctor)
{
    const std::string name{"net.Sdbus.Moduletest"};
    const std::string path{"/net/Sdbus/Moduletest"};

    SdBus server{name, path, false};
    SdBus client{name, path, true};

    EXPECT_EQ(client.sdbusName(), name);
    EXPECT_EQ(client.sdbusPath(), path);
    EXPECT_EQ(server.sdbusName(), name);
    EXPECT_EQ(server.sdbusPath(), path);
}

TEST(Sdbus, ReadProperties)
{
    const std::string name{"net.Sdbus.Moduletest"};
    const std::string path{"/net/Sdbus/Moduletest"};

    SdBus server{name, path, false};
    SdBus client{name, path, true};

    setCheckProperties<uint8_t>(server, client, 1);
    setCheckProperties<uint16_t>(server, client, 1);
    setCheckProperties<uint32_t>(server, client, 1);
    setCheckProperties<uint64_t>(server, client, 1);
    setCheckProperties<int8_t>(server, client, 1);
    setCheckProperties<int16_t>(server, client, 1);
    setCheckProperties<int32_t>(server, client, 1);
    setCheckProperties<int64_t>(server, client, 1);
    setCheckProperties<bool>(server, client, true);
    setCheckProperties<bool>(server, client, false);
    setCheckProperties<std::string>(server, client, "str");
    setCheckProperties<std::string>(server, client, "");
}

TEST(Sdbus, UpdateProperties)
{
    const std::string name{"net.Sdbus.Moduletest"};
    const std::string path{"/net/Sdbus/Moduletest"};

    SdBus server{name, path, false};
    SdBus client{name, path, true};

    resetCheckProperties<uint8_t>(server, client, 1);
    resetCheckProperties<uint16_t>(server, client, 1);
    resetCheckProperties<uint32_t>(server, client, 1);
    resetCheckProperties<uint64_t>(server, client, 1);
    resetCheckProperties<int8_t>(server, client, 1);
    resetCheckProperties<int16_t>(server, client, 1);
    resetCheckProperties<int32_t>(server, client, 1);
    resetCheckProperties<int64_t>(server, client, 1);
    resetCheckProperties<bool>(server, client, true);
    resetCheckProperties<bool>(server, client, false);
    resetCheckProperties<std::string>(server, client, "str");
    resetCheckProperties<std::string>(server, client, "");
}

TEST(Sdbus, ServerCtorWrongPath)
{
    const std::string name{"net.Sdbus.Moduletest"};
    const std::string path{"-net-Sdbus-Moduletest"};

    try
    {
        SdBus server{name, path, false};
        FAIL() << "Expected std::runtime_error";
    }
    catch (const std::runtime_error &e)
    {
        EXPECT_EQ(e.what(), std::string("Unable to set dbus service up"));
    }
}

TEST(Sdbus, DoubleServerSetup)
{
    const std::string name{"net.Sdbus.Moduletest"};
    const std::string path{"/net/Sdbus/Moduletest"};

    try
    {
        SdBus server_1{name, path, false};
        SdBus server_2{name, path, false};
        FAIL() << "Expected std::runtime_error";
    }
    catch (const std::runtime_error &e)
    {
        EXPECT_STREQ(e.what(), "Unable to set dbus service up");
    }
}

TEST(Sdbus, SetUnsupportedTypeProperty)
{
    const std::string name{"net.Sdbus.Moduletest"};
    const std::string path{"/net/Sdbus/Moduletest"};

    SdBus server{name, path, false};
    SdBus client{name, path, true};

    Any sVal = (float)0;
    Any cVal = (float)1;
    std::string keyName{"unsupported_type_property"};

    EXPECT_TRUE(server.setProperty(keyName, sVal));
    EXPECT_FALSE(client.setProperty(keyName, cVal));
}

class SdbusDerived : public SdBus
{

public:
    std::string callbackPropertyName;
    Any callbackOldValue;
    Any callbackNewValue;
    uint callbackCounter;
    SdbusDerived(const std::string &name, const std::string &path, bool actAsClient)
                            : SdBus{name, path, actAsClient}, callbackCounter{0}{};
    virtual void onPropertyChanged(const std::string &name, const Any &oldValue, const Any &newValue)
    {
        callbackPropertyName = name;
        callbackOldValue = oldValue;
        callbackNewValue = newValue;
        callbackCounter++;
    }
};

TEST(Sdbus, Callback)
{
    const std::string name{"net.Sdbus.Moduletest"};
    const std::string path{"/net/Sdbus/Moduletest"};

    SdbusDerived server{name, path, false};
    SdBus client{name, path, true};

    Any sVal = (uint32_t)0;
    Any cVal = (uint32_t)1;
    std::string keyName{"dummy_property"};

    EXPECT_TRUE(server.setProperty(keyName, sVal));
    EXPECT_TRUE(client.setProperty(keyName, cVal));

    EXPECT_EQ(server.callbackPropertyName, keyName);
    EXPECT_EQ(any_cast<uint32_t>(server.callbackOldValue), any_cast<uint32_t>(sVal));
    EXPECT_EQ(any_cast<uint32_t>(server.callbackNewValue), any_cast<uint32_t>(cVal));
    EXPECT_EQ(server.callbackCounter, 1);
}
