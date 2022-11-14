#include "cron.hh"
#include "time_provider.hh"

#include <common/logging/log.hh>
#include <common/stdutils/stdutils.hh>
#include <common/stdutils/timeutils.hh>

#include <system_error>
#include <algorithm>
#include <list>
#include <atomic>
#include <bitset>
#include <memory>

using namespace softeq::common;
using namespace softeq::common::system;

namespace
{
const char *const LOG_DOMAIN = "Cron";

int cron_stoi(const std::string &str)
{
    try
    {
        return std::stoi(str);
    }
    catch (const std::logic_error &e)
    {
        throw Cron::InvalidRule(e.what());
    }
}
} // namespace

class CronJob final
{
    static const int MAX_YEAR = 2099;

    struct candidate_t final
    {
        int year;
        int mon;
        int mday;
        int hour;
        int min;

        explicit candidate_t(time_t rawtime)
        {
            std::tm time = {};
            if (localtime_r(&rawtime, &time) != nullptr)
            {
                year = time.tm_year + 1900;
                mon = time.tm_mon + 1;
                mday = time.tm_mday;
                hour = time.tm_hour;
                min = time.tm_min;
            }
            else
            {
                throw std::system_error(errno, std::generic_category());
            }
        }

        int &operator[](int p)
        {
            switch (p)
            {
            case 0:
                return year;
            case 1:
                return mon;
            case 2:
                return mday;
            case 3:
                return hour;
            case 4:
                return min;
            default:
                throw std::out_of_range("candidate_r::&operator[]");
            }
        }

        int operator[](int p) const
        {
            switch (p)
            {
            case 0:
                return year;
            case 1:
                return mon;
            case 2:
                return mday;
            case 3:
                return hour;
            case 4:
                return min;
            default:
                throw std::out_of_range("candidate_r::operator[]");
            }
        }

        time_t toUnixTimestamp() const
        {
            std::tm tm = {};
            tm.tm_year = year - 1900;
            tm.tm_mon = mon - 1;
            tm.tm_mday = mday;
            tm.tm_hour = hour;
            tm.tm_min = min;
            tm.tm_isdst = -1;

            return mktime(&tm);
        }

        std::string toString() const
        {
            return stdutils::string_format("%d-%d-%d %02d:%02d", year, mon, mday, hour, min);
        }
    };

    class Check
    {
    protected:
        /// For minutes is hours, so at minute overflow, increment hour.
        Check *_overflowPart;
        /// For hours is minutes, so at hour incr, reset minutes.
        Check *_resetPart;

    public:
        Check()
            : _overflowPart(nullptr)
            , _resetPart(nullptr)
        {
        }

        virtual ~Check() = default;

        virtual bool valid(const candidate_t &candidate) const = 0;
        virtual bool incr(candidate_t &candidate) = 0; // After incr, is candidate still valid, or should I look from
                                                       // the beginning again.
        virtual bool reset(candidate_t &candidate)
        {
            (void)candidate;
            return false;
        } // After incr of next part, the prev may need to go to min, as if hour increases, then min should go 0
        virtual std::string toString() const = 0;

        void setOverflowPart(Check *prev)
        {
            _overflowPart = prev;
        }
        void setResetPart(Check *prev)
        {
            _resetPart = prev;
        }
    };

    class InRange final : public Check
    {
        int _min;
        int _max;
        int _each;
        int _partn;

    public:
        InRange(const std::string &rule, int minVal, int maxVal, int partn)
            : _min(minVal)
            , _max(maxVal)
            , _each(1)
            , _partn(partn)
        {
            std::string srule(rule);
            if (srule.find('/') != std::string::npos)
            {
                std::vector<std::string> parts(stdutils::string_split(srule, '/'));
                if (parts.size() != 2)
                {
                    throw Cron::InvalidRule("Expected only one '/' delimiter per rule");
                }
                _each = cron_stoi(parts[1]);
                srule = parts[0];
            }

            if (srule == "*")
            {
                if (_each < 1 || _each > maxVal)
                {
                    throw Cron::Unsatisfiable("Defined interval is out of range");
                }
            }
            else if (srule.find('-') != std::string::npos)
            {
                std::vector<std::string> parts(stdutils::string_split(srule, '-'));
                if (parts.size() != 2)
                {
                    throw Cron::InvalidRule("Expected only one '-' delimiter per rule");
                }

                _min = cron_stoi(parts[0]);
                _max = cron_stoi(parts[1]);

                if (_max < _min)
                {
                    throw Cron::InvalidRule("Wrong placement of the min and max borders of the range");
                }
            }
            else
            {
                _min = _max = cron_stoi(srule); // single number
            }

            if (_min < minVal || _max > maxVal)
            {
                throw Cron::Unsatisfiable("Single value or a sub-range is out of main range");
            }
        }

