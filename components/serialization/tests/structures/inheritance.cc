#include "inheritance.hh"

namespace softeq
{
namespace common
{
namespace serialization
{
template <>
ObjectAssembler<A> Assembler()
{
    // clang-format off
    return ObjectAssembler<A>()
        .define("a", &A::a);
    // clang-format on
}

template <>
ObjectAssembler<B> Assembler()
{
    // clang-format off
    return ObjectAssembler<B>()
        .define("b", &B::b)
        .extend<A>("A")
        .extend<AnotherA>("AnotherA");
}

template <>
ObjectAssembler<AnotherA> Assembler()
{
    // clang-format off
    return ObjectAssembler<AnotherA>()
        .define("aa", &AnotherA::aa);
    // clang-format on
}

template <>
ObjectAssembler<C> Assembler()
{
    // clang-format off
    return ObjectAssembler<C>()
        .define("c", &C::c)
        .extend<B>("B");
    // clang-format on
}

} // namespace serialization
} // namespace common
} // namespace softeq
