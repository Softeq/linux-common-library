#include "vector_of_maps.hh"

namespace softeq
{
namespace common
{
namespace serialization
{
template <>
ObjectAssembler<VectorOfMaps> Assembler()
{
    // clang-format off
    return ObjectAssembler<VectorOfMaps>()
        .define("maps",            &VectorOfMaps::mapsVec)
        .define("string_key_maps", &VectorOfMaps::stringKeyMapsVector)
        ;
    // clang-format on
}

} // namespace serialization
} // namespace common
} // namespace softeq
