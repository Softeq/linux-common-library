#include "json_struct_serializer.hh"
#include "json_array_serializer.hh"

using namespace softeq::common;
using namespace softeq::common::serialization::json;

ProxyCompositeJsonSerializer::ProxyCompositeJsonSerializer(std::reference_wrapper<nlohmann::json> alreadyCreatedObject)
    : _rootJson(alreadyCreatedObject)
{
}

void ProxyCompositeJsonSerializer::serializeValueImpl(const std::string &name, int64_t value)
{
    _rootJson.get()[name] = value;
}

void ProxyCompositeJsonSerializer::serializeValueImpl(const std::string &name, uint64_t value)
{
    _rootJson.get()[name] = value;
}

void ProxyCompositeJsonSerializer::serializeValueImpl(const std::string &name, const std::string &value)
{
    _rootJson.get()[name] = value;
}

void ProxyCompositeJsonSerializer::serializeValueImpl(const std::string &name, double value)
{
    _rootJson.get()[name] = value;
}

void ProxyCompositeJsonSerializer::serializeValueImpl(const std::string &name, bool value)
{
    _rootJson.get()[name] = value;
}

serialization::StructSerializer *ProxyCompositeJsonSerializer::serializeStruct(const std::string &name)
{
    _rootJson.get()[name] = nlohmann::json::object();
    return createStoredInternally<ProxyCompositeJsonSerializer>(std::ref(_rootJson.get()[name]));
}

serialization::ArraySerializer *ProxyCompositeJsonSerializer::serializeArray(const std::string &name)
{
    _rootJson.get()[name] = nlohmann::json::array();
    return createStoredInternally<ProxyCompositeJsonArraySerializer>(std::ref(_rootJson.get()[name]));
}

std::string ProxyCompositeJsonSerializer::dump() const
{
    return _rootJson.get().dump();
}

CompositeJsonSerializer::CompositeJsonSerializer()
    : ProxyCompositeJsonSerializer(std::ref(_rootJson))
{
}
