#include <common/serialization/json/json.hh>

#include "json_struct_serializer.hh"
#include "json_array_serializer.hh"

#include "json_struct_deserializer.hh"
#include "json_array_deserializer.hh"

namespace softeq
{
namespace common
{
namespace serialization
{
namespace json
{
std::unique_ptr<StructSerializer> createStructSerializer()
{
    return std::unique_ptr<StructSerializer>(new CompositeJsonSerializer());
}

std::unique_ptr<StructDeserializer> createStructDeserializer()
{
    return std::unique_ptr<StructDeserializer>(new CompositeJsonDeserializer());
}

std::unique_ptr<ArraySerializer> createArraySerializer()
{
    return std::unique_ptr<ArraySerializer>(new RootJsonArraySerializer());
}

std::unique_ptr<ArrayDeserializer> createArrayDeserializer()
{
    return std::unique_ptr<ArrayDeserializer>(new RootJsonArrayDeserializer());
}

} // namespace json
} // namespace serialization
} // namespace common
} // namespace softeq
