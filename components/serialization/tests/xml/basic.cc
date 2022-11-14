#include "serialization_test_fixture.hh"

#include "structures/basic_structures.hh"
#include "structures/map_object.hh"
#include "structures/enum_object.hh"
#include "structures/primitives_object.hh"

#include "xml_struct_serializer.hh"
#include "xml_struct_deserializer.hh"

#include "xml_array_serializer.hh"
#include "xml_array_deserializer.hh"

#include <common/serialization/xml/xml.hh>

using namespace softeq::common::serialization;

TEST_F(Serialization, XmlBasicSerialization)
{
    testBasicSerialization<xml::CompositeXmlSerializer, xml::CompositeXmlDeserializer>();
}

TEST_F(Serialization, XmlSerializationVectorFirst)
{
    testSerializationVector<xml::CompositeXmlSerializer, xml::CompositeXmlDeserializer>();
}

TEST_F(Serialization, XmlSpecialCharacters)
{
    PrimitivesObject po1{};
    po1.str = "special xml characters are: & < > \" '";
    std::string xmlText = xml::serializeAsXmlObject(po1);
    auto po2 = xml::deserializeFromXmlObject<PrimitivesObject>(xmlText);
    EXPECT_EQ(po1.str, po2.str);
}

TEST_F(Serialization, XmlFloatDeserialization)
{
    FloatObject obj;

    xml::CompositeXmlDeserializer s;

    s.setRawInput(R"(<?xml version="1.0" encoding="UTF-8"?>)"
                  R"(<root>)"
                  R"(<f type="float">-1</f>)"
                  R"(</root>)");
    deserializeObject(s, obj);
    EXPECT_EQ(obj.f, -1);

    s.setRawInput(R"(<?xml version="1.0" encoding="UTF-8"?>)"
                  R"(<root>)"
                  R"(<f type="float">-0.5</f>)"
                  R"(</root>)");
    deserializeObject(s, obj);
    EXPECT_EQ(obj.f, -0.5);

    s.setRawInput(R"(<?xml version="1.0" encoding="UTF-8"?>)"
                  R"(<root>)"
                  R"(<f type="float">0</f>)"
                  R"(</root>)");
    deserializeObject(s, obj);
    EXPECT_EQ(obj.f, 0);

    s.setRawInput(R"(<?xml version="1.0" encoding="UTF-8"?>)"
                  R"(<root>)"
                  R"(<f type="float">0.5</f>)"
                  R"(</root>)");
    deserializeObject(s, obj);
    EXPECT_EQ(obj.f, 0.5);

    s.setRawInput(R"(<?xml version="1.0" encoding="UTF-8"?>)"
                  R"(<root>)"
                  R"(<f type="float">1</f>)"
                  R"(</root>)");
    deserializeObject(s, obj);
    EXPECT_EQ(obj.f, 1);
}

TEST_F(Serialization, XmlOptional)
{
    testSerializationOptional<xml::CompositeXmlSerializer, xml::CompositeXmlDeserializer>();
}

