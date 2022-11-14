#ifndef SOFTEQ_COMMON_OPTIONAL_H_
#define SOFTEQ_COMMON_OPTIONAL_H_

#include <exception>

namespace softeq
{
namespace common
{
namespace stdutils
{
// poor optional implementation
/*!
  \brief The class  manages an optional contained value, i.e. a value that may or may not be present.
         Since the softeq-common lib does not support std::optional <https://en.cppreference.com/w/cpp/utility/optional>
  because of c++11
*/
template <typename T>
class Optional
{
    bool _hasValue;
    T _value;

public:
    using valueType = T;
    class NotInitializedException : public std::exception
    {
    };

    Optional()
        : _hasValue(false)
    {
    }

    Optional(std::nullptr_t)
        : _hasValue(false)
    {
    }

    Optional(const T &newValue)
        : _hasValue(true)
        , _value(newValue)
    {
    }

    Optional(T &&newValue)
        : _hasValue(true)
        , _value(newValue)
    {
    }

    Optional(const Optional &other)
        : _hasValue(other.hasValue())
    {
        if (_hasValue)
            *this = other;
    }

    Optional(Optional &&other) = default;
    Optional& operator=(const Optional&) = default;
    Optional& operator=(Optional&&) = default;

    T &operator=(const T &newValue)
    {
        _hasValue = true;
        _value = newValue;
        return _value;
    }

    T &operator=(T &&newValue)
    {
        _hasValue = true;
        _value = newValue;
        return _value;
    }

    operator const T &() const
    {
        return cValue();
    }

    T *operator->()
    {
        if (_hasValue)
        {
            return &_value;
        }
        throw NotInitializedException();
    }

    const T *operator->() const
    {
        if (_hasValue)
        {
            return &_value;
        }
        throw NotInitializedException();
    }

    T &operator*()
    {
        if (_hasValue)
        {
            return _value;
        }
        throw NotInitializedException();
    }

    const T &operator*() const
    {
        if (_hasValue)
        {
            return _value;
        }
        throw NotInitializedException();
    }

    explicit operator bool() const
    {
        return _hasValue;
    }

    const T &valueOr(const T &defaultValue) const
    {
        return _hasValue ? _value : defaultValue;
    }

    const T &cValue() const
    {
        if (_hasValue)
        {
            return _value;
        }
        throw NotInitializedException();
    }

    bool hasValue() const
    {
        return _hasValue;
    }

    /*!
      \brief If _value exists, call value's dectructor and set _hasValue to false
    */
    void reset()
    {
        if (_hasValue)
        {
            _value.~T();
            _hasValue = false;
        }
    }
};

template <class T, class U>
inline bool operator==(const Optional<T> &x, const Optional<U> &y)
{
    return bool(x) != bool(y) ? false : bool(x) == false ? true : *x == *y;
}

template <class T, class U>
inline bool operator!=(const Optional<T> &x, const Optional<U> &y)
{
    return !(x == y);
}

template <class T, class U>
inline bool operator<(const Optional<T> &x, const Optional<U> &y)
{
    return (!y) ? false : (!x) ? true : *x < *y;
}

template <class T, class U>
inline bool operator<=(const Optional<T> &x, const Optional<U> &y)
{
    return !(y < x);
}

template <class T, class U>
inline bool operator>(const Optional<T> &x, const Optional<U> &y)
{
    return (y < x);
}

template <class T, class U>
inline bool operator>=(const Optional<T> &x, const Optional<U> &y)
{
    return !(x < y);
}

template <class T>
inline bool operator==(const Optional<T> &x, const T &v)
{
    return bool(x) ? *x == v : false;
}

template <class T>
inline bool operator==(const T &v, const Optional<T> &x)
{
    return bool(x) ? v == *x : false;
}

template <class T>
inline bool operator!=(const Optional<T> &x, const T &v)
{
    return bool(x) ? *x != v : true;
}

template <class T>
inline bool operator!=(const T &v, const Optional<T> &x)
{
    return bool(x) ? v != *x : true;
}

template <class T>
inline bool operator<(const Optional<T> &x, const T &v)
{
    return bool(x) ? *x < v : true;
}

template <class T>
inline bool operator>(const T &v, const Optional<T> &x)
{
    return bool(x) ? v > *x : true;
}

template <class T>
inline bool operator>(const Optional<T> &x, const T &v)
{
    return bool(x) ? *x > v : false;
}

template <class T>
inline bool operator<(const T &v, const Optional<T> &x)
{
    return bool(x) ? v < *x : false;
}

template <class T>
inline bool operator>=(const Optional<T> &x, const T &v)
{
    return bool(x) ? *x >= v : false;
}

template <class T>
inline bool operator<=(const T &v, const Optional<T> &x)
{
    return bool(x) ? v <= *x : false;
}

template <class T>
inline bool operator<=(const Optional<T> &x, const T &v)
{
    return bool(x) ? *x <= v : true;
}

template <class T>
inline bool operator>=(const T &v, const Optional<T> &x)
{
    return bool(x) ? v >= *x : true;
}

} // namespace stdutils
} // namespace common
} // namespace softeq

#endif // SOFTEQ_COMMON_OPTIONAL_H_
