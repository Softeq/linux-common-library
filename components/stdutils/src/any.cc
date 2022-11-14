#include "any.hh"

using namespace softeq::common::stdutils;

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
