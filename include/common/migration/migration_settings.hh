#ifndef SOFTEQ_COMMON_MIGRATIONSETTINGS_HH_
#define SOFTEQ_COMMON_MIGRATIONSETTINGS_HH_
#include <algorithm>
#include <common/settings/settings.hh>
#include <common/migration/migration_manager.hh>

namespace softeq
{
namespace common
{
namespace migration
{
/* Task that creates new structure of settings */
template <typename T>
class NewSettingsStructTask : public Task
{
public:
    /*!
     * \brief Constructor of NewSettingsStructTask class
     * \param name[in] name for settings object
     * \param settings[in] default settings for a new settings structure
     */
    NewSettingsStructTask(const Version &from, softeq::common::settings::Settings &settings, const std::string &name,
                          const T &settingsStruct)
        : Task(from, std::bind(&NewSettingsStructTask::update, this, &settings, name, settingsStruct),
               std::bind(&NewSettingsStructTask::rollback, this, &settings, name))
    {
        setDescription("create new settings structure: " + name);
    }

private:
    bool update(softeq::common::settings::Settings *settings, const std::string &name, const T &settingsStruct)
    {
        settings->declare<T>(name);
        settings->access<T>() = settingsStruct;
        return true;
    }

    bool rollback(softeq::common::settings::Settings *settings, const std::string &name)
    {
        settings->undeclare(name);
        return true;
    }
};
} // namespace migration
} // namespace common
} // namespace softeq
#endif
