#include "serialization_test_fixture.hh"

#include "structures/custom_type.hh"
#include "structures/tuples.hh"
#include "structures/primitives_object.hh"
#include "structures/inheritance.hh"
#include "structures/map_object.hh"
#include "structures/vector_of_maps.hh"
#include "structures/enum_object.hh"
#include "structures/arrays_of_primitives.hh"
#include "structures/complex_object.hh"

#include "json_struct_serializer.hh"
#include "json_struct_deserializer.hh"

#include "json_array_serializer.hh"
#include "json_array_deserializer.hh"

using namespace softeq::common::serialization;

TEST(SerializationCases, JsonMultiplePrimitivesArrays)
{
    testMultiplePrimitivesArrays<json::CompositeJsonSerializer, json::CompositeJsonDeserializer>();
}

TEST_F(Serialization, JsonSerialization)
{
    json::CompositeJsonSerializer serializer;
    json::CompositeJsonDeserializer deserializer;
    testComplexStructSerialization(serializer, deserializer);
}

TEST_F(Serialization, JsonDifferentSerializerAndDeserializer)
{
    std::unique_ptr<StructSerializer> serializer(new json::CompositeJsonSerializer());
    std::unique_ptr<StructDeserializer> deserializer(new json::CompositeJsonDeserializer());

    testComplexStructSerialization(*serializer, *deserializer);
}

TEST_F(Serialization, JsonMultiThreading)
{
    testMultiThreading<json::CompositeJsonSerializer, json::CompositeJsonDeserializer>();
}
TEST_F(Serialization, JsonEnum)
{
    json::CompositeJsonSerializer serializer;
    json::CompositeJsonDeserializer deserializer;
    testEnumSerialization(serializer, deserializer);
}

TEST_F(Serialization, DifferentJsonSerializerEnum)
{
    std::unique_ptr<StructSerializer> serializer(new json::CompositeJsonSerializer());
    std::unique_ptr<StructDeserializer> deserializer(new json::CompositeJsonDeserializer());

    testEnumSerialization(*serializer, *deserializer);
}

TEST_F(Serialization, JsonMap)
{
    json::CompositeJsonSerializer serializer;
    json::CompositeJsonDeserializer deserializer;
    testMapSerialization(serializer, deserializer);
}

TEST_F(Serialization, DifferentJsonSerializerMap)
{
    std::unique_ptr<StructSerializer> serializer(new json::CompositeJsonSerializer());
    std::unique_ptr<StructDeserializer> deserializer(new json::CompositeJsonDeserializer());

    testMapSerialization(*serializer, *deserializer);
}

TEST_F(Serialization, DifferentJsonSerializerMapVector)
{
    std::unique_ptr<StructSerializer> serializer(new json::CompositeJsonSerializer());
    std::unique_ptr<StructDeserializer> deserializer(new json::CompositeJsonDeserializer());

    testMapVectorSerialization(*serializer, *deserializer);
}

TEST_F(Serialization, JsonInheritance)
{
    json::CompositeJsonSerializer serializer;
    json::CompositeJsonDeserializer deserializer;
    testInheritance(serializer, deserializer);
}

TEST_F(Serialization, JsonSplitSerializersInheritance)
{
    std::unique_ptr<StructSerializer> serializer(new json::CompositeJsonSerializer());
    std::unique_ptr<StructDeserializer> deserializer(new json::CompositeJsonDeserializer());

    testInheritance(*serializer, *deserializer);
}

// Note: we should make minimal number of tests with raw strings

TEST_F(Serialization, JsonPartialSerialization)
{
    json::CompositeJsonSerializer serializer;
    json::CompositeJsonDeserializer deserializer;
    partialSerialization(serializer, deserializer, R"({"i16":-1,"i8":1})");
}
TEST_F(Serialization, JsonSplitSerializersPartialSerialization)
{
    std::unique_ptr<StructSerializer> serializer(new json::CompositeJsonSerializer());
    std::unique_ptr<StructDeserializer> deserializer(new json::CompositeJsonDeserializer());

    partialSerialization(*serializer, *deserializer, R"({"i16":-1,"i8":1})");
}

TEST_F(Serialization, JsonArraySplitSerializersPartialSerialization)
{
    std::unique_ptr<ArraySerializer> serializer(new json::RootJsonArraySerializer());
    std::unique_ptr<ArrayDeserializer> deserializer(new json::RootJsonArrayDeserializer());

    partialSerialization(*serializer, *deserializer, R"([{"i16":-1,"i8":1}])");
}

TEST_F(Serialization, JsonTupleSplitSerializers)
{
    testTupleSplitSerializers<json::CompositeJsonSerializer, json::CompositeJsonDeserializer>();
}

TEST(CustomTypeSerialization, BasicUsageJson)
{
    testBasicUsage<json::CompositeJsonSerializer, json::CompositeJsonDeserializer>("{\"digit\":\"one\"}");
}
