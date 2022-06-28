#include <algorithm>
#include <cstring>
#include <iostream>
#include <memory>
#include <string>
#include <stdio.h>
#include <csignal>

#include "softeq/common/net/http_server.hh"
#include "softeq/common/net/resthandler.hh"
#include "softeq/common/stdutils.hh"
#include "softeq/common/log.hh"
#include "softeq/common/settings.hh"

#include "softeq/common/serialization/object_assembler.hh"

using namespace softeq::common;
using namespace softeq::common::net;

namespace
{
const char *const LOG_DOMAIN = "ExamplesRestServer";
}

namespace rest
{
struct Param final
{
    int i;
    Optional<int> oi;
    struct SubParam
    {
        std::string ss;
        std::vector<int> vi;
    } s;
    std::list<std::string> ls;
};

struct Result final
{
    int i;
    bool b;
    std::string s;
    float f;
    double d;
    unsigned int u;
};

class MyCommand : public IBaseCommand
{
    bool &_flag;

public:
    explicit MyCommand(bool &flag)
        : _flag(flag)
    {
    }
    std::string name() const override
    {
        return "mycommand";
    }
    bool perform(RestConnection &connection) override
    {
        _flag = true;
        return true;
    }
};

class MyDocCommand : public IDocCommand
{
public:
    MyDocCommand()
    {
        defineRequest<Param>();
        defineResponse<Result>();
    }

    std::string name() const override
    {
        return "mydoccommand";
    }

    std::string description() const override
    {
        return "my doc command description";
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
        Param p = connection.input<Param>();
        if (!p.oi.hasValue())
        {
            LOGI(LOG_DOMAIN, "oi does not exist");
        }

        Result r;
        r.b = false;
        r.d = 3.1415;
        r.f = 3.14f;
        r.u = -1;
        r.s = p.s.ss;
        r.i = p.i;
        connection.setOutput(r);

        return true;
    }
};
} // namespace rest

template <>
serialization::ObjectAssembler<rest::Param::SubParam> serialization::Assembler()
{
    // clang-format off
    return ObjectAssembler<rest::Param::SubParam>()
        .define("ss", &rest::Param::SubParam::ss)
        .define("vi", &rest::Param::SubParam::vi)
        ;
    // clang-format on
}
template <>
serialization::ObjectAssembler<rest::Param> serialization::Assembler()
{
    // clang-format off
    return ObjectAssembler<rest::Param>()
        .define("i", &rest::Param::i)
        .define("oi", &rest::Param::oi)
        .define("s", &rest::Param::s)
        .define("ls", &rest::Param::ls)
        ;
    // clang-format on
}

template <>
serialization::ObjectAssembler<rest::Result> serialization::Assembler()
{
    // clang-format off
    return ObjectAssembler<rest::Result>()
        .define("i", &rest::Result::i)
        .define("b", &rest::Result::b)
        .define("s", &rest::Result::s)
        .define("f", &rest::Result::f)
        .define("d", &rest::Result::d)
        .define("u", &rest::Result::u)
        ;
    // clang-format on
}

template <>
serialization::ObjectAssembler<IHttpServer::settings_t> serialization::Assembler()
{
    // clang-format off
    return ObjectAssembler<IHttpServer::settings_t>()
        .define("certFilePath", &IHttpServer::settings_t::certFilePath)
        .define("keyFilePath", &IHttpServer::settings_t::keyFilePath)
        .define("serverPort", &IHttpServer::settings_t::port)
        ;
    // clang-format on
}

namespace
{
bool global_isActive = true;

void catch_function(int signo)
{
    LOGI(LOG_DOMAIN, "Interactive attention signal caught. %d", signo);
    global_isActive = false;
}
} // namespace

int main(int argc, char *argv[])
{
    bool quit_flag = false;

    Settings::instance().declare<IHttpServer::settings_t>("RestServerSettings");
    IHttpServer::settings_t &restServerSettings = Settings::instance().access<IHttpServer::settings_t>();
    restServerSettings.port = 8880;

    net::RestAutodocHandler restDispatcher("autohelp", WEB_TRANSFORM_PATH);
    HttpServer httpServer(restServerSettings, restDispatcher);
    net::IBaseCommand::UPtr command;
    command.reset(new rest::MyDocCommand());
    restDispatcher.addCommand(std::move(command));
    restDispatcher.addCommand(net::IBaseCommand::UPtr(new rest::MyCommand(quit_flag)));

    httpServer.start();
    LOGI(LOG_DOMAIN, "Rest server has been started. Try http://localhost: %d /autohelp", restServerSettings.port);
    
    if (signal(SIGINT, catch_function) == SIG_ERR)
    {
        LOGE(LOG_DOMAIN, "An error occurred while setting a signal handler.");
        return EXIT_FAILURE;
    }

    while (global_isActive && !quit_flag)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
    LOGI(LOG_DOMAIN, "Exiting");

    return EXIT_SUCCESS;
}
