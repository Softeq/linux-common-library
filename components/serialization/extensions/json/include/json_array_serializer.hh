#ifndef SOFTEQ_COMMON_SERIALIZATION_JSON_ARRAY_H
#define SOFTEQ_COMMON_SERIALIZATION_JSON_ARRAY_H

#include <common/serialization/serializers.hh>
#include <nlohmann/json.hpp>

namespace softeq
{
namespace common
{
namespace serialization
{
namespace json
{

class ProxyCompositeJsonArraySerializer : public ArraySerializer
{
public:
    explicit ProxyCompositeJsonArraySerializer(
            std::reference_wrapper<nlohmann::json> alreadyCreatedObject);

    ~ProxyCompositeJsonArraySerializer() override = default;

    std::string dump() const override;

private:
    void serializeValueImpl(int64_t value) override;
    void serializeValueImpl(uint64_t value) override;
    void serializeValueImpl(double value) override;
    void serializeValueImpl(bool value) override;
    void serializeValueImpl(const std::string &value) override;

    void serializeEmpty() override;

    ArraySerializer* serializeArray() override;
    StructSerializer* serializeStruct() override;

    std::reference_wrapper<nlohmann::json> _jsonObject;
};

class RootJsonArraySerializer : public ProxyCompositeJsonArraySerializer
{
public:
    RootJsonArraySerializer();
    ~RootJsonArraySerializer() override = default;

private:
    ArraySerializer* serializeArray() override;

    nlohmann::json _rootJson = nlohmann::json::array();
};

} // namespace json
} // namespace serialization
} // namespace common
} // namespace softeq

#endif //SOFTEQ_COMMON_SERIALIZATION_JSON_ARRAY_H

