#include <random>
#include <fstream>
#include <future>
#include <chrono>
#include <sys/stat.h>
#include <uuid/uuid.h>
#include <softeq/common/net/http_connection.hh>
#include <softeq/common/net/http_server.hh>
#include <softeq/common/net/curl_helper.hh>
#include <softeq/common/timeutils.hh>
#include <softeq/common/log.hh>
#include <gtest/gtest.h>

using namespace softeq::common::net;
using namespace softeq::common::time;

namespace
{
const char *LOG_DOMAIN = "HttpServerTest";

static std::string rootFileDirName{"/tmp"};
static std::string endpointRoot{"/"};
static std::string endpointUpload{"/upload"};
static std::string endpointDownloadFile{"/download_file.txt"};
static std::string endpointCreateSession{"/create_session"};
static std::string endpointSessionData{"/session_data"};
static std::string endpointCreateSessionWithSleep{"/create_session_with_sleep"};

static std::string headerXSession{"X-Session"};
static std::string headerXSessionExpiry{"X-Session-Expiry"};
}

class TestHttpDispatcher final : public IHttpConnectionDispatcher
{
public:
    struct TestSession : public HttpSession
    {
        std::string data;
        virtual void onExpire()
        {
            LOGI(LOG_DOMAIN, "The session %d has expired", id());
        }
    };

    bool handle(softeq::common::net::IHttpConnection &connection) override
    {
        if (connection.path() == endpointRoot)
        {
            // do nothing
            return true;
        }
        if (connection.path() == endpointUpload)
        {
            _uploadedContent = connection.body();
            return true;
        }
        if (connection.path() == endpointDownloadFile)
        {
            std::string fileName{rootFileDirName + connection.path()};
            connection.sendFile(fileName);
            return true;
        }
        if (connection.path() == endpointCreateSession)
        {
            auto session(std::make_shared<TestSession>());
            session->data = connection.body();
            session->lastActivity();
            connection.attachSession(session);
            return true;
        }
        if (connection.path() == endpointSessionData)
        {
            // it will throw exception in the bad case
            std::shared_ptr<TestSession> session = std::dynamic_pointer_cast<TestSession>(connection.session().lock());
            if (!session)
            {
                connection.setError(HttpStatusCode::STATUS_NOT_FOUND);
                return false;
            }
            connection.setResponseHeader("Content-type", "text/plain");
            connection << session->data;
            return true;
        }
        if (connection.path() == endpointCreateSessionWithSleep)
        {
            auto session(std::make_shared<TestSession>());
            session->data = connection.body();
            session->lastActivity();
            connection.attachSession(session);

            // sleep for 1 sec to have a time to do something async
            std::this_thread::sleep_for(std::chrono::milliseconds(1500));
            return true;
        }

        LOGE(LOG_DOMAIN, "TestHttpDispatcher: unknown endpoint");
        return false;
    }

    std::string uploadedContent()
    {
        return _uploadedContent;
    }

private:
    std::string _uploadedContent;
};

class HttpServerTest : public testing::Test
{
protected:
    void SetUp() override
    {
        _serverSettings.port = 8080;
        _serverSettings.useSSL = false;

        _server.reset(new HttpServer(_serverSettings, _dispatcher));

        ASSERT_TRUE(_server->start());
    }

    void TearDown() override
    {
        _server->stop();
    }

protected:

