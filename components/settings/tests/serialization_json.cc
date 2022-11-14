#include <gtest/gtest.h>

#include "structs.hh"
#include <common/serialization/json/json.hh>

#include <fstream>
#include <vector>

using namespace softeq::common::settings;
using namespace softeq::common::serialization;

namespace softeq
{
namespace common
{
namespace settings
{
STATIC_DECLARE_SETTING(TestSettings, "Static");
STATIC_DECLARE_SETTING(Object, "Object");
}
} // namespace common
} // namespace softeq

class SettingsFromFile : public ::testing::Test
{
protected:
    void SetUp() override {
        char path[] = "/tmp/settings_test_XXXXXX";
        if (mkdtemp(path) == nullptr)
        {
            FAIL() << "cannot create temporary directory";
            return;
        }
        _filePath = std::string(path) + "/serialized_object.tmp";
    }
    std::string _filePath;
};

void serialization(
        const std::string &filePath,
        StructSerializer &serializer,
        StructDeserializer &deserializer,
        TestSettings &testSettingsCreation)
{
    StubSettings settings;
    settings.declare(testSettingsCreation,
                     std::string("_") + softeq::common::stdutils::demangle(typeid(testSettingsCreation).name()));
    settings.serialize(filePath, serializer);

    settings.access<TestSettings>().i = testSettingsCreation.i + 1;
    settings.access<TestSettings>().b = !testSettingsCreation.b;

    settings.deserialize(filePath, deserializer);

    EXPECT_EQ(settings.access<TestSettings>().i, testSettingsCreation.i);
    EXPECT_EQ(settings.access<TestSettings>().b, testSettingsCreation.b);
}

TEST_F(SettingsFromFile, EmptyConfig)
{
    StubSettings stub1;
    std::ofstream fileStream(_filePath);
    EXPECT_NO_THROW(stub1.declare<TestSettings>("TestSettings"));
    stub1.access<TestSettings>().i = 7;
    stub1.access<TestSettings>().b = true;
    EXPECT_EQ(stub1.access<TestSettings>().i, 7);
    EXPECT_EQ(stub1.access<TestSettings>().b, true);
    std::unique_ptr<StructDeserializer> jsonDeserializer1(json::createStructDeserializer());
    EXPECT_THROW(stub1.deserialize(_filePath, *jsonDeserializer1), std::logic_error);
}

TEST_F(SettingsFromFile, Serialization)
{
    TestSettings testSettingsCreation = {42, true};
    std::unique_ptr<StructSerializer> jsonSerializer1(json::createStructSerializer());
    std::unique_ptr<StructDeserializer> jsonDeserializer1(json::createStructDeserializer());
    ::serialization(_filePath, *jsonSerializer1, *jsonDeserializer1, testSettingsCreation);

    // Test deserialization at declaration
    StubSettings stub1;
    std::ofstream fileStream(_filePath);
    fileStream << "{\"TestSettings\": {\"i\": 7, \"b\": true}, \"SimpleSetting\": {\"i\": 8}}";
    fileStream.close();
    std::unique_ptr<StructDeserializer> jsonDeserializer2(json::createStructDeserializer());
    stub1.setDefaultParameters(_filePath, std::move(jsonDeserializer2));
    stub1.declare<TestSettings>("TestSettings");
    EXPECT_EQ(stub1.access<TestSettings>().i, 7);
    EXPECT_EQ(stub1.access<TestSettings>().b, true);
    stub1.declare<SimpleSetting>("SimpleSetting");
    EXPECT_EQ(stub1.access<SimpleSetting>().i, 8);

    // Test one specific structure deserialization at declaration
    StubSettings stub2;
    stub2.declare<SimpleSetting>("SimpleSetting");
    stub2.access<SimpleSetting>().i = -1;
    std::unique_ptr<StructDeserializer> jsonDeserializer3(json::createStructDeserializer());
    stub2.setDefaultParameters(_filePath, std::move(jsonDeserializer3));
    stub2.declare<TestSettings>("TestSettings");
    EXPECT_EQ(stub2.access<TestSettings>().i, 7);
    EXPECT_EQ(stub2.access<TestSettings>().b, true);
    EXPECT_EQ(stub2.access<SimpleSetting>().i, -1);

    // Test if no structure found to deserialize at declaration
    StubSettings stub3;
    std::unique_ptr<StructDeserializer> jsonDeserializer4(json::createStructDeserializer());
    stub3.setDefaultParameters(_filePath, std::move(jsonDeserializer4));
    stub3.declare(testSettingsCreation, "NotInFileTestSettings");
    EXPECT_EQ(stub3.access<TestSettings>().i, testSettingsCreation.i);
    EXPECT_EQ(stub3.access<TestSettings>().b, testSettingsCreation.b);
}

TEST_F(SettingsFromFile, DefaultParameters)
{
    TestSettings testSettings = {42, true};
    std::unique_ptr<StructSerializer> jsonSerializer(json::createStructSerializer());
    std::unique_ptr<StructDeserializer> jsonDeserializer1(json::createStructDeserializer());
    StubSettings stubSettings;
    stubSettings.declare(testSettings, "TestSettings");
    stubSettings.setDefaultParameters(_filePath, std::move(jsonDeserializer1), std::move(jsonSerializer));
    stubSettings.serialize();
    stubSettings.access<TestSettings>().i = testSettings.i + 1;
    stubSettings.access<TestSettings>().b = !testSettings.b;
    stubSettings.deserialize();
    EXPECT_EQ(stubSettings.access<TestSettings>().i, testSettings.i);
    EXPECT_EQ(stubSettings.access<TestSettings>().b, testSettings.b);

    stubSettings.setDefaultParameters("", nullptr, nullptr);
    EXPECT_THROW(stubSettings.serialize(), std::invalid_argument);
    EXPECT_THROW(stubSettings.deserialize(), std::invalid_argument);

    stubSettings.setDefaultParameters(_filePath, nullptr, nullptr);
    EXPECT_THROW(stubSettings.serialize(), std::invalid_argument);
    EXPECT_THROW(stubSettings.deserialize(), std::invalid_argument);

    std::unique_ptr<StructDeserializer> jsonDeserializer2(json::createStructDeserializer());
    stubSettings.setDefaultParameters(_filePath, std::move(jsonDeserializer1));
    EXPECT_THROW(stubSettings.serialize(), std::invalid_argument);
}