TEST_F(Serialization, XmlError)
{
    using XmlDeserializer = xml::CompositeXmlDeserializer;
    // clang-format off
    tryErrorCase<XmlDeserializer, Object::SimpleSubObject>(
        "empty object", R"(<?xml version="1.0" encoding="UTF-8"?>)"
                        R"(<root></root>)");

    tryErrorCase<XmlDeserializer, Object::SimpleSubObject>(
        "wrong name",   R"(<?xml version="1.0" encoding="UTF-8"?>)"
                        R"(<root>)"
                            R"(<i1 type="int">0</i1>)"
                        R"(</root>)");

    tryErrorCase<XmlDeserializer, Object::SimpleSubObject>(
        "wrong primitive type", R"(<?xml version="1.0" encoding="UTF-8"?>)"
                                R"(<root>)"
                                    R"(<i type="string">0</i>)"
                                R"(</root>)");

    tryErrorCase<XmlDeserializer, Object::SimpleSubObject>(
        "not-supported primitive type", R"(<?xml version="1.0" encoding="UTF-8"?>)"
                                        R"(<root>)"
                                            R"(<i type="not-supported-type">0</i>)"
                                        R"(</root>)");

    tryErrorCase<XmlDeserializer, Object::SimpleSubObject>(
        "missing >",    R"(<?xml version="1.0" encoding="UTF-8"?>)"
                        R"(<root>)"
                            R"(<i type="int">0</i)"
                        R"(</root>)");

    tryErrorCase<XmlDeserializer, Object::SimpleSubObject>(
        "missing type attribute",   R"(<?xml version="1.0" encoding="UTF-8"?>)"
                                    R"(<root>)"
                                        R"(<i>0</i>)"
                                    R"(</root>)");

    tryErrorCase<XmlDeserializer, Object::SimpleSubObject>(
        "missing value for numeric primitive",  R"(<?xml version="1.0" encoding="UTF-8"?>)"
                                                R"(<root>)"
                                                    R"(<i type="int"></i>)"
                                                R"(</root>)");

    tryErrorCase<XmlDeserializer, Object::SimpleSubObject>(
        "string instead of int",    R"(<?xml version="1.0" encoding="UTF-8"?>)"
                                    R"(<root>)"
                                        R"(<i type="int">xyz</i>)"
                                    R"(</root>)");

    tryErrorCase<XmlDeserializer, Object::SimpleSubObject>(
        "node instead of primitive",    R"(<?xml version="1.0" encoding="UTF-8"?>)"
                                        R"(<root>)"
                                            R"(<i type="int">)"
                                                R"(<i1>0</i1>)"
                                            R"(</i>)"
                                        R"(</root>)");


    tryErrorCase<XmlDeserializer, VecObject>(
         "wrong array container entry format",
                R"(<?xml version="1.0" encoding="UTF-8"?>)"
                R"(<root>)"
                    R"(<vi>)"
                        R"(<___containerEntry___ type="int">1</___containerEntry___>)"
                        R"(<a type="int">2</a>)"
                    R"(</vi>)"
                R"(</root>)");

    // should be <strm><___containerEntry___><a><i type="int">0</i></a></___containerEntry___></strm>
    tryErrorCase<XmlDeserializer, MapObject>(
         "wrong entry format for map with string key(string key map is serialized as struct)",
                R"(<?xml version="1.0" encoding="UTF-8"?>)"
                R"(<root>)"
                    R"(<strm>)"
                        R"(<___containerEntry___>)"
                            R"(<__mapKey__ type="string">a</__mapKey__>)"
                            R"(<__mapValue__><i type="int">0</i></__mapValue__>)"
                        R"(</___containerEntry___>)"
                    R"(</strm>)"
                    R"(<m/><um/><ms/><ms1/>)"
                R"(</root>)");

    tryErrorCase<XmlDeserializer, MapObject>(
         "wrong map entry key format",
                R"(<?xml version="1.0" encoding="UTF-8"?>)"
                R"(<root>)"
                    R"(<m>)"
                        R"(<___containerEntry___>)"
                            R"(<mapKey__ type="int">0</mapKey__>)"
                            R"(<__mapValue__ type="int">0</__mapValue__>)"
                        R"(</___containerEntry___>)"
                    R"(</m>)"
                    R"(<strm/><um/><ms/><ms1/><strm/>)"
                R"(</root>)");

    tryErrorCase<XmlDeserializer, MapObject>(
         "wrong map entry value format",
                R"(<?xml version="1.0" encoding="UTF-8"?>)"
                R"(<root>)"
                    R"(<m>)"
                        R"(<___containerEntry___>)"
                            R"(<__mapKey__ type="int">0</__mapKey__>)"
                            R"(<mapValue__ type="int">0</mapValue__>)"
                        R"(</___containerEntry___>)"
                    R"(</m>)"
                    R"(<um/><ms/><ms1/><strm/>)"
                R"(</root>)");

    tryErrorCase<XmlDeserializer, MapObject>(
         "empty entry value for map",
                R"(<?xml version="1.0" encoding="UTF-8"?>)"
                R"(<root>)"
                    R"(<strm>)"
                        R"(<a></a>)"
                    R"(</strm>)"
                    R"(<m/><um/><ms/><ms1/>)"
                R"(</root>)");

    tryErrorCase<XmlDeserializer, PrimitivesObject>(
         "int8_t is out of range",
                R"(<?xml version="1.0" encoding="UTF-8"?>)"
                R"(<root>)"
                    R"(<b type="bool">false</b>)"
                    R"(<i8 type="int">4096</i8>)"
                    R"(<ui8 type="uint">0</ui8>)"
                    R"(<i16 type="int">0</i16>)"
                    R"(<ui16 type="uint">0</ui16>)"
                    R"(<i32 type="int">0</i32>)"
                    R"(<ui32 type="uint">0</ui32>)"
                    R"(<i64 type="int">0</i64>)"
                    R"(<ui64 type="uint">0</ui64>)"
                    R"(<f type="float">0.000000</f>)"
                    R"(<d type="float">0.000000</d>)"
                    R"(<str type="string"></str>)"
                R"(</root>)");

    tryErrorCase<XmlDeserializer, PrimitivesObject>(
         "int8_t is negative and out of range",
                R"(<?xml version="1.0" encoding="UTF-8"?>)"
                R"(<root>)"
                    R"(<b type="bool">false</b>)"
                    R"(<i8 type="int">-4096</i8>)"
                    R"(<ui8 type="uint">0</ui8>)"
                    R"(<i16 type="int">0</i16>)"
                    R"(<ui16 type="uint">0</ui16>)"
                    R"(<i32 type="int">0</i32>)"
                    R"(<ui32 type="uint">0</ui32>)"
                    R"(<i64 type="int">0</i64>)"
                    R"(<ui64 type="uint">0</ui64>)"
                    R"(<f type="float">0.000000</f>)"
                    R"(<d type="float">0.000000</d>)"
                    R"(<str type="string"></str>)"
                R"(</root>)");

    tryErrorCase<XmlDeserializer, PrimitivesObject>(
         "uint8_t is negative",
                R"(<?xml version="1.0" encoding="UTF-8"?>)"
                R"(<root>)"
                    R"(<b type="bool">false</b>)"
                    R"(<i8 type="int">0</i8>)"
                    R"(<ui8 type="uint">-1</ui8>)"
                    R"(<i16 type="int">0</i16>)"
                    R"(<ui16 type="uint">0</ui16>)"
                    R"(<i32 type="int">0</i32>)"
                    R"(<ui32 type="uint">0</ui32>)"
                    R"(<i64 type="int">0</i64>)"
                    R"(<ui64 type="uint">0</ui64>)"
                    R"(<f type="float">0.000000</f>)"
                    R"(<d type="float">0.000000</d>)"
                    R"(<str type="string"></str>)"
                R"(</root>)");

    tryErrorCase<XmlDeserializer, PrimitivesObject>(
         "uint8_t is out of range",
                R"(<?xml version="1.0" encoding="UTF-8"?>)"
                R"(<root>)"
                    R"(<b type="bool">false</b>)"
                    R"(<i8 type="int">0</i8>)"
                    R"(<ui8 type="uint">4096</ui8>)"
                    R"(<i16 type="int">0</i16>)"
                    R"(<ui16 type="uint">0</ui16>)"
                    R"(<i32 type="int">0</i32>)"
                    R"(<ui32 type="uint">0</ui32>)"
                    R"(<i64 type="int">0</i64>)"
                    R"(<ui64 type="uint">0</ui64>)"
                    R"(<f type="float">0.000000</f>)"
                    R"(<d type="float">0.000000</d>)"
                    R"(<str type="string"></str>)"
                R"(</root>)");

    tryErrorCase<XmlDeserializer, PrimitivesObject>(
         "wrong boolean value",
                R"(<?xml version="1.0" encoding="UTF-8"?>)"
                R"(<root>)"
                    R"(<b type="bool">undefined</b>)"
                    R"(<i8 type="int">0</i8>)"
                    R"(<ui8 type="uint">0</ui8>)"
                    R"(<i16 type="int">0</i16>)"
                    R"(<ui16 type="uint">0</ui16>)"
                    R"(<i32 type="int">0</i32>)"
                    R"(<ui32 type="uint">0</ui32>)"
                    R"(<i64 type="int">0</i64>)"
                    R"(<ui64 type="uint">0</ui64>)"
                    R"(<f type="float">0.000000</f>)"
                    R"(<d type="float">0.000000</d>)"
                    R"(<str type="string"></str>)"
                R"(</root>)");

    tryErrorCase<XmlDeserializer, EnumObject>(
         "wrong enum value",
                R"(<?xml version="1.0" encoding="UTF-8"?>)"
                R"(<root>)"
                    R"(<ev type="string">enum11</ev>)"
                    R"(<ecv type="string">enumClass1</ecv>)"
                R"(</root>)");

    tryErrorCase<XmlDeserializer, EnumObject>(
         "wrong enum class value",
                R"(<?xml version="1.0" encoding="UTF-8"?>)"
                R"(<root>)"
                    R"(<ev type="string">enum1</ev>)"
                    R"(<ecv type="string">enumClass11</ecv>)"
                R"(</root>)");

    // clang-format on
}

