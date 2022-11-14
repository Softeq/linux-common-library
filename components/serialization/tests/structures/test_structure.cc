#include "test_structure.hh"

#include <common/serialization/object_assembler.hh>

bool operator==(const TestStructure &lhs, const TestStructure &rhs)
{
    return lhs.a == rhs.a && lhs.b == rhs.b;
}

namespace softeq
{
namespace common
{
namespace serialization
{
template <>
ObjectAssembler<TestStructure> Assembler()
{
    return ObjectAssembler<TestStructure>().define("a", &TestStructure::a).define("b", &TestStructure::b);
}

} // namespace serialization
} // namespace common
} // namespace softeq
