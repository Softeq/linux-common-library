#ifndef SOFTEQ_COMMON_SERIALIZATION_TESTS_VECTOR_OF_MAPS_H
#define SOFTEQ_COMMON_SERIALIZATION_TESTS_VECTOR_OF_MAPS_H

#include <gtest/gtest.h>

#include <common/serialization/object_assembler.hh>
#include <common/serialization/helpers.hh>

struct VectorOfMaps
{
    std::vector<std::map<int, std::string>> mapsVec;
    std::vector<std::map<std::string, int>> stringKeyMapsVector;
};

template <typename SerializerType, typename DeserializerType>
void testMapVectorSerialization(SerializerType &serializer, DeserializerType &deserializer)
{
    VectorOfMaps testObject = {{ // int -> string maps:
                                {// map 1
                                 {1, "one"},
                                 {2, "two"}},
                                {// map 2
                                 {10, "ten"},
                                 {20, "twenty"}}},
                               { // string -> int maps:
                                {// map 1
                                 {"one", 1},
                                 {"two", 2}},
                                {// map 2
                                 {"ten", 10},
                                 {"twenty", 20}}}};

    serializeObject(serializer, testObject);
    EXPECT_FALSE(serializer.dump().empty());

    VectorOfMaps restoredObject = {};
    deserializer.setRawInput(serializer.dump());
    deserializeObject(deserializer, restoredObject);

    ASSERT_EQ(testObject.mapsVec, restoredObject.mapsVec);
    ASSERT_EQ(testObject.stringKeyMapsVector, restoredObject.stringKeyMapsVector);
}

#endif // SOFTEQ_COMMON_SERIALIZATION_TESTS_VECTOR_OF_MAPS_H
