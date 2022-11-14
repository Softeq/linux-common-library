#ifndef SOFTEQ_COMMON_SERIALIZATION_TESTS_ENUM_OBJECT_H
#define SOFTEQ_COMMON_SERIALIZATION_TESTS_ENUM_OBJECT_H

#include <gtest/gtest.h>

#include <vector>

enum EnumValues
{
    enum1,
    enum2,
};
enum class EnumClassValues
{
    enumClass1,
    enumClass2,
    enumClass3,
};
struct EnumObject
{
    EnumValues ev;
    EnumClassValues ecv;

    std::vector<EnumValues> enumsVec;
    std::vector<EnumClassValues> enumsClassVec;
};


template <typename SerializerType, typename DeserializerType>
void testEnumSerialization(SerializerType &serializer, DeserializerType &deserializer)
{
    EnumObject obj1 = {enum2, EnumClassValues::enumClass3,
                       {enum1, enum2},
                       {EnumClassValues::enumClass1, EnumClassValues::enumClass2}};
    serializeObject(serializer, obj1);
    EXPECT_FALSE(serializer.dump().empty());

    EnumObject obj2 = {};
    deserializer.setRawInput(serializer.dump());
    deserializeObject(deserializer, obj2);
    EXPECT_EQ(obj1.ev, obj2.ev);
    EXPECT_EQ(obj1.ecv, obj2.ecv);
    EXPECT_EQ(obj1.enumsVec, obj2.enumsVec);
    EXPECT_EQ(obj1.enumsClassVec, obj2.enumsClassVec);
}


#endif //SOFTEQ_COMMON_SERIALIZATION_TESTS_MAP_OBJECT_H
