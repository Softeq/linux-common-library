#ifndef SOFTEQ_COMMON_STDUTILS_H_
#define SOFTEQ_COMMON_STDUTILS_H_

#include <string>
#include <vector>

/*!
  \brief List of functions for manipulate with string.
  since it is some kind of extension of std namespace it violates naming guideline but aligns with std's one
*/
namespace softeq
{
namespace common
{
namespace stdutils
{
/*!
   Formating string to output
*/
std::string string_format(const char *fmtStr, ...);
/*!
  Split string
  \param[in] s String for split
  \param[in] delim Delimiter
  \return String vector
*/
std::vector<std::string> string_split(const std::string &s, char delim);

/*!
  Transforming C++ ABI identifiers (like RTTI symbols) into the original C++ source identifiers
  \param[in] name mangled type representation. How type_info::name returns
  \return Demangled string
*/
std::string demangle(const char *name);

} // namespace stdutils
} // namespace common
} // namespace softeq

#endif // SOFTEQ_COMMON_STDUTILS_H_
