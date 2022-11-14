#ifndef SOFTEQ_COMMON_MIGRATIONTASK_HH_
#define SOFTEQ_COMMON_MIGRATIONTASK_HH_
#include <functional>
#include <iostream>
#include <memory>
#include <sstream>
#include <utility>
#include <common/stdutils/stdutils.hh>

namespace softeq
{
namespace common
{
/*! Migration namespace consists of two classes: Manager and Task
 * Manager ia a main object, that is used to do migrations.
 * Main purpose is to fill it with a bunch of Task"s"  and to launch the update process
 * Tasks could be added to Manger using two ways:
 * - add Task object
 * - add update and/or rollback function
 */
namespace migration
{
/*! \brief class that represents version of the application
 * Holds 4 parameters insde : <generation>.<major>.<minor>.<build>
 * Comparasion of two Version objects is made using "generation" and "major" parameters only
 * generation - is set by the customer
 * major - is changed when public API or ABI of subcomponents or stogare strategy has changed
 * minor - is changed when new feature implemented , major bug fixed but it is not major changes
 * build - is changed each time
 */
struct Version
{
    Version() = default;
    /*!
     * \brief Constructor
     * \param gen[in] generation of the application
     * \param major[in] major version of the application
     * \param minor[in] minor version of the application
     * \param build[in] build number of the application
     */

    Version(int gen, int major, int minor = 0, int build = 0)
        : gen(gen)
        , major(major)
        , minor(minor)
        , build(build)
    {
    }

    /*!
     * \brief preincrement operator setting next version index
     * \return new calculated version inside object
     */
    Version &operator++()
    {
        return this->operator+(1);
    }

    /*!
     * \brief predecrement operator setting previous version index
     * \return new calculated version inside object
     */
    Version &operator--()
    {
        return this->operator+(-1);
    }

    /*!
     * \brief less operator for versions comparasion
     * \param val[in] version to compare with
     * \return is current version is less
     */
    bool operator<(const Version &val) const
    {
        if (this->gen == val.gen)
            return this->major < val.major;
        return this->gen < val.gen;
    }

    /*!
     * \brief equality operator for versions comparasion
     * \param val[in] version to compare with
     * \return is current version is equal
     */
    bool operator==(const Version &val) const
    {
        return this->gen == val.gen && this->major == val.major;
    }

    /*!
     * \brief not greater operator for versions comparasion
     * \param val[in] version to compare with
     * \return is current version not greater
     */
    bool operator<=(const Version &val) const
    {
        return *this < val || *this == val;
    }

    /*!
     * \brief translate object to string format
     * \return string representation of the version
     */
    std::string toString() const
    {
        std::stringstream ss;
        ss << *this;
        return ss.str();
    }

    /*!
     * \brief operator << for using object with std::ostream
     * \param stream[in] stream which is used for output
     * \param val[in] Version to push into the stream
     * \return stream for further usage
     */
    friend std::ostream &operator<<(std::ostream &stream, const Version &val)
    {
        return stream << softeq::common::stdutils::string_format("%d.%d.%d.%d", val.gen, val.major, val.minor,
                                                                 val.build);
    }

    int gen = 0;   ///< generation of the application
    int major = 0; ///< major version of the application
    int minor = 0; ///< minor version of the application
    int build = 0; ///< build number of the application
private:
    /*!
     * \brief add operator for calculation of the next N version
     * \param val[in] version delta
     * \return new calculated version inside object
     */
    Version &operator+(int val)
    {
        major += val;
        minor = 0;
        return *this;
    }
};

class Task
{
public:
    friend class Manager;
    using UPtr = std::unique_ptr<Task>;
    using MigrateFunc = std::function<bool()>;

    /*!
     * \brief Constructor
     * \param to[in] final version to which this task will be updated
     * \param upgradeFunc[in] function that is used for upgrade applying
     * \param rollbackFunc[in] function that is used for rollback applying
     * \return new flag container
     */
    explicit Task(const Version &to, MigrateFunc &&upgradeFunc = nullptr, MigrateFunc &&rollbackFunc = nullptr);

    /*!
     * \brief get version of the application, from which this task is used for upgrade
     * \return version of the application for task
     */
    Version destinationVersion() const;

    /*!
     * \brief set description of the task
     * \param text[in] text description
     */
    void setDescription(const std::string &text);

    /*!
     * \brief get description of the task
     * \return text description
     */
    std::string description() const;

protected:
    // Actions of update process
    enum class Action
    {
        Update,
        Rollback
    };

    /*!
     * \brief run the task
     * \return task execution result
     */
    bool apply(Action action = Action::Update);

    /*!
     * \brief set function that is used for upgrade applying
     * \param func[in] function for upgrade applying
     */
    void setUpdateFunc(MigrateFunc &&func);
    /*!
     * \brief set function that is used for rollback applying
     * \param func[in] function for rollback applying
     */
    void setRollbackFunc(MigrateFunc &&func);

private:
    Version _version{0, 0};
    MigrateFunc _updateFunc;
    MigrateFunc _rollbackFunc;
    std::string _description;
};
} // namespace migration
} // namespace common
} // namespace softeq
#endif
