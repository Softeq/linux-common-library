#include <gtest/gtest.h>
#include <string.h>

#include "softeq/common/exceptions.hh"

using namespace softeq::common;

TEST(Exceptions, ZeroErrno) {
    errno = 0;
    EXPECT_TRUE(strcmp(SystemError().what(), "Success") == 0);
}

TEST(Exceptions, UnknownError) {
    errno = 2931;
    std::string errMsg("Unknown error ");
    errMsg += std::to_string(errno);
    EXPECT_TRUE(strcmp(SystemError().what(), errMsg.c_str()) == 0);
}

TEST(Exceptions, PermissionDenied) {
    errno = EACCES;
    EXPECT_TRUE(strcmp(SystemError().what(), "Permission denied") == 0);
}

TEST(Exceptions, NoSuchFile) {
    errno = ENOENT;
    EXPECT_TRUE(strcmp(SystemError().what(), "No such file or directory") == 0);
}