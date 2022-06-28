#ifndef SOFTEQ_COMMON_ANY_H_
#define SOFTEQ_COMMON_ANY_H_

#include <type_traits>
#include <typeinfo>
#include <memory>

namespace softeq
{
namespace common
{
/*!
  \brief The class any describes a type-safe container for single values of any type (stored in a heap)

  Since the softeq-common lib does not support std::any <https://en.cppreference.com/w/cpp/utility/any> because
  of c++11
*/
class Any final
{
    template <typename T>
    using decayed_t = typename std::decay<T>::type;
    template <typename T>
    using ifNotAny = typename std::enable_if<!std::is_same<decayed_t<T>, Any>::value>::type;

public:
    constexpr Any() = default;
    Any(const Any &other);
    Any(Any &&other) noexcept;
    explicit Any(const char *value);
    template <typename ValueType, typename = ifNotAny<ValueType>>
    Any(ValueType &&value)
        : _content(new Holder<decayed_t<ValueType>>(value))
    {
    }
    ~Any() = default;

    Any &operator=(const Any &rhs);
    Any &operator=(Any &&rhs) noexcept;
    Any &operator=(const char *value);
    template <typename ValueType, typename = ifNotAny<ValueType>>
    Any &operator=(ValueType &&value)
    {
        _content.reset(new Holder<decayed_t<ValueType>>(value));
        return *this;
    }

    template <class ValueType>
    operator ValueType &()
    {
        if (_content)
        {
            return dynamic_cast<Holder<ValueType> &>(*_content).value();
        }
        throw std::bad_cast();
    }

    template <class ValueType>
    operator const ValueType &() const
    {
        if (_content)
        {
            return dynamic_cast<Holder<ValueType> &>(*_content).value();
        }
        throw std::bad_cast();
    }

    template <class ValueType>
    operator ValueType *()
    {
        Holder<ValueType> *holder;

        return (holder = dynamic_cast<Holder<ValueType> *>(_content.get())) ? &holder->value() : nullptr;
    }

    template <class ValueType>
    operator const ValueType *() const
    {
        Holder<ValueType> const *holder;

        return (holder = dynamic_cast<Holder<ValueType> const *>(_content.get())) ? &holder->value() : nullptr;
    }
    /*!
        Type information
        \return Info about type
     */
    const std::type_info &type() const noexcept;
    /*!
        Checking for variable initialization
        \return bool Bool result
     */
    bool hasValue() const noexcept;
    /*!
        Reset variable value
     */
    void reset() noexcept;

    /*!
        Cast to string
        \return string String variable
     */
    std::string toString() const;

private:
    struct PlaceHolder
    {
        virtual ~PlaceHolder() = default;

        virtual const std::type_info &type() const = 0;
        virtual std::unique_ptr<PlaceHolder> clone() const = 0;
    };
    template <typename Type>
    class Holder final : public PlaceHolder
    {
    public:
        explicit Holder(const Type &value)
            : _value(value)
        {
        }
        explicit Holder(const Type &&value)
            : _value(std::move(value))
        {
        }
        Holder &operator=(const Holder &) = delete;
        Holder &operator=(Holder &&) = delete;
        std::unique_ptr<PlaceHolder> clone() const override
        {
            return std::unique_ptr<PlaceHolder>(new Holder(_value));
        }
        const std::type_info &type() const override
        {
            return typeid(_value);
        }

        Type &value()
        {
            return _value;
        }

        const Type &value() const
        {
            return _value;
        }

    private:
        Type _value;
    };

    std::unique_ptr<PlaceHolder> _content;
};

template <typename Type>
static Type &any_cast(Any &any)
{
    return static_cast<Type &>(any);
}

template <typename Type>
static const Type &any_cast(const Any &any)
{
    return static_cast<const Type &>(any);
}

template <typename Type>
static Type *any_cast(Any *any)
{
    if (any == nullptr)
    {
        return nullptr;
    }
    return *any;
}

template <typename Type>
static const Type *any_cast(const Any *any)
{
    if (any == nullptr)
    {
        return nullptr;
    }
    return *any;
}

} // namespace common
} // namespace softeq

#endif // SOFTEQ_COMMON_ANY_H_
