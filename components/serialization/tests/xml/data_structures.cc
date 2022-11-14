#include "serialization_test_fixture.hh"

#include "structures/test_structure.hh"
#include "structures/custom_type.hh"
#include "structures/tuples.hh"
#include "structures/primitives_object.hh"
#include "structures/inheritance.hh"
#include "structures/map_object.hh"
#include "structures/vector_of_maps.hh"
#include "structures/enum_object.hh"
#include "structures/arrays_of_primitives.hh"
#include "structures/complex_object.hh"

#include "xml_struct_serializer.hh"
#include "xml_struct_deserializer.hh"

#include <common/serialization/xml/xml.hh> // xml::initMultiThreading and xml::cleanup

using namespace softeq::common::serialization;

TEST(SerializationCases, XmlMultiplePrimitivesArrays)
{
    testMultiplePrimitivesArrays<xml::CompositeXmlSerializer, xml::CompositeXmlDeserializer>();
}

TEST_F(Serialization, XmlSerialization)
{
    xml::CompositeXmlSerializer serializer;
    xml::CompositeXmlDeserializer deserializer;
    testComplexStructSerialization(serializer, deserializer);
}

TEST_F(Serialization, XmlMultiThreading)
{
    xml::initMultiThreading();
    testMultiThreading<xml::CompositeXmlSerializer, xml::CompositeXmlDeserializer>();
    xml::cleanup();
}

TEST_F(Serialization, XmlEnum)
{
    xml::CompositeXmlSerializer serializer;
    xml::CompositeXmlDeserializer deserializer;
    testEnumSerialization(serializer, deserializer);
}

TEST_F(Serialization, XmlMap)
{
    xml::CompositeXmlSerializer serializer;
    xml::CompositeXmlDeserializer deserializer;
    testMapSerialization(serializer, deserializer);
}

TEST_F(Serialization, DifferentXmlSerializerMapVector)
{
    std::unique_ptr<StructSerializer> serializer(new xml::CompositeXmlSerializer());
    std::unique_ptr<StructDeserializer> deserializer(new xml::CompositeXmlDeserializer());

    testMapVectorSerialization(*serializer, *deserializer);
}

TEST_F(Serialization, XmlInheritance)
{
    xml::CompositeXmlSerializer serializer;
    xml::CompositeXmlDeserializer deserializer;
    testInheritance(serializer, deserializer);
}

TEST_F(Serialization, XmlPartialSerialization)
{
    // Note: we should make minimal number of tests with raw strings

    xml::CompositeXmlSerializer serializer;
    xml::CompositeXmlDeserializer deserializer;
    partialSerialization(serializer, deserializer,
                         "<?xml version=\"1.0\"?>\n<root><i8 type=\"int\">1</i8><i16 type=\"int\">-1</i16></root>\n");
}

TEST_F(Serialization, XmlTupleSplitSerializers)
{
    testTupleSplitSerializers<xml::CompositeXmlSerializer, xml::CompositeXmlDeserializer>();
}

TEST(CustomTypeSerialization, BasicUsageXml)
{
    testBasicUsage<xml::CompositeXmlSerializer, xml::CompositeXmlDeserializer>(
        "<?xml version=\"1.0\"?>\n"
        "<root><digit type=\"string\">one</digit></root>\n");
}