TEST_F(Serialization, XmlOptionalDeserialization)
{
    // clang format off
    std::string xmlErrorStr = R"(<?xml version="1.0" encoding="UTF-8"?>)"
                              R"(<root>)"
                              R"(<oit type="int">-100</oit>)"
                              // NOTE: Probably better throw exception than treat parse error as null
                              R"(<oif type="int">abc</oif>)"
                              R"(<ouit type="uint">100</ouit>)"
                              R"(<ouic type="uint">efg</ouic>)"
                              R"(<ouis type="uint">-10</ouis>)"
                              R"(<oft type="float">0.42</oft>)"
                              R"(<ofc type="float">efg</ofc>)"
                              R"(<ost>)"
                              R"(<i type="int">-255</i>)"
                              R"(</ost>)"
                              R"(<osf>)"
                              R"(<i type="int">hij</i>)"
                              R"(</osf>)"
                              R"(<voit>)"
                              R"(<___containerEntry___ type="int">1</___containerEntry___>)"
                              R"(<___containerEntry___ type="int">2</___containerEntry___>)"
                              R"(<___containerEntry___ type="int">3</___containerEntry___>)"
                              R"(<___containerEntry___ type="int">4</___containerEntry___>)"
                              R"(</voit>)"
                              R"(<voif>)"
                              R"(<___containerEntry___ type="int">1</___containerEntry___>)"
                              R"(<___containerEntry___ type="int">hijk</___containerEntry___>)"
                              R"(<___containerEntry___ type="int">3</___containerEntry___>)"
                              R"(<___containerEntry___ type="int">lmno</___containerEntry___>)"
                              R"(</voif>)"
                              R"(<mt>)"
                              R"(<___containerEntry___>)"
                              R"(<__mapKey__ type="int">1</__mapKey__>)"
                              R"(<__mapValue__ type="int">0</__mapValue__>)"
                              R"(</___containerEntry___>)"
                              R"(</mt>)"
                              R"(<mff>)"
                              R"(<___containerEntry___>)"
                              R"(<__mapKey__ type="int">"1"</__mapKey__>)"
                              R"(<__mapValue__ type="int">0</__mapValue__>)"
                              R"(</___containerEntry___>)"
                              R"(</mff>)"
                              R"(<mfs>)"
                              R"(<___containerEntry___>)"
                              R"(<__mapKey__ type="int">1</__mapKey__>)"
                              R"(<__mapValue__ type="int">"0"</__mapValue__>)"
                              R"(</___containerEntry___>)"
                              R"(</mfs>)"
                              R"(<mts>)"
                              R"(<___containerEntry___>)"
                              R"(<__mapKey__ type="int">1</__mapKey__>)"
                              R"(<__mapValue__ type="int">0</__mapValue__>)"
                              R"(</___containerEntry___>)"
                              R"(</mts>)"
                              R"(<umt>)"
                              R"(<___containerEntry___>)"
                              R"(<__mapKey__ type="int">1</__mapKey__>)"
                              R"(<__mapValue__ type="int">0</__mapValue__>)"
                              R"(</___containerEntry___>)"
                              R"(</umt>)"
                              R"(<umff>)"
                              R"(<___containerEntry___>)"
                              R"(<__mapKey__ type="int">"1"</__mapKey__>)"
                              R"(<__mapValue__ type="int">0</__mapValue__>)"
                              R"(</___containerEntry___>)"
                              R"(</umff>)"
                              R"(<umfs>)"
                              R"(<___containerEntry___>)"
                              R"(<__mapKey__ type="int">1</__mapKey__>)"
                              R"(<__mapValue__ type="int">"0"</__mapValue__>)"
                              R"(</___containerEntry___>)"
                              R"(</umfs>)"
                              R"(<umts>)"
                              R"(<___containerEntry___>)"
                              R"(<__mapKey__ type="int">1</__mapKey__>)"
                              R"(<__mapValue__ type="int">0</__mapValue__>)"
                              R"(</___containerEntry___>)"
                              R"(</umts>)"
                              R"(</root>)";

    auto singleOptionalObject = xml::deserializeFromXmlObject<SingleOptionalObject>(xmlErrorStr);

    EXPECT_EQ(singleOptionalObject.oit.hasValue(), true);
    EXPECT_EQ(singleOptionalObject.oif.hasValue(), false);
    EXPECT_EQ(singleOptionalObject.ouit.hasValue(), true);
    EXPECT_EQ(singleOptionalObject.ouic.hasValue(), false);
    EXPECT_EQ(singleOptionalObject.ouis.hasValue(), false);
    EXPECT_EQ(singleOptionalObject.oft.hasValue(), true);
    EXPECT_EQ(singleOptionalObject.ofc.hasValue(), false);
    EXPECT_EQ(singleOptionalObject.ost.hasValue(), true);
    EXPECT_EQ(singleOptionalObject.osf.hasValue(), false);
    EXPECT_EQ(singleOptionalObject.voit.hasValue(), true);
    EXPECT_EQ(singleOptionalObject.voif.hasValue(), false);
    EXPECT_EQ(singleOptionalObject.mt.hasValue(), true);
    EXPECT_EQ(singleOptionalObject.mff.hasValue(), false);
    EXPECT_EQ(singleOptionalObject.mfs.hasValue(), false);
    EXPECT_EQ(singleOptionalObject.mts.hasValue(), true);
    EXPECT_EQ(singleOptionalObject.umt.hasValue(), true);
    EXPECT_EQ(singleOptionalObject.umff.hasValue(), false);
    EXPECT_EQ(singleOptionalObject.umfs.hasValue(), false);
    EXPECT_EQ(singleOptionalObject.umts.hasValue(), true);
}
