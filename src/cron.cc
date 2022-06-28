#include "softeq/common/cron.hh"

#include "softeq/common/exceptions.hh"
#include "softeq/common/log.hh"
#include "softeq/common/stdutils.hh"
#include "softeq/common/timeutils.hh"

#include <algorithm>
#include <condition_variable>
#include <list>
#include <atomic>
#include <bitset>

using namespace softeq::common;
using namespace softeq::common::time;

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
        throw ICron::InvalidRule(e.what());
    }
}
} // namespace

class CronJob final
{

    using SystemError = softeq::common::SystemError;

    static const int MAX_YEAR = 2099;
    
    struct candidate_t final
    {
        int year;
        int mon;
        int mday;
        int hour;
        int min;

        candidate_t()
        {
            time_t rawtime = std::time(nullptr);
            struct tm time;
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
                throw SystemError();
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
            struct tm tm = {0};
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
                    throw ICron::InvalidRule("Expected only one '/' delimiter per rule");
                }
                _each = cron_stoi(parts[1]);
                srule = parts[0];
            }

            if (srule == "*")
            {
                if (_each < 1 || _each > maxVal)
                {
                    throw ICron::Unsatisfiable("Defined interval is out of range");
                }
            }
            else if (srule.find('-') != std::string::npos)
            {
                std::vector<std::string> parts(stdutils::string_split(srule, '-'));
                if (parts.size() != 2)
                {
                    throw ICron::InvalidRule("Expected only one '-' delimiter per rule");
                }

                _min = cron_stoi(parts[0]);
                _max = cron_stoi(parts[1]);

                if (_max < _min)
                {
                    throw ICron::InvalidRule("Wrong placement of the min and max borders of the range");
                }
            }
            else
            {
                _min = _max = cron_stoi(srule); // single number
            }

            if (_min < minVal || _max > maxVal)
            {
                throw ICron::Unsatisfiable("Single value or a sub-range is out of main range");
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
        static const int _kDaysInWeek = 7;
        std::bitset<_kDaysInWeek> _validDays;

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
                    throw ICron::InvalidRule("Expected only one '-' delimiter per rule");
                }

                int fromDoW = cron_stoi(parts[0]);
                int toDoW = cron_stoi(parts[1]);

                if (toDoW < fromDoW)
                {
                    throw ICron::InvalidRule("Wrong placement of the min and max borders of the range");
                }
                if (fromDoW < 0 || fromDoW > _kDaysInWeek || toDoW < 0 || toDoW > _kDaysInWeek)
                {
                    throw ICron::InvalidRule("Number of a day should be in the range [0;7]");
                }

                for (int dow = fromDoW; dow <= toDoW; ++dow)
                {
                    _validDays[dow % _kDaysInWeek] = true;
                }
            }
            else
            {
                for (const std::string &part : stdutils::string_split(_expr, ','))
                {
                    const int dow{cron_stoi(part)};

                    if (dow > _kDaysInWeek)
                    {
                        throw ICron::InvalidRule("Number of a day should be in the range [0;7], got " +
                                                 std::to_string(dow));
                    }

                    _validDays[dow % _kDaysInWeek] = true;
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
            const int dow = week_day(candidate.mday, candidate.mon, candidate.year);
            if (dow >= 0 && dow < _kDaysInWeek)
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
            const int max_days = days_per_month(candidate.mon, candidate.year);
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
    bool _continued;
    time_t _nextT;
    ICron::Callback _cb;
    ICron::JobId _id;

public:
    CronJob(std::string name, std::string timespec, bool continued, ICron::Callback cb, ICron::JobId id)
        : _name(std::move(name))
        , _timespec(std::move(timespec))
        , _continued(continued)
        , _cb(std::move(cb))
        , _id(id)
    {
        if (!_cb)
        {
            throw std::invalid_argument("invalid callback");
        }

        _nextT = calcNext();
    }

    time_t nextT() const noexcept
    {
        return _nextT;
    }
    ICron::JobId id() const noexcept
    {
        return _id;
    }
    bool continued() const noexcept
    {
        return _continued;
    }
    const std::string &name() const noexcept
    {
        return _name;
    }

    bool perform()
    {
        return _cb();
    }

    time_t calcNext()
    {
        candidate_t candidate;
        std::vector<std::string> parts(stdutils::string_split(_timespec, ' '));
        if (parts.size() != 6)
            throw std::logic_error("must be 6 parts");

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
                throw(ICron::Unsatisfiable("Candidate's year field is out of the range"));
            }
        } while (!valid);

        LOGT(LOG_DOMAIN, "Next time for job %s is %s", _name.c_str(), candidate.toString().c_str());
        _nextT = candidate.toUnixTimestamp();
        return _nextT;
    }
};