    std::string getRandomString(int length)
    {
        static const char alphanum[] =
            "0123456789"
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
    TestHttpDispatcher _dispatcher;
    std::unique_ptr<HttpServer> _server;

    const std::string _serverUrl{"http://localhost:8080"};
    const std::string _endpointUpload{"/upload"};
};

TEST_F(HttpServerTest, StarUseNotAvailableSSL)
{
    _server->stop();
    _serverSettings.useSSL = true;
    ASSERT_FALSE(_server->start());
}

TEST_F(HttpServerTest, UploadContent)
{
    std::string _uploadContent = getRandomString(8192);
    CurlHelper curl(_serverUrl + _endpointUpload);

    // send data to the server
    ASSERT_TRUE(curl.doPost(_uploadContent));
    // check uploaded data
    ASSERT_TRUE(_dispatcher.uploadedContent() == _uploadContent);
}

TEST_F(HttpServerTest, DownloadFile)
{
    std::string data = getRandomString(16384);
    std::string fileName{"/tmp" + endpointDownloadFile};
    std::ofstream file(fileName);
    file << data;
    file.close();

    CurlHelper curl(_serverUrl + endpointDownloadFile);
    ASSERT_TRUE(curl.doGet());

    EXPECT_EQ(curl.responseCode(), HttpStatusCode::STATUS_OK);
    ASSERT_TRUE(curl.responseText() == data);
    ASSERT_TRUE(curl.errorString() == "");

    // clean up
    remove(fileName.c_str());
}

TEST_F(HttpServerTest, DownloadFile_FileNotAccessible)
{
    std::string data = getRandomString(5);
    std::string fileName{"/tmp" + endpointDownloadFile};
    std::ofstream file(fileName);
    file << data;
    file.close();

    // chmod write only, which not allow to open for read
    EXPECT_EQ(chmod(fileName.c_str(), S_IWUSR), 0);

    CurlHelper curl(_serverUrl + endpointDownloadFile);
    curl.setVerbose(true);
    ASSERT_TRUE(curl.doGet());

    // check response
    LOGD(LOG_DOMAIN, "'%s'", curl.responseHeaders().c_str());
    EXPECT_EQ(curl.responseCode(), STATUS_NOT_ACCEPTABLE);
    EXPECT_EQ(curl.responseText().size(), 0);
    ASSERT_TRUE(curl.responseText() == "");
    ASSERT_TRUE(curl.responseHeaders().find("Content-Length: 0") != std::string::npos);
    ASSERT_TRUE(curl.errorString() == "");

    // clean up
    remove(fileName.c_str());
}

class CurlHelperExt : public CurlHelper
{
public:
    explicit CurlHelperExt(const std::string &url) : CurlHelper(url) {}
    virtual ~CurlHelperExt() {};

    bool doUnsupportedMethod() {
        curl_easy_setopt(_curl_handle, CURLOPT_URL, _url.c_str());
        curl_easy_setopt(_curl_handle, CURLOPT_CUSTOMREQUEST, "FOO_METHOD");
        curl_easy_setopt(_curl_handle, CURLOPT_NOSIGNAL, 1);
        if (_verbose)
        {
            curl_easy_setopt(_curl_handle, CURLOPT_STDERR, stdout);
            curl_easy_setopt(_curl_handle, CURLOPT_VERBOSE, 1L);
        }

        _curlErrorCode = curl_easy_perform(_curl_handle);

        if (_curlErrorCode) {
            LOGE(LOG_DOMAIN, "curl_easy_perform failed: %s", curl_easy_strerror(_curlErrorCode));
            return false;
        }

        return true;
    }

    void doDoubleCreateSessionWithSleep(const std::string& _serverUrl) {

        std::string req1_uuid_str;
        uuid_t req1_uuid;
        time_t req1_session_exp_time;
        CurlHelper req1(_serverUrl + endpointCreateSessionWithSleep);

        // do 1st request
        req1.setVerbose(true);
        ASSERT_TRUE(req1.doGet());
        // check request 1 response
        LOGD(LOG_DOMAIN, "'%s'", req1.responseHeaders().c_str());
        EXPECT_EQ(req1.responseCode(), 200);
        EXPECT_EQ(req1.responseText().size(), 0);
        ASSERT_TRUE(req1.responseText() == "");
        ASSERT_TRUE(req1.errorString() == "");

        // check request 1 X-Session header
        ASSERT_TRUE(req1.responseHasHeader(headerXSession));
        req1_uuid_str = req1.responseHeader(headerXSession);
        EXPECT_EQ(uuid_parse(req1_uuid_str.c_str(), req1_uuid), 0);

        // check request 1 X-Session-Expiry header
        ASSERT_TRUE(req1.responseHasHeader(headerXSessionExpiry));
        req1_session_exp_time = string_to_timestamp(req1.responseHeader(headerXSessionExpiry));
        ASSERT_TRUE(req1_session_exp_time != -1);


        // do 2d request with X-Session header from 1st
        req1.addRequestHeader(headerXSession, req1_uuid_str);

        // set timeout to not waste time when server goes down
        curl_easy_setopt(req1._curl_handle, CURLOPT_TIMEOUT, 1);

        // wait for a fail because in sleep time for the 1st request was send server->stop()
        ASSERT_FALSE(req1.doGet());
    }

