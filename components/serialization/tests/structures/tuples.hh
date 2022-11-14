#ifndef SOFTEQ_COMMON_SERIALIZATION_TESTS_TUPLES_H
#define SOFTEQ_COMMON_SERIALIZATION_TESTS_TUPLES_H

#include <gtest/gtest.h>

#include <common/serialization/object_assembler.hh>
#include <common/serialization/helpers.hh>

struct TwoTypesStruct
{
    int a;
    double b;
};

bool operator==(const TwoTypesStruct &left, const TwoTypesStruct &right);

struct ObjectWithTuples
{
    std::tuple<int, double> primitivesTuple;
    std::tuple<TwoTypesStruct, int> tupleWithStruct;
    std::tuple<std::vector<TwoTypesStruct>, int> tupleWithVector;
    std::vector<std::tuple<int, std::string>> tuplesVector;
    std::vector<std::tuple<int, softeq::common::stdutils::Optional<double>, std::string>> tupleWithOptional;
};

template <typename SerializerImpl, typename DeserializerImpl>
void testTupleSplitSerializers()
{
    using namespace softeq::common::serialization;

    std::unique_ptr<StructSerializer> serializer(new SerializerImpl());
    std::unique_ptr<StructDeserializer> deserializer(new DeserializerImpl());

    ObjectWithTuples testObject = {{1, 2.0},
                                   {{10, 20.0}, 100},
                                   {{{100, 200.0}, {101, 202.0}}, 1000},
                                   {{1, "one"}, {2, "two"}, {3, "three"}},
                                   {{1, nullptr, "one"}, {2, 2.0, "two"}}};
    serializeObject(*serializer, testObject);

    deserializer->setRawInput(serializer->dump());

    ObjectWithTuples restoredObject;
    deserializeObject(*deserializer, restoredObject);

    ASSERT_EQ(testObject.primitivesTuple, restoredObject.primitivesTuple);
    ASSERT_EQ(testObject.tupleWithStruct, restoredObject.tupleWithStruct);
}

#endif // SOFTEQ_COMMON_SERIALIZATION_TESTS_TUPLES_H
