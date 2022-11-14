#include <gtest/gtest.h>
#include <common/settings/settings.hh>
#include <common/system/fsutils.hh>
#include <common/serialization/object_assembler.hh>
#include <common/migration/migration_manager.hh>
#include <common/migration/migration_settings.hh>
#include <common/serialization/json/json.hh>
#include <memory>

using namespace softeq::common;
using namespace softeq::common::migration;

namespace
{
constexpr char DESCRIPTION[] = "Description";
constexpr char NO_DESCRIPTION[] = "Task";

struct NewStruct_t
{
    int a = 5;
    std::string name = "test";
};

// Do not use sigleton for tests any more, use StubSettings
class StubSettings : public settings::Settings
{
};

} // namespace

class TestSettingsMigrationManager : public ::testing::Test
{
    bool saveVersion(const Version &value)
    {
        _version = value;
        return true;
    }

    Version loadVersion()
    {
        return _version;
    }

    void SetUp()
    {
        _version = Version();
        _mgr.reset(new Manager);
        _mgr->setLoadVersionFunction(std::bind(&TestSettingsMigrationManager::loadVersion, this));
        _mgr->setSaveVersionFunction(std::bind(&TestSettingsMigrationManager::saveVersion, this, std::placeholders::_1));
    }

    void TearDown()
    {
    }

protected:
    TestSettingsMigrationManager()
    {
        std::unique_ptr<serialization::StructDeserializer> _deserializer =
            serialization::json::createStructDeserializer();
        settings.setDefaultParameters("", std::move(_deserializer));
    }

    std::unique_ptr<Manager> _mgr;
    StubSettings settings;
    Version _version;
};

namespace softeq
{
namespace common
{
namespace serialization
{
template <>
ObjectAssembler<NewStruct_t> Assembler()
{
    // clang-format off
    return ObjectAssembler<NewStruct_t>()
        .define("a", &NewStruct_t::a)
        .define("name", &NewStruct_t::name)
        ;
    // clang-format on
}
} // namespace serialization
} // namespace common
} // namespace softeq

TEST_F(TestSettingsMigrationManager, UpdateSkipTask)
{
    _mgr->setLoadVersionFunction([]() { return Version(0, 1); });
    _mgr->addTask(Task::UPtr(
        new NewSettingsStructTask<NewStruct_t>(Version(0, 1), settings, "NewStruct", NewStruct_t())));
    Version version = _mgr->checkout(Version(0, 1));
    ASSERT_EQ(version, Version(0, 1));
    ASSERT_EQ(_version, Version(0, 1));
    NewStruct_t checkStruct;
    ASSERT_THROW(checkStruct = settings.access<NewStruct_t>(), std::logic_error);
}

TEST_F(TestSettingsMigrationManager, AddSettingsStructureTask)
{
    _mgr->addTask(migration::Task::UPtr(
        new NewSettingsStructTask<NewStruct_t>(Version(0, 1), settings, "NewStruct", NewStruct_t())));
    Version version = _mgr->checkout(Version(0, 1));
    ASSERT_EQ(version, Version(0, 1));
    ASSERT_EQ(_version, Version(0, 1));
    auto &newStruct = settings.access<NewStruct_t>();
    ASSERT_EQ(newStruct.a, NewStruct_t().a);
    ASSERT_EQ(newStruct.name, NewStruct_t().name);
}

TEST_F(TestSettingsMigrationManager, RollbackNewStruct)
{
    settings.declare<NewStruct_t>("NewStruct");
    _version = Version(0, 1, 0, 2000);
    _mgr->addTask(migration::Task::UPtr(
        new NewSettingsStructTask<NewStruct_t>(Version(0, 1), settings, "NewStruct", NewStruct_t())));
    Version version = _mgr->checkout(Version(0, 1));
    ASSERT_EQ(version, Version(0, 1));
    ASSERT_EQ(_version, Version(0, 1));
}

TEST_F(TestSettingsMigrationManager, clearTasks)
{
    _mgr->addTask(migration::Task::UPtr(
        new NewSettingsStructTask<NewStruct_t>(Version(0, 1), settings, "NewStruct", NewStruct_t())));
    _mgr->clear();
    Version version = _mgr->checkout(Version(0, 0));
    ASSERT_EQ(version, Version(0, 0));
    ASSERT_EQ(_version, Version(0, 0));
}