        std::string toString() const override
        {
            std::string result(std::to_string(_partn));
            result.append(" in range ");
            result.append(std::to_string(_min));
            result.append(" - ");
            result.append(std::to_string(_max));
            result.append(" - ");
            result.append(std::to_string(_each));
            return result;
        }

        bool valid(const candidate_t &candidate) const override
        {
            return candidate[_partn] >= _min && candidate[_partn] <= _max &&
                   (candidate[_partn] % _each) == 0; // and each
        }

        bool incr(candidate_t &candidate) override
        {
            if (candidate[_partn] < _min)
            {
                candidate[_partn] = _min;
                if (_resetPart)
                    _resetPart->reset(candidate);
                return true;
            }
            candidate[_partn]++;
            if (candidate[_partn] > _max)
            {
                candidate[_partn] = _min;
                if (_overflowPart)
                    _overflowPart->incr(candidate);
            }
            if (_resetPart)
                _resetPart->reset(candidate);
            return true;
        }

        bool reset(candidate_t &candidate) override
        {
            candidate[_partn] = _min;
            if (_resetPart)
                _resetPart->reset(candidate);
            return true;
        }
    };

    class WeekDay : public Check
    {
        static const int daysInWeek = 7;
        std::bitset<daysInWeek> _validDays;

    public:
        explicit WeekDay(const std::string &_expr)
        {
            if (_expr == "*")
            {
                _validDays.set();
            }
            else if (_expr.find('-') != std::string::npos)
            {
                std::vector<std::string> parts(stdutils::string_split(_expr, '-'));
                if (parts.size() != 2)
                {
                    throw Cron::InvalidRule("Expected only one '-' delimiter per rule");
                }

                int fromDoW = cron_stoi(parts[0]);
                int toDoW = cron_stoi(parts[1]);

                if (toDoW < fromDoW)
                {
                    throw Cron::InvalidRule("Wrong placement of the min and max borders of the range");
                }
                if (fromDoW < 0 || fromDoW > daysInWeek || toDoW < 0 || toDoW > daysInWeek)
                {
                    throw Cron::InvalidRule("Number of a day should be in the range [0;7]");
                }

                for (int dow = fromDoW; dow <= toDoW; ++dow)
                {
                    _validDays[dow % daysInWeek] = true;
                }
            }
            else
            {
                for (const std::string &part : stdutils::string_split(_expr, ','))
                {
                    const int dow{cron_stoi(part)};

                    if (dow > daysInWeek)
                    {
                        throw Cron::InvalidRule("Number of a day should be in the range [0;7], got " +
                                                std::to_string(dow));
                    }

                    _validDays[dow % daysInWeek] = true;
                }
            }
        }

        bool incr(candidate_t &candidate) override
        {
            if (_overflowPart)
            {
                _overflowPart->incr(candidate);
            }

            return false;
        }

        bool valid(const candidate_t &candidate) const override
        {
            const int dow = stdutils::week_day(candidate.mday, candidate.mon, candidate.year);
            if (dow >= 0 && dow < daysInWeek)
            {
                return _validDays[dow];
            }

            return false;
        }

        std::string toString() const override
        {
            return "Week of day";
        }
    };

    class ValidDate : public Check
    {
    public:
        bool valid(const candidate_t &candidate) const override
        {
            const int max_days = stdutils::days_per_month(candidate.mon, candidate.year);
            return (1 <= candidate.mday && candidate.mday <= max_days);
        }

