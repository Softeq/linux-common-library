#include <common/serialization/json/json.hh>
#include <gtest/gtest.h>

#include <common/net/curl_helper/curl_helper.hh>
#include <common/net/rest/resthandler.hh>
#include <common/net/http/http_server.hh>

using namespace softeq::common::net::curl;
using namespace softeq::common::net::rest;
using namespace softeq::common::net::http;
using namespace softeq::common::serialization::json;

namespace rest
{
struct Param final
{
    int i;
    unsigned int u;

    struct SubParam
    {
        bool b;
        float f;
        double d;
        std::string s;
    } sp;
};

struct Result final
{
    int i;
    unsigned int u;
    bool b;
    float f;
    double d;
    std::string s;
};

class BaseCommandGET final : public IBaseCommand
{
public:
    std::string name() const override
    {
        return "get";
    }
    bool perform(RestConnection &connection) override
    {
        rest::Result result;
        result.i = 5;
        result.u = -1;
        result.b = true;
        result.f = 3.14f;
        result.d = 3.1415;
        result.s = "this get command";

        connection.setOutput(result);
        return true;
    }
};

class BaseCommandPOST final : public IBaseCommand
{
public:
    std::string name() const override
    {
        return "post";
    }
    bool perform(RestConnection &connection) override
    {
        Param param = connection.input<Param>();

        Result result;
        result.i = param.i;
        result.u = param.u;
        result.b = param.sp.b;
        result.f = param.sp.f;
        result.d = param.sp.d;
        result.s = param.sp.s;

        connection.setOutput(result);
        return true;
    }
};

class DocCommandGET : public IDocCommand
{
public:
    DocCommandGET()
    {
        defineRequest<Param>();
        defineResponse<Result>();
    }

    std::string name() const override
    {
        return "doc-get";
    }
    std::string description() const override
    {
        return "Doc Get Command description";
    }
    std::string requestDescription() const override
    {
        return "Param struct description";
    }
    std::string responseDescription() const override
    {
        return "Result struct description";
    }

    bool perform(RestConnection &connection) override
    {
        rest::Result result;
        result.i = 5;
        result.u = -1;
        result.b = true;
        result.f = 3.14f;
        result.d = 3.1415;
        result.s = "this doc-get command";

        connection.setOutput(result);
        return true;
    }
};

class DocCommandPOST : public IDocCommand
{
public:
    DocCommandPOST()
    {
        defineRequest<Param>();
        defineResponse<Result>();
    }

    std::string name() const override
    {
        return "doc-post";
    }
    std::string description() const override
    {
        return "Doc Post Command description";
    }
    std::string requestDescription() const override
    {
        return "Param struct description";
    }
    std::string responseDescription() const override
    {
        return "Result struct description";
    }

    bool perform(RestConnection &connection) override
    {
        Param param = connection.input<Param>();

        Result result;
        result.i = param.i;
        result.u = param.u;
        result.b = param.sp.b;
        result.f = param.sp.f;
        result.d = param.sp.d;
        result.s = param.sp.s;

        connection.setOutput(result);
        return true;
    }
};

} // end namespace rest

namespace softeq
{
namespace common
{
namespace serialization
{
template <>
ObjectAssembler<rest::Param::SubParam> Assembler()
{
    // clang-format off
    return ObjectAssembler<rest::Param::SubParam>()
        .define("b", &rest::Param::SubParam::b)
        .define("f", &rest::Param::SubParam::f)
        .define("d", &rest::Param::SubParam::d)
        .define("s", &rest::Param::SubParam::s)
        ;
    // clang-format on
}

template <>
ObjectAssembler<rest::Param> Assembler()
{
    // clang-format off
    return ObjectAssembler<rest::Param>()
        .define("i", &rest::Param::i)
        .define("u", &rest::Param::u)
        .define("sp", &rest::Param::sp)
        ;
    // clang-format on
}

template <>
ObjectAssembler<rest::Result> Assembler()
{
    // clang-format off
    return ObjectAssembler<rest::Result>()
        .define("i", &rest::Result::i)
        .define("u", &rest::Result::u)
        .define("b", &rest::Result::b)
        .define("f", &rest::Result::f)
        .define("d", &rest::Result::d)
        .define("s", &rest::Result::s)
        ;
    // clang-format on
}

template <>
ObjectAssembler<IHttpServer::settings_t> Assembler()
{
    // clang-format off
    return ObjectAssembler<IHttpServer::settings_t>()
        .define("certFilePath", &IHttpServer::settings_t::certFilePath)
        .define("keyFilePath", &IHttpServer::settings_t::keyFilePath)
        .define("serverPort", &IHttpServer::settings_t::port)
        ;
    // clang-format on
}
} // namespace serialization
} // namespace common
} // namespace softeq

