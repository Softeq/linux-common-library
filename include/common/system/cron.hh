#ifndef SOFTEQ_COMMON_CRON_H_
#define SOFTEQ_COMMON_CRON_H_

#include <functional>
#include <memory>
#include <string>
#include <stdexcept>

namespace softeq
{
namespace common
{
namespace system
{
/*!
  The class describes time-based job scheduler with
  cron-like rules to schedule the task <https://en.wikipedia.org/wiki/Cron>

  The cron expression syntax expects a made of six fields which represent the time to execute the command.
  ┌───────────── minute (0 - 59)
  │ ┌───────────── hour (0 - 23)
  │ │ ┌───────────── day of the month (1 - 31)
  │ │ │ ┌───────────── month (1 - 12)
  │ │ │ │ ┌───────────── day of the week (0 - 6) (Sunday to Saturday; 7 is also Sunday on some systems)
  │ │ │ │ │ ┌───────────── year (2099 max value)
  │ │ │ │ │ │
  │ │ │ │ │ │
  * * * * * * \<command to execute\>
*/
class Cron
{
public:
    /** Job callback, if returned value is 'false' the job will be removed from queue
     * The job should be as simple as possible, so it wont block worker thread.
     * If there is a need to schedule long-term jobs asynchronously, you should implement a new ICron for that
     */
    using Callback = std::function<bool()>;

    /**
     * Uptr is a unique pointer to ICron
     */
    using UPtr = std::unique_ptr<Cron>;

    /**
     * JobId is int type
     */
    using JobId = int;
    static const JobId invalidJobId = -1;

    /**
     * Exception: can't satisfy the cron expression request. For example due to a non-existent date.
     */
    class Unsatisfiable : public std::runtime_error
    {
    public:
        explicit Unsatisfiable(const std::string &what_arg)
            : std::runtime_error(what_arg){};

        explicit Unsatisfiable(const char *what_arg)
            : std::runtime_error(what_arg){};
    };

    /**
     * Exception: Trying to set an invalid rule.
     */
    class InvalidRule : public std::runtime_error
    {
    public:
        explicit InvalidRule(const std::string &what_arg)
            : std::runtime_error(what_arg){};

        explicit InvalidRule(const char *what_arg)
            : std::runtime_error(what_arg){};
    };

    virtual ~Cron() = default;

    /**
     * Adds a function to call according to the timespec. Added function will become a 'job' and get unique ID.
     * @param name The name of newly created job
     * @param timespec Schedule description, for more see examples/cron.cc
     * @param continued Should the function 'f' be called repeatedly
     * @param f Function to call, if returned value is 'false' the job will be removed from queue
     * @return ID of newly-created job or 'kInvalidJobId' on failure
     */
    virtual JobId addJob(const std::string &name, const std::string &timespec, const Callback &f) = 0;

    /**
     * Remove a job with given ID
     * @param jobId Job id
     */
    virtual void removeJob(JobId jobId) = 0;

    virtual time_t jobExecutionTime(JobId jobId) const = 0;

    /*!
      Starts the cron daemon, creates a new thread.
    */
    virtual bool start() = 0;

    /*!
      Stops the cron daemon
    */
    virtual void stop() = 0;
};

class CronFactory
{
public:
    static Cron::UPtr create();
};

} // namespace system
} // namespace common
} // namespace softeq

#endif // SOFTEQ_COMMON_CRON_H_
