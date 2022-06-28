#include <gtest/gtest.h>

#include <chrono>
#include <thread>
#include <vector>
#include <memory>

#include <softeq/common/log.hh>
#include <softeq/common/net/curl_helper.hh>
#include <softeq/common/net/http_server.hh>
#include <softeq/common/net/http_connection.hh>
#include <softeq/common/settings.hh>

using namespace softeq::common;
using namespace softeq::common::net;

namespace
{
const char *const LOG_DOMAIN = "HttpSessionTest";
static int SERVER_PORT = 55150;
static int SESSION_LENGTH = 5; // sec
} // namespace

class ISessionWithData : public HttpSession
{
public:
    virtual void setData(const std::string &data) = 0;
    virtual std::string data() const = 0;
};

class SessionType1 : public ISessionWithData
{
public:
    void onExpire() override
    {
        LOGI(LOG_DOMAIN, "The SessionType1 %s has expired", id().c_str());
    }
    void setData(const std::string &data) override
    {
        _data = data;
    }
    std::string data() const override
    {
        return _data;
    }

private:
    std::string _data;
};

class SessionType2 : public ISessionWithData
{
public:
    void onExpire() override
    {
        LOGI(LOG_DOMAIN, "The SessionType2 %s has expired", id().c_str());
    }
    void setData(const std::string &data) override
    {
        _data = data;
    }
    std::string data() const override
    {
        return _data;
    }

private:
    std::string _data;
};

class HttpDispatcher final : public IHttpConnectionDispatcher
{
public:
    bool handle(IHttpConnection &connection)
    {
        std::string webPath = connection.get();

        if (webPath == "/create_session")
        {
            std::shared_ptr<ISessionWithData> session;
            std::string type;

            if (connection.requestHasHeader("SessionType"))
                type = connection.header("SessionType");

            // 1) A session ID is generated for every successful request to the server.
            if (type == "Type1")
                session = std::make_shared<SessionType1>();
            if (type == "Type2")
                session = std::make_shared<SessionType2>();

            if (session)
            {
                // 2) If during the current request the client's data is NOT saved for further work with it,
                // then the lifetime of this client's session ends.
                LOGI(LOG_DOMAIN, "Created session id %s", session->id().c_str());
                connection << session->id();

                // 3) If client data is saved, HttpSession binds the saved data to the previously generated session ID.
                if (connection.body().size())
                {
                    session->touch();
                    session->extendExpiration(std::time(0) + SESSION_LENGTH);
                    session->setData(connection.body());

                    _sessionPool.push_back(session);
                }
            }
        }

        if (webPath == "/restore_session")
        {
            std::string id = connection.body();

            // 4) On the next request, the client sends a personal identifier to the server.
            // The server matches the identifiers and "recognizes" the client within the current session.

            if (!id.empty())
            {
                auto it = find_if(_sessionPool.begin(), _sessionPool.end(),
                                  [&id](HttpSession::SPtr &session) { return session->id() == id; });

                if (it != _sessionPool.end())
                {
                    std::shared_ptr<ISessionWithData> session;
                    std::string type;

                    if (connection.requestHasHeader("SessionType"))
                        type = connection.header("SessionType");

                    if (type == "Type1")
                        session = std::dynamic_pointer_cast<SessionType1>(*it);
                    if (type == "Type2")
                        session = std::dynamic_pointer_cast<SessionType2>(*it);

                    if (session)
                    {
                        session->touch();

                        // 5) As long as the client transmits his personal key, the session is considered active. The
                        // session can end for various reasons, for example, manually on the server side or after a
                        // certain set time (timeout).

                        if (session->expiration() > session->lastActivity())
                        {
                            LOGI(LOG_DOMAIN, "Restored session id %s", id.c_str());

                            connection.setResponseHeader("Content-type", "text/plain");
                            connection << session->data();
                            session->extendExpiration(std::time(0) + SESSION_LENGTH);
                        }
                        else
                            session->onExpire();
                    }
                }
            }
        }

        return true;
    }

private:
    std::vector<HttpSession::SPtr> _sessionPool;
};

