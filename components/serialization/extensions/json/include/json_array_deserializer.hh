#ifndef SOFTEQ_COMMON_SERIALIZATION_JSON_ARRAY_DESERIALIZER_H
#define SOFTEQ_COMMON_SERIALIZATION_JSON_ARRAY_DESERIALIZER_H

#include <bits/c++config.h>
#include <common/serialization/deserializers.hh>

#include <nlohmann/json.hpp>

namespace softeq
{
namespace common
{
namespace serialization
{
namespace json
{
class JsonArrayDeserializer : public ArrayDeserializer
{
public:
    JsonArrayDeserializer() = default;
    explicit JsonArrayDeserializer(nlohmann::json jsonObject);

    ~JsonArrayDeserializer() override = default;

    void setRawInput(const std::string &textInput) override;

    softeq::common::stdutils::Any value() override;
    bool isComplete() const override;
    bool nextValueExists() const override;
    StructDeserializer *deserializeStruct() override;
    ArrayDeserializer *deserializeArray() override;
    std::size_t index() const override;

protected:
    nlohmann::json _jsonObject;
    std::size_t _index = 0;
};

class RootJsonArrayDeserializer : public JsonArrayDeserializer
{
public:
    RootJsonArrayDeserializer() = default;
    ~RootJsonArrayDeserializer() override = default;

private:
    ArrayDeserializer *deserializeArray() override;
};

} // namespace json
} // namespace serialization
} // namespace common
} // namespace softeq

#endif // SOFTEQ_COMMON_SERIALIZATION_JSON_ARRAY_DESERIALIZER_H
