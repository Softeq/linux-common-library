#ifndef SOFTEQ_COMMON_SERIALIZATION_XML_HH
#define SOFTEQ_COMMON_SERIALIZATION_XML_HH

#include <common/serialization/helpers.hh>

namespace softeq
{
namespace common
{
namespace serialization
{
namespace xml
{
void initMultiThreading();
void cleanup();

std::unique_ptr<StructSerializer> createStructSerializer();
std::unique_ptr<StructDeserializer> createStructDeserializer();

std::unique_ptr<ArraySerializer> createArraySerializer();
std::unique_ptr<ArrayDeserializer> createArrayDeserializer();

void initMultiThreading();
void cleanup();

template <typename T>
std::string serializeAsXmlObject(const T &object)
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
T deserializeFromXmlObject(const std::string &xmlStr)
{
    T object;
    std::unique_ptr<StructDeserializer> deserializer = createStructDeserializer();
    if (deserializer)
    {
        deserializer->setRawInput(xmlStr);
        deserializeObject(*deserializer, object);
    }

    return object;
}

template <typename T>
std::string serializeAsXmlArray(const T &object)
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
T deserializeFromXmlArray(const std::string &xmlStr)
{
    T object;
    std::unique_ptr<ArrayDeserializer> deserializer = createArrayDeserializer();
    if (deserializer)
    {
        deserializer->setRawInput(xmlStr);
        deserializeObject(*deserializer, object);
    }

    return object;
}

} // namespace xml
} // namespace serialization
} // namespace common
} // namespace softeq

#endif // SOFTEQ_COMMON_SERIALIZATION_XML_HH
