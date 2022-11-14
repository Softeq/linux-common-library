#include <gtest/gtest.h>

#include "structs.hh"

#include <fstream>
#include <vector>

using namespace softeq::common::settings;

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

TEST(Settings, Singleton)
{
    EXPECT_NO_THROW(Settings::instance().access<TestSettings>());
    EXPECT_THROW(Settings::instance().access<TestObject>(), std::logic_error);
    EXPECT_NO_THROW(Settings::instance().undeclare("Static"));
    EXPECT_THROW(Settings::instance().access<TestSettings>(), std::logic_error);
}

TEST(Settings, Basic)
{
    StubSettings settings;
    TestSettings testSettingsCreation = {42, true};
    settings.declare(testSettingsCreation,
                     std::string("_") + softeq::common::stdutils::demangle(typeid(testSettingsCreation).name()));

    int &i1 = settings.access<TestSettings>().i;
    bool &b1 = settings.access<TestSettings>().b;
    EXPECT_EQ(i1, testSettingsCreation.i);
    EXPECT_EQ(b1, testSettingsCreation.b);

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

    settings.declare(object.s, std::string("_") + softeq::common::stdutils::demangle(typeid(object.s).name()));
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

TEST(Settings, VectorOfStructs)
{
    StubSettings settings;
    VectorTestSettings testSettingsCreation;
    testSettingsCreation.container.push_back({42});
    testSettingsCreation.container.push_back({43});
    testSettingsCreation.container.push_back({44});

    settings.declare(testSettingsCreation,
                     std::string("_") + softeq::common::stdutils::demangle(typeid(testSettingsCreation).name()));

    std::vector<SimpleSetting> &test_vector1 = settings.access<VectorTestSettings>().container;
    EXPECT_EQ(test_vector1.size(), testSettingsCreation.container.size());
    EXPECT_EQ(test_vector1.at(0).i, testSettingsCreation.container.at(0).i);
    EXPECT_EQ(test_vector1.at(1).i, testSettingsCreation.container.at(1).i);
    EXPECT_EQ(test_vector1.at(2).i, testSettingsCreation.container.at(2).i);

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


TEST(Settings, Graph)
{
    Object testSettings1 = {};
    StubSettings stubSettings;
    stubSettings.declare(testSettings1, "TestSettings1");
    std::string digraph = stubSettings.graph();
    std::size_t expectedDigraphSnapshot = 13541670285788325260ul;
    std::size_t actualDigraphSnapshot = std::hash<std::string>()(digraph);
    EXPECT_EQ(expectedDigraphSnapshot, actualDigraphSnapshot);
}

TEST(Settings, StaticDeclaration)
{
    auto object = Settings::instance().access<Object>();

    EXPECT_EQ(object.floatT, {});
    EXPECT_EQ(object.doubleT, {});
    EXPECT_EQ(object.boolT, {});
    EXPECT_EQ(object.charT, {});
    EXPECT_EQ(object.boolT, {});
    EXPECT_EQ(object.charT, {});
    EXPECT_EQ(object.char16T, {});
    EXPECT_EQ(object.wcharT, {});
    EXPECT_EQ(object.intT, {});
    EXPECT_EQ(object.longIntT, {});
    EXPECT_EQ(object.unsignedIntT, {});
    EXPECT_EQ(object.unsignedLongIntT, {});
    EXPECT_EQ(object.stringT.size(), {});
    EXPECT_EQ(object.vectorT.size(), {});
    EXPECT_EQ(object.vectorTVectorInt.size(), {});
    EXPECT_EQ(object.mapTStringInt.size(), {});
    EXPECT_EQ(object.unscopedEnumT, {});
    EXPECT_EQ(object.simpleSubobjectT.intT, {});
    EXPECT_EQ(object.subobjectT.intT, {});
    EXPECT_EQ(object.nestedSubobjectT.intT, {});
    EXPECT_EQ(object.vectorTSubobjectSimple.size(), {});
    EXPECT_EQ(object.vectorTSubobject.size(), {});
    EXPECT_EQ(object.mapTIntSubobjectSimple.size(), {});
    EXPECT_EQ(object.mapTStringSubobjectSimple.size(), {});
    EXPECT_EQ(object.optionalTInt.hasValue(), {});
}