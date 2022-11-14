#include "arrays_of_primitives.hh"

namespace softeq
{
namespace common
{
namespace serialization
{
template <>
ObjectAssembler<MultiplePrimitivesArrays> Assembler()
{
    return ObjectAssembler<MultiplePrimitivesArrays>()
        .define("integers", &MultiplePrimitivesArrays::integers)
        .define("booleans", &MultiplePrimitivesArrays::booleans)
        .define("floats", &MultiplePrimitivesArrays::floats)
        .define("strings", &MultiplePrimitivesArrays::strings);
}

} // namespace serialization
} // namespace common
} // namespace softeq
