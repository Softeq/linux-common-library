#include <common/settings/settings.hh>
#include <common/logging/log.hh>

#include <iostream>
#include <vector>
#include <list>

using namespace softeq::common::settings;
using namespace softeq::common::serialization;

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
    softeq::common::stdutils::Optional<int> oi;
    enum class enum_t
    {
        option1,
        option2,
        option3,
    };
    softeq::common::stdutils::Optional<enum_t> oe;
    std::map<int, SimpleSubObject> mapSO;
    std::map<std::string, SimpleSubObject> hashmapSO;
};

template <>
ObjectAssembler<Object> softeq::common::serialization::Assembler()
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
softeq::common::serialization::ObjectAssembler<Object::SimpleSubObject> softeq::common::serialization::Assembler()
{
    // clang-format off
    return ObjectAssembler<Object::SimpleSubObject>()
        .define("longLongLongLongLongLabel", &Object::SimpleSubObject::i);
    // clang-format on
}

template <>
softeq::common::serialization::ObjectAssembler<Object::SubObject> softeq::common::serialization::Assembler()
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
softeq::common::serialization::ObjectAssembler<Object::NestedSubObject> softeq::common::serialization::Assembler()
{
    // clang-format off
    return ObjectAssembler<Object::NestedSubObject>()
        .define("nsoi", &Object::NestedSubObject::nsoi)
        .extend<Object::SubObject>("so")
        ;
    // clang-format on
}

template <>
softeq::common::serialization::ObjectAssembler<Object::enum_t> softeq::common::serialization::Assembler()
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
namespace settings
{
STATIC_DECLARE_SETTING(int, "foo");
} // namespace settings
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

int main()
{
    Settings &settings = Settings::instance();

    settings.declare<Object>("Settings");
    Doer doer;

    std::cout << settings.graph();
    return doer.does() == 100500 ? EXIT_SUCCESS : EXIT_FAILURE;
}
