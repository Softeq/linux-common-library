#ifndef SOFTEQ_COMMON_SERIALIZATION_TESTS_PRIMITIVES_OBJECT_H
#define SOFTEQ_COMMON_SERIALIZATION_TESTS_PRIMITIVES_OBJECT_H

#include <gtest/gtest.h>

#include <common/serialization/object_assembler.hh>
#include <common/serialization/helpers.hh>

#include <string>

struct PrimitivesObject
{
    bool b;
    int8_t i8;
    uint8_t ui8;
    int16_t i16;
    uint16_t ui16;
    int32_t i32;
    uint32_t ui32;
    int64_t i64;
    uint64_t ui64;
    float f;
    double d;
    std::string str;
};

template <typename SerializerType, typename DeserializerType>
void partialSerialization(SerializerType &serializer, DeserializerType &deserializer, const std::string &expected)
{
    using namespace softeq::common::serialization;

    PrimitivesObject obj = {};
    obj.i8 = 1;
    obj.i16 = -1;
    ObjectAssembler<PrimitivesObject>::accessor().serialize(serializer, obj, &PrimitivesObject::i8,
                                                            &PrimitivesObject::i16);
    EXPECT_EQ(expected, serializer.dump());
    deserializer.setRawInput(serializer.dump());

    obj = {};
    ObjectAssembler<PrimitivesObject>::accessor().deserialize(deserializer, obj, &PrimitivesObject::i8);
    EXPECT_EQ(obj.i8, 1);
    EXPECT_EQ(obj.i16, 0);
}

#endif //SOFTEQ_COMMON_SERIALIZATION_TESTS_PRIMITIVES_OBJECT_H
