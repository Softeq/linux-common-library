#ifndef SOFTEQ_COMMON_SCOPE_GUARD_H_
#define SOFTEQ_COMMON_SCOPE_GUARD_H_

#include <deque>
#include <functional>

namespace softeq
{
namespace common
{
namespace stdutils
{
/*!
  \brief Scope guard implementation.
*/
class scope_guard
{
public:
    scope_guard(scope_guard &&) = default;
    scope_guard() = default;

    template <class Callable>
    explicit scope_guard(Callable &&func)
    {
        _handlers.emplace_front(std::forward<Callable>(func));
    }

    template <class Callable>
    scope_guard &operator+=(Callable &&func)
    {
        _handlers.emplace_front(std::forward<Callable>(func));
        return *this;
    }

    virtual ~scope_guard()
    {
        for (const auto &f : _handlers)
        {
            f(); // must not throw
        }
    }

    /*!
       Reset scope guard
     */
    void dismiss() noexcept
    {
        _handlers.clear();
    }

private:
    scope_guard(const scope_guard &) = delete;
    void operator=(const scope_guard &) = delete;

    // 'noexcept' in std::function specifiaction is not valid since C++17 !
    std::deque<std::function<void() noexcept>> _handlers;
};

} // namespace stdutils
} // namespace common
} // namespace softeq

#endif // SOFTEQ_COMMON_SCOPE_GUARD_H_
