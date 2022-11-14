#include "tuples.hh"

bool operator==(const TwoTypesStruct &left, const TwoTypesStruct &right)
{
    return left.a == right.a && left.b == right.b;
}

namespace softeq
{
namespace common
{
namespace serialization
{
template <>
ObjectAssembler<ObjectWithTuples> Assembler()
{
    return ObjectAssembler<ObjectWithTuples>()
        .define("primitives", &ObjectWithTuples::primitivesTuple)
        .define("with_struct", &ObjectWithTuples::tupleWithStruct)
        .define("with_vector", &ObjectWithTuples::tupleWithVector)
        .define("tuples_vector", &ObjectWithTuples::tuplesVector)
        .define("with_optional", &ObjectWithTuples::tupleWithOptional);
}

template <>
ObjectAssembler<TwoTypesStruct> Assembler()
{
    return ObjectAssembler<TwoTypesStruct>().define("a", &TwoTypesStruct::a).define("b", &TwoTypesStruct::b);
}

} // namespace serialization
} // namespace common
} // namespace softeq
