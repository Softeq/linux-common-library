#ifndef SOFTEQ_COMMON_SERIALIZATION_DESERIALIZERS_H
#define SOFTEQ_COMMON_SERIALIZATION_DESERIALIZERS_H

#include <bits/c++config.h>
#include <common/stdutils/any.hh>

#include <common/serialization/details/internal_pointers_storage.hh>

#include <string>
#include <stdexcept>

namespace softeq
{
namespace common
{
namespace serialization
{
// TODO: need parents node info
class ParseException : public std::logic_error
{
    std::string _node;

public:
    explicit ParseException(const std::string &node, const std::string &what)
        : std::logic_error(what)
        , _node(node)
    {
    }
};

class Deserializable : public ContainsInternalPointersStorage<Deserializable>
{
public:
    virtual ~Deserializable() = default;

    virtual void setRawInput(const std::string &textInput) = 0;
};

class StructDeserializer;
class ArrayDeserializer;

class StructDeserializer : public Deserializable
{
public:
    virtual ~StructDeserializer() = default;

    virtual bool valueExists(const std::string &name) const = 0;
    virtual std::vector<std::string> availableNames() const = 0;

    virtual softeq::common::stdutils::Any value(const std::string &name) = 0;

    virtual StructDeserializer *deserializeStruct(const std::string &name) = 0;
    virtual ArrayDeserializer *deserializeArray(const std::string &name) = 0;
};

class ArrayDeserializer : public Deserializable
{
public:
    virtual ~ArrayDeserializer() = default;

    virtual softeq::common::stdutils::Any value() = 0;
    virtual std::size_t index() const = 0;
    virtual bool isComplete() const = 0;
    virtual bool nextValueExists() const = 0;

    virtual StructDeserializer *deserializeStruct() = 0;
    virtual ArrayDeserializer *deserializeArray() = 0;
};

} // namespace serialization
} // namespace common
} // namespace softeq

#endif // SOFTEQ_COMMON_SERIALIZATION_DESERIALIZERS_H
