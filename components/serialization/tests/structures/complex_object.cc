#include "complex_object.hh"

#include <common/serialization/object_assembler.hh>

namespace softeq
{
namespace common
{
namespace serialization
{
template <>
ObjectAssembler<Object::SimpleSubObject> Assembler()
{
    // clang-format off
    return ObjectAssembler<Object::SimpleSubObject>()
        .define("i", &Object::SimpleSubObject::i);
    // clang-format on
}

template <>
ObjectAssembler<Object::SubObject> Assembler()
{
    // clang-format off
    return ObjectAssembler<Object::SubObject>()
        .define("sri", &Object::SubObject::sri)
        .define("srs", &Object::SubObject::srs)
        .define("svi", &Object::SubObject::svi)
        .define("svvi", &Object::SubObject::svvi)
        .define("svvs", &Object::SubObject::svvs)
        ;
    // clang-format on
}

template <>
ObjectAssembler<Object> Assembler()
{
    // clang-format off
    return ObjectAssembler<Object>()
        .define("i", &Object::i)
        .define("b", &Object::b)
        .define("s", &Object::s)
        .define("f", &Object::f)
        .define("d", &Object::d)
        .define("sr", &Object::sr)
        .define("vi", &Object::vi)
        .define("vss", &Object::vss)
        .define("vsr", &Object::vsr)
        .define("us", &Object::us)
        .define("ui", &Object::ui)
        .define("li", &Object::li)
        .define("oi1", &Object::oi1)
        .define("oi2", &Object::oi2)
        ;
    // clang-format on
}
} // namespace serialization
} // namespace common
} // namespace softeq

bool operator<(const Object::SimpleSubObject &l, const Object::SimpleSubObject &r)
{
    return l.i < r.i;
}
