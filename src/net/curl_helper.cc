#include "softeq/common/net/curl_helper.hh"
#include <cstring>
#include <cassert>
#include <algorithm>
#include <iostream>
#include <sstream>
#include "softeq/common/log.hh"
#include "softeq/common/stdutils.hh"
#include "softeq/common/scope_guard.hh"

#define SKIP_PEER_VERIFICATION 1
#define SKIP_HOSTNAME_VERIFICATION 1

namespace
{
constexpr int MAX_COOKIES_SIZE = 10000;
} // namespace

using namespace softeq::common::net;

namespace
{
const char *LOG_DOMAIN = "CurlHelper";
}

CurlHelper::CurlHelper(const std::string &url)
    : _curl_handle(curl_easy_init())
    , _url(url)
{
    assert(_curl_handle);
}

CurlHelper::~CurlHelper()
{
    curl_easy_cleanup(_curl_handle);
    if (_headerList)
        curl_slist_free_all(_headerList);
}

void CurlHelper::setVerbose(bool mode)
{
    _verbose = mode;
}

void CurlHelper::addRequestHeader(const std::string &name, const std::string &value)
{
    if (_curl_handle)
    {
        std::string header = stdutils::string_format("%s: %s", name.c_str(), value.c_str());
        _headerList = curl_slist_append(_headerList, header.c_str());
    }
}

// string to map
void CurlHelper::deserialize(const std::string &cookie)
{
    // Field number, what type and example data and the meaning of cook:
    // 1. string example.com - the domain name
    // 2. boolean FALSE - include subdomains
    // 3. string /foobar/ - path
    // 4. boolean TRUE - send/receive over HTTPS only
    // 5. number 1462299217 - expires at - seconds since Jan 1st 1970, or 0
    // 6. string person - name of the cookie
    // 7. string daniel - value of the cookie
    std::string data = cookie;
    if (data.at(0) == '#') // Lines that start with # are treated as comments.
    {
        if (data.find('\n') == std::string::npos)
            return;
    }

    ulong map_size = 0;
    for (const auto &p : _cookies_repo)
    {
        map_size += p.first.length();
        map_size += p.second.length();
    }
    char delimiter = '\t';

    std::string token;

    std::vector<std::string> tmp = stdutils::string_split(data, delimiter);
    if (map_size + tmp[6].length() + tmp[5].length() <= MAX_COOKIES_SIZE)
    {
        _cookies_repo.insert(std::make_pair(tmp[5], tmp[6]));
    }
}

// map to string
void CurlHelper::serialize(std::string &cookie) const
{
    for (auto it : _cookies_repo)
    {
        cookie += stdutils::string_format("%s=%s; ", it.first.c_str(), it.second.c_str());
    }
}

CurlHelper::cookieMap_t CurlHelper::cookies() const
{
    return _cookies_repo;
}

void CurlHelper::setCookies(const cookieMap_t &map)
{
    _cookies_repo = map;
}

void CurlHelper::setCookie(const std::string &key, const std::string &value)
{
    auto search = _cookies_repo.find(key);
    if (search != _cookies_repo.end() && search->second == value)
    {
        return;
    }
    else
    {
        ulong map_size = 0;
        for (const auto &it : _cookies_repo)
        {
            map_size += it.first.length();
            map_size += it.second.length();
        }

        if ((map_size + value.length() + key.length()) <= MAX_COOKIES_SIZE)
        {
            _cookies_repo.insert((std::make_pair(key, value)));
        }
    }
}

std::string CurlHelper::cookie(const std::string &key) const
{
    return _cookies_repo.at(key);
}