class HttpSessionTest : public testing::Test
{
protected:
    void SetUp() override
    {
        _serverUrl = "http://localhost:" + std::to_string(SERVER_PORT);
        _serverSettings.useSSL = false;
        _serverSettings.port = SERVER_PORT++;

        _httpServer.reset(new HttpServer(_serverSettings, _dispatcher));
        ASSERT_TRUE(_httpServer->start());
    }
    void TearDown() override
    {
        _httpServer->stop();
    }

protected:
    std::string _serverUrl;
    HttpDispatcher _dispatcher;
    std::unique_ptr<HttpServer> _httpServer;
    IHttpServer::settings_t _serverSettings;
};

TEST_F(HttpSessionTest, CreateSessionID)
{
    CurlHelper curl(_serverUrl + "/create_session");
    curl.addRequestHeader("SessionType", "Type1");

    ASSERT_TRUE(curl.doGet());
    ASSERT_TRUE(curl.responseCode() == HttpStatusCode::STATUS_OK);
    ASSERT_TRUE(curl.responseText().size() == 36);

    curl.addRequestHeader("SessionType", "Type2");

    ASSERT_TRUE(curl.doGet());
    ASSERT_TRUE(curl.responseCode() == HttpStatusCode::STATUS_OK);
    ASSERT_TRUE(curl.responseText().size() == 36);
}

TEST_F(HttpSessionTest, RestoreSession)
{
    CurlHelper curl_create(_serverUrl + "/create_session");
    curl_create.addRequestHeader("SessionType", "Type1");

    std::string data = "UserName";
    ASSERT_TRUE(curl_create.doPost(data));
    ASSERT_TRUE(curl_create.responseCode() == HttpStatusCode::STATUS_OK);
    std::string id = curl_create.responseText();

    CurlHelper curl_restore(_serverUrl + "/restore_session");
    curl_restore.addRequestHeader("SessionType", "Type1");

    ASSERT_TRUE(curl_restore.doPost(id));
    ASSERT_TRUE(curl_restore.responseCode() == HttpStatusCode::STATUS_OK);
    ASSERT_TRUE(data == curl_restore.responseText());
}

TEST_F(HttpSessionTest, MultiSession)
{
    CurlHelper curl_create(_serverUrl + "/create_session");
    curl_create.addRequestHeader("SessionType", "Type1");

    std::string data_01 = "UserName_01";
    ASSERT_TRUE(curl_create.doPost(data_01));
    std::string id_01 = curl_create.responseText();

    curl_create.addRequestHeader("SessionType", "Type2");

    std::string data_02 = "UserName_02";
    ASSERT_TRUE(curl_create.doPost(data_02));
    std::string id_02 = curl_create.responseText();

    //=====================================================

    CurlHelper curl_restore(_serverUrl + "/restore_session");
    curl_restore.addRequestHeader("SessionType", "Type2");

    ASSERT_TRUE(curl_restore.doPost(id_02));
    ASSERT_TRUE(curl_restore.responseCode() == HttpStatusCode::STATUS_OK);
    ASSERT_TRUE(data_02 == curl_restore.responseText());

    curl_restore.addRequestHeader("SessionType", "Type1");

    ASSERT_TRUE(curl_restore.doPost(id_01));
    ASSERT_TRUE(curl_restore.responseCode() == HttpStatusCode::STATUS_OK);
    ASSERT_TRUE(data_01 == curl_restore.responseText());
}

TEST_F(HttpSessionTest, WrongSessionID)
{
    CurlHelper curl_create(_serverUrl + "/create_session");
    curl_create.addRequestHeader("SessionType", "Type1");

    std::string data = "UserName";
    ASSERT_TRUE(curl_create.doPost(data));
    ASSERT_TRUE(curl_create.responseCode() == HttpStatusCode::STATUS_OK);

    CurlHelper curl_restore(_serverUrl + "/restore_session");
    curl_restore.addRequestHeader("SessionType", "Type1");

    std::string id = "wrong-id-wrong-id-wrong-id-wrong-id";

    ASSERT_TRUE(curl_restore.doPost(id));
    ASSERT_TRUE(curl_restore.responseCode() == HttpStatusCode::STATUS_OK);
    ASSERT_FALSE(data == curl_restore.responseText());
}

