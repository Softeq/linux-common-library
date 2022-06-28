#include <gtest/gtest.h>

#include "softeq/common/log.hh"

#include "softeq/common/serialization/object_assembler.hh"
#include "softeq/common/serialization/json_serializer.hh"
#include "softeq/common/serialization/xml_serializer.hh"
#include "softeq/common/serialization/helpers.hh"

using namespace softeq::common;

class Serialization : public ::testing::Test
{
protected:
    void SetUp()
    {
        savedLevel = softeq::common::log().level();
        softeq::common::log().level(LogLevel::NONE);
    }
    void Teardown()
    {
        softeq::common::log().level(savedLevel);
    }
    LogLevel savedLevel;
};

struct Object
{
    int i;
    bool b;
    std::string s;
    float f;
    double d;
    struct SimpleSubObject
    {
        int i;
    };
    struct SubObject
    {
        int sri;
        std::string srs;
        std::vector<int> svi;
        std::vector<std::vector<int>> svvi;
        std::vector<std::vector<SimpleSubObject>> svvs;
    } sr;
    std::vector<int> vi;

    std::vector<SimpleSubObject> vss;
    std::vector<SubObject> vsr;
    unsigned int ui;
    unsigned short us;
    std::list<int> li;
    Optional<int> oi1;
    Optional<int> oi2;
};

bool operator<(const Object::SimpleSubObject &l, const Object::SimpleSubObject &r)
{
    return l.i < r.i;
}

template <>
serialization::ObjectAssembler<Object> serialization::Assembler()
{
    // clang-format off
    return ObjectAssembler<Object>()
        .define("i", &Object::i)
        .define("b", &Object::b)
        .define("s", &Object::s)
        .define("f", &Object::f)
        .define("d", &Object::d)
        .define("sr", &Object::sr)
        .define("vi", &Object::vi)
        .define("vss", &Object::vss)
        .define("vsr", &Object::vsr)
        .define("us", &Object::us)
        .define("ui", &Object::ui)
        .define("li", &Object::li)
        .define("oi1", &Object::oi1)
        .define("oi2", &Object::oi2)
        ;
    // clang-format on
}

template <>
serialization::ObjectAssembler<Object::SimpleSubObject> serialization::Assembler()
{
    // clang-format off
    return ObjectAssembler<Object::SimpleSubObject>()
        .define("i", &Object::SimpleSubObject::i);
    // clang-format on
}

template <>
serialization::ObjectAssembler<Object::SubObject> serialization::Assembler()
{
    // clang-format off
    return ObjectAssembler<Object::SubObject>()
        .define("sri", &Object::SubObject::sri)
        .define("srs", &Object::SubObject::srs)
        .define("svi", &Object::SubObject::svi)
        .define("svvi", &Object::SubObject::svvi)
        .define("svvs", &Object::SubObject::svvs)
        ;
    // clang-format on
}

struct PrimitivesObject
{
    bool b;
    int8_t i8;
    uint8_t ui8;
    int16_t i16;
    uint16_t ui16;
    int32_t i32;
    uint32_t ui32;
    int64_t i64;
    uint64_t ui64;
    float f;
    double d;
    std::string str;
};

template <>
serialization::ObjectAssembler<PrimitivesObject> serialization::Assembler()
{
    // clang-format off
    return ObjectAssembler<PrimitivesObject>()
        .define("b", &PrimitivesObject::b)
        .define("i8", &PrimitivesObject::i8)
        .define("ui8", &PrimitivesObject::ui8)
        .define("i16", &PrimitivesObject::i16)
        .define("ui16", &PrimitivesObject::ui16)
        .define("i32", &PrimitivesObject::i32)
        .define("ui32", &PrimitivesObject::ui32)
        .define("i64", &PrimitivesObject::i64)
        .define("ui64", &PrimitivesObject::ui64)
        .define("f", &PrimitivesObject::f)
        .define("d", &PrimitivesObject::d)
        .define("str", &PrimitivesObject::str)
        ;
    // clang-format on
}

struct VecObject
{
    std::vector<int> vi;
};

template <>
serialization::ObjectAssembler<VecObject> serialization::Assembler()
{
    // clang-format off
    return ObjectAssembler<VecObject>()
        .define("vi", &VecObject::vi)
        ;
    // clang-format on
}

struct FloatObject
{
    float f;
};

template <>
serialization::ObjectAssembler<FloatObject> serialization::Assembler()
{
    // clang-format off
    return ObjectAssembler<FloatObject>()
        .define("f", &FloatObject::f)
        ;
    // clang-format on
}


struct OptionalObject
{
    Optional<int> oi;
    std::vector<Optional<int>> voi;
    Optional<Object::SimpleSubObject> oss;
    std::vector<Optional<Object::SimpleSubObject>> voss;
};

