#ifndef SOFTEQ_COMMON_EXCEPTIONS_H
#define SOFTEQ_COMMON_EXCEPTIONS_H

#include <stdexcept>
#include <string>

namespace softeq
{
namespace common
{
/*!
  \brief The class SystemError describes the common type of exception. 
  
  The error message contains the error description of the error code or 
  the error code itself in case the description cannot be obtained. 
*/
class SystemError : public std::runtime_error
{
public:
    explicit SystemError()
        : std::runtime_error(getErrorMessage()){};

private:
    std::string getErrorMessage() const;
};

} // namespace common
} // namespace softeq

#endif // SOFTEQ_COMMON_EXCEPTIONS_H