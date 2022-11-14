#ifndef SOFTEQ_COMMON_SERIALIZATION_JSON_HELPERS_H
#define SOFTEQ_COMMON_SERIALIZATION_JSON_HELPERS_H

#include <common/serialization/helpers.hh>

namespace softeq
{
namespace common
{
namespace serialization
{
namespace json
{
std::unique_ptr<StructSerializer> createStructSerializer();
std::unique_ptr<StructDeserializer> createStructDeserializer();

std::unique_ptr<ArraySerializer> createArraySerializer();
std::unique_ptr<ArrayDeserializer> createArrayDeserializer();

template <typename T>
std::string serializeAsJsonObject(const T &object)
{
    std::unique_ptr<StructSerializer> serializer = createStructSerializer();
    if (serializer)
    {
        serializeObject(*serializer, object);
        return serializer->dump();
    }
    else
    {
        return "";
    }
}

template <typename T>
T deserializeFromJsonObject(const std::string &jsonStr)
{
    T object;
    std::unique_ptr<StructDeserializer> deserializer = createStructDeserializer();
    if (deserializer)
    {
        deserializer->setRawInput(jsonStr);
        deserializeObject(*deserializer, object);
    }

    return object;
}

template <typename T>
std::string serializeAsJsonArray(const T &object)
{
    std::unique_ptr<ArraySerializer> serializer = createArraySerializer();
    if (serializer)
    {
        serializeObject(*serializer, object);
        return serializer->dump();
    }
    else
    {
        return "";
    }
}

template <typename T>
T deserializeFromJsonArray(const std::string &jsonStr)
{
    T object;
    std::unique_ptr<ArrayDeserializer> deserializer = createArrayDeserializer();
    if (deserializer)
    {
        deserializer->setRawInput(jsonStr);
        deserializeObject(*deserializer, object);
    }

    return object;
}

} // namespace json
} // namespace serialization
} // namespace common
} // namespace softeq

#endif // SOFTEQ_COMMON_SERIALIZATION_JSON_HELPERS_H
