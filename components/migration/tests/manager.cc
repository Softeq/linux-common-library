#include <gtest/gtest.h>
#include <common/migration/migration_manager.hh>
#include <common/migration/migration_task.hh>
#include <memory>

using namespace softeq::common;
using namespace softeq::common::migration;

namespace
{
constexpr char DESCRIPTION[] = "Description";
constexpr char NO_DESCRIPTION[] = "Task";
bool update()
{
    return true;
}

bool rollback()
{
    return true;
}

} // namespace

class TestMigrationManager : public ::testing::Test
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
        _mgr->setLoadVersionFunction(std::bind(&TestMigrationManager::loadVersion, this));
        _mgr->setSaveVersionFunction(std::bind(&TestMigrationManager::saveVersion, this, std::placeholders::_1));
    }

    void TearDown()
    {
    }

protected:
    TestMigrationManager()
    {
    }

    std::unique_ptr<Manager> _mgr;
    Version _version;
};

TEST_F(TestMigrationManager, VersionIncrement)
{
    Version v(0, 0, 5, 2000);
    ++v;
    EXPECT_EQ(v.gen, 0);
    EXPECT_EQ(v.major, 1);
    EXPECT_EQ(v.minor, 0);
    EXPECT_EQ(v.build, 2000);
    ++v;
    EXPECT_EQ(v.gen, 0);
    EXPECT_EQ(v.major, 2);
    EXPECT_EQ(v.minor, 0);
    EXPECT_EQ(v.build, 2000);
}

TEST_F(TestMigrationManager, VersionDecrement)
{
    Version v(0, 3, 5, 2000);
    --v;
    EXPECT_EQ(v.gen, 0);
    EXPECT_EQ(v.major, 2);
    EXPECT_EQ(v.minor, 0);
    EXPECT_EQ(v.build, 2000);
    --v;
    EXPECT_EQ(v.gen, 0);
    EXPECT_EQ(v.major, 1);
    EXPECT_EQ(v.minor, 0);
    EXPECT_EQ(v.build, 2000);
}

TEST_F(TestMigrationManager, EmptyTaskWithDescription)
{
    Task task(Version(0, 0));
    task.setDescription(DESCRIPTION);
    ASSERT_STREQ(DESCRIPTION, task.description().c_str());
}

TEST_F(TestMigrationManager, TaskVersion)
{
    Task task(Version(0, 1, 2, 3));
    EXPECT_EQ(task.destinationVersion().gen, 0);
    EXPECT_EQ(task.destinationVersion().major, 1);
    EXPECT_EQ(task.destinationVersion().minor, 2);
    EXPECT_EQ(task.destinationVersion().build, 3);
}

TEST_F(TestMigrationManager, PrintVersion)
{
    Version v(0, 1, 2, 3);
    ASSERT_STREQ(v.toString().c_str(), "0.1.2.3");
}

TEST_F(TestMigrationManager, StreamVersion)
{
    std::stringstream ss;
    Version v(0, 1, 2, 3);
    ss << v;
    ASSERT_STREQ(ss.str().c_str(), "0.1.2.3");
}

TEST_F(TestMigrationManager, VersionCompare)
{
    EXPECT_TRUE(Version(0, 0) < Version(0, 1));
    EXPECT_TRUE(Version(0, 1) < Version(1, 0));
    EXPECT_TRUE(Version(1, 5) < Version(1, 6));
    EXPECT_FALSE(Version(1, 1) < Version(1, 1));
    EXPECT_FALSE(Version(1, 0) < Version(0, 0));
}

TEST_F(TestMigrationManager, AddNullTask)
{
    ASSERT_THROW(_mgr->addTask(std::unique_ptr<Task>()), Exception);
}

TEST_F(TestMigrationManager, LatestVersion)
{
    int i = 0;
    Version version{};
    ASSERT_THROW(version = _mgr->checkout(), Exception);
    _mgr->addTask(Version(0, 2), "update counter", [&i]() {
        i += 3;
        return true;
    });
    EXPECT_EQ(version, Version(0, 0));

    _mgr->addTask(Version(0, 1), "update counter", [&i]() {
        i += 5;
        return true;
    });
    version = _mgr->checkout();
    EXPECT_EQ(version, Version(0, 2));
    EXPECT_EQ(i, 8);
}

