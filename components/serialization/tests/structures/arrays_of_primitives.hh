#ifndef SOFTEQ_COMMON_SERIALIZATION_TESTS_ARRAY_OF_PRIMITIVES_H
#define SOFTEQ_COMMON_SERIALIZATION_TESTS_ARRAY_OF_PRIMITIVES_H

#include <gtest/gtest.h>

#include <common/serialization/object_assembler.hh>
#include <common/serialization/helpers.hh>

struct MultiplePrimitivesArrays
{
    std::vector<int> integers;
    std::list<bool> booleans;
    std::vector<double> floats;
    std::vector<std::string> strings;
};

template <typename SerializerImpl, typename DeserializerImpl>
void testMultiplePrimitivesArrays()
{
    MultiplePrimitivesArrays testObject = {.integers = {1, 2, 3},
                                           .booleans = {true, false, true, false},
                                           .floats = {1.0, 2.0, 3.0},
                                           .strings = {"abc", "cba", "xyz"}};

    std::unique_ptr<softeq::common::serialization::StructSerializer> serializer(new SerializerImpl());
    std::unique_ptr<softeq::common::serialization::StructDeserializer> deserializer(new DeserializerImpl());

    serializeObject(*serializer, testObject);
    deserializer->setRawInput(serializer->dump());

    MultiplePrimitivesArrays restoredObject;
    deserializeObject(*deserializer, restoredObject);

    ASSERT_EQ(testObject.integers, restoredObject.integers);
    ASSERT_EQ(testObject.booleans, restoredObject.booleans);
    ASSERT_EQ(testObject.floats, restoredObject.floats);
    ASSERT_EQ(testObject.strings, restoredObject.strings);
}

#endif // SOFTEQ_COMMON_SERIALIZATION_TESTS_ARRAY_OF_PRIMITIVES_H
