#include <softeq/common/fsutils.hh>
#include <softeq/common/log.hh>

#include <softeq/common/net/http_connection.hh>
#include <softeq/common/net/http_server.hh>
#include <softeq/common/settings.hh>

#include <algorithm>
#include <chrono>
#include <csignal>
#include <iostream>
#include <thread>
#include <vector>

#ifdef HAVE_MAGIC
#include <magic.h>
#endif

#ifndef WWW_ROOT
#define WWW_ROOT "./"
#endif
#define INDEX_FILE "index.html"

using namespace softeq::common;
using namespace softeq::common::net;

namespace
{
const char *const LOG_DOMAIN = "ExamplesHttpServer";
}

class MyHttpDispatcher final : public IHttpConnectionDispatcher
{
    std::string _rootDir;
#ifdef HAVE_MAGIC
    magic_t _magicCookie;
#endif
public:
    MyHttpDispatcher()
        : _rootDir(WWW_ROOT)
    {
        const char *rootDir = ::getenv("WWW_ROOT");
        if (rootDir)
            _rootDir = rootDir;
#ifdef HAVE_MAGIC
        _magicCookie = magic_open(MAGIC_NODESC);
        if (_magicCookie == nullptr)
        {
            LOGE(LOG_DOMAIN, "Unable to initialize magic library");
            return;
        }

        LOGI(LOG_DOMAIN, "Loading default magic database");

        if (magic_load(_magicCookie, nullptr) != 0)
        {
            LOGE(LOG_DOMAIN, "Cannot load magic database - %s", magic_error(_magicCookie));
            magic_close(_magicCookie);
            _magicCookie = nullptr;
        }
#endif
    }
    ~MyHttpDispatcher()
    {
#ifdef HAVE_MAGIC
        if (_magicCookie)
            magic_close(_magicCookie);
#endif
    }

    struct MySession : public HttpSession
    {
        std::string data;
        virtual void onExpire()
        {
            LOGI(LOG_DOMAIN, "The session %s has expired", id().c_str());
        }
    };

    bool handle(IHttpConnection &connection)
    {
        std::string web_path = connection.get();
        if (web_path == "/create_session")
        {
            std::shared_ptr<MySession> session(std::make_shared<MySession>());
            session->data = connection.body();
            connection.attachSession(session);
            LOGI(LOG_DOMAIN, "Created session id %s", session->id().c_str());

            return true;
        }

        if (web_path == "/restore_session")
        {
            // it will throw exception in the bad case
            std::shared_ptr<MySession> session = std::dynamic_pointer_cast<MySession>(connection.session().lock());
            if (!session)
            {
                connection.setError(HttpStatusCode::STATUS_NOT_FOUND);
                return false;
            }
            LOGI(LOG_DOMAIN, "Restored session id %s", session->id().c_str());
            connection.setResponseHeader("Content-type", "text/plain");
            connection << session->data;
            return true;
        }

        fsutils::Path fspath(_rootDir);

        fspath += web_path;

        if (!fspath.exist())
        {
            connection.setError(HttpStatusCode::STATUS_NOT_FOUND);
            connection << "404: Path (" << fspath << ") does not exist";
            return true;
        }

        if (fsutils::isdir(fspath))
        {
            if (web_path[web_path.length() - 1] != '/')
            {
                web_path += "/";
            }
            connection.setResponseHeader("Content-type", "text/html");
            if (fsutils::exist(fspath + INDEX_FILE) && !fsutils::isdir(fspath + INDEX_FILE))
            {
                connection.sendFile(fspath + INDEX_FILE);
                return true;
            }
            std::list<std::string> content = fsutils::dirContent(fspath);

            content.sort([&fspath](const std::string &str1, const std::string &str2) -> bool {
                bool str1dir = fsutils::isdir(fspath + str1);
                bool str2dir = fsutils::isdir(fspath + str2);
                if (str1dir == str2dir)
                {
                    return str1 < str2;
                }
                return str1dir && !str2dir;
            });
            connection << "Current path: " << fspath << "<br>";
            for (std::string item : content)
            {
                if (item == ".")
                    continue;
                std::string fullpath = fspath + item;
                if (fsutils::isdir(fullpath))
                {
                    connection << "[DIR]";
                    item += "/";
                }
                else
                    connection << "&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;";

                connection << "&nbsp;<a href=\"" << web_path << item << "\">" << item << "</a><br>";
            }
        }
        else
        {
            std::string ext = fspath.extension();
            if (ext == "css")
            {
                connection.setResponseHeader("Content-type", "text/css");
            }
            else if (ext == "html")
            {
                connection.setResponseHeader("Content-type", "text/html");
            }
            else if (ext == "js")
            {
                connection.setResponseHeader("Content-type", "text/javascript");
            }
#ifdef HAVE_MAGIC
            else if (_magicCookie)
            {
                const char *fullMagic = magic_file(_magicCookie, static_cast<std::string>(fspath).c_str());
                if (fullMagic != nullptr)
                    connection.setResponseHeader("Content-type", fullMagic);
                else
                    connection.setResponseHeader("Content-type", "application/octet-stream");
            }
#endif
            else
            {
                connection.setResponseHeader("Content-type", "text/plain");
            }
            connection.sendFile(fspath);
        }
        return true;
    }
};

