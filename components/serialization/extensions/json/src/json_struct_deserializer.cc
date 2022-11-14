#include "json_struct_deserializer.hh"
#include "json_array_deserializer.hh"

#include "json_utils.hh"

#include <sstream>

using namespace softeq::common;
using namespace softeq::common::serialization;
using namespace softeq::common::serialization::json;

namespace
{
const nlohmann::json *findNestedNotNullElement(const nlohmann::json &topLevelObject, const nlohmann::json &name)
{
    auto jsonObjectIterator = topLevelObject.find(name);
    if (jsonObjectIterator != topLevelObject.end())
    {
        const nlohmann::json &foundElement = *jsonObjectIterator;
        if (!foundElement.is_null())
        {
            return &foundElement;
        }
    }
    return nullptr;
}
} // anonymous namespace

CompositeJsonDeserializer::CompositeJsonDeserializer(const nlohmann::json &existingObject)
    : _jsonObject(existingObject)
{
}

void CompositeJsonDeserializer::setRawInput(const std::string &textInput)
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

bool CompositeJsonDeserializer::valueExists(const std::string &name) const
{
    return findNestedNotNullElement(_jsonObject, name) != nullptr;
}

std::vector<std::string> CompositeJsonDeserializer::availableNames() const
{
    std::vector<std::string> foundNames;
    foundNames.reserve(_jsonObject.size());
    for (auto it = _jsonObject.begin(); it != _jsonObject.end(); ++it)
    {
        foundNames.emplace_back(it.key());
    }
    return foundNames;
}

stdutils::Any CompositeJsonDeserializer::value(const std::string &name)
{
    nlohmann::json element = _jsonObject[name];
    if (element.is_primitive())
    {
        return getPrimitiveDataFrom(element);
    }
    return stdutils::Any();
}

serialization::StructDeserializer *CompositeJsonDeserializer::deserializeStruct(const std::string &name)
{
    const nlohmann::json *element = findNestedNotNullElement(_jsonObject, name);
    if (element)
    {
        try
        {
            return createStoredInternally<CompositeJsonDeserializer>(*element);
        }
        catch (const nlohmann::detail::type_error &ex)
        {
            throw ParseException(name, std::string("Expect object") + ex.what());
        }
    }

    return nullptr;
}

serialization::ArrayDeserializer *CompositeJsonDeserializer::deserializeArray(const std::string &name)
{
    const nlohmann::json *element = findNestedNotNullElement(_jsonObject, name);
    if (element)
    {
        try
        {
            return createStoredInternally<JsonArrayDeserializer>(*element);
        }
        catch (const nlohmann::detail::type_error &ex)
        {
            throw ParseException(name, std::string("Expect array") + ex.what());
        }
    }
    return nullptr;
}