        bool incr(candidate_t &candidate) override
        {
            if (_overflowPart)
                _overflowPart->incr(candidate);
            return false;
        }

        std::string toString() const override
        {
            return "Valid date (leap years, 30 or 31 months...)";
        }
    };

private:
    std::string _name;
    std::string _timespec;
    time_t _nextTime;
    Cron::Callback _cb;
    Cron::JobId _id;
    TimeProvider::WPtr _timeProvider;

public:
    CronJob(TimeProvider::WPtr tp, std::string name, std::string timespec, Cron::Callback cb, Cron::JobId id)
        : _name(std::move(name))
        , _timespec(std::move(timespec))
        , _cb(std::move(cb))
        , _id(id)
        , _timeProvider(tp)
    {
        if (!_cb)
        {
            throw std::invalid_argument("invalid callback");
        }

        _nextTime = calcNext();
    }

    time_t nextTime() const noexcept
    {
        return _nextTime;
    }

    Cron::JobId id() const noexcept
    {
        return _id;
    }

    const std::string &name() const noexcept
    {
        return _name;
    }

    bool perform() const
    {
        return _cb();
    }

    time_t calcNext()
    {
        candidate_t candidate(_timeProvider.lock()->now());
        std::vector<std::string> parts(stdutils::string_split(_timespec, ' '));
        if (parts.size() != 6)
        {
            throw Cron::InvalidRule("must be 6 parts");
        }
        InRange minutes(parts[0], 0, 59, 4);
        InRange hours(parts[1], 0, 23, 3);
        InRange dayOfMonth(parts[2], 1, 31, 2);
        InRange month(parts[3], 1, 12, 1);
        InRange year(parts[5], candidate.year, MAX_YEAR, 0);
        WeekDay week_day(parts[4]);
        ValidDate validDate;

        std::vector<Check *> rules{&minutes, &hours, &dayOfMonth, &month, &year, &week_day, &validDate};
        for (int i = 0; i < 4; i++)
        {
            rules[i]->setOverflowPart(rules[i + 1]);
            rules[i + 1]->setResetPart(rules[i]);
        }
        rules[5]->setOverflowPart(rules[2]);
        rules[6]->setOverflowPart(rules[2]);

        rules[0]->incr(candidate);

        bool valid;
        do
        {
            valid = true;
            for (Check *r : rules)
            {
                while (!r->valid(candidate))
                {
                    valid &= r->incr(candidate);
                    if (!valid) // Start over again, this is normally not good day of week, or 30 feb style dates.
                        break;
                }
            }
            if (candidate[0] >= MAX_YEAR)
            {
                throw(Cron::Unsatisfiable("Candidate's year field is out of the range"));
            }
        } while (!valid);

        LOGD(LOG_DOMAIN, "Next time for job '%s' is %s @%ld", _name.c_str(), candidate.toString().c_str(),
             candidate.toUnixTimestamp());
        _nextTime = candidate.toUnixTimestamp();
        return _nextTime;
    }
};

bool operator<(const CronJob &lhs, const CronJob &rhs)
{
    return lhs.nextTime() < rhs.nextTime();
}

class CronImpl final : public Cron
{
    std::list<CronJob> _jobQueue;
    std::thread _jobThread;
    mutable std::mutex _queueMutex;
    std::atomic_bool _working{false};
    JobId _nextJobId{1};
    TimeProvider::SPtr _timeProvider;

public:
    explicit CronImpl()
        : _timeProvider(TimeProvider::instance())
    {
    }
    ~CronImpl() override
    {
        stop();
    }

    /// Adds a function to call according to that timespec
    JobId addJob(const std::string &name, const std::string &timespec, const Cron::Callback &cb) override
    {
        JobId jobId = invalidJobId;

        if (!cb)
        {
            LOGE(LOG_DOMAIN, "Couldn't add job '%s' due to invalid callback.", name.c_str());
            return jobId;
        }

        try
        {
            std::unique_lock<std::mutex> lock(_queueMutex);
            _jobQueue.emplace_back(CronJob(_timeProvider, name, timespec, cb, _nextJobId));
            jobId = _jobQueue.back().id();
            _jobQueue.sort();
            _nextJobId++;
            lock.unlock();
            _timeProvider->interruptWait();
        }
        catch (const std::exception &ex)
        {
            LOGE(LOG_DOMAIN, "Unable to add job '%s' Reason: %s", timespec.c_str(), ex.what());
            return invalidJobId;
        }

        return jobId;
    }

