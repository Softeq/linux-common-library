#include "json_utils.hh"

using namespace softeq::common;

namespace softeq
{
namespace common
{
namespace serialization
{
namespace json
{
stdutils::Any getPrimitiveDataFrom(const nlohmann::json &node)
{
    switch (node.type())
    {
    case nlohmann::detail::value_t::boolean:
        return stdutils::Any(node.get<bool>());
    case nlohmann::detail::value_t::number_unsigned:
        return stdutils::Any(node.get<uint64_t>());
    case nlohmann::detail::value_t::number_integer:
        return stdutils::Any(node.get<int64_t>());
    case nlohmann::detail::value_t::number_float:
        return stdutils::Any(node.get<double>());
    case nlohmann::detail::value_t::string:
        return stdutils::Any(node.get<std::string>());
    default:
        break;
    }

    return stdutils::Any();
}

} // namespace json
} // namespace serialization
} // namespace common
} // namespace softeq
