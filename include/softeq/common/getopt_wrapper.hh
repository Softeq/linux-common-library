#ifndef SOFTEQ_COMMON_GETOPT_WRAPPER_H_
#define SOFTEQ_COMMON_GETOPT_WRAPPER_H_

#include <cassert>
#include <getopt.h>
#include <string>
#include <vector>
#include <map>

#include <softeq/common/optional.hh>

/*
  \example ../examples/getopt.cpp
*/

namespace softeq
{
namespace common
{
/*!
  \brief The class is wrapper for parsing command-line options.
*/
class GetoptWrapper final
{
public:
    struct ParsedOption final
    {
        char shortName;
        Optional<std::string> value;
    };
    /*!
       ParsedOptions is a vector of ParsedOption
     */
    using ParsedOptions = std::vector<ParsedOption>;

    class DescOption final
    {
        friend class GetoptWrapper;

    public:
        enum class Arg
        {
            NONE,
            OPTIONAL,
            REQUIRED
        };

        DescOption(const std::string &longName, char shortName, Arg argument, const std::string &desc)
            : _longName(longName)
            , _shortName(shortName)
            , _argType(argument)
            , _description(desc)
            , _argsMap{{Arg::NONE, no_argument}, {Arg::OPTIONAL, optional_argument}, {Arg::REQUIRED, required_argument}}
        {
        }

        bool operator==(const DescOption &val) const
        {
            return _shortName == val._shortName || _longName == val._longName;
        }

        /*!
           Get long option name
           \return  String long option name
         */
        std::string longName() const
        {
            return _longName;
        }
        /*!
           Get short option name
           \return  String short option name
         */
        char shortName() const
        {
            return _shortName;
        }
        /*!
           Get option description
           \return  String description
         */
        std::string description() const
        {
            return _description;
        }
        /*!
           Get type of argument
           \return  String option name
         */
        Arg argType() const
        {
            return _argType;
        }

    protected:
        struct option rawOption() const
        {
            return {_longName.c_str(), _argsMap.at(_argType), nullptr, _shortName};
        }

    private:
        std::string _longName;
        char _shortName = 0;
        Arg _argType = Arg::NONE;
        std::string _description;
        const std::map<Arg, int> _argsMap;
    };

    using Argument = DescOption::Arg;
    using DescOptions = std::vector<DescOption>;

    GetoptWrapper(const DescOptions &opts);
    /*!
       Method to procces options
       \param[in] argc argument count
       \param[in] argv argument vector
       \param[in] failed_args
       \return Vector of options
     */
    ParsedOptions process(int argc, char **argv, bool *failed_args = nullptr) const;
    /*!
       Get help info
       \return Help info string
     */
    std::string getHelp() const;

private:
    enum class ProcessStatus
    {
        OK,
        BAD_OPTION,
        LAST_OPTION
    };

    ProcessStatus process(int argc, char **argv, int *index, GetoptWrapper::ParsedOption &parsed) const;

    std::string _optString;
    std::string _helpString;
    DescOptions _options;
    void setOptions(const DescOptions &opts);
};

} // namespace common
} // namespace softeq

#endif
