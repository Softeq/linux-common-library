#ifndef SOFTEQ_COMMON_SERIALIZATION_JSON_HELPERS_H
#define SOFTEQ_COMMON_SERIALIZATION_JSON_HELPERS_H

#include "softeq/common/serialization/helpers.hh"
#include "softeq/common/serialization/json_serializer.hh"

namespace softeq
{
namespace common
{
namespace serialization
{

template <typename T>
std::string serializeObjectToJson(const T &object)
{
    JSonSerializer serializer;
    serializeObject(serializer, object);
    return serializer;
}

template <typename T>
T deserializeObjectFromJson(const std::string &jsonStr)
{
    T object;
    JSonSerializer serializer;
    serializer.setRawInput(jsonStr);
    deserializeObject(serializer, object);
    return object;
}

} // namespace serialization
} // namespace common
} // namespace softeq

#endif // SOFTEQ_COMMON_SERIALIZATION_JSON_HELPERS_H