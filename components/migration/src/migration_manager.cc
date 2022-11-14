#include <algorithm>
#include <deque>
#include <iterator>

#include <common/logging/log.hh>
#include <common/migration/migration_manager.hh>
#include <common/migration/migration_task.hh>
#include <common/stdutils/stdutils.hh>

namespace
{
const char *const LOG_DOMAIN = "Migration";

bool compareTasks(const softeq::common::migration::Task::UPtr &l,
                  const softeq::common::migration::Task::UPtr &r)
{
    return l.get()->destinationVersion() < r.get()->destinationVersion();
}
} // namespace

using namespace softeq::common::migration;
using namespace softeq::common::stdutils;

Manager::Manager()
    : _tasks(compareTasks)
{
}

Manager &Manager::addTask(Task::UPtr task)
{
    if (!task)
    {
        throw Exception("Null task was added to the MigrationManager");
    }
    _tasks.insert(std::move(task));
    return *this;
}

Manager &Manager::addTask(const Version &to, const std::string &description, Task::MigrateFunc &&upgradeFunc,
                          Task::MigrateFunc &&rollbackFunc)
{
    if (upgradeFunc == nullptr && rollbackFunc == nullptr)
    {
        throw Exception("No update and rollback functions were set");
    }
    Task::UPtr task(new Task(to, std::move(upgradeFunc), std::move(rollbackFunc)));
    task->setDescription(description);
    return addTask(std::move(task));
}

void Manager::clear()
{
    _tasks.clear();
}

void Manager::setLoadVersionFunction(LoadVersionFunc func)
{
    _loadVFunc = std::move(func);
}

void Manager::setSaveVersionFunction(SaveVersionFunc func)
{
    _saveVFunc = std::move(func);
}

void Manager::checkCallbacks() const
{
    if (!_loadVFunc)
    {
        throw Exception("No callback for application version retrieving is assigned");
    }

    if (!_saveVFunc)
    {
        throw Exception("No callback for application version storing is assigned");
    }
}

Version Manager::latestVersion() const
{
    if (_tasks.empty())
    {
        throw Exception("No version exist.");
    }

    return _tasks.rbegin()->get()->destinationVersion();
}

Version Manager::currentVersion() const
{
    return _loadVFunc();
}

std::vector<Version> Manager::allAddedTaskVersionList() const
{
    std::vector<Version> versionList{};
    for (const auto &task : _tasks)
    {
        versionList.push_back(task->destinationVersion());
    }
    return versionList;
}

Version Manager::checkout() const
{
    Version latestVersion = this->latestVersion();
    Version migrationVersion = checkout(latestVersion);
    return migrationVersion;
}

Version Manager::checkout(const Version &nextVersion) const
{
    checkCallbacks();
    bool status = true;

    auto isVersionLess = [](const Version &ver, const Task::UPtr &task)
    {
        return ver < task->destinationVersion();
    };
    auto isVersionLessInReverse = [](const Task::UPtr &task, const Version &ver)
    {
        return  ver < task->destinationVersion();
    };

    const Version currentVersion = _loadVFunc();
    if(nextVersion == currentVersion)
    {
        _saveVFunc(currentVersion);
        return currentVersion;
    }

    // create changeset of tasks and select action type
    Task::Action action{};
    std::deque<Task*> changeset;
    if(nextVersion < currentVersion)
    {
        auto currentVersionUpperBondIt = std::lower_bound(
            _tasks.rbegin(), _tasks.rend(), currentVersion, isVersionLessInReverse);
        for(auto task = currentVersionUpperBondIt; task != _tasks.rend(); ++task)
        {
            if(task->get()->destinationVersion() < nextVersion)
            {
                break;
            }
            changeset.push_back(task->get());
            }
        action = Task::Action::Rollback;
    }
    else
            {
        auto currentVersionLowerBondIt = std::upper_bound(
            _tasks.begin(), _tasks.end(), currentVersion, isVersionLess);

        for(auto task = currentVersionLowerBondIt; task != _tasks.end(); ++task)
        {
            if(nextVersion < task->get()->destinationVersion())
            {
                break;
            }
            changeset.push_back(task->get());
        }
        action = Task::Action::Update;
    }

    // apply changeset
    Version temporalVersion{};
    Version lastSuccessVersion = currentVersion;
    std::string statusMessage{};
    for(auto const &task: changeset)
    {

        try
        {
            temporalVersion = task->destinationVersion();
            status = status && task->apply(action);
            statusMessage = (status ? "SUCCESS" : "FAILED");
        }
        catch (const std::exception &e)
        {
            status = false;
            statusMessage = e.what();
        }
        LOGD(LOG_DOMAIN, "Checkout task \"%s\" to version %s, exec_status %s", task->description().c_str(),
            task->destinationVersion().toString().c_str(), statusMessage.c_str());

        if(status == false)
        {
            break;
        }
        lastSuccessVersion = temporalVersion;
    }
    _saveVFunc(lastSuccessVersion);
    return lastSuccessVersion;
}
