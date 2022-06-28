#include <fstream>
#include <vector>

#include <gtest/gtest.h>

#include "softeq/common/serialization/xml_serializer.hh"
#include <softeq/common/settings.hh>

using namespace softeq::common;

// Singleton tests
struct TestSettings
{
    int i;
    bool b;
};

template <>
serialization::ObjectAssembler<TestSettings> serialization::Assembler()
{
    // clang-format off
    return ObjectAssembler<TestSettings>()
        .define("i", &TestSettings::i)
        .define("b", &TestSettings::b)
        ;
    // clang-format on
}

struct TestObject
{
    TestObject()
    {
        s.b = false;
    }
    struct TestSettings
    {
        bool b;
    } s;
};

template <>
serialization::ObjectAssembler<TestObject> serialization::Assembler()
{
    // clang-format off
    return ObjectAssembler<TestObject>()
        .define("s", &TestObject::s)
        ;
    // clang-format on
}

template <>
serialization::ObjectAssembler<TestObject::TestSettings> serialization::Assembler()
{
    // clang-format off
    return ObjectAssembler<TestObject::TestSettings>()
        .define("b", &TestObject::TestSettings::b)
        ;
    // clang-format on
}

namespace softeq
{
namespace common
{
STATIC_DECLARE_SETTING(TestSettings, "Static");
}
} // namespace softeq

TEST(Settings, Singleton)
{
    EXPECT_NO_THROW(Settings::instance().access<TestSettings>());
    EXPECT_THROW(Settings::instance().access<TestObject>(), std::logic_error);
    EXPECT_NO_THROW(Settings::instance().undeclare("Static"));
    EXPECT_THROW(Settings::instance().access<TestSettings>(), std::logic_error);
}

// Do not use sigleton for tests any more, use StubSettings
class StubSettings : public Settings
{
};

TEST(Settings, Basic)
{
    StubSettings settings;
    TestSettings test_settings_creation = {42, true};
    settings.declare(test_settings_creation,
                     std::string("_") + stdutils::demangle(typeid(test_settings_creation).name()));

    int &i1 = settings.access<TestSettings>().i;
    bool &b1 = settings.access<TestSettings>().b;
    EXPECT_EQ(i1, test_settings_creation.i);
    EXPECT_EQ(b1, test_settings_creation.b);

    i1++;
    b1 = !b1;
    int &i2 = settings.access<TestSettings>().i;
    bool &b2 = settings.access<TestSettings>().b;
    EXPECT_EQ(i2, i1);
    EXPECT_EQ(b2, b1);
}

TEST(Settings, BasicTypes)
{
    StubSettings settings;
    int res;
    EXPECT_NO_THROW(settings.declare<int>("int"));
    EXPECT_NO_THROW(settings.access<int>() = 100500);
    EXPECT_NO_THROW(res = settings.access<int>());
    EXPECT_EQ(res, 100500);
    EXPECT_NO_THROW(settings.undeclare("int"));
    EXPECT_THROW(settings.access<int>(), std::logic_error);
}

TEST(Settings, Inner)
{
    TestObject object;
    StubSettings settings;

    settings.declare(object.s, std::string("_") + stdutils::demangle(typeid(object.s).name()));
    settings.access<TestObject::TestSettings>().b = true;

    try
    {
        settings.access<TestObject>();
        FAIL() << "The call should throw an error\n";
    }
    catch (std::logic_error &exception)
    {
        EXPECT_EQ(std::string(exception.what()), "setting 'TestObject' is not found");
    }
    catch (std::exception &exception)
    {
        FAIL() << "Was expecting logic_error: " << exception.what() << std::endl;
    }
    catch (...)
    {
        FAIL() << "ERROR: Unexpected exception thrown: " << std::current_exception << std::endl;
    }

    bool &b = settings.access<TestObject::TestSettings>().b;
    EXPECT_EQ(b, true);
    b = false;
    EXPECT_EQ(settings.access<TestObject::TestSettings>().b, false);
}

struct SimpleSetting
{
    int i;
};

template <>
serialization::ObjectAssembler<SimpleSetting> serialization::Assembler()
{
    // clang-format off
    return ObjectAssembler<SimpleSetting>()
        .define("i", &SimpleSetting::i)
        ;
    // clang-format on
}

struct VectorTestSettings
{
    std::vector<SimpleSetting> container;
};

template <>
serialization::ObjectAssembler<VectorTestSettings> serialization::Assembler()
{
    // clang-format off
    return ObjectAssembler<VectorTestSettings>()
        .define("container", &VectorTestSettings::container)
        ;
    // clang-format on
}

