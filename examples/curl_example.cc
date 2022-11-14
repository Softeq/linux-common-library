#include <common/net/curl_helper/curl_helper.hh>

#include <sstream>

using namespace softeq::common::net::curl;

// TODO: add description for each example

int main()
{
    CurlHelper curl1("https://postman-echo.com/cookies/set?foo1=bar1&foo2=bar2");
    if (curl1.doGet())
    {
        CurlHelper curl2("https://postman-echo.com");
        CurlHelper::cookieMap_t cookies_map = curl1.cookies();
        curl2.setCookies(cookies_map);
        curl2.doPost();
    }
    return EXIT_SUCCESS;
}
