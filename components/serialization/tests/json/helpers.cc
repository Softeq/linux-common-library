#include "serialization_test_fixture.hh"

#include "structures/test_structure.hh"
#include "structures/automatic_serialization.hh"

#include "json_struct_serializer.hh"
#include "json_struct_deserializer.hh"

#include "json_array_serializer.hh"
#include "json_array_deserializer.hh"

#include <common/serialization/json/json.hh>

using namespace softeq::common::serialization;

TEST(SerializationHelpers, JsonAsObjectSerializationHelper)
{
    TestStructure testObject = {.a = 10, .b = 42.0};
    std::string jsonOutput = json::serializeAsJsonObject(testObject);

    EXPECT_FALSE(jsonOutput.empty());

    TestStructure deserializedObject = json::deserializeFromJsonObject<TestStructure>(jsonOutput);

    EXPECT_EQ(deserializedObject, testObject);
}

TEST(SerializationHelpers, JsonAsArraySerializationHelper)
{
    std::vector<TestStructure> testObjects = {{.a = 10, .b = 42.0}, {.a = 12, .b = 64.0}};
    std::string jsonOutput = json::serializeAsJsonArray(testObjects);

    EXPECT_FALSE(jsonOutput.empty());

    std::vector<TestStructure> deserializedObjects =
        json::deserializeFromJsonArray<std::vector<TestStructure>>(jsonOutput);

    EXPECT_EQ(deserializedObjects, testObjects);
}

TEST(CompositeSerializers, BasicJsonObjectAssembleUsage)
{
    testBasicObjectAssembleUsage<json::CompositeJsonSerializer, json::CompositeJsonDeserializer>();
}

TEST(CompositeJsonSerializer, NestedStructsSerialization)
{
    testNestedStructsSerialization<json::CompositeJsonSerializer, json::CompositeJsonDeserializer>();
}

TEST(CompositeJsonSerializer, NestedArrayStructSerialization)
{
    testNestedArrayStructSerialization<json::CompositeJsonSerializer, json::CompositeJsonDeserializer>();
}

TEST(CompositeJsonDeserializer, OptionalStructureSerialization)
{
    testOptionalStructureSerialization<json::CompositeJsonSerializer, json::CompositeJsonDeserializer>();
}

TEST(CompositeJsonDeserializer, SerializationInArray)
{
    testSerializationInArray<json::RootJsonArraySerializer, json::RootJsonArrayDeserializer>();
}
