#ifndef SOFTEQ_COMMON_SERIALIZATION_TESTS_INHERITANCE_H
#define SOFTEQ_COMMON_SERIALIZATION_TESTS_INHERITANCE_H

#include <gtest/gtest.h>

#include <common/serialization/object_assembler.hh>
#include <common/serialization/helpers.hh>

struct A
{
    int a;
};

struct AnotherA
{
    int aa;
};

struct B
    : A
    , AnotherA
{
    int b;
};

struct C : B
{
    int c;
};

template <typename SerializerType, typename DeserializerType>
void testInheritance(SerializerType &serializer, DeserializerType &deserializer)
{
    using namespace softeq::common::serialization;

    C c;
    c.a = 10;
    c.aa = 11;
    c.b = 15;
    c.c = 20;
    ObjectAssembler<C>::accessor().serialize(serializer, c);
    deserializer.setRawInput(serializer.dump());

    C c1 = {};
    ObjectAssembler<C>::accessor().deserialize(deserializer, c1);
    EXPECT_EQ(c1.a, c.a);
    EXPECT_EQ(c1.aa, c.aa);
    EXPECT_EQ(c1.b, c.b);
    EXPECT_EQ(c1.c, c.c);
}

#endif // SOFTEQ_COMMON_SERIALIZATION_TESTS_INHERITANCE_H
