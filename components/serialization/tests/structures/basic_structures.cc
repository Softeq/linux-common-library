#include "basic_structures.hh"

namespace softeq
{
namespace common
{
namespace serialization
{
template <>
ObjectAssembler<FloatObject> Assembler()
{
    // clang-format off
    return ObjectAssembler<FloatObject>()
        .define("f", &FloatObject::f)
        ;
    // clang-format on
}

template <>
ObjectAssembler<VecObject> Assembler()
{
    // clang-format off
    return ObjectAssembler<VecObject>()
        .define("vi", &VecObject::vi)
        ;
    // clang-format on
}
template <>
ObjectAssembler<OptionalObject> Assembler()
{
    // clang-format off
    return ObjectAssembler<OptionalObject>()
        .define("oi", &OptionalObject::oi)
        .define("voi", &OptionalObject::voi)
        .define("oss", &OptionalObject::oss)
        .define("voss", &OptionalObject::voss)
        ;
    // clang-format on
}

template <>
ObjectAssembler<SingleOptionalObject> Assembler()
{
    // clang-format off
    return ObjectAssembler<SingleOptionalObject>()
        .define("oit", &SingleOptionalObject::oit)
        .define("oif", &SingleOptionalObject::oif)
        .define("ouit", &SingleOptionalObject::ouit)
        .define("ouic", &SingleOptionalObject::ouic)
        .define("ouis", &SingleOptionalObject::ouis)
        .define("oft", &SingleOptionalObject::oft)
        .define("ofc", &SingleOptionalObject::ofc)
        .define("ost", &SingleOptionalObject::ost)
        .define("osf", &SingleOptionalObject::osf)
        .define("voit", &SingleOptionalObject::voit)
        .define("voif", &SingleOptionalObject::voif)
        .define ("mt", &SingleOptionalObject::mt)
        .define ("mff", &SingleOptionalObject::mff)
        .define ("mfs", &SingleOptionalObject::mfs)
        .define ("mts", &SingleOptionalObject::mts)
        .define ("umt", &SingleOptionalObject::umt)
        .define ("umff", &SingleOptionalObject::umff)
        .define ("umfs", &SingleOptionalObject::umfs)
        .define ("umts", &SingleOptionalObject::umts)
    ;
    // clang-format on
}

template <>
ObjectAssembler<ErrorNameDefineObject> Assembler()
{
    // clang-format off
    return ObjectAssembler<ErrorNameDefineObject>()
        .define("i1", &ErrorNameDefineObject::i1)
        .define("i1", &ErrorNameDefineObject::i2)
        ;
    // clang-format on
}
template <>
ObjectAssembler<ErrorMemberDefineObject> Assembler()
{
    // clang-format off
    return ObjectAssembler<ErrorMemberDefineObject>()
        .define("i1", &ErrorMemberDefineObject::i1)
        .define("i2", &ErrorMemberDefineObject::i1)
        ;
    // clang-format on
}
template <>
ObjectAssembler<EnumMemberErrorValues> Assembler()
{
    // clang-format off
    return ObjectAssembler<EnumMemberErrorValues>()
        .define("enumError1", enumError1)
        .define("enumError1", enumError2)
        ;
    // clang-format on
}

} // namespace serialization
} // namespace common
} // namespace softeq
