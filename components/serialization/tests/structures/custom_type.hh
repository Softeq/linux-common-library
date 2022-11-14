#ifndef SOFTEQ_COMMON_SERIALIZATION_TESTS_CUSTOM_TYPE_H
#define SOFTEQ_COMMON_SERIALIZATION_TESTS_CUSTOM_TYPE_H

#include <gtest/gtest.h>

#include <common/serialization/object_assembler.hh>
#include <common/serialization/helpers.hh>

struct OneDigitStruct
{
    int digit;
};

std::string nameDigit(const int &digit);
int digitByName(const std::string &name);

template <typename SerializerImpl, typename DeserializerImpl>
void testBasicUsage(const std::string &expectedString)
{
    OneDigitStruct testObject = {1};
    SerializerImpl serializer;
    DeserializerImpl deserializer;

    serializeObject(serializer, testObject);

    ASSERT_EQ(serializer.dump(), expectedString);

    OneDigitStruct restoredObject;

    deserializer.setRawInput(serializer.dump());
    deserializeObject(deserializer, restoredObject);

    ASSERT_EQ(restoredObject.digit, testObject.digit);
}

#endif // SOFTEQ_COMMON_SERIALIZATION_TESTS_CUSTOM_TYPE_H
