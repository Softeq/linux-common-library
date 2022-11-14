#include <common/migration/migration_task.hh>

using namespace softeq::common::migration;
using namespace softeq::common::stdutils;

Task::Task(const Version &to, MigrateFunc &&upgradeFunc, MigrateFunc &&rollbackFunc)
    : _version(to)
    , _updateFunc(std::move(upgradeFunc))
    , _rollbackFunc(std::move(rollbackFunc))
{
    std::string updateStr, rollbackStr;
    if (_updateFunc)
    {
        updateStr = std::string(" Update(") + demangle(_updateFunc.target_type().name()) + ")";
    }
    if (_rollbackFunc)
    {
        rollbackStr = std::string(" Rollback(") + demangle(_rollbackFunc.target_type().name()) + ")";
    }

    setDescription(std::string("Task") + updateStr + rollbackStr);
}

void Task::setUpdateFunc(MigrateFunc &&func)
{
    _updateFunc = std::move(func);
}

void Task::setRollbackFunc(MigrateFunc &&func)
{
    _rollbackFunc = std::move(func);
}

void Task::setDescription(const std::string &text)
{
    _description = text;
}

std::string Task::description() const
{
    return _description;
}

Version Task::destinationVersion() const
{
    return _version;
}

bool Task::apply(Action action)
{
    switch (action)
    {
    case Action::Update:
        return _updateFunc ? _updateFunc() : true;
    case Action::Rollback:
        return _rollbackFunc ? _rollbackFunc() : true;
    }
    return false;
}