TEST_F(TestMigrationManager, AllVersionList)
{
    uint taskCount = 3;
    while(taskCount--)
    {
        _mgr->addTask(Version(0, taskCount), "", []{return true;});
    }

    for(const auto &version : _mgr->allAddedTaskVersionList())
    {
        taskCount++;
        EXPECT_EQ(version, Version(0, taskCount));
    }
}

TEST_F(TestMigrationManager, EmptyTaskWithoutDescription)
{
    Task task(Version(0, 1));
    ASSERT_STREQ(NO_DESCRIPTION, task.description().c_str());
}

TEST_F(TestMigrationManager, FakeTaskWithoutDescription)
{
    Task task(Version(0, 1), update, rollback);
    ASSERT_STREQ((std::string(NO_DESCRIPTION) + " Update(bool (*)()) Rollback(bool (*)())").c_str(),
                 task.description().c_str());
}

TEST_F(TestMigrationManager, UpdateBadTask)
{
    _mgr->addTask(Version(0, 1), "bad update", []() { return false; });
    Version version = _mgr->checkout(Version(0, 1));
    ASSERT_EQ(version, Version(0, 0));
    ASSERT_EQ(_version, Version(0, 0));
}

TEST_F(TestMigrationManager, UpdateTaskWithException)
{
    _mgr->addTask(Version(0, 1), "update exception", []() {
        throw std::runtime_error("error");
        return false;
    });
    Version version = _mgr->checkout(Version(0, 1));
    ASSERT_EQ(version, Version(0, 0));
    ASSERT_EQ(_version, Version(0, 0));
}

TEST_F(TestMigrationManager, UpdateSequence)
{
    int i = 0;
    _mgr->addTask(Version(0, 0), "update counter", [&i]() {
        i -= 150;
        return true;
    }); // shouldn't be executed
    _mgr->addTask(Version(0, 3), "update counter", [&i]() {
        i += 5;
        return true;
    });
     _mgr->addTask(Version(0, 4), "update counter", [&i]() {
        i += 5;
        return true;
    });
    _mgr->addTask(Version(0, 1), "update counter", [&i]() {
        i++;
        return true;
    });
    _mgr->addTask(Version(0, 2), "update counter", [&i]() {
        i *= 5;
        return true;
    });
    Version version = _mgr->checkout(Version(0, 3));
    ASSERT_EQ(version, Version(0, 3));
    ASSERT_EQ(_version, Version(0, 3));
    ASSERT_EQ(i, 10);
}

TEST_F(TestMigrationManager, UpdateBadSequence)
{
    int i = 0;
    _mgr->addTask(Version(0, 3), "update counter 3", [&i]() {
        i += 5;
        return true;
    });
    _mgr->addTask(Version(0, 1), "update counter 1", [&i]() {
        i++;
        return true;
    });
    _mgr->addTask(Version(0, 2), "update counter 2", []() { return false; });
    Version version = _mgr->checkout(Version(0, 3));
    ASSERT_EQ(version, Version(0, 1));
    ASSERT_EQ(_version, Version(0, 1));
    ASSERT_EQ(i, 1);
}

TEST_F(TestMigrationManager, UpdateSequenceWithException)
{
    int i = 0;
    _mgr->addTask(Version(0, 3), "update counter 3", [&i]() {
        i += 5;
        return true;
    });
    _mgr->addTask(Version(0, 1), "update counter 1", [&i]() {
        i++;
        return true;
    });
    _mgr->addTask(Version(0, 2), "update counter exception", []() {
        throw std::runtime_error("error");
        return false;
    });
    Version version = _mgr->checkout(Version(0, 3));
    ASSERT_EQ(version, Version(0, 1));
    ASSERT_EQ(_version, Version(0, 1));
    ASSERT_EQ(i, 1);
}

TEST_F(TestMigrationManager, RollbackBadTask)
{
    _version = Version(0, 1, 0, 2000);
    _mgr->addTask(Version(0, 1), "bad rollback", nullptr, []() { return false; });
    Version version = _mgr->checkout(Version(0, 0));
    ASSERT_EQ(version, Version(0, 1));
    ASSERT_EQ(_version, Version(0, 1));
}

TEST_F(TestMigrationManager, RollbackTaskWithException)
{
    _version = Version(0, 1, 0, 2000);
    _mgr->addTask(Version(0, 1), "rollback exception", nullptr, []() {
        throw std::runtime_error("error");
        return false;
    });
    Version version = _mgr->checkout(Version(0, 0));
    ASSERT_EQ(version, Version(0, 1));
    ASSERT_EQ(_version, Version(0, 1));
}

