#ifndef SOFTEQ_COMMON_SERIALIZATION_JSON_DESERIALIZATION_UTILS_H
#define SOFTEQ_COMMON_SERIALIZATION_JSON_DESERIALIZATION_UTILS_H

#include <nlohmann/json.hpp>
#include <common/stdutils/any.hh>

namespace softeq
{
namespace common
{
namespace serialization
{
namespace json
{

softeq::common::stdutils::Any getPrimitiveDataFrom(const nlohmann::json &node);

} // namespace json
} // namespace serialization
} // namespace common
} // namespace softeq

#endif //SOFTEQ_COMMON_SERIALIZATION_JSON_DESERIALIZATION_UTILS_H

