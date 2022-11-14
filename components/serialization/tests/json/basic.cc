#include "serialization_test_fixture.hh"

#include "structures/basic_structures.hh"
#include "structures/map_object.hh"
#include "structures/enum_object.hh"
#include "structures/primitives_object.hh"

#include "json_struct_serializer.hh"
#include "json_struct_deserializer.hh"

#include "json_array_serializer.hh"
#include "json_array_deserializer.hh"

#include <common/serialization/json/json.hh>

using namespace softeq::common::serialization;

TEST_F(Serialization, JsonBasicSerialization)
{
    testBasicSerialization<json::CompositeJsonSerializer, json::CompositeJsonDeserializer>();
}

TEST_F(Serialization, JsonSerializationVectorFirst)
{
    testSerializationVector<json::CompositeJsonSerializer, json::CompositeJsonDeserializer>();
}

TEST_F(Serialization, JsonSpecialCharacters)
{
    PrimitivesObject po1{};
    po1.str = "special json characters are: \" ' \n \f \t \f \\";
    std::string jsonText = json::serializeAsJsonObject(po1);
    auto po2 = json::deserializeFromJsonObject<PrimitivesObject>(jsonText);
    EXPECT_EQ(po1.str, po2.str);
}

TEST_F(Serialization, JsonFloatDeserialization)
{
    FloatObject obj;

    json::CompositeJsonDeserializer s;

    s.setRawInput(R"({"f":-1})");
    deserializeObject(s, obj);
    EXPECT_EQ(obj.f, -1);

    s.setRawInput(R"({"f":-0.5})");
    deserializeObject(s, obj);
    EXPECT_EQ(obj.f, -0.5);

    s.setRawInput(R"({"f":0})");
    deserializeObject(s, obj);
    EXPECT_EQ(obj.f, 0);

    s.setRawInput(R"({"f":0.5})");
    deserializeObject(s, obj);
    EXPECT_EQ(obj.f, 0.5);

    s.setRawInput(R"({"f":1})");
    deserializeObject(s, obj);
    EXPECT_EQ(obj.f, 1);
}

TEST_F(Serialization, JsonOptional)
{
    testSerializationOptional<json::CompositeJsonSerializer, json::CompositeJsonDeserializer>();

    // json null testing
    OptionalObject obj1 = {};
    json::CompositeJsonDeserializer s1;
    s1.setRawInput(R"({"oi":null,"voi":[1,null,3],"oss":null,"voss":[{"i":4},null,{"i":6}]})");

    deserializeObject(s1, obj1);
    EXPECT_FALSE(obj1.oi.hasValue());
    EXPECT_EQ(obj1.voi.size(), 3);
    EXPECT_EQ(obj1.voi[0], 1);
    EXPECT_FALSE(obj1.voi[1].hasValue());
    EXPECT_EQ(obj1.voi[2], 3);

    EXPECT_FALSE(obj1.oss.hasValue());
    EXPECT_EQ(obj1.voss.size(), 3);
    EXPECT_EQ(obj1.voss[0].cValue().i, 4);
    EXPECT_FALSE(obj1.voss[1].hasValue());
    EXPECT_EQ(obj1.voss[2].cValue().i, 6);
}

