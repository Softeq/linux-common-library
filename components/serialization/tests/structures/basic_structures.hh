#ifndef SOFTEQ_COMMON_SERIALIZATION_TESTS_BASIC_STRUCTURES_H
#define SOFTEQ_COMMON_SERIALIZATION_TESTS_BASIC_STRUCTURES_H

#include <gtest/gtest.h>

#include <common/serialization/object_assembler.hh>
#include <common/serialization/helpers.hh>
#include <common/serialization/deserializers.hh>

#include "complex_object.hh"

using softeq::common::stdutils::Optional;
using softeq::common::serialization::ParseException;

struct FloatObject
{
    float f;
};

struct VecObject
{
    std::vector<int> vi;
};

struct OptionalObject
{
    Optional<int> oi;
    std::vector<Optional<int>> voi;
    Optional<Object::SimpleSubObject> oss;
    std::vector<Optional<Object::SimpleSubObject>> voss;
};

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

struct ErrorNameDefineObject
{
    int i1;
    int i2;
};

struct ErrorMemberDefineObject
{
    int i1;
    int i2;
};

enum EnumMemberErrorValues
{
    enumError1,
    enumError2,
};

template <typename SerializerImpl, typename DeserializerImpl>
void testBasicSerialization()
{
    FloatObject fo = {1.0};
    SerializerImpl ser;
    serializeObject(ser, fo);

    FloatObject fo2{};
    DeserializerImpl deser;
    deser.setRawInput(ser.dump());
    deserializeObject(deser, fo2);

    EXPECT_FLOAT_EQ(fo.f, fo2.f);
}

template <typename SerializerImpl, typename DeserializerImpl>
void testSerializationVector()
{
    VecObject vo = {{1, 2, 3, 4}};
    SerializerImpl s;
    serializeObject(s, vo);
    DeserializerImpl s1;
    EXPECT_FALSE(s.dump().empty());
    s1.setRawInput(s.dump());
    VecObject vo1;
    deserializeObject(s1, vo1);
    EXPECT_EQ(vo.vi, vo1.vi);
}

template <typename SerializerImpl, typename DeserializerImpl>
void testSerializationOptional()
{
    {
        OptionalObject obj1 = {};
        SerializerImpl s1;
        serializeObject(s1, obj1);

        DeserializerImpl s2;
        assert(!s1.dump().empty());
        s2.setRawInput(s1.dump());
        OptionalObject obj2 = {};
        deserializeObject(s2, obj2);
        EXPECT_EQ(obj1.oi.hasValue(), obj2.oi.hasValue());
        EXPECT_EQ(obj1.voi.size(), obj2.voi.size());
        EXPECT_EQ(obj1.oss.hasValue(), obj2.oss.hasValue());
        EXPECT_EQ(obj1.voss.size(), obj2.voss.size());
    }

    {
        SerializerImpl s1;
        DeserializerImpl s2;
        OptionalObject obj1 = {};
        obj1.oi = Optional<int>(1);
        obj1.oss = Optional<Object::SimpleSubObject>({1});
        serializeObject(s1, obj1);
        EXPECT_FALSE(s1.dump().empty());
        s2.setRawInput(s1.dump());
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
        SerializerImpl s1;
        DeserializerImpl s2;
        OptionalObject obj1 = {};
        obj1.voi.push_back(Optional<int>(4));
        obj1.voi.push_back(Optional<int>());
        obj1.voi.push_back(Optional<int>(6));
        obj1.voss.push_back(Optional<Object::SimpleSubObject>({4}));
        obj1.voss.push_back(Optional<Object::SimpleSubObject>());
        obj1.voss.push_back(Optional<Object::SimpleSubObject>({6}));
        serializeObject(s1, obj1);
        EXPECT_FALSE(s1.dump().empty());
        s2.setRawInput(s1.dump());
        OptionalObject obj2 = {};
        deserializeObject(s2, obj2);
        EXPECT_EQ(obj1.oi.hasValue(), obj2.oi.hasValue());
        EXPECT_EQ(obj1.voi.size(), obj2.voi.size());
        EXPECT_EQ(obj1.voi[0], obj2.voi[0]);
        EXPECT_FALSE(obj2.voi[1].hasValue());
        EXPECT_EQ(obj1.voi[2], obj2.voi[2]);
        EXPECT_EQ(obj1.voss.size(), obj2.voss.size());
        EXPECT_EQ(obj1.voss[0].cValue().i, obj2.voss[0].cValue().i);
        EXPECT_EQ(obj1.voss[2].cValue().i, obj2.voss[2].cValue().i);
        EXPECT_FALSE(obj2.voss[1].hasValue());
    }
}

template <typename DESERIALIZER_T, typename STRUCT_T>
void tryErrorCase(const std::string &name, const std::string &str)
{
    // NOTE: testing errors is a good idea, but we should probably make sure that the error
    // is exactly the one we expected, i.e. it does not fail for another reason
    std::function<void(void)> action = [&]() {
        DESERIALIZER_T s;
        s.setRawInput(str);
        STRUCT_T obj = {};
        deserializeObject(s, obj);
    };
    EXPECT_THROW(action(), ParseException) << "Error case: " << name;
}

#endif // SOFTEQ_COMMON_SERIALIZATION_TESTS_BASIC_STRUCTURES_H