typedef std::shared_ptr<CronJob> CronJobSPtr;
bool operator<(const CronJobSPtr &lhs, const CronJobSPtr &rhs)
{
    if (!(lhs && rhs))
        throw std::invalid_argument("operator<(CronJobSPtr)");

    return lhs->nextT() < rhs->nextT();
}

class Cron final : public ICron
{
    std::list<CronJobSPtr> _jobQueue;
    std::thread _jobThread;
    std::condition_variable _condVar;
    std::mutex _queueMutex;
    std::atomic_bool _working{false};
    JobId _nextJobId{1};

public:
    Cron() = default;

    ~Cron() override
    {
        stop();
    }

    /// Adds a function to call according to that timespec
    JobId addJob(const std::string &name, const std::string &timespec, bool continued, ICron::Callback cb) override
    {
        JobId jobId = kInvalidJobId;

        if (!cb)
        {
            LOGE(LOG_DOMAIN, "Couldn't add job '%s' due to invalid callback.", name.c_str());
            return jobId;
        }

        try
        {
            std::unique_lock<std::mutex> lock(_queueMutex);
            _jobQueue.emplace_back(std::make_shared<CronJob>(name, timespec, continued, cb, _nextJobId));
            jobId = _jobQueue.back()->id();
            _jobQueue.sort();
            _nextJobId++;
            lock.unlock();
            _condVar.notify_one();
        }
        catch (const std::exception &ex)
        {
            LOGE(LOG_DOMAIN, "Unable to add job '%s' Reason: %s", timespec.c_str(), ex.what());
            return kInvalidJobId;
        }

        return jobId;
    }

    time_t jobExecutionTime(JobId jobId) override
    {
        std::lock_guard<std::mutex> lock(_queueMutex);

        if (!_jobQueue.empty())
        {
            auto iter = std::find_if(_jobQueue.begin(), _jobQueue.end(),
                                     [jobId](const CronJobSPtr &jobSPtr) { return jobSPtr->id() == jobId; });

            if (iter != _jobQueue.end())
            {
                return (*iter)->nextT();
            }
        }

        return 0;
    }

    void removeJob(JobId jobId) override
    {
        std::lock_guard<std::mutex> lock(_queueMutex);
        _removeJob(jobId);
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
        _jobThread = std::thread(&Cron::worker, this);

        return true;
    }

    /// Stops the cron daemon
    void stop() noexcept override
    {
        if (!_working)
            return;

        LOGI(LOG_DOMAIN, "Stopping, %d tasks in queue.", static_cast<int>(_jobQueue.size()));
        _working = false;
        _condVar.notify_one();

        if (_jobThread.joinable())
        {
            _jobThread.join();
        }
        // job_queue.clear();
        LOGD(LOG_DOMAIN, "Stopped.");
    }

private:
    void _removeJob(JobId jobId)
    {
        if (!_jobQueue.empty())
        {
            _jobQueue.remove_if([jobId](const CronJobSPtr &jobSPtr) { return jobSPtr->id() == jobId; });
        }
    }

    void worker()
    {
        while (_working)
        {
            try
            {
                std::unique_lock<std::mutex> lock(_queueMutex);
                if (!_jobQueue.empty())
                {
                    // wait until time comes for job execution (by timeout) or job is added/removed
                    _condVar.wait_until(lock, std::chrono::system_clock::from_time_t(_jobQueue.front()->nextT()));
                    processNextJob();
                }
                else
                {
                    _condVar.wait(lock);
                }
            }
            catch (const std::exception &ex)
            {
                LOGE(LOG_DOMAIN, "Exception in worker thread: %s", ex.what());
            }
        } // while (working)
    }

    void processNextJob()
    {
        if (_jobQueue.empty())
            return;

        CronJobSPtr job = _jobQueue.front();

        // an exception during job execution or calculating of next time mustn't stop worker thread
        try
        {
            if (std::time(nullptr) >= job->nextT())
            {
                LOGI(LOG_DOMAIN, "Perform job '%s'", job->name().c_str());
                if (!job->perform() || !job->continued())
                {
                    _removeJob(job->id());
                    return;
                }

                // job was executed well and it's continued job, so recalculate new execution timestamp
                job->calcNext();
            }

            _jobQueue.sort();
        }
        catch (const std::exception &ex)
        {
            LOGE(LOG_DOMAIN, "Exception when execution job or calculating next time: %s", ex.what());
            _removeJob(job->id());
        }
    }
};

ICron::UPtr CronFactory::create()
{
    return std::unique_ptr<ICron>(new Cron());
}
