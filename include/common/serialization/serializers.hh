#ifndef SOFTEQ_COMMON_SERIALIZATION_SERIALIZERS_H
#define SOFTEQ_COMMON_SERIALIZATION_SERIALIZERS_H

#include <string>

#include <common/serialization/details/internal_pointers_storage.hh>
#include <common/serialization/details/serialization_types.hh>

namespace softeq
{
namespace common
{
namespace serialization
{

class StructSerializer;
class ArraySerializer;

class Serializable : public ContainsInternalPointersStorage<Serializable>
{
public:
    virtual ~Serializable() = default;

    virtual std::string dump() const = 0;
};

template<typename T>
using TypeIsString =
    typename std::enable_if<std::is_same<T, std::string>::value, bool>::type;

template<typename T>
using TypeIsNotString =
    typename std::enable_if<!std::is_same<T, std::string>::value, bool>::type;

class StructSerializer : public Serializable
{
public:
    virtual ~StructSerializer() = default;

    template<typename T, TypeIsNotString<T> = true>
    void serializeValue(const std::string &name, T value)
    {
        serializeValueImpl(name,
                           static_cast<typename SerializationTypeFor<T>::type>(value));
    }

    template<typename T, TypeIsString<T> = true>
    void serializeValue(const std::string &name, T value)
    {
        serializeValueImpl(name, value);
    }

    virtual StructSerializer* serializeStruct(const std::string &name) = 0;
    virtual ArraySerializer* serializeArray(const std::string &name) = 0;
    
protected:
    virtual void serializeValueImpl(const std::string &name, int64_t value) = 0;
    virtual void serializeValueImpl(const std::string &name, uint64_t value) = 0;
    virtual void serializeValueImpl(const std::string &name, double value) = 0;
    virtual void serializeValueImpl(const std::string &name, bool value) = 0;
    virtual void serializeValueImpl(const std::string &name, const std::string &value) = 0;
private:
};

class ArraySerializer : public Serializable
{
public:
    virtual ~ArraySerializer() = default;

    template<typename T, TypeIsNotString<T> = true>
    void serializeValue(T value)
    {
        serializeValueImpl(static_cast<typename SerializationTypeFor<T>::type>(value));
    }

    template<typename T, TypeIsString<T> = true>
    void serializeValue(T value)
    {
        serializeValueImpl(value);
    }

    virtual void serializeEmpty() = 0;

    virtual ArraySerializer* serializeArray() = 0;
    virtual StructSerializer* serializeStruct() = 0;

protected:
    virtual void serializeValueImpl(int64_t value) = 0;
    virtual void serializeValueImpl(uint64_t value) = 0;
    virtual void serializeValueImpl(double value) = 0;
    virtual void serializeValueImpl(bool value) = 0;
    virtual void serializeValueImpl(const std::string &value) = 0;
};

} // namespace serialization
} // namespace common
} // namespace softeq

#endif // SOFTEQ_COMMON_SERIALIZATION_SERIALIZERS_H_
