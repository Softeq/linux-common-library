#include "custom_type.hh"

#include <map>

using namespace softeq::common::serialization;

std::map<int, std::string> digitsNames = // clang-format off
{
    {1, "one"},
    {2, "two"},
    {3, "three"},
    {4, "four"},
    {5, "five"}
}; // clang-format on

std::string nameDigit(const int &digit)
{
    return digitsNames.at(digit);
};

int digitByName(const std::string &name)
{
    auto it = std::find_if(digitsNames.cbegin(), digitsNames.cend(),
                           [&name](const std::pair<int, std::string> &record) { return record.second == name; });
    if (it != digitsNames.cend())
    {
        return (*it).first;
    }
    else
    {
        return 0;
    }
}

namespace softeq
{
namespace common
{
namespace serialization
{
template <>
ObjectAssembler<OneDigitStruct> Assembler()
{
    return ObjectAssembler<OneDigitStruct>().defineAs<std::string>("digit", &OneDigitStruct::digit, nameDigit,
                                                                   digitByName);

    // clang-format on
}

} // namespace serialization
} // namespace common
} // namespace softeq

