#include "map_object.hh"

#include <common/serialization/object_assembler.hh>

namespace softeq
{
namespace common
{
namespace serialization
{

template <>
ObjectAssembler<MapObject> Assembler()
{
    // clang-format off
    return ObjectAssembler<MapObject>()
        .define("m", &MapObject::m)
        .define("um", &MapObject::um)
        .define("ms", &MapObject::ms)
        .define("ms1", &MapObject::ms1)
        .define("strm", &MapObject::strm)
        ;
    // clang-format on
}

} // namespace serialization
} // namespace common
} // namespace softeq
