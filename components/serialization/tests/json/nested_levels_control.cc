#include <gtest/gtest.h>

#include "json_struct_serializer.hh"
#include "json_array_serializer.hh"

using namespace softeq::common::serialization;

TEST(NestedLevelSerialization, AutomaticNestedLevelCreation)
{
    std::unique_ptr<StructSerializer> topLevelSerializer(new json::CompositeJsonSerializer());

    topLevelSerializer->serializeValue("a", 10);
    ASSERT_EQ(topLevelSerializer->dump(), "{\"a\":10}");

    StructSerializer *nestedStructSerializer = topLevelSerializer->serializeStruct("b");

    nestedStructSerializer->serializeValue("c", 20);

    ASSERT_EQ(topLevelSerializer->dump(), "{\"a\":10,\"b\":{\"c\":20}}");
}

TEST(NestedLevelSerialization, AutomaticNestedArrayCreation)
{
    std::unique_ptr<StructSerializer> topLevelSerializer(new json::CompositeJsonSerializer());

    topLevelSerializer->serializeValue("a", 10);

    ArraySerializer *nestedArraySerializer = topLevelSerializer->serializeArray("b");

    ASSERT_NE(nestedArraySerializer, nullptr);

    nestedArraySerializer->serializeValue(1);
    nestedArraySerializer->serializeValue(2);
    nestedArraySerializer->serializeValue(3);

    ASSERT_EQ(topLevelSerializer->dump(), "{\"a\":10,\"b\":[1,2,3]}");
}

TEST(NestedLevelSerialization, ArrayOfArraysCreation)
{
    std::unique_ptr<StructSerializer> topLevelSerializer(new json::CompositeJsonSerializer());

    ArraySerializer *nestedArraySerializer = topLevelSerializer->serializeArray("a");

    ArraySerializer *firstArraySerializer = nestedArraySerializer->serializeArray();
    ASSERT_NE(firstArraySerializer, nullptr);

    firstArraySerializer->serializeValue(1);
    firstArraySerializer->serializeValue(2);
    firstArraySerializer->serializeValue(3);

    ArraySerializer *secondArraySerializer = nestedArraySerializer->serializeArray();
    ASSERT_NE(secondArraySerializer, nullptr);

    secondArraySerializer->serializeValue(4);
    secondArraySerializer->serializeValue(5);
    secondArraySerializer->serializeValue(6);

    ASSERT_EQ(topLevelSerializer->dump(), "{\"a\":[[1,2,3],[4,5,6]]}");
}

TEST(NestedLevelSerialization, ArrayOfObjectsCreation)
{
    std::unique_ptr<StructSerializer> topLevelSerializer(new json::CompositeJsonSerializer());

    ArraySerializer *nestedArraySerializer = topLevelSerializer->serializeArray("a");

    StructSerializer *firstObjectSerializer = nestedArraySerializer->serializeStruct();

    firstObjectSerializer->serializeValue("b", 1);

    StructSerializer *secondObjectSerializer = nestedArraySerializer->serializeStruct();

    secondObjectSerializer->serializeValue("c", 1);

    ASSERT_EQ(topLevelSerializer->dump(), "{\"a\":[{\"b\":1},{\"c\":1}]}");
}

TEST(NestedLevelSerialization, EmptyValueArraySerialization)
{
    std::unique_ptr<StructSerializer> topLevelSerializer(new json::CompositeJsonSerializer());

    ArraySerializer *nestedArraySerializer = topLevelSerializer->serializeArray("a");

    nestedArraySerializer->serializeValue(1);
    nestedArraySerializer->serializeEmpty();
    nestedArraySerializer->serializeValue(2);

    ASSERT_EQ(topLevelSerializer->dump(), "{\"a\":[1,null,2]}");
}

TEST(NestedLevelSerialization, RootArraySerialization)
{
    std::unique_ptr<ArraySerializer> jsonArraySerializer(new json::RootJsonArraySerializer());

    jsonArraySerializer->serializeValue(1);
    jsonArraySerializer->serializeValue(2);
    jsonArraySerializer->serializeValue(3);

    ASSERT_EQ(jsonArraySerializer->dump(), "[1,2,3]");
}
