#include "getopt_wrapper.hh"

#include <common/stdutils/stdutils.hh>

#include <algorithm>
#include <string>
#include <sstream>
#include <unordered_map>
#include <iostream>

using namespace softeq::common::system;
using namespace softeq::common::stdutils;

namespace
{
const GetoptWrapper::DescOptions systemOpts = {{"help", 'h', GetoptWrapper::Argument::OPTIONAL, "This help"}};

void checkForDuplicates(const GetoptWrapper::DescOptions &opts)
{
    int errcnt = 0;
    for (const GetoptWrapper::DescOption &systemOpt : systemOpts)
    {
        if (std::find_if(opts.begin(), opts.end(),
                         [&systemOpt](const GetoptWrapper::DescOption &val) { return systemOpt == val; }) != opts.end())
        {
            errcnt++;
            std::cerr << systemOpt.shortName() << "(" << systemOpt.longName()
                      << ") - is predefined option, but was duplicated externally." << std::endl;
        }
    }

    // count inner duplicates
    std::map<char, int> shortDupMap;
    std::map<std::string, int> longDupMap;
    for (const GetoptWrapper::DescOption &option : opts)
    {
        shortDupMap[option.shortName()]++;
        longDupMap[option.longName()]++;
    }
    for (const auto &shortDup : shortDupMap)
    {
        if (shortDup.second > 1)
        {
            errcnt += shortDup.second - 1;
            std::cerr << "Short name of parameter'" << shortDup.first << "' is duplicated" << std::endl;
        }
    }
    for (const auto &longDup : longDupMap)
    {
        if (longDup.second > 1)
        {
            errcnt += longDup.second - 1;
            std::cerr << "Long name of parameter'" << longDup.first << "' is duplicated" << std::endl;
        }
    }

    if (errcnt)
    {
        throw std::invalid_argument(string_format("%s(): duplicates found - %zu", __func__, errcnt));
    }
}

} // namespace

GetoptWrapper::GetoptWrapper(const DescOptions &opts)
{
    setOptions(opts);
}

void GetoptWrapper::setOptions(const DescOptions &opts)
{
    std::stringstream ssOpt;
    std::stringstream ssHelp;

    checkForDuplicates(opts);

    for (const DescOptions *category_options : {&systemOpts, &opts})
    {
        for (const DescOption &option : *category_options)
        {
            ssOpt << option.shortName();
            ssHelp << " -" << option.shortName() << " ";
            switch (option.argType())
            {
            case Argument::OPTIONAL:
                ssHelp << "[" << option.longName() << " argument]";
                ssOpt << "::";
                break;
            case Argument::REQUIRED:
                ssHelp << "<" << option.longName() << " argument>";
                ssOpt << ':';
                break;
            case Argument::NONE:
                ssHelp << "\t";
                break;
            }

            ssHelp << "\t--" << option.longName();
            ssHelp << "\t\t" << option.description() << std::endl;
        }
    }

    _options.clear();
    _options.reserve(opts.size() + systemOpts.size() + 1);
    std::copy(opts.begin(), opts.end(), std::back_inserter(_options));
    std::copy(systemOpts.begin(), systemOpts.end(), std::back_inserter(_options));

    _optString = ssOpt.str();
    _helpString = ssHelp.str();
}

std::string GetoptWrapper::getHelp() const
{
    return _helpString;
}

GetoptWrapper::ParsedOptions GetoptWrapper::process(int argc, char **argv, bool *failed_args) const
{
    int index = -1;
    ParsedOptions result;
    ParsedOption opt;
    ProcessStatus status;
    bool failed = false;

    while ((status = process(argc, argv, &index, opt)) != ProcessStatus::LAST_OPTION)
    {
        switch (status)
        {
        case ProcessStatus::OK:
            result.push_back(opt);
            break;
        case ProcessStatus::BAD_OPTION:
            failed = true;
            break;
        default:
            break;
        }
    }

    if (failed_args)
        *failed_args = failed;
    if (failed)
    {
        std::cerr << "Possible params are:\n" << getHelp() << std::endl;
    }
    optind = 1;
    return result;
}

GetoptWrapper::ProcessStatus GetoptWrapper::process(int argc, char **argv, int *index, ParsedOption &parsed) const
{
    std::vector<struct option> longopts;
    longopts.reserve(_options.size() + 1);
    std::transform(_options.begin(), _options.end(), std::back_inserter(longopts),
                   [](const DescOption &val) { return val.rawOption(); });
    longopts.push_back({nullptr, 0, nullptr, 0});

    int val = getopt_long(argc, argv, _optString.c_str(), longopts.data(), index);

    ProcessStatus status = ProcessStatus::OK;

    switch (val)
    {
    case '?':
    case ':':
        status = ProcessStatus::BAD_OPTION;
        break;
    case -1:
        status = ProcessStatus::LAST_OPTION;
        break;
    default:
        parsed.shortName = char(val);
        parsed.value = optarg ? Optional<std::string>(optarg) : Optional<std::string>();
        break;
    }
    return status;
}
