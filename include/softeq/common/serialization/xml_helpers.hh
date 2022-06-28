#ifndef SOFTEQ_COMMON_SERIALIZATION_XML_HELPERS_H
#define SOFTEQ_COMMON_SERIALIZATION_XML_HELPERS_H

#include "softeq/common/serialization/helpers.hh"
#include "softeq/common/serialization/xml_serializer.hh"

namespace softeq
{
namespace common
{
namespace serialization
{

template <typename T>
std::string serializeObjectToXml(const T &object)
{
    XmlSerializer serializer;
    serializeObject(serializer, object);
    return serializer;
}

template <typename T>
void deserializeObjectFromXml(const std::string &xmlStr, T &object)
{
    XmlSerializer serializer;
    serializer.setRawInput(xmlStr);
    deserializeObject(serializer, object);
}

} // namespace serialization
} // namespace common
} // namespace softeq

#endif // SOFTEQ_COMMON_SERIALIZATION_XML_HELPERS_H