bool CurlHelper::doGet()
{
    curl_easy_setopt(_curl_handle, CURLOPT_URL, _url.c_str());
    curl_easy_setopt(_curl_handle, CURLOPT_HTTPGET, 1L);
    curl_easy_setopt(_curl_handle, CURLOPT_NOSIGNAL, 1);
    curl_easy_setopt(_curl_handle, CURLOPT_TIMEOUT, 30L);

    if (_verbose)
    {
        curl_easy_setopt(_curl_handle, CURLOPT_STDERR, stdout);
        curl_easy_setopt(_curl_handle, CURLOPT_VERBOSE, 1L);
    }

#ifdef SKIP_PEER_VERIFICATION
    /*
     * If you want to connect to a site who isn't using a certificate that is
     * signed by one of the certs in the CA bundle you have, you can skip the
     * verification of the server's certificate. This makes the connection
     * A LOT LESS SECURE.
     *
     * If you have a CA cert for the server stored someplace else than in the
     * default bundle, then the CURLOPT_CAPATH option might come handy for
     * you.
     */
    curl_easy_setopt(_curl_handle, CURLOPT_SSL_VERIFYPEER, 0L);
#endif

#ifdef SKIP_HOSTNAME_VERIFICATION
    /*
     * If the site you're connecting to uses a different host name that what
     * they have mentioned in their server certificate's commonName (or
     * subjectAltName) fields, libcurl will refuse to connect. You can skip
     * this check, but this will make the connection less secure.
     */
    curl_easy_setopt(_curl_handle, CURLOPT_SSL_VERIFYHOST, 0L);
#endif

    std::string cookie;
    serialize(cookie);

    curl_easy_setopt(_curl_handle, CURLOPT_COOKIEFILE, ""); // start "cookie engine"
    curl_easy_setopt(_curl_handle, CURLOPT_COOKIE, cookie.c_str());

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

    char *url_info;
    long response_code;
    double elapsed;

    _curlErrorCode = curl_easy_perform(_curl_handle);

    if (!_curlErrorCode)
    {
        scope_guard cleanup;
        struct curl_slist *cookies = NULL;
        int res = curl_easy_getinfo(_curl_handle, CURLINFO_COOKIELIST, &cookies);
        if (!res && cookies)
        {
            cleanup += [cookies]() { curl_slist_free_all(cookies); };
            struct curl_slist *each = cookies;
            while (each)
            {
                deserialize(each->data);
                each = each->next;
            }
        }
        _response.bytes.shrink_to_fit();
    }

    parseResponseHeaders();

    curl_easy_getinfo(_curl_handle, CURLINFO_RESPONSE_CODE, &response_code);
    curl_easy_getinfo(_curl_handle, CURLINFO_TOTAL_TIME, &elapsed);
    curl_easy_getinfo(_curl_handle, CURLINFO_EFFECTIVE_URL, &url_info);

    if (_headerList)
    {
        curl_slist_free_all(_headerList);
        _headerList = nullptr;
    }

    if (_curlErrorCode)
    {
        LOGE(LOG_DOMAIN, "curl_easy_perform failed: %s", curl_easy_strerror(_curlErrorCode));
        return false;
    }

    return true;
}

bool CurlHelper::doPost(const std::string &data /*=""*/)
{
    curl_easy_setopt(_curl_handle, CURLOPT_URL, _url.c_str());
    curl_easy_setopt(_curl_handle, CURLOPT_POST, 1L);
    curl_easy_setopt(_curl_handle, CURLOPT_NOSIGNAL, 1);
    curl_easy_setopt(_curl_handle, CURLOPT_TIMEOUT, 30L);

    if (_verbose)
    {
        curl_easy_setopt(_curl_handle, CURLOPT_STDERR, stdout);
        curl_easy_setopt(_curl_handle, CURLOPT_VERBOSE, 1L);
    }

#ifdef SKIP_PEER_VERIFICATION
    /*
     * If you want to connect to a site who isn't using a certificate that is
     * signed by one of the certs in the CA bundle you have, you can skip the
     * verification of the server's certificate. This makes the connection
     * A LOT LESS SECURE.
     *
     * If you have a CA cert for the server stored someplace else than in the
     * default bundle, then the CURLOPT_CAPATH option might come handy for
     * you.
     */
    curl_easy_setopt(_curl_handle, CURLOPT_SSL_VERIFYPEER, 0L);
#endif

#ifdef SKIP_HOSTNAME_VERIFICATION
    /*
     * If the site you're connecting to uses a different host name that what
     * they have mentioned in their server certificate's commonName (or
     * subjectAltName) fields, libcurl will refuse to connect. You can skip
     * this check, but this will make the connection less secure.
     */
    curl_easy_setopt(_curl_handle, CURLOPT_SSL_VERIFYHOST, 0L);
#endif

    std::string cookie;
    serialize(cookie);

    curl_easy_setopt(_curl_handle, CURLOPT_COOKIEFILE, ""); // start "cookie engine"
    curl_easy_setopt(_curl_handle, CURLOPT_COOKIE, cookie.c_str());

    curlData_t read_data;

    read_data.host = this;
    read_data.rdOffset = 0;
    read_data.bytes.assign(data.cbegin(), data.cend());

    curl_easy_setopt(_curl_handle, CURLOPT_READDATA, &read_data);
    curl_easy_setopt(_curl_handle, CURLOPT_READFUNCTION, CurlHelper::readDataCallback);
    curl_easy_setopt(_curl_handle, CURLOPT_POSTFIELDSIZE, read_data.bytes.size());

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

    _curlErrorCode = curl_easy_perform(_curl_handle);

    if (!_curlErrorCode)
    {
        scope_guard cleanup;
        struct curl_slist *cookies = NULL;
        int res = curl_easy_getinfo(_curl_handle, CURLINFO_COOKIELIST, &cookies);
        if (!res && cookies)
        {
            cleanup += [cookies] { curl_slist_free_all(cookies); };
            struct curl_slist *each = cookies;
            while (each)
            {
                deserialize(each->data);
                each = each->next;
            }
            /* free cookies when done */
        }
        _response.bytes.shrink_to_fit();
    }

    parseResponseHeaders();

    if (_headerList)
    {
        curl_slist_free_all(_headerList);
        _headerList = nullptr;
    }
    if (_curlErrorCode)
    {
        LOGE(LOG_DOMAIN, "curl_easy_perform failed: %s", curl_easy_strerror(_curlErrorCode));
        return false;
    }

    return true;
}

