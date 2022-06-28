#ifndef SOFTEQ_COMMON_SERIALIZATION_HELPERS_H
#define SOFTEQ_COMMON_SERIALIZATION_HELPERS_H

#include "softeq/common/serialization/object_assembler.hh"

namespace softeq
{
namespace common
{
namespace serialization
{

template <typename T>
void serializeObject(Serializer &serializer, const T &object)
{
    ObjectAssembler<T>::accessor().serialize(serializer, object);
}

template <typename T>
void deserializeObject(Serializer &serializer, T &object)
{
    ObjectAssembler<T>::accessor().deserialize(serializer, object);
}

template <typename T>
std::string getObjectGraph()
{
    return ObjectAssembler<T>::accessor().graph();
}

template <typename T>
std::string getObjectGraph(const T &obj)
{
    (void)obj;
    return getObjectGraph<T>();
}

} // namespace serialization
} // namespace common
} // namespace softeq

#endif // SOFTEQ_COMMON_SERIALIZATION_HELPERS_H