TEST(Settings, VectorOfStructs)
{
    StubSettings settings;
    VectorTestSettings test_settings_creation;
    test_settings_creation.container.push_back({42});
    test_settings_creation.container.push_back({43});
    test_settings_creation.container.push_back({44});

    settings.declare(test_settings_creation,
                     std::string("_") + stdutils::demangle(typeid(test_settings_creation).name()));

    std::vector<SimpleSetting> &test_vector1 = settings.access<VectorTestSettings>().container;
    EXPECT_EQ(test_vector1.size(), test_settings_creation.container.size());
    EXPECT_EQ(test_vector1.at(0).i, test_settings_creation.container.at(0).i);
    EXPECT_EQ(test_vector1.at(1).i, test_settings_creation.container.at(1).i);
    EXPECT_EQ(test_vector1.at(2).i, test_settings_creation.container.at(2).i);

    test_vector1.push_back({45});
    for (auto &it : test_vector1)
    {
        it.i++;
    }

    std::vector<SimpleSetting> &test_vector2 = settings.access<VectorTestSettings>().container;
    EXPECT_EQ(test_vector2.size(), 4);
    EXPECT_EQ(test_vector2.at(0).i, test_vector1.at(0).i);
    EXPECT_EQ(test_vector2.at(1).i, test_vector1.at(1).i);
    EXPECT_EQ(test_vector2.at(2).i, test_vector1.at(2).i);
    EXPECT_EQ(test_vector2.at(3).i, test_vector1.at(3).i);
}

void serialization(const std::string &filePath, Settings::SerializationType type, TestSettings &test_settings_creation)
{
    StubSettings settings;
    settings.declare(test_settings_creation,
                     std::string("_") + stdutils::demangle(typeid(test_settings_creation).name()));

    settings.serialize(filePath, type);
    settings.access<TestSettings>().i = test_settings_creation.i + 1;
    settings.access<TestSettings>().b = !test_settings_creation.b;
    settings.deserialize(filePath);

    EXPECT_EQ(settings.access<TestSettings>().i, test_settings_creation.i);
    EXPECT_EQ(settings.access<TestSettings>().b, test_settings_creation.b);
}

TEST(Settings, NotExistingConfig)
{
    char path[] = "/tmp/settings_test_XXXXXX";
    if (mkdtemp(path) == nullptr)
    {
        FAIL() << "cannot create temporary directory";
        return;
    }
    std::string filePath = std::string(path) + "/serialized_object.tmp";
    StubSettings stub1;
    EXPECT_NO_THROW(stub1.declare<TestSettings>("TestSettings"));
    stub1.access<TestSettings>().i = 7;
    stub1.access<TestSettings>().b = true;
    EXPECT_EQ(stub1.access<TestSettings>().i, 7);
    EXPECT_EQ(stub1.access<TestSettings>().b, true);
    stub1.setDefaultSerializationParameters(filePath, Settings::SerializationType::Xml);
    stub1.access<TestSettings>().i = 2;
    stub1.access<TestSettings>().b = false;
    EXPECT_NO_THROW(stub1.deserialize());

    EXPECT_EQ(stub1.access<TestSettings>().i, 2);
    EXPECT_EQ(stub1.access<TestSettings>().b, false);

    serialization::XmlSerializer::cleanup();
}

TEST(Settings, Serialization)
{
    char path[] = "/tmp/settings_test_XXXXXX";
    if (mkdtemp(path) == nullptr)
    {
        FAIL() << "cannot create temporary directory";
        return;
    }
    std::string filePath = std::string(path) + "/serialized_object.tmp";

    TestSettings test_settings_creation = {42, true};
    ::serialization(filePath, Settings::SerializationType::Json, test_settings_creation);
    ::serialization(filePath, Settings::SerializationType::Xml, test_settings_creation);

    // Test deserialization at declaration
    StubSettings stub1;
    std::ofstream fileStream(filePath);
    fileStream << "{\"TestSettings\": {\"i\": 7, \"b\": true}, \"SimpleSetting\": {\"i\": 8}}";
    fileStream.close();
    stub1.setDefaultSerializationParameters(filePath, Settings::SerializationType::Json);
    stub1.declare<TestSettings>("TestSettings");
    EXPECT_EQ(stub1.access<TestSettings>().i, 7);
    EXPECT_EQ(stub1.access<TestSettings>().b, true);
    stub1.declare<SimpleSetting>("SimpleSetting");
    EXPECT_EQ(stub1.access<SimpleSetting>().i, 8);

    // Test one specific structure deserialization at declaration
    StubSettings stub2;
    stub2.declare<SimpleSetting>("SimpleSetting");
    stub2.access<SimpleSetting>().i = -1;
    stub2.setDefaultSerializationParameters(filePath, Settings::SerializationType::Json);
    stub2.declare<TestSettings>("TestSettings");
    EXPECT_EQ(stub2.access<TestSettings>().i, 7);
    EXPECT_EQ(stub2.access<TestSettings>().b, true);
    EXPECT_EQ(stub2.access<SimpleSetting>().i, -1);

    // Test if no structure found to deserialize at declaration
    StubSettings stub3;
    stub3.setDefaultSerializationParameters(filePath, Settings::SerializationType::Json);
    stub3.declare(test_settings_creation, "NotInFileTestSettings");
    EXPECT_EQ(stub3.access<TestSettings>().i, test_settings_creation.i);
    EXPECT_EQ(stub3.access<TestSettings>().b, test_settings_creation.b);

    serialization::XmlSerializer::cleanup();
}
