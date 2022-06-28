#include <iostream>
#include <vector>
#include <list>

#include <softeq/common/settings.hh>
#include "softeq/common/log.hh"

using namespace softeq::common;

namespace
{
const char *const LOG_DOMAIN = "ExamplesSettings";
}

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
        int soi;
        std::string sos;
        std::vector<int> sovi;
        std::vector<std::vector<int>> sovvi;
        std::vector<SimpleSubObject> sovvs;
    } so;
    std::vector<int> vi;
    struct NestedSubObject : SubObject
    {
        int nsoi;
    } nso;

    std::vector<SimpleSubObject> vss;
    std::vector<SubObject> vsr;
    unsigned int ui;
    unsigned short us;
    std::list<int> li;
    Optional<int> oi;
    enum class enum_t
    {
        option1,
        option2,
        option3,
    };
    Optional<enum_t> oe;
    std::map<int, SimpleSubObject> mapSO;
    std::map<std::string, SimpleSubObject> hashmapSO;
};

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
        .define("so", &Object::so)
        .define("nso", &Object::nso)
        .define("vi", &Object::vi)
        .define("vss", &Object::vss)
        .define("vsr", &Object::vsr)
        .define("us", &Object::us)
        .define("ui", &Object::ui)
        .define("li", &Object::li)
        .define("oi", &Object::oi)
        .define("oe", &Object::oe)
        .define("mapSO", &Object::mapSO)
        .define("hashmapSO", &Object::hashmapSO)
        ;
    // clang-format on
}

template <>
serialization::ObjectAssembler<Object::SimpleSubObject> serialization::Assembler()
{
    // clang-format off
    return ObjectAssembler<Object::SimpleSubObject>()
        .define("longLongLongLongLongLabel", &Object::SimpleSubObject::i);
    // clang-format on
}

template <>
serialization::ObjectAssembler<Object::SubObject> serialization::Assembler()
{
    // clang-format off
    return ObjectAssembler<Object::SubObject>()
        .define("soi", &Object::SubObject::soi)
        .define("sos", &Object::SubObject::sos)
        .define("sovi", &Object::SubObject::sovi)
        .define("sovvi", &Object::SubObject::sovvi)
        .define("sovvs", &Object::SubObject::sovvs)
        ;
    // clang-format on
}

template <>
serialization::ObjectAssembler<Object::NestedSubObject> serialization::Assembler()
{
    // clang-format off
    return ObjectAssembler<Object::NestedSubObject>()
        .define("nsoi", &Object::NestedSubObject::nsoi)
        .extend<Object::SubObject>("so")
        ;
    // clang-format on
}

template <>
serialization::ObjectAssembler<Object::enum_t> serialization::Assembler()
{
    // clang-format off
    return ObjectAssembler<Object::enum_t>()
        .define("option1", Object::enum_t::option1)
        .define("option2", Object::enum_t::option2)
        .define("option3", Object::enum_t::option3)
        ;
    // clang-format on
}

//! [Static usage motivation]
// doer_header.hh

class Doer final
{
public:
    Doer();
    int does() const;
};

// doer_source.cc

namespace softeq
{
namespace common
{
STATIC_DECLARE_SETTING(int, "foo");
} // namespace common
} // namespace softeq

Doer::Doer()
{
    Settings::instance().access<int>() = 100500;
}

int Doer::does() const
{
    return Settings::instance().access<int>();
}
//! [Static usage motivation]

int main(int argc, char *argv[])
{
    Settings &settings = Settings::instance();

    settings.declare<Object>("Settings");
    Doer doer;

    std::cout << settings.graph();
    return doer.does() == 100500 ? EXIT_SUCCESS : EXIT_FAILURE;
}
