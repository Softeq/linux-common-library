#include <common/system/getopt_wrapper.hh>

#include <iostream>
#include <cstdlib>

using namespace softeq::common::system;

namespace
{
const GetoptWrapper::DescOptions longopts = {
    {"required", 'r', GetoptWrapper::Argument::REQUIRED, "Argument is required"},
    {"optional", 'o', GetoptWrapper::Argument::OPTIONAL, "Argument is optional"},
    {"without", 'w', GetoptWrapper::Argument::NONE, "Argument is not needed"}};
}

int main(int argc, char **argv)
{
    const GetoptWrapper getOpt(longopts);
    bool failed = false;
    for (const GetoptWrapper::ParsedOption &option : getOpt.process(argc, argv, &failed))
    {
        if (failed)
            return EXIT_FAILURE;

        switch (option.shortName)
        {
        case 'r':
            if (!option.value.hasValue())
                return EXIT_FAILURE;
            std::cout << "Parameter 'required' exists, value=" << option.value.cValue() << std::endl;
            break;
        case 'o':
            std::cout << "Parameter 'optional' exists, value";
            if (option.value.hasValue())
                std::cout << "=" << option.value.cValue() << std::endl;
            else
                std::cout << " does not present" << std::endl;
            break;
        case 'w':
            std::cout << "Parameter 'without' exists" << std::endl;
            break;
        case 'h':
            std::cout << getOpt.getHelp() << std::endl;
            if (option.value.hasValue())
                std::cout << "Help page for argument " << option.value.cValue() << std::endl;
            return EXIT_SUCCESS;
        default:
            break;
        }
    }

    return EXIT_SUCCESS;
}