    time_t jobExecutionTime(JobId jobId) const override
    {
        std::lock_guard<std::mutex> lock(_queueMutex);

        if (!_jobQueue.empty())
        {
            auto iter = std::find_if(_jobQueue.begin(), _jobQueue.end(),
                                     [jobId](const CronJob &job) { return job.id() == jobId; });

            if (iter != _jobQueue.end())
            {
                return iter->nextTime();
            }
        }

        return 0;
    }

    void removeJob(JobId jobId) override
    {
        std::lock_guard<std::mutex> lock(_queueMutex);
        unsafeRemoveJob(jobId);
    }

    /// Starts the cron daemon, creates a new thread.
    bool start() override
    {
        LOGI(LOG_DOMAIN, "Starting.");
        if (_working)
        {
            LOGE(LOG_DOMAIN, "Already working.");
            return false;
        }

        _working = true;
        _jobThread = std::thread(&CronImpl::worker, this);

        return true;
    }

    /// Stops the cron daemon
    void stop() noexcept override
    {
        if (!_working)
            return;

        std::unique_lock<std::mutex> lock(_queueMutex);
        LOGI(LOG_DOMAIN, "Stopping, %d tasks in queue.", static_cast<int>(_jobQueue.size()));
        _working = false;
        lock.unlock();

        _timeProvider->interruptWait();

        if (_jobThread.joinable())
        {
            _jobThread.join();
        }
        LOGD(LOG_DOMAIN, "Stopped.");
    }

private:
    void unsafeRemoveJob(JobId jobId)
    {
        _jobQueue.remove_if([jobId](const CronJob &job) { return job.id() == jobId; });
    }

    void worker()
    {
        while (_working)
        {
            /*
              Main thread must sleep most of the time till the next task
              It can wake up when:
              - a job is added/removed to recalculate new next job
              - system time is changed. all task must be rescheduled
              - all jobs are removed. then sleep till some job is added again
              - cron is stopped
            */
            try
            {
                std::unique_lock<std::mutex> lock(_queueMutex);
                if (!_jobQueue.empty())
                {
                    CronJob &frontJob = _jobQueue.front();
                    LOGD(LOG_DOMAIN, "Wait from %ld till @%ld", _timeProvider->now(), frontJob.nextTime());
                    TimeProvider::State state = _timeProvider->waitUntil(lock, frontJob.nextTime());
                    std::time_t current = _timeProvider->now();
                    LOGD(LOG_DOMAIN, "Now @%ld %s", current,
                         state == TimeProvider::State::TimeChange ? " changed" : "");

                    for (CronJob &itJob : _jobQueue)
                    {
                        if (itJob.nextTime() <= current)
                        {
                            if (state != TimeProvider::State::TimeChange)
                            {
                                LOGI(LOG_DOMAIN, "Perform job '%s' for @%ld", itJob.name().c_str(), itJob.nextTime());
                                if (!itJob.perform())
                                {
                                    unsafeRemoveJob(itJob.id());
                                    return;
                                }
                            }
                            // job was executed well and it's continued job, so recalculate new execution timestamp
                            try
                            {
                                itJob.calcNext();
                            }
                            catch (const std::exception &ex)
                            {
                                // exception may occur only in the case when the job is one time and next time is
                                // unreachable
                                LOGD(LOG_DOMAIN, "Cannot calculate next time for job '%s'", itJob.name().c_str());
                                unsafeRemoveJob(itJob.id());
                            }
                        }
                    }
                    _jobQueue.sort();
                }
                else
                {
                    _timeProvider->wait(lock);
                }
            }
            catch (const std::exception &ex)
            {
                LOGE(LOG_DOMAIN, "Exception in worker thread: %s", ex.what());
            }
        } // while (working)
    }
};

Cron::UPtr CronFactory::create()
{
    return std::unique_ptr<Cron>(new CronImpl());
}
