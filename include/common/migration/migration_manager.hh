#ifndef SOFTEQ_COMMON_MIGRATIONMANAGER_HH_
#define SOFTEQ_COMMON_MIGRATIONMANAGER_HH_

#include <functional>
#include <memory>
#include <set>
#include <stdexcept>

#include <common/migration/migration_task.hh>

/*! Migration namespace consists of two classes: Manager and Task
 * Manager ia a main object, that is used to do migrations.
 * Main purpose is to fill it with a bunch of Task"s"  and to launch the update process
 * Tasks could be added to Manger using two ways:
 * - add Task object
 * - add update and/or rollback function
 */
namespace softeq
{
namespace common
{
namespace migration
{

struct Exception : public std::logic_error
{
    using std::logic_error::logic_error;
};

class Manager final
{
public:
    using LoadVersionFunc = std::function<Version()>;
    using SaveVersionFunc = std::function<bool(const Version &)>;

    //TODO: Need to pass current app version to check that major updates have migration tasks
    /*!
     * \brief Manager constructor
     */
    Manager();

    /*!
     * \brief add new task to Manager
     * \param task[in] task to add
     */
    Manager &addTask(Task::UPtr task);

    /*!
     * \brief add new task to Manager
     * \param to[in] source version from which this task is used for update
     * \param description[in] text description for the task
     * \param rollbackFunc[in] function that is used for rollback applying
     */
    Manager &addTask(const Version &to, const std::string &description, Task::MigrateFunc &&upgradeFunc,
                     Task::MigrateFunc &&rollbackFunc = nullptr);

    /*!
     * \brief remove all tasks from update manager
     */
    void clear();

    /*!
     * \brief perform tasks to latest version
     * \return version of migration
     */
    Version checkout() const;

    /*!
     * \brief perform tasks to specific version
     * \return version of migration
     */
    Version checkout(const Version &version) const;

    /*!
     * \brief get latest version
     * \return latest version
     */
    Version latestVersion() const;

    /*!
     * \brief get current version
     * \return current version
     */
    Version currentVersion() const;

    /*!
     * \brief get list of all added task version
     * \return list of all added task version
     */
    std::vector<Version> allAddedTaskVersionList() const;

    /*!
     * \brief set function for retrieving current version of the app
     * \param func[in] - function for retrieving current version of the app
     */
    void setLoadVersionFunction(LoadVersionFunc func);

    /* */
    /*!
     * \brief set function for storing new version of the app after all tasks are applied
     * \param func[in] - function for storing new version of the app
     */
    void setSaveVersionFunction(SaveVersionFunc func);

private:
    void checkCallbacks() const;

    using CompareTasksFunc = std::function<bool(const migration::Task::UPtr &, const migration::Task::UPtr &)>;
    std::multiset<Task::UPtr, CompareTasksFunc> _tasks;
    LoadVersionFunc _loadVFunc;
    SaveVersionFunc _saveVFunc;
};
} // namespace migration
} // namespace common
} // namespace softeq
#endif // SOFTEQ_COMMON_MIGRATIONMANAGER_HH_
