#include "stdutils.hh"

#include <cstring>
#include <cstdarg>
#include <sstream>
#include <memory>

namespace softeq
{
namespace common
{
namespace stdutils
{

std::string string_format(const char *fmtStr, ...)
{
    std::size_t size = (std::strlen(fmtStr)) * 2 + 50; // just a guess
    std::string str;
    std::va_list args;

    while (1)
    { // Maximum two passes on a POSIX system...
        str.resize(size);
        va_start(args, fmtStr);
        int n = vsnprintf((char *)str.data(), size, fmtStr, args);
        va_end(args);
        if (n > -1 && (static_cast<std::size_t>(n) < size))
        { // Everything worked
            str.resize(n);
            return str;
        }
        if (n > -1)       // Needed size returned
            size = n + 1; // For null char
        else
            size *= 2; // Guess at a larger size (OS specific)
    }
}

std::vector<std::string> string_split(const std::string &s, char delim)
{
    std::vector<std::string> elems;
    std::stringstream ss(s);
    std::string item;
    while (getline(ss, item, delim))
    {
        elems.push_back(item);
    }
    return elems;
}

#ifdef __GNUG__
#include <cstdlib>
#include <cxxabi.h>

std::string demangle(const char *name)
{
    int status = 0;

    std::unique_ptr<char, void (*)(void *)> res{abi::__cxa_demangle(name, NULL, NULL, &status), std::free};

    return (status == 0) ? res.get() : name;
}

#else

// does nothing if not g++
std::string demangle(const char *name)
{
    return name;
}

#endif

} // namespace stdutils
} // namespace common
} // namespace softeq