int CurlHelper::responseCode()
{
    long http_code = 0;
    curl_easy_getinfo(_curl_handle, CURLINFO_RESPONSE_CODE, &http_code);
    return http_code;
}

std::string CurlHelper::errorString()
{
    std::string s = curl_easy_strerror(_curlErrorCode);
    if (s == std::string("No error"))
    {
        s.clear();
    }

    return s;
}

std::string CurlHelper::responseText()
{
    return std::string(reinterpret_cast<const char *>(_response.bytes.data()), _response.bytes.size());
}

size_t CurlHelper::responseRaw(const uint8_t **body)
{
    assert(body);

    *body = _response.bytes.data();
    return _response.bytes.size();
}

std::string CurlHelper::responseHeaders()
{
    return std::string(reinterpret_cast<const char *>(_response.headers.data()), _response.headers.size());
}

size_t CurlHelper::readDataCallback(void *ptr, size_t size, size_t nmemb, curlData_t *data)
{
    assert(ptr && data);

    size_t nread = std::min(data->bytes.size() - data->rdOffset, size * nmemb);
    if (!nread)
        return 0;

    std::memcpy(ptr, &data->bytes.data()[data->rdOffset], nread);
    data->rdOffset += nread;
    return nread;
}

size_t CurlHelper::writeDataCallback(void *ptr, size_t size, size_t nmemb, curlData_t *data)
{
    assert(ptr && data);

    uint8_t const *const src = reinterpret_cast<const uint8_t *>(ptr);
    data->bytes.insert(data->bytes.cend(), src, src + size * nmemb);

    return size * nmemb;
}

size_t CurlHelper::headerDataCallback(char *ptr, size_t size, size_t nmemb, curlData_t *data)
{
    uint8_t const *const src = reinterpret_cast<const uint8_t *>(ptr);
    assert(src && data);

    data->headers.insert(data->headers.cend(), src, src + size * nmemb);

    return size * nmemb;
}

bool CurlHelper::parseResponseHeaders()
{
    bool rc = false;

    if (_response.headers.size() > 0)
    {
        std::stringstream data_stream(std::string(_response.headers.begin(), _response.headers.end()));
        std::string line;
        while (std::getline(data_stream, line))
        {
            // trim at ending
            line.erase(line.find_last_not_of('\r') + 1);

            std::size_t p = line.find(':');
            if (p != std::string::npos)
            {
                std::string n(line.substr(0, p));
                std::string v(line.substr(p + 2)); // skip colon and space

                _response.headersList.emplace(n, v);

                rc = true;
            }
        }
    }

    return rc;
}

bool CurlHelper::responseHasHeader(const std::string &name) const
{
    return _response.headersList.find(name) != _response.headersList.end();
}

std::string CurlHelper::responseHeader(const std::string &name) const
{
    auto it = _response.headersList.find(name);

    // If element was found
    if (it != _response.headersList.end())
    {
        return it->second;
    }
    else
    {
        return std::string();
    }
}