TEST_F(TestMigrationManager, RollbackSequence)
{
    int i = 0;
    _version = Version(0, 3, 0, 2000);
    _mgr->addTask(Version(0, 4), "rollback counter", nullptr, [&i]() {
        i -= 150;
        return true;
    }); // shouldn't be executed
    _mgr->addTask(Version(0, 3), "rollback counter", nullptr, [&i]() {
        i += 5;
        return true;
    });
    _mgr->addTask(Version(0, 1), "rollback counter", nullptr, [&i]() {
        i += 5;
        return true;
    });
    _mgr->addTask(Version(0, 0), "rollback counter", nullptr, [&i]() {
        i -= 150;
        return true;
    });
    _mgr->addTask(Version(0, 2), "rollback counter", nullptr, [&i]() {
        i *= 5;
        return true;
    });
    Version version = _mgr->checkout(Version(0, 1));
    ASSERT_EQ(version, Version(0, 1));
    ASSERT_EQ(_version, Version(0, 1));
    ASSERT_EQ(i, 30);
}

TEST_F(TestMigrationManager, RollbackBadSequence)
{
    int i = 0;
    _version = Version(0, 3, 0, 2000);
    _mgr->addTask(Version(0, 3), "rollback counter 3", nullptr, [&i]() {
        i += 5;
        return true;
    });
    _mgr->addTask(Version(0, 1), "rollback counter 1", nullptr, [&i]() {
        i++;
        return true;
    });
    _mgr->addTask(Version(0, 2), "rollback counter 2", nullptr, []() { return false; });
    Version version = _mgr->checkout(Version(0, 0));
    ASSERT_EQ(version, Version(0, 3));
    ASSERT_EQ(_version, Version(0, 3));
    ASSERT_EQ(i, 5);
}

TEST_F(TestMigrationManager, RollbackSequenceWithException)
{
    int i = 0;
    _version = Version(0, 3, 0, 2000);
    _mgr->addTask(Version(0, 3), "rollback counter 3", nullptr, [&i]() {
        i += 5;
        return true;
    });
    _mgr->addTask(Version(0, 1), "rollback counter 1", nullptr, [&i]() {
        i++;
        return true;
    });
    _mgr->addTask(Version(0, 2), "rollback counter exception", nullptr, []() {
        throw std::runtime_error("error");
        return false;
    });
    Version version = _mgr->checkout(Version(0, 0));
    ASSERT_EQ(version, Version(0, 3));
    ASSERT_EQ(_version, Version(0, 3));
    ASSERT_EQ(i, 5);
}

TEST_F(TestMigrationManager, noLoadVersionFunc)
{
    _mgr->setLoadVersionFunction(nullptr);
    ASSERT_THROW(_mgr->checkout(Version(0, 0)), Exception);
    ASSERT_EQ(_version, Version(0, 0));
}

TEST_F(TestMigrationManager, noSaveVersionFunc)
{
    _mgr->setSaveVersionFunction(nullptr);
    ASSERT_THROW(_mgr->checkout(Version(0, 0)), Exception);
    ASSERT_EQ(_version, Version(0, 0));
}

TEST_F(TestMigrationManager, addTaskUpdateFuncs)
{
    int i = 0;
    _mgr->addTask(Version(0, 1), "update counter", [&i]() {
        i++;
        return true;
    });
    Version version = _mgr->checkout(Version(0, 1));
    ASSERT_EQ(version, Version(0, 1));
    ASSERT_EQ(_version, Version(0, 1));
    ASSERT_EQ(i, 1);
}

TEST_F(TestMigrationManager, addTaskWithNullFuncs)
{
    ASSERT_THROW(_mgr->addTask(Version(0, 1), "update counter", nullptr), Exception);
    ASSERT_EQ(_version, Version(0, 0));
}

TEST_F(TestMigrationManager, addTaskWithNullUpdateFunc)
{
    _mgr->addTask(Version(0, 1), "update counter", nullptr, []() { return true; });
    Version version = _mgr->checkout(Version(0, 1));
    ASSERT_EQ(version, Version(0, 1));
    ASSERT_EQ(_version, Version(0, 1));
}
