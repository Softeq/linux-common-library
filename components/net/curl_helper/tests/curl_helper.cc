#include <gtest/gtest.h>

#include <common/net/curl_helper/curl_helper.hh>
#include <common/net/http/http_server.hh>
#include <common/net/http/http_connection.hh>
#include <common/logging/log.hh>

using namespace softeq::common::net::curl;
using namespace softeq::common::net::http;

namespace
{
const char *LOG_DOMAIN = "CurlHelperTest";

static const std::string firstIpAddr{"127.0.0.1"};
static std::string firstParamName{"firstParamName"};
static std::string firstParamValue{"firstParamValue"};
static std::string secondParamName{"secondParamName"};
static std::string secondParamValue{"secondParamValue"};
static std::string endpointParams{"/params"};

static int CURL_MAX_INPUT_LENGTH{8000000};
} // namespace

class TestDispatcher final : public IHttpConnectionDispatcher
{
public:
    bool handle(IHttpConnection &connection)
    {
        if (connection.path() == endpointParams)
        {
            if (connection.hasField(firstParamName) &&
                    connection.field(firstParamName) == firstParamValue &&
                    connection.hasField(secondParamName) &&
                    connection.field(secondParamName) == secondParamValue)
            {
                return true;
            }
            return false;
        }

        LOGE(LOG_DOMAIN, "TestDispatcher: unknown endpoint");
        return false;
    }
};

class CurlHelperTest : public testing::Test
{
protected:
    void SetUp() override
    {
        _serverSettings.port = 8080;
        _serverSettings.enableSecure = false;

        _server.reset(new HttpServer(_serverSettings, _dispatcher));

        ASSERT_TRUE(_server->start());
    }

    void TearDown() override
    {
        _server->stop();
    }

    std::string getRandomString(int length)
    {
        static const char alphanum[] = "0123456789"
                                       "!@#$%^&*"
                                       "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                                       "abcdefghijklmnopqrstuvwxyz";
        std::string s;
        for (int i = 0; i < length; ++i)
        {
            s += alphanum[rand() % (sizeof(alphanum) - 1)];
        }
        return s;
    }

    IHttpServer::settings_t _serverSettings;
    TestDispatcher _dispatcher;
    std::unique_ptr<HttpServer> _server;
};

TEST_F(CurlHelperTest, Params)
{
    CurlHelper curl(firstIpAddr + ":8080" + endpointParams);
    curl.addRequestParam(firstParamName, firstParamValue);
    curl.addRequestParam(secondParamName, secondParamValue);
    ASSERT_TRUE(curl.doGet());
    EXPECT_EQ(curl.responseCode(), HttpStatusCode::STATUS_OK);
    ASSERT_TRUE(curl.errorString() == "");

    CurlHelper curl2(firstIpAddr + ":8080" + endpointParams);
    curl2.addRequestParam("tooLong", getRandomString(CURL_MAX_INPUT_LENGTH * 3 + 1));
    curl2.addRequestParam("non-printable", "\x00\x1f\x7f");
    ASSERT_FALSE(curl2.doGet());

    CurlHelper emptyCurl("");
    emptyCurl.addRequestParam("", "");
    ASSERT_FALSE(emptyCurl.doGet());
}

TEST(CurlHelper, deserialize)
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

TEST(CurlHelper, doGet_empty_url)
{
    CurlHelper curl("");
    ASSERT_FALSE(curl.doGet());
}

TEST(CurlHelper, doPost_empty_url)
{
    CurlHelper curl("");
    ASSERT_FALSE(curl.doPost());
}

TEST(CurlHelper, set_cookie_twice)
{
    CurlHelper curl("");

    for (int i = 0; i < 2; i++)
    {
        curl.setCookie("key", "value");
    }
    CurlHelper::cookieMap_t cookies_map = curl.cookies();
    ASSERT_EQ(1, cookies_map.size());
}

TEST(CurlHelper, set_cookies_size_limit)
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

TEST(CurlHelper, response_code_emptyurl)
{
    CurlHelper curl("");
    ASSERT_EQ(0, curl.responseCode());
}
