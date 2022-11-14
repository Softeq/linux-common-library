#include "serialization_test_fixture.hh"

#include "structures/test_structure.hh"
#include "structures/automatic_serialization.hh"

#include "xml_struct_serializer.hh"
#include "xml_struct_deserializer.hh"

#include "xml_array_serializer.hh"
#include "xml_array_deserializer.hh"

#include <common/serialization/xml/xml.hh>

using namespace softeq::common::serialization;

TEST(SerializationHelpers, XmlAsObjectSerializationHelper)
{
    TestStructure testObject = {.a = 10, .b = 42.0};
    std::string xmlOutput = xml::serializeAsXmlObject(testObject);
    EXPECT_FALSE(xmlOutput.empty());

    TestStructure deserializedObject = xml::deserializeFromXmlObject<TestStructure>(xmlOutput);

    EXPECT_EQ(deserializedObject, testObject);
}

TEST(SerializationHelpers, XmlAsArraySerializationHelper)
{
    std::vector<TestStructure> testObjects = {{.a = 10, .b = 42.0}, {.a = 12, .b = 64.0}};
    std::string xmlOutput = xml::serializeAsXmlArray(testObjects);

    EXPECT_FALSE(xmlOutput.empty());

    std::vector<TestStructure> deserializedObjects =
        xml::deserializeFromXmlArray<std::vector<TestStructure>>(xmlOutput);

    EXPECT_EQ(deserializedObjects, testObjects);
}

TEST(CompositeSerializers, BasicXmlObjectAssembleUsage)
{
    testBasicObjectAssembleUsage<xml::CompositeXmlSerializer, xml::CompositeXmlDeserializer>();
}

TEST(CompositeXmlSerializer, NestedStructsSerialization)
{
    testNestedStructsSerialization<xml::CompositeXmlSerializer, xml::CompositeXmlDeserializer>();
}

TEST(CompositeXmlSerializer, NestedArrayStructSerialization)
{
    testNestedStructsSerialization<xml::CompositeXmlSerializer, xml::CompositeXmlDeserializer>();
}

TEST(CompositeXmlDeserializer, OptionalStructureSerialization)
{
    testOptionalStructureSerialization<xml::CompositeXmlSerializer, xml::CompositeXmlDeserializer>();
}

TEST(CompositeXmlDeserializer, SerializationInArray)
{
    testSerializationInArray<xml::RootXmlArraySerializer, xml::RootXmlArrayDeserializer>();
}