TEST_F(HttpSessionTest, SessionTimeOut)
{
    CurlHelper curl_create(_serverUrl + "/create_session");
    curl_create.addRequestHeader("SessionType", "Type1");

    std::string data = "UserName";
    ASSERT_TRUE(curl_create.doPost(data));
    ASSERT_TRUE(curl_create.responseCode() == HttpStatusCode::STATUS_OK);
    std::string id = curl_create.responseText();

    CurlHelper curl_restore(_serverUrl + "/restore_session");

    std::this_thread::sleep_for(std::chrono::seconds(SESSION_LENGTH - 2));

    curl_restore.addRequestHeader("SessionType", "Type1");
    ASSERT_TRUE(curl_restore.doPost(id));
    ASSERT_TRUE(curl_restore.responseCode() == HttpStatusCode::STATUS_OK);
    ASSERT_TRUE(data == curl_restore.responseText());

    std::this_thread::sleep_for(std::chrono::seconds(SESSION_LENGTH + 2));

    curl_restore.addRequestHeader("SessionType", "Type1");
    ASSERT_TRUE(curl_restore.doPost(id));
    ASSERT_TRUE(curl_restore.responseCode() == HttpStatusCode::STATUS_OK);
    ASSERT_FALSE(data == curl_restore.responseText());
}

TEST_F(HttpSessionTest, SessionDataChecking)
{
    CurlHelper curl_request(_serverUrl + "/create_session");

    std::string data_01 = "data_01";
    std::string data_02 = "data_02";
    std::string data_03 = "data_03";
    std::string data_04 = "data_04";
    std::string data_05 = "data_05";

    curl_request.addRequestHeader("SessionType", "Type1");
    ASSERT_TRUE(curl_request.doPost(data_01));
    std::string id_01 = curl_request.responseText();

    curl_request.addRequestHeader("SessionType", "Type2");
    ASSERT_TRUE(curl_request.doPost(data_02));
    std::string id_02 = curl_request.responseText();

    curl_request.addRequestHeader("SessionType", "Type1");
    ASSERT_TRUE(curl_request.doPost(data_03));
    std::string id_03 = curl_request.responseText();

    curl_request.addRequestHeader("SessionType", "Type2");
    ASSERT_TRUE(curl_request.doPost(data_04));
    std::string id_04 = curl_request.responseText();

    curl_request.addRequestHeader("SessionType", "Type1");
    ASSERT_TRUE(curl_request.doPost(data_05));
    std::string id_05 = curl_request.responseText();

    //======================================================

    CurlHelper curl_restore(_serverUrl + "/restore_session");

    curl_restore.addRequestHeader("SessionType", "Type1");
    ASSERT_TRUE(curl_restore.doPost(id_01));
    ASSERT_TRUE(curl_restore.responseCode() == HttpStatusCode::STATUS_OK);
    std::string responce_01 = curl_restore.responseText();

    curl_restore.addRequestHeader("SessionType", "Type2");
    ASSERT_TRUE(curl_restore.doPost(id_02));
    ASSERT_TRUE(curl_restore.responseCode() == HttpStatusCode::STATUS_OK);
    std::string responce_02 = curl_restore.responseText();

    curl_restore.addRequestHeader("SessionType", "Type1");
    ASSERT_TRUE(curl_restore.doPost(id_03));
    ASSERT_TRUE(curl_restore.responseCode() == HttpStatusCode::STATUS_OK);
    std::string responce_03 = curl_restore.responseText();

    curl_restore.addRequestHeader("SessionType", "Type2");
    ASSERT_TRUE(curl_restore.doPost(id_04));
    ASSERT_TRUE(curl_restore.responseCode() == HttpStatusCode::STATUS_OK);
    std::string responce_04 = curl_restore.responseText();

    curl_restore.addRequestHeader("SessionType", "Type1");
    ASSERT_TRUE(curl_restore.doPost(id_05));
    ASSERT_TRUE(curl_restore.responseCode() == HttpStatusCode::STATUS_OK);
    std::string responce_05 = curl_restore.responseText();

    ASSERT_TRUE(data_01 == responce_01);
    ASSERT_TRUE(data_02 == responce_02);
    ASSERT_TRUE(data_03 == responce_03);
    ASSERT_TRUE(data_04 == responce_04);
    ASSERT_TRUE(data_05 == responce_05);
}