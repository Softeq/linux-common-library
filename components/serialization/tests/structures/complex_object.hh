#ifndef SOFTEQ_COMMON_SERIALIZATION_TESTS_COMPLEX_OBJECT_H
#define SOFTEQ_COMMON_SERIALIZATION_TESTS_COMPLEX_OBJECT_H

#include <gtest/gtest.h> // for EXPECT_EQ etc

#include <common/stdutils/optional.hh>

#include <vector>
#include <string>
#include <list>
#include <thread>

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
    softeq::common::stdutils::Optional<int> oi1;
    softeq::common::stdutils::Optional<int> oi2;
};

bool operator<(const Object::SimpleSubObject &l, const Object::SimpleSubObject &r);


template <typename SerializerType, typename DeserializerType>
void testComplexStructSerialization(SerializerType &serializer, DeserializerType &deserializer)
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
    obj.oi2 = softeq::common::stdutils::Optional<int>();

    serializeObject(serializer, obj);
    Object obj1 = {};

    EXPECT_FALSE(serializer.dump().empty());

    deserializer.setRawInput(serializer.dump()); // parsing happens here
    deserializeObject(deserializer, obj1);
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


template <typename SerializerImpl, typename DeserializerImpl>
void testMultiThreading()
{
    const int cCountThread = 64;

    std::vector<std::thread> threads;

    for (auto i = 0; i < cCountThread; i++)
    {
        threads.emplace_back(std::thread([] {
            SerializerImpl serializer;
            DeserializerImpl deserializer;
            testComplexStructSerialization(serializer, deserializer);
        }));
    }

    for (auto &thread : threads)
    {
        thread.join();
    }
}

#endif //SOFTEQ_COMMON_SERIALIZATION_TESTS_COMPLEX_OBJECT_H
