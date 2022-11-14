#include <gtest/gtest.h>

#include <common/system/getopt_wrapper.hh>

#include <vector>

using namespace testing;
using namespace softeq::common::system;
namespace
{
const GetoptWrapper::DescOptions longopts = {
    {"required", 'r', GetoptWrapper::Argument::REQUIRED, "Argument is required"},
    {"optional", 'o', GetoptWrapper::Argument::OPTIONAL, "Argument is optional"},
    {"without", 'w', GetoptWrapper::Argument::NONE, "Argument is not needed"}};
} // namespace

using char_ptr = char *;

class GetOptTests: public ::testing::Test
{
protected:
    void SetUp()
    {
        // Use freopen to redirect stderr to dev/null:
        if ((fp = freopen("/dev/null", "w", stderr)) == nullptr)
        {
            fprintf(stderr, "Cannot reopen stderr.\n");
        }
    }
    void TearDown()
    {
        fclose(fp);
    }
    FILE *fp = nullptr;
};

TEST_F(GetOptTests, FullLongOpt)
{
    const GetoptWrapper getOpt(longopts);
    // Use all of 3 arguments as long
    char_ptr option_string[] = {char_ptr("getopt_test"),  char_ptr("--required"), char_ptr("1"),
                                char_ptr("--optional=2"), char_ptr("--without"),  nullptr};
    int count = 0;
    bool failed = false;

    for (const GetoptWrapper::ParsedOption &val : getOpt.process(5, option_string, &failed))
    {
        count++;
        switch (val.shortName)
        {
        case 'r':
            ASSERT_STREQ("1", val.value.cValue().c_str());
            break;
        case 'o':
            ASSERT_STREQ("2", val.value.cValue().c_str());
            break;
        case 'w':
            ASSERT_FALSE(val.value.hasValue());
            break;
        default:
            ASSERT_FALSE(true);
            break;
        }
    }

    ASSERT_FALSE(failed);
    ASSERT_EQ(3, count);
}

TEST_F(GetOptTests, MixedOpt)
{
    const GetoptWrapper getOpt(longopts);
    // Use all of 3 options properly
    char_ptr option_string[] = {char_ptr("getopt_test"), char_ptr("-r"),        char_ptr("1"),
                                char_ptr("--optional"),  char_ptr("--without"), nullptr};
    int count = 0;
    bool failed = false;

    for (const GetoptWrapper::ParsedOption &val : getOpt.process(5, option_string, &failed))
    {
        count++;
        switch (val.shortName)
        {
        case 'r':
            ASSERT_STREQ("1", val.value.cValue().c_str());
            break;
        case 'o':
            ASSERT_FALSE(val.value.hasValue());
            break;
        case 'w':
            ASSERT_FALSE(val.value.hasValue());
            break;
        default:
            ASSERT_FALSE(true);
            break;
        }
    }

    ASSERT_FALSE(failed);
    ASSERT_EQ(3, count);
}

TEST_F(GetOptTests, FullShortOpt)
{
    const GetoptWrapper getOpt(longopts);
    // Use all of 3 options properly
    char_ptr option_string[] = {char_ptr("getopt_test"), char_ptr("-r1"), char_ptr("-o2"), char_ptr("-w"), nullptr};
    int count = 0;
    bool failed = false;

    for (const GetoptWrapper::ParsedOption &val : getOpt.process(4, option_string, &failed))
    {
        count++;
        switch (val.shortName)
        {
        case 'r':
            ASSERT_STREQ("1", val.value.cValue().c_str());
            break;
        case 'o':
            ASSERT_STREQ("2", val.value.cValue().c_str());
            break;
        case 'w':
            ASSERT_FALSE(val.value.hasValue());
            break;
        default:
            ASSERT_FALSE(true);
            break;
        }
    }

    ASSERT_FALSE(failed);
    ASSERT_EQ(3, count);
}

