#ifndef SOFTEQ_COMMON_SERIALIZATION_SERIALIZER_H
#define SOFTEQ_COMMON_SERIALIZATION_SERIALIZER_H

#include <cassert>
#include <cstdint>
#include <string>
#include <vector>

#include "softeq/common/any.hh"

namespace softeq
{
namespace common
{
namespace serialization
{

class ParseException : public std::logic_error
{
public:
    explicit ParseException(const std::string &what)
        : std::logic_error(what)
    {
    }
};

class NullNodeException : public std::logic_error
{
public:
    explicit NullNodeException(const std::string &what)
        : std::logic_error(what)
    {
    }
};

class Serializer
{
public:
    enum class CurrentObjectType
    {
        STRUCT,
        ARRAY
    };

    Serializer() = default;
    Serializer(const Serializer &) = delete;
    Serializer(Serializer &&) = delete;
    Serializer &operator=(const Serializer &) = delete;
    Serializer &operator=(Serializer &&) = delete;

    virtual ~Serializer() = default;
    virtual void name(const std::string &) = 0;

    virtual void beginObjectSerialization(CurrentObjectType type) = 0;

    template <typename T, typename std::enable_if<std::is_arithmetic<T>::value, int>::type = 0>
    void value(const T &v)
    {
        if (std::is_floating_point<T>::value)
        {
            valuePrimitive(static_cast<double>(v));
        }
        else if (std::is_same<T, bool>::value)
        {
            valuePrimitive(static_cast<bool>(v));
        }
        else if (std::is_unsigned<T>::value)
        {
            valuePrimitive(static_cast<uint64_t>(v));
        }
        else if (std::is_signed<T>::value)
        {
            valuePrimitive(static_cast<int64_t>(v));
        }
        else
        {
            assert(0);
        }
    }

    void value(const std::string &v)
    {
        valuePrimitive(v);
    }

    virtual void endObjectSerialization() = 0;

    virtual size_t beginObjectDeserialization(CurrentObjectType type) = 0;
    virtual Any value() = 0;
    virtual void endObjectDeserialization() = 0;

    virtual bool hasNamedNode() const = 0;
    virtual CurrentObjectType getCurrentObjectType() const = 0;

    virtual std::string dump() const = 0;
    virtual operator std::string() const = 0;
    virtual const std::vector<std::string> getNodeKeys() const = 0;

    virtual void setRawInput(const std::string &input) = 0;

protected:
    virtual void valuePrimitive(bool v) = 0;
    virtual void valuePrimitive(int64_t v) = 0;
    virtual void valuePrimitive(uint64_t v) = 0;
    virtual void valuePrimitive(double v) = 0;
    virtual void valuePrimitive(const std::string &v) = 0;
};

} // namespace serialization
} // namespace common
} // namespace softeq

#endif // SOFTEQ_COMMON_SERIALIZATION_SERIALIZER_H_