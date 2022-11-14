#ifndef SOFTEQ_COMMON_SERIALIZATION_JSON_STRUCT_DESERIALIZER_H
#define SOFTEQ_COMMON_SERIALIZATION_JSON_STRUCT_DESERIALIZER_H

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
class CompositeJsonDeserializer : public StructDeserializer
{
public:
    CompositeJsonDeserializer() = default;
    explicit CompositeJsonDeserializer(const nlohmann::json &existingObject);

    ~CompositeJsonDeserializer() override = default;

    void setRawInput(const std::string &textInput) override;

    bool valueExists(const std::string &name) const override;
    std::vector<std::string> availableNames() const override;

    softeq::common::stdutils::Any value(const std::string &name) override;
    StructDeserializer *deserializeStruct(const std::string &name) override;
    ArrayDeserializer *deserializeArray(const std::string &name) override;

private:
    nlohmann::json _jsonObject;
};

} // namespace json
} // namespace serialization
} // namespace common
} // namespace softeq

#endif // SOFTEQ_COMMON_SERIALIZATION_JSON_STRUCT_DESERIALIZER_H