TEST_F(Serialization, JsonError)
{
    // TODO: need more tests for null .we need to think how to test all assebler specializations at once
    tryErrorCase<json::RootJsonArrayDeserializer, std::list<std::string>>("null in array", "[null]");

    tryErrorCase<json::CompositeJsonDeserializer, Object::SimpleSubObject>("empty object", "{}");
    tryErrorCase<json::CompositeJsonDeserializer, Object::SimpleSubObject>("wrong name", R"({"i1":0})");
    tryErrorCase<json::CompositeJsonDeserializer, Object::SimpleSubObject>("string instead of int", R"({"i":"0"})");
    tryErrorCase<json::CompositeJsonDeserializer, Object::SimpleSubObject>("array instead of int", R"({"i":[0]})");
    tryErrorCase<json::CompositeJsonDeserializer, Object::SimpleSubObject>("null instead of int", R"({"i":null})");
    tryErrorCase<json::CompositeJsonDeserializer, Object::SimpleSubObject>("missing }", R"({"i":0)");

    tryErrorCase<json::CompositeJsonDeserializer, Object::SubObject>(
        "array instead of object", R"({"sri":0,"srs":"","svi":[],"svvi":[],"svvs":[[{"i":1}, [2]]] })");
    tryErrorCase<json::CompositeJsonDeserializer, Object::SubObject>(
        "null instead of object", R"({"sri":0,"srs":"","svi":[],"svvi":[],"svvs":[[null]] })");
    tryErrorCase<json::CompositeJsonDeserializer, Object::SubObject>(
        "int instead of object", R"({"sri":0,"srs":"","svi":[],"svvi":[],"svvs":[[1]] })");
    // TODO: actually json is valid and will be deserialized as empty
    //    tryErrorCase<json::CompositeJsonDeserializer, Object::SubObject>(
    //        "object instead of array", R"({"sri":0,"srs":"","svi":{},"svvi":[],"svvs":[]})");
    tryErrorCase<json::CompositeJsonDeserializer, Object::SubObject>(
        "int instead of array", R"({"sri":0,"srs":"","svi":[],"svvi":[],"svvs":[1] })");
    tryErrorCase<json::CompositeJsonDeserializer, Object::SubObject>(
        "null instead of array", R"({"sri":0,"srs":"","svi":[],"svvi":[],"svvs": null })");

    tryErrorCase<json::CompositeJsonDeserializer, MapObject>(
        "wrong entry format for map with string key(string key map is serialized as struct)",
        R"({"m":[],"ms":[],"ms1":[],"strm":[{"__mapKey__":"a","__mapValue__":0}],"um":[]})");

    tryErrorCase<json::CompositeJsonDeserializer, PrimitivesObject>(
        "int instead of bool", R"({"b":1,"d":0.0,"f":0.0,"i16":0,"i32":0,"i64":0,)"
                               R"("i8":0,"str":"","ui16":0,"ui32":0,"ui64":0,"ui8":0})");

    tryErrorCase<json::CompositeJsonDeserializer, PrimitivesObject>(
        "int8_t out of range", R"({"b":true,"d":0.0,"f":0.0,"i16":0,"i32":0,"i64":0,)"
                               R"("i8":4096,"str":"","ui16":0,"ui32":0,"ui64":0,"ui8":0})");

    tryErrorCase<json::CompositeJsonDeserializer, PrimitivesObject>(
        "int8_t negative and out of range", R"({"b":true,"d":0.0,"f":0.0,"i16":0,"i32":0,"i64":0,)"
                                            R"("i8":-4096,"str":"","ui16":0,"ui32":0,"ui64":0,"ui8":0})");

    tryErrorCase<json::CompositeJsonDeserializer, PrimitivesObject>(
        "uint8_t is negative", R"({"b":true,"d":0.0,"f":0.0,"i16":0,"i32":0,"i64":0,)"
                               R"("i8":0,"str":"","ui16":0,"ui32":0,"ui64":0,"ui8":-1})");

    tryErrorCase<json::CompositeJsonDeserializer, PrimitivesObject>(
        "uint8_t is out of range", R"({"b":true,"d":0.0,"f":0.0,"i16":0,"i32":0,"i64":0,)"
                                   R"("i8":0,"str":"","ui16":0,"ui32":0,"ui64":0,"ui8": 4096})");

    tryErrorCase<json::CompositeJsonDeserializer, PrimitivesObject>(
        "wrong boolean value", R"({"b":wrong,"d":0.0,"f":0.0,"i16":0,"i32":0,"i64":0,)"
                               R"("i8":0,"str":"","ui16":0,"ui32":0,"ui64":0,"ui8": 0})");
}

TEST_F(Serialization, JsonOptionalDeserialization)
{
    std::string jsonErrorStr = R"(
        {
           "oit":-100,
           "oif":"abc",
           "ouit":100,
           "ouic":"efg",
           "ouis":-10,
           "oft":0.42,
           "ofc":"efg",
           "ost":{
              "i":-255
           },
           "osf":{
              "i":"hij"
           },
           "voit":[1, 2, 3, 4],
           "voif":[1, "hijk", "3","lmno"],
           "mt":[
              {
                 "__mapKey__":1,
                 "__mapValue__":0
              }
           ],
           "mtf":[
              {
                 "__mapKey__":"1",
                 "__mapValue__":0
              }
           ],
           "mfs":[
              {
                 "__mapKey__":1,
                 "__mapValue__":"0"
              }
           ],
           "mts":[
              {
                 "__mapKey__":1,
                 "__mapValue__":0
              }
           ],
           "umt":[
              {
                 "__mapKey__":1,
                 "__mapValue__":0
              }
           ],
           "umff":[
              {
                 "__mapKey__":"1",
                 "__mapValue__":0
              }
           ],
           "umfs":[
              {
                 "__mapKey__":1,
                 "__mapValue__":"0"
              }
           ],
           "umts":[
              {
                 "__mapKey__":1,
                 "__mapValue__":0
              }
           ]
        }
    )";

    auto singleOptionalObject = json::deserializeFromJsonObject<SingleOptionalObject>(jsonErrorStr);

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
