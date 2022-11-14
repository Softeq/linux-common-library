#include <common/serialization/xml/xml.hh>

#include "xml_struct_deserializer.hh"
#include "xml_struct_serializer.hh"

#include "xml_array_deserializer.hh"
#include "xml_array_serializer.hh"

namespace softeq
{
namespace common
{
namespace serialization
{
namespace xml
{
void initMultiThreading()
{
    CompositeXmlDeserializer::initMultiThreading();
}

void cleanup()
{
    CompositeXmlDeserializer::cleanup();
}

std::unique_ptr<StructSerializer> createStructSerializer()
{
    return std::unique_ptr<StructSerializer>(new CompositeXmlSerializer());
}

std::unique_ptr<StructDeserializer> createStructDeserializer()
{
    return std::unique_ptr<StructDeserializer>(new CompositeXmlDeserializer());
}

std::unique_ptr<ArraySerializer> createArraySerializer()
{
    return std::unique_ptr<ArraySerializer>(new RootXmlArraySerializer());
}

std::unique_ptr<ArrayDeserializer> createArrayDeserializer()
{
    return std::unique_ptr<ArrayDeserializer>(new RootXmlArrayDeserializer());
}

} // namespace xml
} // namespace serialization
} // namespace common
} // namespace softeq
