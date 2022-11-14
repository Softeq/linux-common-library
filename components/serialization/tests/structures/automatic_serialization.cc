#include "automatic_serialization.hh"

bool operator==(const TwoIntsStructure &left, const TwoIntsStructure &right)
{
    return (left.a == right.a) && (left.b == right.b);
}

namespace softeq
{
namespace common
{
namespace serialization
{
template <>
ObjectAssembler<TwoIntsStructure> Assembler()
{
    return ObjectAssembler<TwoIntsStructure>().define("a", &TwoIntsStructure::a).define("b", &TwoIntsStructure::b);
}

template <>
ObjectAssembler<NestedStructure> Assembler()
{
    return ObjectAssembler<NestedStructure>().define("bottom", &NestedStructure::bottom);
}

template <>
ObjectAssembler<TopLevelStruct> Assembler()
{
    return ObjectAssembler<TopLevelStruct>()
        .define("top", &TopLevelStruct::top)
        .define("nested", &TopLevelStruct::nested);
}

template <>
ObjectAssembler<ArrayContainer> Assembler()
{
    return ObjectAssembler<ArrayContainer>().define("array", &ArrayContainer::array);
}

template <>
ObjectAssembler<OptionalContainer> Assembler()
{
    return ObjectAssembler<OptionalContainer>().define("optionalInt", &OptionalContainer::optionalInt);
}
} // namespace serialization
} // namespace common
} // namespace softeq
