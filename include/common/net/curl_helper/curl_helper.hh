#ifndef SOFTEQ_COMMON_CURL_HELPER_H_
#define SOFTEQ_COMMON_CURL_HELPER_H_

#include <string>
#include <functional>
#include <map>
#include <iterator>
#include <vector>

#include <curl/curl.h>

namespace softeq
{
namespace common
{
namespace net
{
namespace curl
{
// TODO: need to take care about concurrency. This class is not thread safe. Create separated instance for each call

/*!
  \brief The class describes CURL wrapper.
*/
class CurlHelper
{
public:
    explicit CurlHelper(const std::string &url);
    virtual ~CurlHelper();

    /**
     * Enable/Disable verbose mode in the URL library (output to the stderr)
     */
    void setVerbose(bool mode);

    /*!
       cookieMap_t is a map in order to store cookies by key,value
     */
    using cookieMap_t = std::map<std::string, std::string>;

    /* API is declared but not implemented. If you need some one to use then implement it */
    //    void add_request_file(const std::string& name, const std::string& filename, const std::string& description =
    //    "", const std::string& content_type = "");

    /*!
        Add param to request
        \param[in] name Name
        \param[in] value Value
      */
    void addRequestParam(const std::string &name, const std::string &value);

    /*!
        Add header to list
        \param[in] name Name
        \param[in] value Value
      */
    void addRequestHeader(const std::string &name, const std::string &value);
    //    void set_user_agent(const std::string& user_agent);
    /*!
        Send POST request
        \param[in] data Data in request
        \return Bool execution result
      */
    bool doPost(const std::string &data = "");
    /*!
        Send GET request
        \return Bool execution result
      */
    bool doGet();
    /* TODO: add
      bool do_upload_multipart_data();
bool do_upload(const std::string& filename,const std::string &data);*/

    /*void set_method(const std::string &str);
    bool async_perform();
    void set_progress_callback(std::function<void(CurlHelper&, void*)> func, void *data);
    int async_status();
        */
    /*!
        Get response code
        \return Response code
      */
    int responseCode();
    /*!
        Get error
         \return String error
      */
    std::string errorString();
    /*!
        Get text of response
         \return String error
      */
    std::string responseText();
    /*!
        Get raw response
        \param[in] body Pointer to row data
        \return Raw data size
      */
    size_t responseRaw(const uint8_t **body);
    /*!
        Get response headers
        \return String of headers
      */
    std::string responseHeaders();
    /*!
        Find particular header in the header list
        \param[in] name Name of header element
        \return TRUE if header element present, otherwise FALSE
      */
    bool responseHasHeader(const std::string &name) const;
    /*!
        Get response header  value by name
        \param[in] name Name of header element
        \return String with header element value if present, or empty string
      */
    std::string responseHeader(const std::string &name) const;

    /*   not implemented
int response_header_size();

void set_proxy(const std::string &host, int port, int type);
void set_proxy_credentials(const std::string &username, const std::string password);

void setReferer(const std::string &str);
void set_output(const std:: &str);
void set_upload_buffer_size(const int size);

template <typename T>
void set_curl_option(int option, const T &value);
int curl_result();
CURL* curl_handle();
    */
    static const std::string urlEncode(const std::string &str);
    /*Cookies interface*/
    /*!
        Get all cookies
        \return All cookies in map
      */
    cookieMap_t cookies() const;
    /*!
        Set cookies
        \param[in] map Map of cookies
     */
    void setCookies(const cookieMap_t &map);
    /*!
        Set cookie
        \param[in] key Key for cookie map
        \param[in] value Cookie value
      */
    void setCookie(const std::string &key, const std::string &value);
    /*!
        Get cookie
        \param[in] key Cookie key
        \return Value by the key in cookie map
      */
    std::string cookie(const std::string &key) const;

// TODO: remove define. rework test approach
#ifdef UNIT_TEST
public:
#else
private:
#endif
    void deserialize(const std::string &cookie);
    CURL *_curl_handle{};
    bool _verbose = false;
    std::string _url;
    bool _hasParams = false;
    struct curlData_t final
    {
        CurlHelper *host{};
        std::vector<uint8_t> bytes;
        std::size_t rdOffset;
        std::vector<uint8_t> headers;
        std::map<std::string, std::string> headersList;
    } _response;
    CURLcode _curlErrorCode{CURLE_OK};
    curl_slist *_headerList{};

    static size_t writeDataCallback(void *ptr, size_t size, size_t nmemb, curlData_t *data);
    static size_t headerDataCallback(char *ptr, size_t size, size_t nmemb, curlData_t *data);
    bool parseResponseHeaders();

    void setDefaultCurlHandleOptions();
    bool curlPerform();

private:
    cookieMap_t _cookies_repo;
    static size_t readDataCallback(void *ptr, size_t size, size_t nmemb, curlData_t *data);
    void serialize(std::string &cookie) const;
};

} // namespace curl
} // namespace net
} // namespace common
} // namespace softeq

#endif // SOFTEQ_COMMON_CURL_HELPER_H_