namespace
{
bool global_isActive = true;

void catch_function(int signo)
{
    LOGI(LOG_DOMAIN, "Interactive attention signal caught. %d", signo);

    global_isActive = false;
}
} // namespace

struct ServerSettings
{
    IHttpServer::settings_t httpsServerSettings;
    IHttpServer::settings_t httpServerSettings;
};

template <>
serialization::ObjectAssembler<ServerSettings> serialization::Assembler()
{
    // clang-format off
    return ObjectAssembler<ServerSettings>()
        .define("httpServerSettings", &ServerSettings::httpServerSettings)
        .define("httpsServerSettings", &ServerSettings::httpsServerSettings)
        ;
    // clang-format on
}

template <>
serialization::ObjectAssembler<IHttpServer::settings_t> serialization::Assembler()
{
    // clang-format off
    return ObjectAssembler<IHttpServer::settings_t>()
        .define("serverPort", &IHttpServer::settings_t::port)
        .define("useSSL", &IHttpServer::settings_t::useSSL)
        .define("keyFilePath", &IHttpServer::settings_t::keyFilePath)
        .define("certFilePath", &IHttpServer::settings_t::certFilePath)
        ;
    // clang-format on
}

int main(int argc, char *argv[])
{
    Settings::instance().declare<ServerSettings>("Settings");

    // The files containing a key and a sertificate can be generated
    // on the local host using the following command:
    // openssl req -newkey rsa:2048 -new -nodes -x509 -days 3650 -keyout ./key.pem -out ./cert.pem

    ServerSettings &settings = Settings::instance().access<ServerSettings>();
    settings.httpsServerSettings.port = 8443;
    settings.httpsServerSettings.keyFilePath = "./key.pem";
    settings.httpsServerSettings.certFilePath = "./cert.pem";
    settings.httpsServerSettings.useSSL = true;

    MyHttpDispatcher dispatcher;
#if defined(HTTPS_ENABLED)
    HttpServer httpsServer(settings.httpsServerSettings, dispatcher);
    if (!httpsServer.start())
    {
        LOGE(LOG_DOMAIN, "Cannot start HTTPS server. Make sure that certificate and key file are created");
        LOGE(LOG_DOMAIN, "openssl req -newkey rsa:2048 -new -nodes -x509 -days 3650 -keyout ./key.pem -out ./cert.pem");
        return EXIT_FAILURE;
    }
#endif

    settings.httpServerSettings.port = 8080;
    settings.httpServerSettings.useSSL = false;

    HttpServer httpServer(settings.httpServerSettings, dispatcher);
    if (!httpServer.start())
    {
        LOGE(LOG_DOMAIN, "Cannot start HTTPS server.");
        return EXIT_FAILURE;
    }

    if (signal(SIGINT, catch_function) == SIG_ERR)
    {
        LOGE(LOG_DOMAIN, "An error occurred while setting a signal handler.");
        return EXIT_FAILURE;
    }

    while (global_isActive)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
    LOGI(LOG_DOMAIN, "Exiting");

    return EXIT_SUCCESS;
}
