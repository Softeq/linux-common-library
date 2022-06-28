#include "softeq/common/any.hh"

using namespace softeq::common;

Any::Any(const Any &other)
    : _content(other._content ? other._content->clone() : nullptr)
{
}

Any::Any(Any &&other) noexcept
    : _content(std::move(other._content))
{
}

Any::Any(const char *value)
    : _content(new Holder<std::string>(value))
{
}

Any &Any::operator=(const Any &rhs)
{
    _content = std::move(Any(rhs)._content);
    return *this;
}

Any &Any::operator=(Any &&rhs) noexcept
{
    _content = std::move(rhs._content);
    return *this;
}

Any &Any::operator=(const char *value)
{
    _content.reset(new Holder<std::string>(value));
    return *this;
}

const std::type_info &Any::type() const noexcept
{
    return hasValue() ? _content->type() : typeid(void);
}

bool Any::hasValue() const noexcept
{
    return _content != nullptr;
}

void Any::reset() noexcept
{
    _content.reset();
}

std::string Any::toString() const
{
    if (!hasValue())
        return "undefined";
    else if (type() == typeid(int))
        return std::to_string(any_cast<int>(*this));
    else if (type() == typeid(long))
        return std::to_string(any_cast<long>(*this));
    else if (type() == typeid(long long))
        return std::to_string(any_cast<long long>(*this));
    else if (type() == typeid(unsigned))
        return std::to_string(any_cast<unsigned>(*this));
    else if (type() == typeid(unsigned long))
        return std::to_string(any_cast<unsigned long>(*this));
    else if (type() == typeid(unsigned long long))
        return std::to_string(any_cast<unsigned long long>(*this));
    else if (type() == typeid(float))
        return std::to_string(any_cast<float>(*this));
    else if (type() == typeid(double))
        return std::to_string(any_cast<double>(*this));
    else if (type() == typeid(long double))
        return std::to_string(any_cast<long double>(*this));
    else if (type() == typeid(std::string))
        return any_cast<std::string>(*this);
    else
        return "not std is not implemented";
}
