#include "json_array_serializer.hh"
#include "json_struct_serializer.hh"

using namespace softeq::common;
using namespace softeq::common::serialization::json;

ProxyCompositeJsonArraySerializer::ProxyCompositeJsonArraySerializer(
    std::reference_wrapper<nlohmann::json> alreadyCreatedObject)
    : _jsonObject(alreadyCreatedObject)
{
}

void ProxyCompositeJsonArraySerializer::serializeValueImpl(int64_t value)
{
    _jsonObject.get().push_back(value);
}

void ProxyCompositeJsonArraySerializer::serializeValueImpl(uint64_t value)
{
    _jsonObject.get().push_back(value);
}

void ProxyCompositeJsonArraySerializer::serializeValueImpl(double value)
{
    _jsonObject.get().push_back(value);
}

void ProxyCompositeJsonArraySerializer::serializeValueImpl(bool value)
{
    _jsonObject.get().push_back(value);
}

void ProxyCompositeJsonArraySerializer::serializeValueImpl(const std::string &value)
{
    _jsonObject.get().push_back(value);
}

void ProxyCompositeJsonArraySerializer::serializeEmpty()
{
    _jsonObject.get().push_back(nlohmann::json());
}

serialization::ArraySerializer *ProxyCompositeJsonArraySerializer::serializeArray()
{
    _jsonObject.get().push_back(nlohmann::json::array());
    return createStoredInternally<ProxyCompositeJsonArraySerializer>(std::ref(_jsonObject.get().back()));
}

serialization::StructSerializer *ProxyCompositeJsonArraySerializer::serializeStruct()
{
    _jsonObject.get().push_back(nlohmann::json::object());
    return createStoredInternally<ProxyCompositeJsonSerializer>(std::ref(_jsonObject.get().back()));
}

std::string ProxyCompositeJsonArraySerializer::dump() const
{
    return _jsonObject.get().dump();
}

RootJsonArraySerializer::RootJsonArraySerializer()
    : ProxyCompositeJsonArraySerializer(std::ref(_rootJson))
{
}

// This override does not create new nested array because the root object is
// already an array
serialization::ArraySerializer *RootJsonArraySerializer::serializeArray()
{
    return createStoredInternally<ProxyCompositeJsonArraySerializer>(std::ref(_rootJson));
}