template <>
serialization::ObjectAssembler<OptionalObject> serialization::Assembler()
{
    // clang-format off
    return ObjectAssembler<OptionalObject>()
        .define("oi", &OptionalObject::oi)
        .define("voi", &OptionalObject::voi)
        .define("oss", &OptionalObject::oss)
        .define("voss", &OptionalObject::voss)
        ;
    // clang-format on
}

struct SingleOptionalObject
{
    Optional<int> oit;
    Optional<int> oif;
    Optional<uint> ouit;
    Optional<uint> ouic;
    Optional<uint> ouis;
    Optional<float> oft;
    Optional<float> ofc;
    Optional<Object::SimpleSubObject> ost;
    Optional<Object::SimpleSubObject> osf;
    Optional<std::vector<int>> voit;
    Optional<std::vector<int>> voif;
    Optional<std::map<int, int>> mt;
    Optional<std::map<int, int>> mff;
    Optional<std::map<int, int>> mfs;
    Optional<std::map<int, int>> mts;
    Optional<std::unordered_map<int, int>> umt;
    Optional<std::unordered_map<int, int>> umff;
    Optional<std::unordered_map<int, int>> umfs;
    Optional<std::unordered_map<int, int>> umts;
};

template <>
serialization::ObjectAssembler<SingleOptionalObject> serialization::Assembler()
{
    // clang-format off
    return ObjectAssembler<SingleOptionalObject>()
    .define("oit", &SingleOptionalObject::oit)
    .define("oif", &SingleOptionalObject::oif)
    .define("ouit", &SingleOptionalObject::ouit)
    .define("ouic", &SingleOptionalObject::ouic)
    .define("ouis", &SingleOptionalObject::ouis)
    .define("oft", &SingleOptionalObject::oft)
    .define("ofc", &SingleOptionalObject::ofc)
    .define("ost", &SingleOptionalObject::ost)
    .define("osf", &SingleOptionalObject::osf)
    .define("voit", &SingleOptionalObject::voit)
    .define("voif", &SingleOptionalObject::voif)
    .define ("mt", &SingleOptionalObject::mt)
    .define ("mff", &SingleOptionalObject::mff)
    .define ("mfs", &SingleOptionalObject::mfs)
    .define ("mts", &SingleOptionalObject::mts)
    .define ("umt", &SingleOptionalObject::umt)
    .define ("umff", &SingleOptionalObject::umff)
    .define ("umfs", &SingleOptionalObject::umfs)
    .define ("umts", &SingleOptionalObject::umts)
    ;
    // clang-format on
}

struct MapObject
{
    std::map<int, int> m;
    std::unordered_map<int, int> um;
    std::map<int, Object::SimpleSubObject> ms;
    std::map<Object::SimpleSubObject, Object::SimpleSubObject> ms1;
    std::map<std::string, Object::SimpleSubObject> strm;
};

template <>
serialization::ObjectAssembler<MapObject> serialization::Assembler()
{
    // clang-format off
    return ObjectAssembler<MapObject>()
        .define("m", &MapObject::m)
        .define("um", &MapObject::um)
        .define("ms", &MapObject::ms)
        .define("ms1", &MapObject::ms1)
        .define("strm", &MapObject::strm)
        ;
    // clang-format on
}

enum EnumValues
{
    enum1,
    enum2,
};

template <>
serialization::ObjectAssembler<EnumValues> serialization::Assembler()
{
    // clang-format off
    return ObjectAssembler<EnumValues>()
        .define("enum1", enum1)
        .define("enum2", enum2)
        ;
    // clang-format on
}

enum class EnumClassValues
{
    enumClass1,
    enumClass2,
    enumClass3,
};

template <>
serialization::ObjectAssembler<EnumClassValues> serialization::Assembler()
{
    // clang-format off
    return ObjectAssembler<EnumClassValues>()
        .define("enumClass1", EnumClassValues::enumClass1)
        .define("enumClass2", EnumClassValues::enumClass2)
        .define("enumClass3", EnumClassValues::enumClass3)
        ;
    // clang-format on
}

struct EnumObject
{
    EnumValues ev;
    EnumClassValues ecv;
};

template <>
serialization::ObjectAssembler<EnumObject> serialization::Assembler()
{
    // clang-format off
    return ObjectAssembler<EnumObject>()
        .define("ev", &EnumObject::ev)
        .define("ecv", &EnumObject::ecv)
        ;
    // clang-format on
}

struct ErrorNameDefineObject
{
    int i1;
    int i2;
};

template <>
serialization::ObjectAssembler<ErrorNameDefineObject> serialization::Assembler()
{
    // clang-format off
    return ObjectAssembler<ErrorNameDefineObject>()
        .define("i1", &ErrorNameDefineObject::i1)
        .define("i1", &ErrorNameDefineObject::i2)
        ;
    // clang-format on
}

struct ErrorMemberDefineObject
{
    int i1;
    int i2;
};

