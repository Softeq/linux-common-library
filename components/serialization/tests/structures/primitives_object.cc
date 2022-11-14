#include "primitives_object.hh"

#include <common/serialization/object_assembler.hh>

namespace softeq
{
namespace common
{
namespace serialization
{
template <>
ObjectAssembler<PrimitivesObject> Assembler()
{
    // clang-format off
    return ObjectAssembler<PrimitivesObject>()
        .define("b", &PrimitivesObject::b)
        .define("i8", &PrimitivesObject::i8)
        .define("ui8", &PrimitivesObject::ui8)
        .define("i16", &PrimitivesObject::i16)
        .define("ui16", &PrimitivesObject::ui16)
        .define("i32", &PrimitivesObject::i32)
        .define("ui32", &PrimitivesObject::ui32)
        .define("i64", &PrimitivesObject::i64)
        .define("ui64", &PrimitivesObject::ui64)
        .define("f", &PrimitivesObject::f)
        .define("d", &PrimitivesObject::d)
        .define("str", &PrimitivesObject::str)
        ;
    // clang-format on
}

} // namespace serialization
} // namespace common
} // namespace softeq
