#include <gtest/gtest.h>

#include "json_array_deserializer.hh"
#include "json_struct_deserializer.hh"

using namespace softeq::common;
using namespace softeq::common::serialization;

TEST(NestedLevelDeserialization, SimpleObjectValuesDeserialization)
{
    std::unique_ptr<StructDeserializer> jsonDeserializer(new json::CompositeJsonDeserializer());

    jsonDeserializer->setRawInput("{\"a\":1}");

    ASSERT_TRUE(jsonDeserializer->valueExists("a"));
    ASSERT_EQ(jsonDeserializer->availableNames(), std::vector<std::string>{"a"});
    stdutils::Any deserializedValue = jsonDeserializer->value("a");

    ASSERT_EQ(stdutils::any_cast<uint64_t>(deserializedValue), 1);
}

TEST(NestedLevelDeserialization, NestedStructDeserialization)
{
    std::unique_ptr<StructDeserializer> jsonDeserializer(new json::CompositeJsonDeserializer());

    jsonDeserializer->setRawInput("{\"a\":{\"b\":2}}");

    ASSERT_FALSE(jsonDeserializer->value("a").hasValue());

    StructDeserializer *nestedStructDeserializer = jsonDeserializer->deserializeStruct("a");

    ASSERT_NE(nestedStructDeserializer, nullptr);
    ASSERT_EQ(stdutils::any_cast<uint64_t>(nestedStructDeserializer->value("b")), 2);
}

TEST(NestedStructDeserialization, ArrayDeserialization)
{
    std::unique_ptr<ArrayDeserializer> jsonDeserializer(new json::JsonArrayDeserializer());

    jsonDeserializer->setRawInput("[1,2,3]");

    ASSERT_EQ(stdutils::any_cast<uint64_t>(jsonDeserializer->value()), 1);
    ASSERT_EQ(stdutils::any_cast<uint64_t>(jsonDeserializer->value()), 2);
    ASSERT_EQ(stdutils::any_cast<uint64_t>(jsonDeserializer->value()), 3);

    ASSERT_TRUE(jsonDeserializer->isComplete());
    ASSERT_FALSE(jsonDeserializer->value().hasValue());
}

TEST(NestedStructDeserialization, ArrayElementDeserialization)
{
    std::unique_ptr<StructDeserializer> jsonDeserializer(new json::CompositeJsonDeserializer());

    jsonDeserializer->setRawInput("{\"a\":[1,2]}");

    ArrayDeserializer *nestedArraySerializer = jsonDeserializer->deserializeArray("a");

    ASSERT_EQ(stdutils::any_cast<uint64_t>(nestedArraySerializer->value()), 1);
    ASSERT_EQ(stdutils::any_cast<uint64_t>(nestedArraySerializer->value()), 2);

    ASSERT_FALSE(nestedArraySerializer->value().hasValue());
}

TEST(NestedStructDeserialization, ArrayOfObjectDeserialization)
{
    std::unique_ptr<ArrayDeserializer> jsonDeserializer(new json::JsonArrayDeserializer());

    jsonDeserializer->setRawInput("[{\"a\":1},{\"a\":2}]");

    StructDeserializer *firstStructDeserializer = jsonDeserializer->deserializeStruct();
    ASSERT_NE(firstStructDeserializer, nullptr);
    ASSERT_EQ(stdutils::any_cast<uint64_t>(firstStructDeserializer->value("a")), 1);

    StructDeserializer *secondStructDeserializer = jsonDeserializer->deserializeStruct();
    ASSERT_NE(secondStructDeserializer, nullptr);
    ASSERT_EQ(stdutils::any_cast<uint64_t>(secondStructDeserializer->value("a")), 2);

    ASSERT_EQ(jsonDeserializer->deserializeStruct(), nullptr);
}

TEST(NestedStructDeserialization, ArrayOfArrayDeserialization)
{
    std::unique_ptr<ArrayDeserializer> jsonDeserializer(new json::JsonArrayDeserializer());

    jsonDeserializer->setRawInput("[[1,2],[3,4]]");

    ArrayDeserializer *firstArrayDeserializer = jsonDeserializer->deserializeArray();
    ASSERT_EQ(stdutils::any_cast<uint64_t>(firstArrayDeserializer->value()), 1);
    ASSERT_EQ(stdutils::any_cast<uint64_t>(firstArrayDeserializer->value()), 2);

    ArrayDeserializer *secondArrayDeserializer = jsonDeserializer->deserializeArray();
    ASSERT_EQ(stdutils::any_cast<uint64_t>(secondArrayDeserializer->value()), 3);
    ASSERT_EQ(stdutils::any_cast<uint64_t>(secondArrayDeserializer->value()), 4);

    ASSERT_EQ(jsonDeserializer->deserializeArray(), nullptr);
}

TEST(NestedStructDeserialization, MissedOrNullValueDesiralization)
{
    std::unique_ptr<StructDeserializer> jsonDeserializer(new json::CompositeJsonDeserializer());

    jsonDeserializer->setRawInput("{\"a\":null}");

    ASSERT_FALSE(jsonDeserializer->valueExists("a"));
    ASSERT_EQ(jsonDeserializer->deserializeArray("a"), nullptr);
    ASSERT_EQ(jsonDeserializer->deserializeStruct("a"), nullptr);

    ASSERT_FALSE(jsonDeserializer->valueExists("wrong_name"));
    ASSERT_EQ(jsonDeserializer->deserializeArray("wrong_name"), nullptr);
    ASSERT_EQ(jsonDeserializer->deserializeStruct("wrong_name"), nullptr);
}

TEST(NestedStructDeserialization, NullValuesInArray)
{
    std::unique_ptr<ArrayDeserializer> jsonDeserializer(new json::JsonArrayDeserializer());

    jsonDeserializer->setRawInput("[1,null,2]");
    ASSERT_EQ(stdutils::any_cast<uint64_t>(jsonDeserializer->value()), 1);
    ASSERT_FALSE(jsonDeserializer->nextValueExists());
    ASSERT_FALSE(jsonDeserializer->value().hasValue());
    ASSERT_TRUE(jsonDeserializer->nextValueExists());
    ASSERT_EQ(stdutils::any_cast<uint64_t>(jsonDeserializer->value()), 2);
    ASSERT_FALSE(jsonDeserializer->nextValueExists());
}