template <>
serialization::ObjectAssembler<ErrorMemberDefineObject> serialization::Assembler()
{
    // clang-format off
    return ObjectAssembler<ErrorMemberDefineObject>()
        .define("i1", &ErrorMemberDefineObject::i1)
        .define("i2", &ErrorMemberDefineObject::i1)
        ;
    // clang-format on
}

enum EnumMemberErrorValues
{
    enumError1,
    enumError2,
};

template <>
serialization::ObjectAssembler<EnumMemberErrorValues> serialization::Assembler()
{
    // clang-format off
    return ObjectAssembler<EnumMemberErrorValues>()
        .define("enumError1", enumError1)
        .define("enumError1", enumError2)
        ;
    // clang-format on
}

struct A
{
    int a;
};

template <>
serialization::ObjectAssembler<A> serialization::Assembler()
{
    // clang-format off
    return ObjectAssembler<A>()
        .define("a", &A::a);
    // clang-format on
}

struct AnotherA
{
    int aa;
};

template <>
serialization::ObjectAssembler<AnotherA> serialization::Assembler()
{
    // clang-format off
    return ObjectAssembler<AnotherA>()
        .define("aa", &AnotherA::aa);
    // clang-format on
}

struct B
    : A
    , AnotherA
{
    int b;
};

template <>
serialization::ObjectAssembler<B> serialization::Assembler()
{
    // clang-format off
    return ObjectAssembler<B>()
        .define("b", &B::b)
        .extend<A>("A")
        .extend<AnotherA>("AnotherA");
    // clang-format on
}

struct C : B
{
    int c;
};

template <>
serialization::ObjectAssembler<C> serialization::Assembler()
{
    // clang-format off
    return ObjectAssembler<C>()
        .define("c", &C::c)
        .extend<B>("B");
    // clang-format on
}

template <typename T>
void ComplexStructSerialization()
{
    Object obj;
    obj.i = -1;
    obj.b = true;
    obj.s = "abc";
    obj.f = 2.0;
    obj.d = 3.0;
    obj.sr.sri = 4;
    obj.sr.srs = "def";
    obj.sr.svi = {5, 6};
    obj.sr.svvi = {{7, 8, 9}, {10, 11, 12, 13}, {14, 15}};
    obj.sr.svvs = {{{16}, {17}, {18}}, {{19}}, {{20}}, {{21}}};
    obj.vi = {22, 23, 24, 25};
    obj.vss = {{26}, {27}};
    obj.us = 28;
    obj.ui = 29;
    obj.vsr = {obj.sr, obj.sr};
    obj.li = {30, 31, 32, 33};
    obj.oi1 = {34};
    obj.oi2 = Optional<int>();

    T s;
    serializeObject(s, obj);
    Object obj1 = {};

    // This sleep is for a guaranteed thread switch
    // in case of a multithreading test
    std::this_thread::sleep_for(std::chrono::milliseconds(20));

    T s1;
    EXPECT_FALSE(s.dump().empty());
    s1.setRawInput(s);
    deserializeObject(s1, obj1);
    EXPECT_EQ(obj.i, obj1.i);
    EXPECT_EQ(obj.b, obj1.b);
    EXPECT_EQ(obj.s, obj1.s);
    EXPECT_EQ(obj.f, obj1.f);
    EXPECT_EQ(obj.d, obj1.d);

    EXPECT_EQ(obj.sr.sri, obj1.sr.sri);
    EXPECT_EQ(obj.sr.srs, obj1.sr.srs);
    EXPECT_EQ(obj.sr.svi, obj1.sr.svi);
    EXPECT_EQ(obj.sr.svvi, obj1.sr.svvi);
    EXPECT_EQ(obj.sr.svvs.size(), obj1.sr.svvs.size());
    for (size_t i = 0; i < obj.sr.svvs.size(); i++)
    {
        EXPECT_EQ(obj.sr.svvs[i].size(), obj1.sr.svvs[i].size());
        for (size_t j = 0; j < obj.sr.svvs[i].size(); j++)
        {
            EXPECT_EQ(obj.sr.svvs[i][j].i, obj1.sr.svvs[i][j].i);
        }
    }

    EXPECT_EQ(obj.vi, obj1.vi);
    EXPECT_EQ(obj.vss.size(), obj1.vss.size());
    for (size_t i = 0; i < obj.vss.size(); i++)
    {
        EXPECT_EQ(obj.vss[i].i, obj1.vss[i].i);
    }

    EXPECT_EQ(obj.us, obj1.us);
    EXPECT_EQ(obj.ui, obj1.ui);

    EXPECT_EQ(obj.vsr.size(), obj1.vsr.size());
    for (size_t i = 0; i < obj.vsr.size(); i++)
    {
        EXPECT_EQ(obj.vsr[i].sri, obj1.vsr[i].sri);
        EXPECT_EQ(obj.vsr[i].srs, obj1.vsr[i].srs);
        EXPECT_EQ(obj.vsr[i].svi, obj1.vsr[i].svi);
        EXPECT_EQ(obj.vsr[i].svvi, obj1.vsr[i].svvi);
        EXPECT_EQ(obj.vsr[i].svvs.size(), obj1.vsr[i].svvs.size());
        for (size_t j = 0; j < obj.vsr[i].svvs.size(); j++)
        {
            EXPECT_EQ(obj.vsr[i].svvs[j].size(), obj1.vsr[i].svvs[j].size());
            for (size_t k = 0; k < obj.vsr[i].svvs[j].size(); k++)
            {
                EXPECT_EQ(obj.vsr[i].svvs[j][k].i, obj1.vsr[i].svvs[j][k].i);
            }
        }
    }
    EXPECT_EQ(obj.li, obj1.li);
    EXPECT_EQ(obj.oi1, obj1.oi1);
    EXPECT_FALSE(obj1.oi2.hasValue());
}