TEST_F(GetOptTests, WrongCaseOpt)
{
    const GetoptWrapper getOpt(longopts);
    // Use 3 options, but 2 with wrong upper case
    char_ptr option_string[] = {char_ptr("getopt_test"), char_ptr("-R1"), char_ptr("-O2"), char_ptr("-w"), nullptr};

    bool failed = false;
    auto opts = getOpt.process(4, option_string, &failed);

    // only 1 recognized option
    ASSERT_EQ(1, opts.size());
    ASSERT_TRUE(failed);
    ASSERT_EQ('w', opts.front().shortName);
    ASSERT_FALSE(opts.front().value.hasValue());
}

TEST_F(GetOptTests, WrongUsageOpt)
{
    const GetoptWrapper getOpt(longopts);
    // Use required argument in a wrong way
    char_ptr option_string[] = {char_ptr("getopt_test"), char_ptr("-r"), nullptr};

    bool failed = false;
    auto opts = getOpt.process(2, option_string, &failed);

    ASSERT_TRUE(failed);
    ASSERT_TRUE(opts.empty());
}

TEST_F(GetOptTests, WithoutOpt)
{
    const GetoptWrapper getOpt(longopts);
    // Use without any arguments
    char_ptr option_string[] = {char_ptr("getopt_test"), nullptr};
    bool failed = false;
    auto opts = getOpt.process(1, option_string, &failed);

    ASSERT_FALSE(failed);
    ASSERT_TRUE(opts.empty());
}

TEST_F(GetOptTests, StringWithoutOpt)
{
    const GetoptWrapper getOpt(longopts);
    // Use without any arguments
    char_ptr option_string[] = {char_ptr("getopt_test"), char_ptr("abc"), nullptr};

    bool failed = false;
    auto opts = getOpt.process(1, option_string, &failed);

    ASSERT_FALSE(failed);
    ASSERT_TRUE(opts.empty());
    ASSERT_STREQ(option_string[optind], "abc");
}

TEST_F(GetOptTests, SysDuplicateShortParam)
{
    const GetoptWrapper::DescOptions longopts = {
        {"required", 'h', GetoptWrapper::Argument::REQUIRED, "Argument is required"},
        {"optional", 'o', GetoptWrapper::Argument::OPTIONAL, "Argument is optional"},
        {"without", 'w', GetoptWrapper::Argument::NONE, "Argument is not needed"}};
    EXPECT_THROW(const GetoptWrapper getOpt(longopts), std::invalid_argument);
}

TEST_F(GetOptTests, SysDuplicateLongParam)
{
    const GetoptWrapper::DescOptions longopts = {
        {"help", 'r', GetoptWrapper::Argument::REQUIRED, "Argument is required"},
        {"optional", 'o', GetoptWrapper::Argument::OPTIONAL, "Argument is optional"},
        {"without", 'w', GetoptWrapper::Argument::NONE, "Argument is not needed"}};
    EXPECT_THROW(const GetoptWrapper getOpt(longopts), std::invalid_argument);
}

TEST_F(GetOptTests, InnerDuplicateShortParam)
{
    const GetoptWrapper::DescOptions longopts = {
        {"required", 'r', GetoptWrapper::Argument::REQUIRED, "Argument is required"},
        {"optional", 'r', GetoptWrapper::Argument::OPTIONAL, "Argument is optional"},
        {"without", 'w', GetoptWrapper::Argument::NONE, "Argument is not needed"}};
    EXPECT_THROW(const GetoptWrapper getOpt(longopts), std::invalid_argument);
}

TEST_F(GetOptTests, InnerDuplicateLongParam)
{
    const GetoptWrapper::DescOptions longopts = {
        {"required", 'r', GetoptWrapper::Argument::REQUIRED, "Argument is required"},
        {"required", 'o', GetoptWrapper::Argument::OPTIONAL, "Argument is optional"},
        {"without", 'w', GetoptWrapper::Argument::NONE, "Argument is not needed"}};
    EXPECT_THROW(const GetoptWrapper getOpt(longopts), std::invalid_argument);
}
