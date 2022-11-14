#ifndef SOFTEQ_COMMON_SERIALIZATION_SERIALIZATION_TYPES_H
#define SOFTEQ_COMMON_SERIALIZATION_SERIALIZATION_TYPES_H

#include <type_traits>

namespace softeq
{
namespace common
{
namespace serialization
{

template<typename T, typename Enable = void>
struct SerializationTypeFor;

template<typename T>
struct SerializationTypeFor<T, typename std::enable_if<
                                            !std::is_same<T, bool>::value
                                            && std::is_integral<T>::value
                                            && std::is_unsigned<T>::value>::type>
{
    using type = uint64_t;
};

template<typename T>
struct SerializationTypeFor<T, typename std::enable_if<
                                            !std::is_same<T, bool>::value
                                            && std::is_integral<T>::value
                                            && std::is_signed<T>::value>::type>
{
    using type = int64_t;
};

template<typename T>
struct SerializationTypeFor<T, typename std::enable_if<
                                            std::is_floating_point<T>::value>::type>
{
    using type = double;
};

template<typename T>
struct SerializationTypeFor<T, typename std::enable_if<
                                            std::is_same<T, bool>::value>::type>
{
    using type = bool;
};

} // namespace serialization
} // namespace common
} // namespace softeq

#endif //SOFTEQ_COMMON_SERIALIZATION_SERIALIZATION_TYPES_H