#define BASE_PORT 55350
#define SERVER_PORT (BASE_PORT + __COUNTER__)

template <uint T>
class RestServerTest : public testing::Test
{
protected:
    void SetUp() override
    {
        _serverUrl = "http://localhost:" + std::to_string(_serverPort);

        addCommandToHandler();

        _serverSettings.enableSecure = false;
        _serverSettings.port = _serverPort;

        _httpServer.reset(new HttpServer(_serverSettings, _restHandler));
        ASSERT_TRUE(_httpServer->start());
    }
    void TearDown() override
    {
        _httpServer->stop();
    }
    void addCommandToHandler()
    {
        IBaseCommand::UPtr get_command(new rest::BaseCommandGET());
        _restHandler.addCommand(std::move(get_command));

        IBaseCommand::UPtr post_command(new rest::BaseCommandPOST());
        _restHandler.addCommand(std::move(post_command));
    }
    void removeAllCommand()
    {
        _restHandler.removeCommand("get");
        _restHandler.removeCommand("post");
    }

protected:
    uint _serverPort = T;
    std::string _serverUrl;
    IHttpServer::settings_t _serverSettings;
    std::unique_ptr<HttpServer> _httpServer;
    RestHandler _restHandler;
};

template <uint T>
class RestAutoDocTest : public testing::Test
{
public:
    RestAutoDocTest()
        : _restDocHandler("help", WEB_TRANSFORM_PATH)
    {
    }

protected:
    void SetUp() override
    {
        _serverUrl = "http://localhost:" + std::to_string(_serverPort);

        addCommandToHandler();

        _serverSettings.enableSecure = false;
        _serverSettings.port = _serverPort;

        _httpServer.reset(new HttpServer(_serverSettings, _restDocHandler));
        ASSERT_TRUE(_httpServer->start());
    }
    void TearDown() override
    {
        _httpServer->stop();
    }
    void addCommandToHandler()
    {
        IBaseCommand::UPtr doc_get_command(new rest::DocCommandGET());
        _restDocHandler.addCommand(std::move(doc_get_command));

        IBaseCommand::UPtr doc_post_command(new rest::DocCommandPOST());
        _restDocHandler.addCommand(std::move(doc_post_command));
    }
    void removeXsltPath()
    {
        _restDocHandler = RestAutodocHandler("help", "");
    }

protected:
    uint _serverPort = T;
    std::string _serverUrl;
    IHttpServer::settings_t _serverSettings;
    std::unique_ptr<HttpServer> _httpServer;
    RestAutodocHandler _restDocHandler;
};

using RestServerTest1 = RestServerTest<SERVER_PORT>;
TEST_F(RestServerTest1, GET)
{
    CurlHelper curl(_serverUrl + "/get");

    rest::Result result;
    result.b = true;
    result.i = 5;
    result.u = -1;
    result.f = 3.14f;
    result.d = 3.1415;
    result.s = "this get command";

    ASSERT_TRUE(curl.doGet());
    ASSERT_TRUE(curl.responseCode() == 200);
    ASSERT_TRUE(curl.responseText() == serializeAsJsonObject(result));
}

