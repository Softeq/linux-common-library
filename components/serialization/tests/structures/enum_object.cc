#include "enum_object.hh"

#include <common/serialization/object_assembler.hh>

namespace softeq
{
namespace common
{
namespace serialization
{
template <>
ObjectAssembler<EnumValues> Assembler()
{
    // clang-format off
    return ObjectAssembler<EnumValues>()
        .define("enum1", enum1)
        .define("enum2", enum2)
        ;
    // clang-format on
}
template <>
ObjectAssembler<EnumClassValues> Assembler()
{
    // clang-format off
    return ObjectAssembler<EnumClassValues>()
        .define("enumClass1", EnumClassValues::enumClass1)
        .define("enumClass2", EnumClassValues::enumClass2)
        .define("enumClass3", EnumClassValues::enumClass3)
        ;
    // clang-format on
}
template <>
ObjectAssembler<EnumObject> Assembler()
{
    // clang-format off
    return ObjectAssembler<EnumObject>()
        .define("ev", &EnumObject::ev)
        .define("ecv", &EnumObject::ecv)
        .define("vec_ev", &EnumObject::enumsVec)
        .define("vec_ecv", &EnumObject::enumsClassVec)
        ;
    // clang-format on
}

} // namespace serialization
} // namespace common
} // namespace softeq

