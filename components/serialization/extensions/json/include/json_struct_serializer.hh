#ifndef SOFTEQ_COMMON_SERIALIZATION_JSON_STRUCT_H
#define SOFTEQ_COMMON_SERIALIZATION_JSON_STRUCT_H

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
class ProxyCompositeJsonSerializer : public StructSerializer
{
public:
    explicit ProxyCompositeJsonSerializer(std::reference_wrapper<nlohmann::json> alreadyCreatedObject);

    StructSerializer *serializeStruct(const std::string &name) override;
    ArraySerializer *serializeArray(const std::string &name) override;
    std::string dump() const override;

private:
    void serializeValueImpl(const std::string &name, const std::string &value) override;
    void serializeValueImpl(const std::string &name, int64_t value) override;
    void serializeValueImpl(const std::string &name, uint64_t value) override;
    void serializeValueImpl(const std::string &name, double value) override;
    void serializeValueImpl(const std::string &name, bool value) override;

    std::reference_wrapper<nlohmann::json> _rootJson;
};

class CompositeJsonSerializer : public ProxyCompositeJsonSerializer
{
public:
    CompositeJsonSerializer();

private:
    nlohmann::json _rootJson = nlohmann::json::object();
};

} // namespace json
} // namespace serialization
} // namespace common
} // namespace softeq

#endif // SOFTEQ_COMMON_SERIALIZATION_JSON_STRUCT_H
