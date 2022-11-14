#ifndef SOFTEQ_COMMON_PADDED_H_
#define SOFTEQ_COMMON_PADDED_H_

namespace softeq
{
namespace common
{
namespace stdutils
{
// Enabled via a template parameter
template <class T, int size, class Enable = void>
union Padded; // undefined

/*!
  \brief Padded<Type, size> is a relatively transparent way of making sure a structure 'T' will
  take at least 'size' bytes
*/
template <class T, int size>
union Padded<T, size, typename std::enable_if<(sizeof(T) <= size)>::type>
{
    T value;
    uint8_t payload[size];

    Padded()
        : value()
    {
    }
    Padded(const T &value)
        : value(value)
    {
    }
    operator const T &() const
    {
        return value;
    }
    operator T &()
    {
        return value;
    }
    T *operator->()
    {
        return &value;
    }
    const T *operator->() const
    {
        return &value;
    }
};

} // namespace stdutils
} // namespace common
} // namespace softeq

#endif // SOFTEQ_COMMON_PADDED_H_
