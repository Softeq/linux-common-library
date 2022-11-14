#include "structs.hh"

namespace softeq
{
namespace common
{
namespace serialization
{
template <>
ObjectAssembler<TestSettings> Assembler()
{
    // clang-format off
    return ObjectAssembler<TestSettings>()
        .define("i", &TestSettings::i)
        .define("b", &TestSettings::b)
        ;
    // clang-format on
}

template <>
ObjectAssembler<TestObject> Assembler()
{
    // clang-format off
    return ObjectAssembler<TestObject>()
        .define("s", &TestObject::s)
        ;
    // clang-format on
}

template <>
ObjectAssembler<TestObject::TestSettings> Assembler()
{
    // clang-format off
    return ObjectAssembler<TestObject::TestSettings>()
        .define("b", &TestObject::TestSettings::b)
        ;
    // clang-format on
}

template <>
ObjectAssembler<SimpleSetting> Assembler()
{
    // clang-format off
    return ObjectAssembler<SimpleSetting>()
        .define("i", &SimpleSetting::i)
        ;
    // clang-format on
}

template <>
ObjectAssembler<VectorTestSettings> Assembler()
{
    // clang-format off
    return ObjectAssembler<VectorTestSettings>()
        .define("container", &VectorTestSettings::container)
        ;
    // clang-format on
}

template <>
ObjectAssembler<Object> softeq::common::serialization::Assembler()
{
    // clang-format off
    return ObjectAssembler<Object>()
        // .define("nullptrT", &Object::nullptrT)
        .define("floatT", &Object::floatT)
        .define("doubleT", &Object::doubleT)
        .define("boolT", &Object::boolT)
        .define("charT", &Object::charT)
        .define("char16T", &Object::char16T)
        .define("wcharT", &Object::wcharT)
        .define("intT", &Object::intT)
        .define("longIntT", &Object::longIntT)
        .define("unsignedIntT", &Object::unsignedIntT)
        .define("unsignedLongIntT", &Object::unsignedLongIntT)
        .define("stringT", &Object::stringT)
        .define("vectorT", &Object::vectorT)
        .define("vectorTVectorInt", &Object::vectorTVectorInt)
        .define("mapTStringInt", &Object::mapTStringInt)
        .define("simpleSubobjectT", &Object::simpleSubobjectT)
        .define("subobjectT", &Object::subobjectT)
        .define("nestedSubobjectT", &Object::nestedSubobjectT)
        .define("vectorTSubobjectSimple", &Object::vectorTSubobjectSimple)
        .define("vectorTSubobject", &Object::vectorTSubobject)
        .define("mapTIntSubobjectSimple", &Object::mapTIntSubobjectSimple)
        .define("mapTStringSubobjectSimple", &Object::mapTStringSubobjectSimple)
        .define("optionalTInt", &Object::optionalTInt)
        ;
    // clang-format on
}

template <>
softeq::common::serialization::ObjectAssembler<Object::UnscopedEnum> softeq::common::serialization::Assembler()
{
    // clang-format off
    return ObjectAssembler<Object::UnscopedEnum>()
        .define("enumerator1", Object::UnscopedEnum::enumerator1)
        .define("enumerator2", Object::UnscopedEnum::enumerator2)
        .define("enumerator3", Object::UnscopedEnum::enumerator3)
        ;
    // clang-format on
}

template <>
softeq::common::serialization::ObjectAssembler<Object::SimpleSubobject> softeq::common::serialization::Assembler()
{
    // clang-format off
    return ObjectAssembler<Object::SimpleSubobject>()
        .define("simpleSubobjectTInt", &Object::SimpleSubobject::intT);
    // clang-format on
}

template <>
softeq::common::serialization::ObjectAssembler<Object::Subobject> softeq::common::serialization::Assembler()
{
    // clang-format off
    return ObjectAssembler<Object::Subobject>()
        .define("subobjectIntT", &Object::Subobject::intT)
        .define("subobjectStringT", &Object::Subobject::stringT)
        .define("subobjectVectorTInt", &Object::Subobject::vectorTInt)
        .define("subobjectVectorTVectorInt", &Object::Subobject::vectorTVectorInt)
        .define("subobjectVectorTSubobjectSimple", &Object::Subobject::vectorTSubobjectSimple)
        ;
    // clang-format on
}

template <>
softeq::common::serialization::ObjectAssembler<Object::NestedSubobject> softeq::common::serialization::Assembler()
{
    // clang-format off
    return ObjectAssembler<Object::NestedSubobject>()
        .define("nestedSubobjectIntT", &Object::NestedSubobject::intT)
        .extend<Object::Subobject>("subobjectT")
        ;
    // clang-format on
}

} // namespace serialization
} // namespace common
} // namespace softeq
