#include "json_array_deserializer.hh"
#include "json_struct_deserializer.hh"
#include "json_utils.hh"

#include <bits/c++config.h>
#include <nlohmann/json.hpp>
#include <sstream>

using namespace softeq::common;
namespace softeq
{
namespace common
{
namespace serialization
{
namespace json
{
JsonArrayDeserializer::JsonArrayDeserializer(nlohmann::json jsonObject)
    : _jsonObject(jsonObject)
{
}

void JsonArrayDeserializer::setRawInput(const std::string &textInput)
{
    std::stringstream inputStream(textInput);
    try
    {
        inputStream >> _jsonObject;
    }
    catch (const nlohmann::detail::parse_error &ex)
    {
        throw ParseException("", ex.what());
    }
}

stdutils::Any JsonArrayDeserializer::value()
{
    if (_index < _jsonObject.size())
    {
        nlohmann::json element = _jsonObject[_index];
        if (element.is_primitive())
        {
            // move to next element only if value was returned
            ++_index;
            return getPrimitiveDataFrom(element);
        }
    }
    return stdutils::Any();
}

std::size_t JsonArrayDeserializer::index() const
{
    return _index;
}

serialization::StructDeserializer *JsonArrayDeserializer::deserializeStruct()
{
    if (_index < _jsonObject.size())
    {
        try
        {
            return createStoredInternally<CompositeJsonDeserializer>(_jsonObject[_index++]);
        }
        catch (const nlohmann::detail::type_error &ex)
        {
            throw ParseException(std::to_string(_index), std::string("Expect object by index. ") + ex.what());
        }
    }
    return nullptr;
}

serialization::ArrayDeserializer *JsonArrayDeserializer::deserializeArray()
{
    if (_index < _jsonObject.size())
    {
        try
        {
            return createStoredInternally<JsonArrayDeserializer>(_jsonObject[_index++]);
        }
        catch (const nlohmann::detail::type_error &ex)
        {
            throw ParseException(std::to_string(_index), std::string("Expect array by index ") + ex.what());
        }
    }
    return nullptr;
}

bool JsonArrayDeserializer::isComplete() const
{
    return _index == _jsonObject.size();
}

bool JsonArrayDeserializer::nextValueExists() const
{
    return isComplete() ? false : !(_jsonObject[_index].is_null());
}

// Similar as for serialization the array deserialization starts for current level because
// the root is an array
serialization::ArrayDeserializer *RootJsonArrayDeserializer::deserializeArray()
{
    try
    {
        return createStoredInternally<JsonArrayDeserializer>(_jsonObject);
    }
    catch (const nlohmann::detail::type_error &ex)
    {
        throw ParseException("", std::string("Expect array on the top. ") + ex.what());
    }
}
} // namespace json
} // namespace serialization
} // namespace common
} // namespace softeq
