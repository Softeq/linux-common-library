#include <gtest/gtest.h>

#include <softeq/common/net/curl_helper.hh>

using namespace softeq::common::net;

TEST(TestDeserialize, deserialize)
{
    CurlHelper curl("");

    std::vector<std::string> data;

    data.push_back("#comment,please\npostman-echo.com\tFALSE\t/\tFALSE\t0\tfoo1\tbar1");
    data.push_back("#comment,please\npostman-echo.com\tFALSE\t/\tFALSE\t0\tfoo2\tbar2");

    for (std::string x : data)
    {
        curl.deserialize(x);
    }

    CurlHelper::cookieMap_t cookies_map = curl.cookies();

    ASSERT_STREQ("bar1", cookies_map.at("foo1").c_str());
    ASSERT_STREQ("bar2", cookies_map.at("foo2").c_str());
}

TEST(doGet, empty_url)
{
    CurlHelper curl("");
    ASSERT_FALSE(curl.doGet());
}

TEST(doPost, empty_url)
{
    CurlHelper curl("");
    ASSERT_FALSE(curl.doPost());
}

TEST(set_cookie, twice)
{
    CurlHelper curl("");

    for (int i = 0; i < 2; i++)
    {
        curl.setCookie("key", "value");
    }
    CurlHelper::cookieMap_t cookies_map = curl.cookies();
    ASSERT_EQ(1, cookies_map.size());
}

TEST(set_cookies, size_limit)
{
    CurlHelper curl("");
    std::string key = "key";
    std::string value = "value";
    for (int i = 0; i < 200; i++)
    {
        key += std::to_string(i);
        value += std::to_string(i);
        curl.setCookie(key, value);
    }
    CurlHelper::cookieMap_t cookies_map = curl.cookies();
    ulong map_size = 0;
    for (const auto &p : cookies_map)
    {
        map_size += p.first.length();
        map_size += p.second.length();
    }
    ASSERT_EQ(9738, map_size);
}

TEST(response_code, emptyurl)
{
    CurlHelper curl("");
    ASSERT_EQ(0, curl.responseCode());
}
