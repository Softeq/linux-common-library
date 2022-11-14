#include <gtest/gtest.h>

#include "serialization_test_fixture.hh"

#include <common/serialization/helpers.hh>
#include <common/serialization/object_assembler.hh>
#include <common/serialization/xml/xml.hh>

#include "xml_struct_deserializer.hh"
#include "xml_struct_serializer.hh"

#include <thread>

/*
    Thread safety

Starting with 2.4.7, libxml2 makes provisions to ensure that concurrent threads can safely work in parallel parsing
different documents. There is however a couple of things to do to ensure it:
    * configure the library accordingly using the --with-threads options
    * call xmlInitParser() in the "main" thread before using any of the libxml2 API

Note that the thread safety cannot be ensured for multiple threads sharing the same document, the
locking must be done at the application level.
The parts of the library checked for thread safety are:
    * concurrent loading
    * file access resolution
    * catalog access
    * catalog building
    * entities lookup/accesses
    * validation
    * global variables per-thread override
    * memory handling

XPath has been tested for threaded usage on non-modified document for example when using libxslt, but make 100% sure the
documents are accessed read-only!
*/

struct ObjectMt
{
    int i;
    bool b;
    float f;
    double d;
    std::string s;
};

bool operator==(const ObjectMt &lhs, const ObjectMt &rhs)
{
    return lhs.i == rhs.i && lhs.b == rhs.b && lhs.f == rhs.f && lhs.d == rhs.d && lhs.s == rhs.s;
}

struct VectorObjects
{
    std::vector<ObjectMt> objects;
};

namespace softeq
{
namespace common
{
namespace serialization
{
template <>
ObjectAssembler<ObjectMt> Assembler()
{
    // clang-format off
    return ObjectAssembler<ObjectMt>()
        .define("i", &ObjectMt::i)
        .define("b", &ObjectMt::b)
        .define("f", &ObjectMt::f)
        .define("d", &ObjectMt::d)
        .define("s", &ObjectMt::s)
        ;
    // clang-format on
}

template <>
ObjectAssembler<VectorObjects> Assembler()
{
    // clang-format off
    return ObjectAssembler<VectorObjects>()
        .define("objects", &VectorObjects::objects)
        ;
    // clang-format on
}
} // namespace serialization
} // namespace common
} // namespace softeq

TEST_F(Serialization, XmlBigMultithreading)
{
    using namespace softeq::common::serialization;

    // We generate two identical objects filled with an array of structures with data

    VectorObjects vector1;
    VectorObjects vector2;

    for (int i = 0; i < 10000; i++)
    {
        ObjectMt obj;
        obj.i = i;
        obj.b = true;
        obj.s = "abc";
        obj.f = 2.5;
        obj.d = 3.8;

        vector1.objects.push_back(obj);
        vector2.objects.push_back(obj);
    }

    // Initialization function for the XML parser of Libxm2. The function should be called once
    // before processing in case of use in multithreaded programs. It is not necessary to call
    // it in single-threaded programs.
    // THE FUNCTION HAS TO BE CALLED FROM THE MAIN THREAD.
    xml::initMultiThreading();

    // We will work in a separate anonymous scope to guarantee the destruction of
    // all XmlSerializer objects. If this is not done, calling XmlSerializer::cleanup()
    // will result in an Segmentation fault error!
    {
        // Serialize ObjectMt

        // Let's create two serializer objects into which our data will be converted
        xml::CompositeXmlSerializer serializer1;
        xml::CompositeXmlSerializer serializer2;

        // Note that the thread safety cannot be ensured for multiple threads sharing the same document,
        // the locking must be done at the application level
        std::thread t1([&serializer1, &vector1] { serializeObject<VectorObjects>(serializer1, vector1); });
        std::thread t2([&serializer2, &vector2] { serializeObject<VectorObjects>(serializer2, vector2); });
        t1.join();
        t2.join();

        std::string res1(serializer1.dump());
        std::string res2(serializer2.dump());

        ASSERT_EQ(res1, res2);

        // Deserialize ObjectMt

        xml::CompositeXmlDeserializer serializer3;
        xml::CompositeXmlDeserializer serializer4;

        VectorObjects vector3;
        VectorObjects vector4;

        // Note that the thread safety cannot be ensured for multiple threads sharing the same document,
        // the locking must be done at the application level
        std::thread t3([&serializer3, &vector3, &res1] {
            serializer3.setRawInput(res1);
            deserializeObject<VectorObjects>(serializer3, vector3);
        });
        std::thread t4([&serializer4, &vector4, &res2] {
            serializer4.setRawInput(res2);
            deserializeObject<VectorObjects>(serializer4, vector4);
        });
        t3.join();
        t4.join();

        ASSERT_EQ(vector3.objects, vector4.objects);
    }

    // Is a centralized routine to free the library state and data.
    // The function should be called after ALL INSTANCES of XmlSerializer ARE DELETED
    // both for a single and a multithreaded programs.
    // THE FUNCTION HAS TO BE CALLED FROM THE MAIN THREAD IN CASE OF MULTITHREADED PROGRAMS.
    xml::cleanup();
}
