#ifndef SOFTEQ_COMMON_SERIALIZATION_TESTS_AUTOMATIC_SERIALIZATION_H
#define SOFTEQ_COMMON_SERIALIZATION_TESTS_AUTOMATIC_SERIALIZATION_H

#include <gtest/gtest.h>

#include <common/serialization/object_assembler.hh>
#include <common/serialization/helpers.hh>

struct TwoIntsStructure
{
    int a;
    int b;
};

bool operator==(const TwoIntsStructure &left, const TwoIntsStructure &right);

struct NestedStructure
{
    int bottom;
};

struct TopLevelStruct
{
    int top;
    NestedStructure nested;
};

struct ArrayContainer
{
    std::vector<int> array;
};

struct OptionalContainer
{
    softeq::common::stdutils::Optional<int> optionalInt;
};

template <typename SerializerImpl, typename DeserializerImpl>
void testBasicObjectAssembleUsage()
{
    using namespace softeq::common::serialization;

    TwoIntsStructure testObject = {.a = 1, .b = 2};

    std::unique_ptr<StructSerializer> topLevelSerializer(new SerializerImpl());
    ObjectAssembler<TwoIntsStructure>::accessor().serialize(*topLevelSerializer, testObject);

    // Json serializer doesn't guarantee the order of object's members, so we do not check the string content,
    // but for json it will be like:
    //    {"a":1,"b":2}
    // and for xml:
    //    <?xml version="1.0"?>
    //    <root><a type="int">1</a><b type="int">2</b></root>

    ASSERT_FALSE(topLevelSerializer->dump().empty());

    std::unique_ptr<StructDeserializer> topLevelDeserializer(new DeserializerImpl());
    topLevelDeserializer->setRawInput(topLevelSerializer->dump());

    TwoIntsStructure restoredObject = {0, 0};
    ObjectAssembler<TwoIntsStructure>::accessor().deserialize(*topLevelDeserializer, restoredObject);

    ASSERT_EQ(restoredObject.a, testObject.a);
    ASSERT_EQ(restoredObject.b, testObject.b);
}

template <typename SerializerImpl, typename DeserializerImpl>
void testNestedStructsSerialization()
{
    using namespace softeq::common::serialization;

    NestedStructure nestedObject = {.bottom = 2};
    TopLevelStruct topLevelObject = {.top = 1, .nested = nestedObject};

    std::unique_ptr<StructSerializer> topLevelSerializer(new SerializerImpl());

    ObjectAssembler<TopLevelStruct>::accessor().serialize(*topLevelSerializer, topLevelObject);

    // Json serializer doesn't guarantee the order of object's members.
    // So the following assertion can't be checked:
    //
    // ASSERT_EQ(topLevelSerializer->dump(), "{\"top\":1,\"nested\":{\"bottom\":2}}");
    ASSERT_FALSE(topLevelSerializer->dump().empty());

    std::unique_ptr<StructDeserializer> topLevelDeserializer(new DeserializerImpl());
    topLevelDeserializer->setRawInput(topLevelSerializer->dump());

    TopLevelStruct restoredObject;
    ObjectAssembler<TopLevelStruct>::accessor().deserialize(*topLevelDeserializer, restoredObject);

    ASSERT_EQ(topLevelObject.top, restoredObject.top);
    ASSERT_EQ(topLevelObject.nested.bottom, restoredObject.nested.bottom);
}

template <typename SerializerImpl, typename DeserializerImpl>
void testNestedArrayStructSerialization()
{
    using namespace softeq::common::serialization;

    ArrayContainer arrayStruct = {.array = {1, 2, 3}};

    std::unique_ptr<StructSerializer> topLevelSerializer(new SerializerImpl());

    ObjectAssembler<ArrayContainer>::accessor().serialize(*topLevelSerializer, arrayStruct);

    ASSERT_FALSE(topLevelSerializer->dump().empty());

    std::unique_ptr<StructDeserializer> topLevelDeserializer(new DeserializerImpl());
    topLevelDeserializer->setRawInput(topLevelSerializer->dump());

    ArrayContainer restoredObject;
    ObjectAssembler<ArrayContainer>::accessor().deserialize(*topLevelDeserializer, restoredObject);
    ASSERT_EQ(restoredObject.array, arrayStruct.array);
}

template <typename SerializerImpl, typename DeserializerImpl>
void testOptionalStructureSerialization()
{
    using namespace softeq::common::serialization;

    OptionalContainer testObjectWithOptional;
    std::unique_ptr<StructSerializer> serializer(new SerializerImpl());

    ObjectAssembler<OptionalContainer>::accessor().serialize(*serializer, testObjectWithOptional);

    ASSERT_FALSE(serializer->dump().empty());

    std::unique_ptr<StructDeserializer> deserializer(new DeserializerImpl());
    deserializer->setRawInput(serializer->dump());

    OptionalContainer restoredObject;
    restoredObject.optionalInt = 2;

    ASSERT_TRUE(restoredObject.optionalInt.hasValue());
    ObjectAssembler<OptionalContainer>::accessor().deserialize(*deserializer, restoredObject);
    ASSERT_FALSE(restoredObject.optionalInt.hasValue());
}

template <typename SerializerImpl, typename DeserializerImpl>
void testSerializationInArray()
{
    using namespace softeq::common::serialization;
    using TwoIntsStructures = std::vector<TwoIntsStructure>;

    TwoIntsStructures testObject = {{1, 2}, {3, 4}};
    std::unique_ptr<ArraySerializer> serializer(new SerializerImpl());

    ObjectAssembler<TwoIntsStructures>::accessor().serialize(*serializer, testObject);
    ASSERT_FALSE(serializer->dump().empty());

    std::unique_ptr<ArrayDeserializer> deserializer(new DeserializerImpl());
    deserializer->setRawInput(serializer->dump());

    TwoIntsStructures restoredObject;
    ObjectAssembler<TwoIntsStructures>::accessor().deserialize(*deserializer, restoredObject);
    ASSERT_EQ(testObject, restoredObject);
}

#endif // SOFTEQ_COMMON_SERIALIZATION_TESTS_AUTOMATIC_SERIALIZATION_H
