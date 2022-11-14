#ifndef SOFTEQ_COMMON_SERIALIZATION_TESTS_MAP_OBJECT_H
#define SOFTEQ_COMMON_SERIALIZATION_TESTS_MAP_OBJECT_H

#include "complex_object.hh"

#include <map>
#include <unordered_map>
#include <string>

struct MapObject
{
    std::map<int, int> m;
    std::unordered_map<int, int> um;
    std::map<int, Object::SimpleSubObject> ms;
    std::map<Object::SimpleSubObject, Object::SimpleSubObject> ms1;
    std::map<std::string, Object::SimpleSubObject> strm;
};

template <typename SerializerType, typename DeserializerType>
void testMapSerialization(SerializerType &serializer, DeserializerType &deserializer)
{
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

    serializeObject(serializer, obj1);
    EXPECT_FALSE(serializer.dump().empty());

    MapObject obj2 = {};
    deserializer.setRawInput(serializer.dump());
    deserializeObject(deserializer, obj2);
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

#endif //SOFTEQ_COMMON_SERIALIZATION_TESTS_MAP_OBJECT_H
