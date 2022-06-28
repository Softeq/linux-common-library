#include <gtest/gtest.h>

#include <softeq/common/stdutils.hh>

using namespace softeq::common;

TEST(StdUtils,StringFormat)
{
    std::string tmp = stdutils::string_format("Hello world!");
    ASSERT_STREQ(tmp.c_str(), "Hello world!");

    tmp = stdutils::string_format("%s%s", "1", "2");
    ASSERT_STREQ(tmp.c_str(), "12");

    tmp = stdutils::string_format("%d%d", 1, 2);
    ASSERT_STREQ(tmp.c_str(), "12");

    tmp = stdutils::string_format("%d%d%d%d", 1, 2, 3, 4);
    ASSERT_STREQ(tmp.c_str(), "1234");

    tmp = stdutils::string_format("%02d%02d", 1, 2);
    ASSERT_STREQ(tmp.c_str(), "0102");

    tmp = stdutils::string_format("%x%03x", 11, 12);
    ASSERT_STREQ(tmp.c_str(), "b00c");

    tmp = stdutils::string_format("%X%04X", 11, 12);
    ASSERT_STREQ(tmp.c_str(), "B000C");

    tmp = stdutils::string_format("%s%s", nullptr, nullptr);
    ASSERT_STREQ(tmp.c_str(), "(null)(null)");
}

TEST(StdUtils,StringFormatLongString)
{
    const int countA = 512;
    const int countB = 513;
    std::string longStringA (countA, 'A'); // Fills the string with 512 consecutive copies of character 'A'.
    std::string longStringB (countB, 'B'); // Fills the string with 513 consecutive copies of character 'B'.
    std::string longStringAB = longStringA + longStringB;

    std::string tmp = stdutils::string_format("%s%s", longStringA.c_str(), longStringB.c_str());
    ASSERT_STREQ(tmp.c_str(), longStringAB.c_str());
}

TEST(StdUtils,StringSplit)
{
    std::string path = "/tmp/foo/bar.o";

    std::vector<std::string> tmp = stdutils::string_split(path, '/');

    ASSERT_EQ(4, tmp.size());
    ASSERT_STREQ(tmp[0].c_str(), "");
    ASSERT_STREQ(tmp[1].c_str(), "tmp");
    ASSERT_STREQ(tmp[2].c_str(), "foo");
    ASSERT_STREQ(tmp[3].c_str(), "bar.o");

    tmp = stdutils::string_split("", '/');
    ASSERT_EQ(0, tmp.size());
}
