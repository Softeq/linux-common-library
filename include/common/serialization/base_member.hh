#ifndef SOFTEQ_COMMON_SERIALIZATION_BASE_MEMBER_H
#define SOFTEQ_COMMON_SERIALIZATION_BASE_MEMBER_H

#include <common/serialization/serializers.hh>
#include <common/serialization/deserializers.hh>

#include <cassert>

namespace softeq
{
namespace common
{
namespace serialization
{
template <typename Base>
class BaseMember
{
public:
    using Ptr = std::shared_ptr<BaseMember<Base>>;

    explicit BaseMember(const std::string &name)
        : _name(name)
    {
    }

    virtual ~BaseMember() = default;

    inline const std::string &name() const
    {
        return _name;
    }

    virtual void serialize(StructSerializer &serializer, const Base &node) const = 0;
    virtual void deserialize(StructDeserializer &deserializer, Base &node) const = 0;
    virtual std::string graph(const std::string &assignedNodeName) const = 0;
    virtual bool operator==(const BaseMember<Base> &member) const = 0;
    virtual const std::type_info &type() const = 0;

private:
    BaseMember(const BaseMember &) = delete;
    BaseMember(BaseMember &&) = delete;
    BaseMember &operator=(const BaseMember &) = delete;
    BaseMember &operator=(BaseMember &&) = delete;

    const std::string _name;
};

} // namespace serialization
} // namespace common
} // namespace softeq

#endif // SOFTEQ_COMMON_SERIALIZATION_BASE_MEMBER_H