using RestServerTest2 = RestServerTest<SERVER_PORT>;
TEST_F(RestServerTest2, POST)
{
    CurlHelper curl(_serverUrl + "/post");

    rest::Param param;
    param.i = 10;
    param.u = -1;
    param.sp.b = true;
    param.sp.f = 2.15f;
    param.sp.d = 2.1514;
    param.sp.s = "this post command";

    ASSERT_TRUE(curl.doPost(serializeAsJsonObject(param)));

    rest::Result result;
    result.i = param.i;
    result.u = param.u;
    result.b = param.sp.b;
    result.f = param.sp.f;
    result.d = param.sp.d;
    result.s = param.sp.s;

    ASSERT_TRUE(curl.responseCode() == 200);
    ASSERT_TRUE(curl.responseText() == serializeAsJsonObject(result));
}

using RestServerTest3 = RestServerTest<SERVER_PORT>;
TEST_F(RestServerTest3, RemoveCommand)
{
    CurlHelper curl(_serverUrl + "/get");

    rest::Result result;
    result.b = true;
    result.i = 5;
    result.u = -1;
    result.f = 3.14f;
    result.d = 3.1415;
    result.s = "this get command";

    ASSERT_TRUE(curl.doGet());
    ASSERT_TRUE(curl.responseText() == serializeAsJsonObject(result));

    removeAllCommand();

    ASSERT_TRUE(curl.doGet());
    ASSERT_FALSE(curl.responseText() == serializeAsJsonObject(result));

    addCommandToHandler();

    ASSERT_TRUE(curl.doGet());
    ASSERT_TRUE(curl.responseText() == serializeAsJsonObject(result));
}

using RestAutoDocTest1 = RestAutoDocTest<SERVER_PORT>;
TEST_F(RestAutoDocTest1, GET)
{
    CurlHelper curl(_serverUrl + "/doc-get");

    rest::Result result;
    result.b = true;
    result.i = 5;
    result.u = -1;
    result.f = 3.14f;
    result.d = 3.1415;
    result.s = "this doc-get command";

    ASSERT_TRUE(curl.doGet());
    ASSERT_TRUE(curl.responseCode() == 200);
    ASSERT_TRUE(curl.responseText() == serializeAsJsonObject(result));
}

using RestAutoDocTest2 = RestAutoDocTest<SERVER_PORT>;
TEST_F(RestAutoDocTest2, POST)
{
    CurlHelper curl(_serverUrl + "/doc-post");

    rest::Param param;
    param.i = 10;
    param.u = -1;
    param.sp.b = true;
    param.sp.f = 2.15f;
    param.sp.d = 2.1514;
    param.sp.s = "this doc-post command";

    ASSERT_TRUE(curl.doPost(serializeAsJsonObject(param)));

    rest::Result result;
    result.i = param.i;
    result.u = param.u;
    result.b = param.sp.b;
    result.f = param.sp.f;
    result.d = param.sp.d;
    result.s = param.sp.s;

    ASSERT_TRUE(curl.responseCode() == 200);
    ASSERT_TRUE(curl.responseText() == serializeAsJsonObject(result));
}

using RestAutoDocTest3 = RestAutoDocTest<SERVER_PORT>;
TEST_F(RestAutoDocTest3, StylesheetExists)
{
    CurlHelper curl(_serverUrl + "/help");

    ASSERT_TRUE(curl.doGet());
    ASSERT_TRUE(curl.responseCode() == 200);

    std::string response = curl.responseText();
    ASSERT_FALSE(response.find("ERROR:Stylesheet is missing") != std::string::npos);
}

using RestAutoDocTest4 = RestAutoDocTest<SERVER_PORT>;
TEST_F(RestAutoDocTest4, CheckParam)
{
    CurlHelper curl(_serverUrl + "/help?xslt");

    ASSERT_TRUE(curl.doGet());
    ASSERT_TRUE(curl.responseCode() == 200);

    std::string response = curl.responseText();
    ASSERT_FALSE(response.find("ERROR:Stylesheet is missing") != std::string::npos);
}

using RestAutoDocTest5 = RestAutoDocTest<SERVER_PORT>;
TEST_F(RestAutoDocTest5, RemoveXsltPath)
{
    CurlHelper curl(_serverUrl + "/help?xslt");

    removeXsltPath();

    ASSERT_TRUE(curl.doGet());
    ASSERT_TRUE(curl.responseCode() == 200);

    std::string response = curl.responseText();
    ASSERT_TRUE(response.find("ERROR:Stylesheet is missing") != std::string::npos);
}
