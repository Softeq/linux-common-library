#ifndef LIBCOMMON_SETTINGS_TESTS_STRUCTS_HH_
#define LIBCOMMON_SETTINGS_TESTS_STRUCTS_HH_

#include <common/serialization/object_assembler.hh>
#include <common/settings/settings.hh>

enum class SerializationType
{
    Json,
    Xml
};

// Singleton tests
struct TestSettings
{
    int i;
    bool b;
};

// Do not use sigleton for tests any more, use StubSettings
class StubSettings : public softeq::common::settings::Settings
{
};

struct SimpleSetting
{
    int i;
};

struct VectorTestSettings
{
    std::vector<SimpleSetting> container;
};

struct TestObject
{
    TestObject()
    {
        s.b = false;
    }
    struct TestSettings
    {
        bool b;
    } s;
};

struct Object
{
    // fundamental types
    float floatT;
    double doubleT;
    bool boolT;
    char charT;
    char16_t char16T;
    wchar_t wcharT;
    int intT;
    long int longIntT;
    unsigned int unsignedIntT;
    unsigned long int unsignedLongIntT;

    // std library types
    std::string stringT;
    std::vector<int> vectorT;
    std::vector<std::vector<int>> vectorTVectorInt;
    std::map<std::string, int> mapTStringInt;

    // compound types
    enum UnscopedEnum
    {
        enumerator1,
        enumerator2,
        enumerator3,
    } unscopedEnumT;

    enum class ScopedEnum
    {
        enumerator1,
        enumerator2,
        enumerator3,
    } scopedEnumT;

    // subobjects
    struct SimpleSubobject
    {
        int intT;
    } simpleSubobjectT;

    struct Subobject
    {
        int intT;
        std::string stringT;
        std::vector<int> vectorTInt;
        std::vector<std::vector<int>> vectorTVectorInt;
        std::vector<SimpleSubobject> vectorTSubobjectSimple;
    } subobjectT;

    struct NestedSubobject : Subobject
    {
        int intT;
        int intT2;
    } nestedSubobjectT;

    std::vector<SimpleSubobject> vectorTSubobjectSimple;
    std::vector<Subobject> vectorTSubobject;
    std::map<int, SimpleSubobject> mapTIntSubobjectSimple;
    std::map<std::string, SimpleSubobject> mapTStringSubobjectSimple;

    // softeq types
    softeq::common::stdutils::Optional<int> optionalTInt;
};

#endif // !defined LIBCOMMON_SETTINGS_TESTS_STRUCTS_HH_