    bool doOptionsMethod() {
        curl_easy_setopt(_curl_handle, CURLOPT_URL, _url.c_str());
        curl_easy_setopt(_curl_handle, CURLOPT_CUSTOMREQUEST, "OPTIONS");
        curl_easy_setopt(_curl_handle, CURLOPT_NOSIGNAL, 1);
        if (_verbose)
        {
            curl_easy_setopt(_curl_handle, CURLOPT_STDERR, stdout);
            curl_easy_setopt(_curl_handle, CURLOPT_VERBOSE, 1L);
        }

        _response.host = this;
        _response.bytes.clear();

        curl_easy_setopt(_curl_handle, CURLOPT_WRITEDATA, &_response);
        curl_easy_setopt(_curl_handle, CURLOPT_WRITEFUNCTION, CurlHelper::writeDataCallback);

        _response.headers.clear();
        curl_easy_setopt(_curl_handle, CURLOPT_HEADERDATA, &_response);
        curl_easy_setopt(_curl_handle, CURLOPT_HEADERFUNCTION, CurlHelper::headerDataCallback);

        if (_headerList)
        {
            curl_easy_setopt(_curl_handle, CURLOPT_HTTPHEADER, _headerList);
        }

        // Exec
        _curlErrorCode = curl_easy_perform(_curl_handle);

        if (!_curlErrorCode)
        {
            parseResponseHeaders();
        }

        if (_headerList)
        {
            curl_slist_free_all(_headerList);
            _headerList = nullptr;
        }

        if (_curlErrorCode) {
            LOGE(LOG_DOMAIN, "curl_easy_perform failed: %s", curl_easy_strerror(_curlErrorCode));
            return false;
        }

        return true;
    }
};

TEST_F(HttpServerTest, UnsupportedMethodWhileServerDown)
{
    // Test to emulate situation when came 2 requests in a one connection
    // ans server goes down while 1st request is not finished

    // send 2 CreateSession requests with 1sec sleep
    CurlHelperExt curlExt(_serverUrl);
    auto async = std::async(std::launch::async,
                            &CurlHelperExt::doDoubleCreateSessionWithSleep,
                            &curlExt,
                            _serverUrl);

    // wait for be sure that async already started
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    // try to down server at the moment of the 1st CreateSessionWithSleep request
    _server->stop();
}

TEST_F(HttpServerTest, UnsupportedHttpMethod)
{
    CurlHelperExt curl(_serverUrl);

    LOGD(LOG_DOMAIN, "Send Unsupported request");
    // expects that server just closes connection without any return data
    ASSERT_FALSE(curl.doUnsupportedMethod());
}

TEST_F(HttpServerTest, OptionsHttpMethod)
{
    CurlHelperExt curl(_serverUrl);
    ASSERT_TRUE(curl.doOptionsMethod());

    LOGD(LOG_DOMAIN, "%s", curl.responseHeaders().c_str());
    EXPECT_EQ(curl.responseCode(), HttpStatusCode::STATUS_OK);
    EXPECT_EQ(curl.responseText().size(), 0);
    ASSERT_TRUE(curl.responseHasHeader("Access-Control-Allow-Credentials"));
    ASSERT_TRUE(curl.responseHasHeader("Access-Control-Allow-Headers"));
    ASSERT_TRUE(curl.responseHasHeader("Access-Control-Allow-Methods"));
    ASSERT_TRUE(curl.responseHasHeader("Access-Control-Allow-Origin"));
    ASSERT_TRUE(curl.errorString() == "");
}

TEST_F(HttpServerTest, DownloadFileWithRange1_ok_StartDataRange)
{
    std::string fileName{"/tmp" + endpointDownloadFile};
    std::string data = getRandomString(16384);

    std::ofstream file(fileName);
    file << data;
    file.close();

    CurlHelper curl(_serverUrl + endpointDownloadFile);
    curl.addRequestHeader("Range", "bytes=0-1023");
    ASSERT_TRUE(curl.doGet());

    // check response
    //LOGD(LOG_DOMAIN, "'%s'", curl.responseHeaders().c_str());
    EXPECT_EQ(curl.responseCode(), 206);
    EXPECT_EQ(curl.responseText().size(), 1024);
    ASSERT_TRUE(curl.responseText() == data.substr(0, 1024 /*len*/));
    ASSERT_TRUE(curl.responseHeaders().find("Content-Length: 1024") != std::string::npos);
    ASSERT_TRUE(curl.responseHeaders().find("Content-Range: bytes 0-1023/16384") != std::string::npos);
    ASSERT_TRUE(curl.errorString() == "");

    // clean up
    remove(fileName.c_str());
}

TEST_F(HttpServerTest, DownloadFileWithRange2_ok_EndDataRange)
{
    std::string data{"1234567890"};
    std::string fileName{"/tmp" + endpointDownloadFile};

    std::ofstream file(fileName);
    file << data;
    file.close();

    CurlHelper curl(_serverUrl + endpointDownloadFile);
    curl.addRequestHeader("Range", "bytes=5-9");
    ASSERT_TRUE(curl.doGet());

    // check response
    //LOGD(LOG_DOMAIN, "'%s'", curl.responseHeaders().c_str());
    EXPECT_EQ(curl.responseCode(), 206);
    EXPECT_EQ(curl.responseText().size(), 5);
    ASSERT_TRUE(curl.responseText() == data.substr(5, 5 /*len*/));
    ASSERT_TRUE(curl.responseHeaders().find("Content-Length: 5") != std::string::npos);
    ASSERT_TRUE(curl.responseHeaders().find("Content-Range: bytes 5-9/10") != std::string::npos);
    ASSERT_TRUE(curl.errorString() == "");

    // clean up
    remove(fileName.c_str());
}

TEST_F(HttpServerTest, DownloadFileWithRange2_ok_MiddleDataRange)
{
    std::string data{"1234567890"};
    std::string fileName{"/tmp" + endpointDownloadFile};

    std::ofstream file(fileName);
    file << data;
    file.close();

    CurlHelper curl(_serverUrl + endpointDownloadFile);
    curl.addRequestHeader("Range", "bytes=2-7");
    ASSERT_TRUE(curl.doGet());

    // check response
    //LOGD(LOG_DOMAIN, "'%s'", curl.responseHeaders().c_str());
    EXPECT_EQ(curl.responseCode(), 206);
    EXPECT_EQ(curl.responseText().size(), 6);
    ASSERT_TRUE(curl.responseText() == data.substr(2, 6 /*len*/));
    ASSERT_TRUE(curl.responseHeaders().find("Content-Length: 6") != std::string::npos);
    ASSERT_TRUE(curl.responseHeaders().find("Content-Range: bytes 2-7/10") != std::string::npos);
    ASSERT_TRUE(curl.errorString() == "");

    // clean up
    remove(fileName.c_str());
}

TEST_F(HttpServerTest, DownloadFileWithRange2_ok_1byte)
{
    std::string data = getRandomString(5);
    std::string fileName{"/tmp" + endpointDownloadFile};

    std::ofstream file(fileName);
    file << data;
    file.close();

    CurlHelper curl(_serverUrl + endpointDownloadFile);
    curl.addRequestHeader("Range", "bytes=0-0");
    ASSERT_TRUE(curl.doGet());

    // check response
    //LOGD(LOG_DOMAIN, "'%s'", curl.responseHeaders().c_str());
    EXPECT_EQ(curl.responseCode(), 206);
    EXPECT_EQ(curl.responseText().size(), 1);
    ASSERT_TRUE(curl.responseText() == data.substr(0, 1 /*len*/));
    ASSERT_TRUE(curl.responseHeaders().find("Content-Length: 1") != std::string::npos);
    ASSERT_TRUE(curl.responseHeaders().find("Content-Range: bytes 0-0/5") != std::string::npos);
    ASSERT_TRUE(curl.errorString() == "");

    // clean up
    remove(fileName.c_str());
}

TEST_F(HttpServerTest, DownloadFileWithRange3_ok_2bytes)
{
    std::string data = getRandomString(5);
    std::string fileName{"/tmp" + endpointDownloadFile};

    std::ofstream file(fileName);
    file << data;
    file.close();

    CurlHelper curl(_serverUrl + endpointDownloadFile);
    curl.setVerbose(true);
    curl.addRequestHeader("Range", "bytes=0-1");
    ASSERT_TRUE(curl.doGet());

    // check response
    LOGD(LOG_DOMAIN, "'%s'", curl.responseHeaders().c_str());
    EXPECT_EQ(curl.responseCode(), 206);
    EXPECT_EQ(curl.responseText().size(), 2);
    ASSERT_TRUE(curl.responseText() == data.substr(0, 2 /*len*/));
    ASSERT_TRUE(curl.responseHeaders().find("Content-Length: 2") != std::string::npos);
    ASSERT_TRUE(curl.responseHeaders().find("Content-Range: bytes 0-1/5") != std::string::npos);
    ASSERT_TRUE(curl.errorString() == "");

    // clean up
    remove(fileName.c_str());
}

TEST_F(HttpServerTest, DownloadFileWithRange4_NoBytesPrefix)
{
    std::string data = getRandomString(5);
    std::string fileName{"/tmp" + endpointDownloadFile};

    std::ofstream file(fileName);
    file << data;
    file.close();

    CurlHelper curl(_serverUrl + endpointDownloadFile);
    curl.setVerbose(true);
    curl.addRequestHeader("Range", "0-1"); // should be 'bytes=0-1'
    ASSERT_TRUE(curl.doGet());

    // check response
    //LOGD(LOG_DOMAIN, "'%s'", curl.responseHeaders().c_str());
    EXPECT_EQ(curl.responseCode(), 416);
    EXPECT_EQ(curl.responseText().size(), 0);
    ASSERT_TRUE(curl.responseText() == "");
    ASSERT_TRUE(curl.responseHeaders().find("Content-Length: 0") != std::string::npos);
    ASSERT_TRUE(curl.errorString() == "");

    // clean up
    remove(fileName.c_str());
}

TEST_F(HttpServerTest, DownloadFileWithRange5_StartOffsetMoreFileSize)
{
    std::string data = getRandomString(5);
    std::string fileName{"/tmp" + endpointDownloadFile};

    std::ofstream file(fileName);
    file << data;
    file.close();

    CurlHelper curl(_serverUrl + endpointDownloadFile);
    curl.setVerbose(true);
    curl.addRequestHeader("Range", "bytes=100-200");
    ASSERT_TRUE(curl.doGet());

    // check response
    //LOGD(LOG_DOMAIN, "'%s'", curl.responseHeaders().c_str());
    EXPECT_EQ(curl.responseCode(), 416);
    EXPECT_EQ(curl.responseText().size(), 0);
    ASSERT_TRUE(curl.responseText() == "");
    ASSERT_TRUE(curl.responseHeaders().find("Content-Length: 0") != std::string::npos);
    ASSERT_TRUE(curl.errorString() == "");

    // clean up
    remove(fileName.c_str());
}

TEST_F(HttpServerTest, DownloadFileWithRange6_StartOffsetMoreEndOffset)
{
    std::string data = getRandomString(5);
    std::string fileName{"/tmp" + endpointDownloadFile};

    std::ofstream file(fileName);
    file << data;
    file.close();

    CurlHelper curl(_serverUrl + endpointDownloadFile);
    curl.setVerbose(true);
    curl.addRequestHeader("Range", "bytes=200-100");
    ASSERT_TRUE(curl.doGet());

    // check response
    //LOGD(LOG_DOMAIN, "'%s'", curl.responseHeaders().c_str());
    EXPECT_EQ(curl.responseCode(), 416);
    EXPECT_EQ(curl.responseText().size(), 0);
    ASSERT_TRUE(curl.responseText() == "");
    ASSERT_TRUE(curl.responseHeaders().find("Content-Length: 0") != std::string::npos);
    ASSERT_TRUE(curl.errorString() == "");

    // clean up
    remove(fileName.c_str());
}

TEST_F(HttpServerTest, DownloadFileWithRange7_FileNotFound)
{
    std::string data = getRandomString(1000);
    std::string fileName{"/tmp" + endpointDownloadFile + ".foo"};

    std::ofstream file(fileName);
    file << data;
    file.close();

    CurlHelper curl(_serverUrl + endpointDownloadFile);
    curl.setVerbose(true);
    curl.addRequestHeader("Range", "bytes=0-99");
    ASSERT_TRUE(curl.doGet());

    // check response
    LOGD(LOG_DOMAIN, "'%s'", curl.responseHeaders().c_str());
    EXPECT_EQ(curl.responseCode(), STATUS_NOT_FOUND);
    EXPECT_EQ(curl.responseText().size(), 0);
    ASSERT_TRUE(curl.responseText() == "");
    ASSERT_TRUE(curl.responseHeaders().find("Content-Length: 0") != std::string::npos);
    ASSERT_TRUE(curl.errorString() == "");

    // clean up
    remove(fileName.c_str());
}

TEST_F(HttpServerTest, DownloadFileWithRange8_FileNotAccessible)
{
    std::string data = getRandomString(5);
    std::string fileName{"/tmp" + endpointDownloadFile};
    std::ofstream file(fileName);
    file << data;
    file.close();

    // chmod write only, which not allow to open for read
    EXPECT_EQ(chmod(fileName.c_str(), S_IWUSR), 0);

    CurlHelper curl(_serverUrl + endpointDownloadFile);
    curl.setVerbose(true);
    curl.addRequestHeader("Range", "bytes=0-1");
    ASSERT_TRUE(curl.doGet());

    // check response
    LOGD(LOG_DOMAIN, "'%s'", curl.responseHeaders().c_str());
    EXPECT_EQ(curl.responseCode(), STATUS_INTERNAL_ERROR);
    EXPECT_EQ(curl.responseText().size(), 0);
    ASSERT_TRUE(curl.responseText() == "");
    ASSERT_TRUE(curl.responseHeaders().find("Content-Length: 0") != std::string::npos);
    ASSERT_TRUE(curl.errorString() == "");

    // clean up
    remove(fileName.c_str());
}

TEST_F(HttpServerTest, Session_UpdateExpirationTime)
{
    std::string req1_uuid_str;
    std::string req2_uuid_str;
    uuid_t req1_uuid;
    uuid_t req2_uuid;
    time_t req1_session_exp_time;
    time_t req2_session_exp_time;

    LOGD(LOG_DOMAIN, "=============== Request 1 ===============");

    CurlHelper req1(_serverUrl + endpointCreateSession);
    req1.setVerbose(true);
    ASSERT_TRUE(req1.doGet());

    // check request 1 response
    LOGD(LOG_DOMAIN, "'%s'", req1.responseHeaders().c_str());
    EXPECT_EQ(req1.responseCode(), HttpStatusCode::STATUS_OK);
    EXPECT_EQ(req1.responseText().size(), 0);
    ASSERT_TRUE(req1.responseText() == "");
    ASSERT_TRUE(req1.errorString() == "");

    // check request 1 X-Session header
    ASSERT_TRUE(req1.responseHasHeader(headerXSession));
    req1_uuid_str = req1.responseHeader(headerXSession);
    EXPECT_EQ(uuid_parse(req1_uuid_str.c_str(), req1_uuid), 0);

    // check request 1 X-Session-Expiry header
    ASSERT_TRUE(req1.responseHasHeader(headerXSessionExpiry));
    req1_session_exp_time = string_to_timestamp(req1.responseHeader(headerXSessionExpiry));
    ASSERT_TRUE(req1_session_exp_time != -1);

    // TODO: replace sleep with mock
    sleep(1);

    LOGD(LOG_DOMAIN, "=============== Request 2 ===============");

    CurlHelper req2(_serverUrl);
    req2.setVerbose(true);
    req2.addRequestHeader(headerXSession, req1_uuid_str);
    ASSERT_TRUE(req2.doGet());

    // check request 2 response
    EXPECT_EQ(req2.responseCode(), HttpStatusCode::STATUS_OK);
    EXPECT_EQ(req2.responseText().size(), 0);
    ASSERT_TRUE(req2.responseText() == "");
    ASSERT_TRUE(req2.errorString() == "");

    // check request 2 X-Session header
    ASSERT_TRUE(req2.responseHasHeader(headerXSession));
    req2_uuid_str = req2.responseHeader(headerXSession);
    EXPECT_EQ(uuid_parse(req2_uuid_str.c_str(), req2_uuid), 0);

    // check request 2 X-Session-Expiry header
    ASSERT_TRUE(req2.responseHasHeader(headerXSessionExpiry));
    req2_session_exp_time = string_to_timestamp(req2.responseHeader(headerXSessionExpiry));
    ASSERT_TRUE(req2_session_exp_time != -1);

    ASSERT_TRUE(req1_uuid_str == req2_uuid_str);
    EXPECT_EQ(req2_session_exp_time - req1_session_exp_time, 1);
}

TEST_F(HttpServerTest, Session_CreateTwoSessions)
{
    std::string req1_uuid_str;
    std::string req2_uuid_str;
    uuid_t req1_uuid;
    uuid_t req2_uuid;
    time_t req1_session_exp_time;
    time_t req2_session_exp_time;

    LOGD(LOG_DOMAIN, "=============== Request 1 ===============");

    // Send request to create a new Session
    CurlHelper req1(_serverUrl + endpointCreateSession);
    req1.setVerbose(true);
    ASSERT_TRUE(req1.doGet());

    // check request 1 response
    LOGD(LOG_DOMAIN, "'%s'", req1.responseHeaders().c_str());
    EXPECT_EQ(req1.responseCode(), HttpStatusCode::STATUS_OK);
    EXPECT_EQ(req1.responseText().size(), 0);
    ASSERT_TRUE(req1.responseText() == "");
    ASSERT_TRUE(req1.errorString() == "");

    // check request 1 X-Session header
    ASSERT_TRUE(req1.responseHasHeader("X-Session"));
    req1_uuid_str = req1.responseHeader("X-Session");
    EXPECT_EQ(uuid_parse(req1_uuid_str.c_str(), req1_uuid), 0);

    // check request 1 X-Session-Expiry header
    ASSERT_TRUE(req1.responseHasHeader("X-Session-Expiry"));
    req1_session_exp_time = string_to_timestamp(req1.responseHeader("X-Session-Expiry"));
    ASSERT_TRUE(req1_session_exp_time != -1);

    // TODO: replace sleep with mock
    sleep(1);

    LOGD(LOG_DOMAIN, "=============== Request 2 ===============");

    CurlHelper req2(_serverUrl + endpointCreateSession);
    req2.setVerbose(true);
    ASSERT_TRUE(req2.doGet());

    // check request 2 response
    EXPECT_EQ(req2.responseCode(), HttpStatusCode::STATUS_OK);
    EXPECT_EQ(req2.responseText().size(), 0);
    ASSERT_TRUE(req2.responseText() == "");
    ASSERT_TRUE(req2.errorString() == "");

    // check request 2 X-Session header
    ASSERT_TRUE(req2.responseHasHeader(headerXSession));
    req2_uuid_str = req2.responseHeader(headerXSession);
    EXPECT_EQ(uuid_parse(req2_uuid_str.c_str(), req2_uuid), 0);

    // check request 2 X-Session-Expiry header
    ASSERT_TRUE(req2.responseHasHeader(headerXSessionExpiry));
    req2_session_exp_time = string_to_timestamp(req2.responseHeader(headerXSessionExpiry));
    ASSERT_TRUE(req2_session_exp_time != -1);

    ASSERT_TRUE(req1_uuid_str != req2_uuid_str);
    EXPECT_EQ(req2_session_exp_time - req1_session_exp_time, 1);
}

TEST_F(HttpServerTest, Session_CreateNewSessionFromPreviousOne)
{
    std::string req1_uuid_str;
    std::string req2_uuid_str;
    uuid_t req1_uuid;
    uuid_t req2_uuid;
    time_t req1_session_exp_time;
    time_t req2_session_exp_time;

    LOGD(LOG_DOMAIN, "=============== Request 1 ===============");

    // Send request to create a new Session
    CurlHelper req1(_serverUrl + endpointCreateSession);
    req1.setVerbose(true);
    ASSERT_TRUE(req1.doGet());

    // check request 1 response
    EXPECT_EQ(req1.responseCode(), HttpStatusCode::STATUS_OK);
    EXPECT_EQ(req1.responseText().size(), 0);
    ASSERT_TRUE(req1.responseText() == "");
    ASSERT_TRUE(req1.errorString() == "");

    // check request 1 X-Session header
    ASSERT_TRUE(req1.responseHasHeader(headerXSession));
    req1_uuid_str = req1.responseHeader(headerXSession);
    EXPECT_EQ(uuid_parse(req1_uuid_str.c_str(), req1_uuid), 0);

    // check request 1 X-Session-Expiry header
    ASSERT_TRUE(req1.responseHasHeader(headerXSessionExpiry));
    req1_session_exp_time = string_to_timestamp(req1.responseHeader(headerXSessionExpiry));
    ASSERT_TRUE(req1_session_exp_time != -1);

    // TODO: replace sleep with mock
    sleep(1);

    LOGD(LOG_DOMAIN, "=============== Request 2 ===============");

    // Send request to create a new Session with the header of the previous Session
    CurlHelper req2(_serverUrl + endpointCreateSession);
    req2.setVerbose(true);
    req2.addRequestHeader(headerXSession, req1_uuid_str);
    ASSERT_TRUE(req2.doGet());

    // check request 2 response
    LOGD(LOG_DOMAIN, "'%s'", req2.responseHeaders().c_str());
    EXPECT_EQ(req2.responseCode(), HttpStatusCode::STATUS_OK);
    EXPECT_EQ(req2.responseText().size(), 0);
    ASSERT_TRUE(req2.responseText() == "");
    ASSERT_TRUE(req2.errorString() == "");

    // check request 2 X-Session header
    ASSERT_TRUE(req2.responseHasHeader(headerXSession));
    req2_uuid_str = req2.responseHeader(headerXSession);
    EXPECT_EQ(uuid_parse(req2_uuid_str.c_str(), req2_uuid), 0);

    // check request 2 X-Session-Expiry header
    ASSERT_TRUE(req2.responseHasHeader(headerXSessionExpiry));
    req2_session_exp_time = string_to_timestamp(req2.responseHeader(headerXSessionExpiry));
    ASSERT_TRUE(req2_session_exp_time != -1);

    // uuid of second session must be different
    ASSERT_TRUE(req1_uuid_str != req2_uuid_str);
    EXPECT_EQ(req2_session_exp_time - req1_session_exp_time, 1);
}

TEST_F(HttpServerTest, Session_SessionData)
{
    std::string req1_uuid_str;
    std::string req2_uuid_str;
    uuid_t req1_uuid;
    uuid_t req2_uuid;
    time_t req1_session_exp_time;
    time_t req2_session_exp_time;
    std::string session_data{"session_internal_data"};

    LOGD(LOG_DOMAIN, "=============== Request 1 ===============");

    // Send request to create a new Session with some session data
    CurlHelper req1(_serverUrl + endpointCreateSession);
    req1.setVerbose(true);
    req1.addRequestHeader("Content-Type", "text/plain");
    ASSERT_TRUE(req1.doPost(session_data));

    // check request 1 response
    EXPECT_EQ(req1.responseCode(), HttpStatusCode::STATUS_OK);
    EXPECT_EQ(req1.responseText().size(), 0);
    ASSERT_TRUE(req1.responseText() == "");
    ASSERT_TRUE(req1.errorString() == "");

    // check request 1 X-Session header
    req1_uuid_str = req1.responseHeader(headerXSession);
    EXPECT_EQ(uuid_parse(req1_uuid_str.c_str(), req1_uuid), 0);

    // check request 1 X-Session-Expiry header
    req1_session_exp_time = string_to_timestamp(req1.responseHeader(headerXSessionExpiry));
    ASSERT_TRUE(req1_session_exp_time != -1);

    // TODO: replace sleep with mock
    sleep(1);

    LOGD(LOG_DOMAIN, "=============== Request 2 ===============");

    // Send request to create a new Session with the header of the previous Session
    CurlHelper req2(_serverUrl + endpointSessionData);
    req2.setVerbose(true);

    req2.addRequestHeader(headerXSession, req1_uuid_str);
    ASSERT_TRUE(req2.doGet());

    // check request 2 response
    EXPECT_EQ(req2.responseCode(), HttpStatusCode::STATUS_OK);
    EXPECT_EQ(req2.responseText().size(), session_data.size());
    ASSERT_TRUE(req2.responseText() == session_data);
    ASSERT_TRUE(req2.errorString() == "");

    // check request 2 X-Session header
    req2_uuid_str = req2.responseHeader(headerXSession);
    EXPECT_EQ(uuid_parse(req2_uuid_str.c_str(), req2_uuid), 0);

    // check request 2 X-Session-Expiry header
    req2_session_exp_time = string_to_timestamp(req2.responseHeader(headerXSessionExpiry));
    ASSERT_TRUE(req2_session_exp_time != -1);

    ASSERT_TRUE(req1_uuid_str == req2_uuid_str);
    EXPECT_EQ(req2_session_exp_time - req1_session_exp_time, 1);
}

TEST_F(HttpServerTest, Session_IgnoreWrongSession)
{
    CurlHelper curl(_serverUrl);
    curl.setVerbose(true);
    curl.addRequestHeader(headerXSession, "11111111-1111-1111-1111-111111111111");
    ASSERT_TRUE(curl.doGet());

    // check response
    EXPECT_EQ(curl.responseCode(), HttpStatusCode::STATUS_OK);
    ASSERT_TRUE(curl.responseText() == "");
    ASSERT_TRUE(curl.errorString() == "");
    ASSERT_FALSE(curl.responseHasHeader(headerXSession));
    ASSERT_FALSE(curl.responseHasHeader(headerXSessionExpiry));
}