TEST_F(Serialization, JsonSerialization)
{
    ComplexStructSerialization<serialization::JSonSerializer>();
}

TEST_F(Serialization, XmlSerialization)
{
    ComplexStructSerialization<serialization::XmlSerializer>();
    serialization::XmlSerializer::cleanup();
}

template <typename T>
void SerializationVectorFirst()
{
    VecObject vo = {{1, 2, 3, 4}};
    T s;
    serializeObject(s, vo);
    T s1;
    EXPECT_FALSE(s.dump().empty());
    s1.setRawInput(s);
    VecObject vo1;
    deserializeObject(s1, vo1);
    EXPECT_EQ(vo.vi, vo1.vi);
}

TEST_F(Serialization, JsonSerializationVectorFirst)
{
    SerializationVectorFirst<serialization::JSonSerializer>();
}

TEST_F(Serialization, XmlSerializationVectorFirst)
{
    SerializationVectorFirst<serialization::XmlSerializer>();
    serialization::XmlSerializer::cleanup();
}

TEST_F(Serialization, JsonFloatDeserialization)
{
    FloatObject obj;
    serialization::JSonSerializer s;

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

void XmlFloatDeserialization()
{
    FloatObject obj;

    serialization::XmlSerializer s;

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

TEST_F(Serialization, XmlFloatDeserialization)
{
    XmlFloatDeserialization();
    serialization::XmlSerializer::cleanup();
}

template <typename T>
void SerializationOptional()
{
    {
        OptionalObject obj1 = {};
        T s1;
        serializeObject(s1, obj1);

        T s2;
        assert(!s1.dump().empty());
        s2.setRawInput(s1);
        OptionalObject obj2 = {};
        deserializeObject(s2, obj2);
        EXPECT_EQ(obj1.oi.hasValue(), obj2.oi.hasValue());
        EXPECT_EQ(obj1.voi.size(), obj2.voi.size());
        EXPECT_EQ(obj1.oss.hasValue(), obj2.oss.hasValue());
        EXPECT_EQ(obj1.voss.size(), obj2.voss.size());
    }

    {
        T s1;
        T s2;
        OptionalObject obj1 = {};
        obj1.oi = Optional<int>(1);
        obj1.oss = Optional<Object::SimpleSubObject>({1});
        serializeObject(s1, obj1);
        EXPECT_FALSE(s1.dump().empty());
        s2.setRawInput(s1);
        OptionalObject obj2 = {};
        deserializeObject(s2, obj2);
        EXPECT_EQ(obj1.oi.hasValue(), obj2.oi.hasValue());
        EXPECT_EQ(obj1.oi, obj2.oi);
        EXPECT_EQ(obj1.voi.size(), obj2.voi.size());
        EXPECT_EQ(obj1.oss.hasValue(), obj2.oss.hasValue());
        EXPECT_EQ(obj1.oss.cValue().i, obj2.oss.cValue().i);
        EXPECT_EQ(obj1.voss.size(), obj2.voss.size());
    }

    {
        T s1;
        T s2;
        OptionalObject obj1 = {};
        obj1.voi.push_back(Optional<int>(4));
        obj1.voi.push_back(Optional<int>());
        obj1.voi.push_back(Optional<int>(6));
        obj1.voss.push_back(Optional<Object::SimpleSubObject>({4}));
        obj1.voss.push_back(Optional<Object::SimpleSubObject>());
        obj1.voss.push_back(Optional<Object::SimpleSubObject>({6}));
        serializeObject(s1, obj1);
        EXPECT_FALSE(s1.dump().empty());
        s2.setRawInput(s1);
        OptionalObject obj2 = {};
        deserializeObject(s2, obj2);
        EXPECT_EQ(obj1.oi.hasValue(), obj2.oi.hasValue());
        EXPECT_EQ(obj1.voi.size() - 1, obj2.voi.size());
        EXPECT_EQ(obj1.voi[0], obj2.voi[0]);
        EXPECT_EQ(obj1.voi[2], obj2.voi[1]);
        EXPECT_EQ(obj1.voss.size() - 1, obj2.voss.size());
        EXPECT_EQ(obj1.voss[0].cValue().i, obj2.voss[0].cValue().i);
        EXPECT_EQ(obj1.voss[2].cValue().i, obj2.voss[1].cValue().i);
    }
}

TEST_F(Serialization, JsonOptional)
{
    SerializationOptional<serialization::JSonSerializer>();

    // json null testing
    OptionalObject obj1 = {};
    serialization::JSonSerializer s1;
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

TEST_F(Serialization, XmlOptional)
{
    SerializationOptional<serialization::XmlSerializer>();
    serialization::XmlSerializer::cleanup();
}

template <typename T>
void MapSerialization()
{
    T s1;
    MapObject obj1 = {};
    obj1.m[1] = -1;
    obj1.m[2] = -2;
    obj1.m[3] = -3;

    obj1.um[4] = -4;
    obj1.um[5] = -5;
    obj1.um[6] = -6;

    obj1.ms[7] = {-7};
    obj1.ms[8] = {-8};
    obj1.ms[9] = {-9};

    obj1.ms1[{10}] = {-10};
    obj1.ms1[{11}] = {-11};
    obj1.ms1[{12}] = {-12};

    obj1.strm["a"] = {-13};
    obj1.strm["b"] = {-14};
    obj1.strm["c"] = {-15};

    serializeObject(s1, obj1);
    EXPECT_FALSE(s1.dump().empty());

    T s2;
    MapObject obj2 = {};
    s2.setRawInput(s1);
    deserializeObject(s2, obj2);
    EXPECT_EQ(obj1.m.size(), obj2.m.size());
    EXPECT_EQ(obj1.m, obj2.m);

    EXPECT_EQ(obj1.um.size(), obj2.um.size());
    EXPECT_EQ(obj1.um, obj2.um);

    EXPECT_EQ(obj1.ms.size(), obj2.ms.size());
    for (auto it = obj1.ms.begin(); it != obj1.ms.end(); it++)
    {
        EXPECT_TRUE(obj2.ms.find(it->first) != obj1.ms.end());
        EXPECT_EQ(it->second.i, obj2.ms[it->first].i);
    }

    EXPECT_EQ(obj1.ms1.size(), obj2.ms1.size());
    for (auto it = obj1.ms1.begin(); it != obj1.ms1.end(); it++)
    {
        EXPECT_TRUE(obj2.ms1.find(it->first) != obj1.ms1.end());
        EXPECT_EQ(it->second.i, obj2.ms1[it->first].i);
    }

    EXPECT_EQ(obj1.strm.size(), obj2.strm.size());
    for (auto it = obj1.strm.begin(); it != obj1.strm.end(); it++)
    {
        EXPECT_TRUE(obj2.strm.find(it->first) != obj1.strm.end());
        EXPECT_EQ(it->second.i, obj2.strm[it->first].i);
    }
}

TEST_F(Serialization, JsonMap)
{
    MapSerialization<serialization::JSonSerializer>();
}

TEST_F(Serialization, XmlMap)
{
    MapSerialization<serialization::XmlSerializer>();
    serialization::XmlSerializer::cleanup();
}

template <typename T>
void EnumSerialization()
{
    T s1;
    EnumObject obj1 = {enum2, EnumClassValues::enumClass3};
    serializeObject(s1, obj1);
    EXPECT_FALSE(s1.dump().empty());

    T s2;
    EnumObject obj2 = {};
    s2.setRawInput(s1);
    deserializeObject(s2, obj2);
    EXPECT_EQ(obj1.ev, obj2.ev);
    EXPECT_EQ(obj1.ecv, obj2.ecv);
}

TEST_F(Serialization, JsonEnum)
{
    EnumSerialization<serialization::JSonSerializer>();
}

TEST_F(Serialization, XmlEnum)
{
    EnumSerialization<serialization::XmlSerializer>();
    serialization::XmlSerializer::cleanup();
}

template <typename SERIALIZER_T, typename STRUCT_T>
void tryErrorCase(const std::string &name, const std::string &str)
{
    std::function<void(void)> action = [=]() {
        SERIALIZER_T s;
        s.setRawInput(str);
        STRUCT_T obj = {};
        deserializeObject(s, obj);
    };
    EXPECT_THROW(action(), serialization::ParseException);
}

TEST_F(Serialization, JsonError)
{
    using softeq::common::serialization::JSonSerializer;

    tryErrorCase<JSonSerializer, Object::SimpleSubObject>("empty object", "{}");
    tryErrorCase<JSonSerializer, Object::SimpleSubObject>("wrong name", R"({"i1":0})");
    tryErrorCase<JSonSerializer, Object::SimpleSubObject>("string instead of int", R"({"i":"0"})");
    tryErrorCase<JSonSerializer, Object::SimpleSubObject>("array instead of int", R"({"i":[0]})");
    tryErrorCase<JSonSerializer, Object::SimpleSubObject>("null instead of int", R"({"i":null})");
    tryErrorCase<JSonSerializer, Object::SimpleSubObject>("missing }", R"({"i":0)");

    tryErrorCase<JSonSerializer, Object::SubObject>(
        "array instead of object", R"({"sri":0,"srs":"","svi":[],"svvi":[],"svvs":[[{"i":1}, [2]]] })");
    tryErrorCase<JSonSerializer, Object::SubObject>(
        "null instead of object", R"({"sri":0,"srs":"","svi":[],"svvi":[],"svvs":[[null]] })");
    tryErrorCase<JSonSerializer, Object::SubObject>("int instead of object",
                                                    R"({"sri":0,"srs":"","svi":[],"svvi":[],"svvs":[[1]] })");
    tryErrorCase<JSonSerializer, Object::SubObject>("object instead of array",
                                                    R"({"sri":0,"srs":"","svi":{},"svvi":[],"svvs":[]})");
    tryErrorCase<JSonSerializer, Object::SubObject>("int instead of array",
                                                    R"({"sri":0,"srs":"","svi":[],"svvi":[],"svvs":[1] })");
    tryErrorCase<JSonSerializer, Object::SubObject>("null instead of array",
                                                    R"({"sri":0,"srs":"","svi":[],"svvi":[],"svvs":[null] })");
    tryErrorCase<JSonSerializer, PrimitivesObject>("int instead of bool",
                                                   R"({"b":1,"d":0.0,"f":0.0,"i16":0,"i32":0,"i64":0,)"
                                                   R"("i8":0,"str":"","ui16":0,"ui32":0,"ui64":0,"ui8":0})");

    tryErrorCase<JSonSerializer, PrimitivesObject>(
        "int8_t out of range", R"({"b":true,"d":0.0,"f":0.0,"i16":0,"i32":0,"i64":0,)"
                               R"("i8":4096,"str":"","ui16":0,"ui32":0,"ui64":0,"ui8":0})");

    tryErrorCase<JSonSerializer, PrimitivesObject>(
        "int8_t negative and out of range", R"({"b":true,"d":0.0,"f":0.0,"i16":0,"i32":0,"i64":0,)"
                                            R"("i8":-4096,"str":"","ui16":0,"ui32":0,"ui64":0,"ui8":0})");

    tryErrorCase<JSonSerializer, PrimitivesObject>(
        "uint8_t is negative", R"({"b":true,"d":0.0,"f":0.0,"i16":0,"i32":0,"i64":0,)"
                               R"("i8":0,"str":"","ui16":0,"ui32":0,"ui64":0,"ui8":-1})");

    tryErrorCase<JSonSerializer, PrimitivesObject>(
        "uint8_t is out of range", R"({"b":true,"d":0.0,"f":0.0,"i16":0,"i32":0,"i64":0,)"
                                   R"("i8":0,"str":"","ui16":0,"ui32":0,"ui64":0,"ui8": 4096})");

    tryErrorCase<JSonSerializer, PrimitivesObject>(
        "wrong boolean value", R"({"b":wrong,"d":0.0,"f":0.0,"i16":0,"i32":0,"i64":0,)"
                               R"("i8":0,"str":"","ui16":0,"ui32":0,"ui64":0,"ui8": 0})");
}

TEST_F(Serialization, XmlError)
{
    using softeq::common::serialization::XmlSerializer;

    // clang-format off
    tryErrorCase<XmlSerializer, Object::SimpleSubObject>(
        "empty object", R"(<?xml version="1.0" encoding="UTF-8"?>)"
                        R"(<root></root>)");

    tryErrorCase<XmlSerializer, Object::SimpleSubObject>(
        "wrong name",   R"(<?xml version="1.0" encoding="UTF-8"?>)"
                        R"(<root>)"
                            R"(<i1 type="int">0</i1>)"
                        R"(</root>)");

    tryErrorCase<XmlSerializer, Object::SimpleSubObject>(
        "wrong primitive type", R"(<?xml version="1.0" encoding="UTF-8"?>)"
                                R"(<root>)"
                                    R"(<i type="string">0</i>)"
                                R"(</root>)");

    tryErrorCase<XmlSerializer, Object::SimpleSubObject>(
        "not-supported primitive type", R"(<?xml version="1.0" encoding="UTF-8"?>)"
                                        R"(<root>)"
                                            R"(<i type="not-supported-type">0</i>)"
                                        R"(</root>)");

    tryErrorCase<XmlSerializer, Object::SimpleSubObject>(
        "missing >",    R"(<?xml version="1.0" encoding="UTF-8"?>)"
                        R"(<root>)"
                            R"(<i type=\"int\">0</i)"
                        R"(</root>)");

    tryErrorCase<XmlSerializer, Object::SimpleSubObject>(
        "missing type attribute",   R"(<?xml version=\"1.0\" encoding=\"UTF-8\"?>)"
                                    R"(<root>)"
                                        R"(<i>0</i>)"
                                    R"(</root>)");

    tryErrorCase<XmlSerializer, Object::SimpleSubObject>(
        "missing value for numeric primitive",  R"(<?xml version="1.0" encoding="UTF-8"?>)"
                                                R"(<root>)"
                                                    R"(<i type="int"></i>)"
                                                R"(</root>)");

    tryErrorCase<XmlSerializer, Object::SimpleSubObject>(
        "string instead of int",    R"(<?xml version="1.0" encoding="UTF-8"?>)"
                                    R"(<root>)"
                                        R"(<i type="int">xyz</i>)"
                                    R"(</root>)");
    tryErrorCase<XmlSerializer, Object::SimpleSubObject>(
        "node instead of primitive",    R"(<?xml version="1.0" encoding="UTF-8"?>)"
                                        R"(<root>)"
                                            R"(<i type="int">)"
                                                R"(<i1>0</i1>)"
                                            R"(</i>)"
                                        R"(</root>)");


    tryErrorCase<XmlSerializer, VecObject>(
         "wrong array container entry format",
                R"(<?xml version="1.0" encoding="UTF-8"?>)"
                R"(<root>)"
                    R"(<vi>)"
                        R"(<___containerEntry___ type="int">1</___containerEntry___>)"
                        R"(<a type="int">2</a>)"
                    R"(</vi>)"
                R"(</root>)");

    tryErrorCase<XmlSerializer, MapObject>(
         "wrong entry format for map with string key(string key map is serialized as struct)",
                R"(<?xml version="1.0" encoding="UTF-8"?>)"
                R"(<root>)"
                    R"(<strm>)"
                        R"(<___containerEntry___>)"
                            R"(<__mapKey__ type="string">a</__mapKey__>)"
                            R"(<__mapValue__><i type="int">0</i></__mapValue__>)"
                        R"(</___containerEntry___>)"
                    R"(</strm>)"
                    R"(<m/><um/><ms/><ms1/><strm/>)"
                R"(</root>)");

    tryErrorCase<XmlSerializer, MapObject>(
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

    tryErrorCase<XmlSerializer, MapObject>(
         "wrong map entry value format",
                R"(<?xml version="1.0" encoding="UTF-8"?>)"
                R"(<root>)"
                    R"(<m>)"
                        R"(<___containerEntry___>)"
                            R"(<__mapKey__ type="int">0</__mapKey__>)"
                            R"(<mapValue__ type="int">0</mapValue__>)"
                        R"(</___containerEntry___>)"
                    R"(</m>)"
                    R"(<strm/><um/><ms/><ms1/><strm/>)"
                R"(</root>)");

    tryErrorCase<XmlSerializer, MapObject>(
         "empty entry value for map",
                R"(<?xml version="1.0" encoding="UTF-8"?>)"
                R"(<root>)"
                    R"(<strm>)"
                        R"(<a></a>)"
                    R"(</strm>)"
                    R"(<m/><um/><ms/><ms1/><strm/>)"
                R"(</root>)");


    tryErrorCase<XmlSerializer, PrimitivesObject>(
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

    tryErrorCase<XmlSerializer, PrimitivesObject>(
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

    tryErrorCase<XmlSerializer, PrimitivesObject>(
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

    tryErrorCase<XmlSerializer, PrimitivesObject>(
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

    tryErrorCase<XmlSerializer, PrimitivesObject>(
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

    tryErrorCase<XmlSerializer, EnumObject>(
         "wrong enum value",
                R"(<?xml version="1.0" encoding="UTF-8"?>)"
                R"(<root>)"
                    R"(<ev type="string">enum11</ev>)"
                    R"(<ecv type="string">enumClass1</ecv>)"
                R"(</root>)");

    tryErrorCase<XmlSerializer, EnumObject>(
         "wrong enum class value",
                R"(<?xml version="1.0" encoding="UTF-8"?>)"
                R"(<root>)"
                    R"(<ev type="string">enum1</ev>)"
                    R"(<ecv type="string">enumClass11</ecv>)"
                R"(</root>)");

    // clang-format on

    serialization::XmlSerializer::cleanup();
}

template <typename SERIALIZER>
void alternateCorrectAndWrongOptionalDeserialization(std::string &errorStr)
{
    SERIALIZER serializer;
    serializer.setRawInput(errorStr);

    SingleOptionalObject singleOptionalObject;
    deserializeObject(serializer, singleOptionalObject);

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

TEST_F(Serialization, alternateXmlOptionalDeserialization)
{
    // clang format off
    std::string xmlErrorStr =
         R"(<?xml version="1.0" encoding="UTF-8"?>)"
         R"(<root>)"
             R"(<oit type="int">-100</oit>)"
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

     alternateCorrectAndWrongOptionalDeserialization<serialization::XmlSerializer>(xmlErrorStr);
}

TEST_F(Serialization, JsonOptionalDeserialization)
{
    std::string jsonErrorStr = R"({"oit":-100,"oif":"abc","ouit":100,"ouic":"efg","ouis":-10,)"
                               R"("oft":0.42,"ofc":"efg","ost": {"i":-255},"osf": {"i": "hij"},)"
                               R"("voit": [1,2,3,4],"voif": [1,"hijk","3","lmno"],)"
                               R"("mt": [{"__mapKey__":1,"__mapValue__":0}],)"
                               R"("mtf": [{"__mapKey__":"1","__mapValue__":0}],)"
                               R"("mfs": [{"__mapKey__":1,"__mapValue__":"0"}],)"
                               R"("mts": [{"__mapKey__":1,"__mapValue__":0}],)"
                               R"("umt": [{"__mapKey__":1,"__mapValue__":0}],)"
                               R"("umff": [{"__mapKey__":"1","__mapValue__":0}],)"
                               R"("umfs": [{"__mapKey__":1,"__mapValue__":"0"}],)"
                               R"("umts": [{"__mapKey__":1,"__mapValue__":0}]})";

    alternateCorrectAndWrongOptionalDeserialization<serialization::JSonSerializer>(jsonErrorStr);
}

template <typename SERIALIZER>
void partialSerialization(const std::string &expected)
{
    SERIALIZER serializer;
    PrimitivesObject obj = {};
    obj.i8 = 1;
    obj.i16 = -1;
    serialization::ObjectAssembler<PrimitivesObject>::accessor().serialize(serializer, obj, &PrimitivesObject::i8,
                                                                           &PrimitivesObject::i16);
    EXPECT_EQ(expected, serializer.dump());
    serializer.setRawInput(serializer);

    obj = {};
    serialization::ObjectAssembler<PrimitivesObject>::accessor().deserialize(serializer, obj, &PrimitivesObject::i8);
    EXPECT_EQ(obj.i8, 1);
    EXPECT_EQ(obj.i16, 0);
}

TEST_F(Serialization, PartialSerialization)
{
    partialSerialization<serialization::JSonSerializer>(R"({"i16":-1,"i8":1})");

    partialSerialization<serialization::XmlSerializer>(
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<root><i8 type=\"int\">1</i8><i16 type=\"int\">-1</i16></root>\n");
    serialization::XmlSerializer::cleanup();
}

TEST_F(Serialization, RedeclarationErrors)
{
    bool thrown = false;
    try
    {
        serialization::ObjectAssembler<ErrorNameDefineObject>::accessor();
    }
    catch (const std::logic_error &err)
    {
        thrown = true;
        EXPECT_EQ(err.what(), std::string("redeclaration of memeber with name 'i1'"));
    }
    EXPECT_EQ(thrown, true);

    thrown = false;
    try
    {
        serialization::ObjectAssembler<ErrorMemberDefineObject>::accessor();
    }
    catch (const std::logic_error &err)
    {
        thrown = true;
        EXPECT_EQ(err.what(), std::string("redeclaration of pointer memeber"));
    }
    EXPECT_EQ(thrown, true);

    thrown = false;
    try
    {
        serialization::ObjectAssembler<EnumMemberErrorValues>::accessor();
    }
    catch (const std::logic_error &err)
    {
        thrown = true;
        EXPECT_EQ(err.what(), std::string("redeclaration of enum with name 'enumError1'"));
    }
    EXPECT_EQ(thrown, true);
}

template <typename SERIALIZER>
void InheritanceTest()
{
    C c;
    c.a = 10;
    c.aa = 11;
    c.b = 15;
    c.c = 20;
    SERIALIZER s;
    serialization::ObjectAssembler<C>::accessor().serialize(s, c);
    s.setRawInput(s.dump());

    C c1 = {};
    serialization::ObjectAssembler<C>::accessor().deserialize(s, c1);
    EXPECT_EQ(c1.a, c.a);
    EXPECT_EQ(c1.aa, c.aa);
    EXPECT_EQ(c1.b, c.b);
    EXPECT_EQ(c1.c, c.c);
}

TEST_F(Serialization, Inheritance)
{
    InheritanceTest<serialization::XmlSerializer>();
    serialization::XmlSerializer::cleanup();

    InheritanceTest<serialization::JSonSerializer>();
}

template <typename SERIALIZER>
void MultiThreadingTest()
{
    const int cCountThread = 64;

    std::vector<std::thread> threads;
    for (auto i = 0; i < cCountThread; i++)
    {
        threads.emplace_back(std::thread([] { ComplexStructSerialization<SERIALIZER>(); }));
    }

    for (auto &thread : threads)
    {
        thread.join();
    }
}

TEST_F(Serialization, MultiThreading)
{
    serialization::XmlSerializer::initMultiThreading();
    MultiThreadingTest<serialization::XmlSerializer>();
    serialization::XmlSerializer::cleanup();

    MultiThreadingTest<serialization::JSonSerializer>();
}

// TODO: Create test cases for graph()
