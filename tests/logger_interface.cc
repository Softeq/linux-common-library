#include <gtest/gtest.h>

#include "softeq/common/logger_interface.hh"

using namespace softeq::common;

TEST(LoggerInterface, LogLevelFromString)
{
    LogLevel result;
    
    ASSERT_NO_THROW(result = logLevelFromString("none"));
    ASSERT_EQ(LogLevel::NONE, result);    
    ASSERT_NO_THROW(result = logLevelFromString("fatal"));
    ASSERT_EQ(LogLevel::FATAL, result);    
    ASSERT_NO_THROW(result = logLevelFromString("critical"));
    ASSERT_EQ(LogLevel::CRITICAL, result);    
    ASSERT_NO_THROW(result = logLevelFromString("error"));
    ASSERT_EQ(LogLevel::ERROR, result);   
    ASSERT_NO_THROW(result = logLevelFromString("warning"));
    ASSERT_EQ(LogLevel::WARNING, result);    
    ASSERT_NO_THROW(result = logLevelFromString("info"));
    ASSERT_EQ(LogLevel::INFO, result);    
    ASSERT_NO_THROW(result = logLevelFromString("debug"));
    ASSERT_EQ(LogLevel::DEBUG, result);    
    ASSERT_NO_THROW(result = logLevelFromString("trace"));
    ASSERT_EQ(LogLevel::TRACE, result);  
    
    ASSERT_NO_THROW(result = logLevelFromString("0"));
    ASSERT_EQ(LogLevel::NONE, result);    
    ASSERT_NO_THROW(result = logLevelFromString("1"));
    ASSERT_EQ(LogLevel::FATAL, result);   
    ASSERT_NO_THROW(result = logLevelFromString("2"));
    ASSERT_EQ(LogLevel::CRITICAL, result);   
    ASSERT_NO_THROW(result = logLevelFromString("3"));
    ASSERT_EQ(LogLevel::ERROR, result);    
    ASSERT_NO_THROW(result = logLevelFromString("4"));
    ASSERT_EQ(LogLevel::WARNING, result);    
    ASSERT_NO_THROW(result = logLevelFromString("5"));
    ASSERT_EQ(LogLevel::INFO, result);   
    ASSERT_NO_THROW(result = logLevelFromString("6"));
    ASSERT_EQ(LogLevel::DEBUG, result);   
    ASSERT_NO_THROW(result = logLevelFromString("7"));
    ASSERT_EQ(LogLevel::TRACE, result);    
    
    ASSERT_THROW(logLevelFromString("NONE"), std::logic_error); 
    ASSERT_THROW(logLevelFromString("-1"), std::logic_error); 
    ASSERT_THROW(logLevelFromString(" "), std::logic_error); 
}

TEST(LoggerInterface, LogLevelToChar)
{
    ASSERT_TRUE(logLevelToChar(LogLevel::NONE) == '\0');
    ASSERT_TRUE(logLevelToChar(LogLevel::FATAL) == 'F');
    ASSERT_TRUE(logLevelToChar(LogLevel::CRITICAL) == 'C');
    ASSERT_TRUE(logLevelToChar(LogLevel::ERROR) == 'E');
    ASSERT_TRUE(logLevelToChar(LogLevel::WARNING) == 'W');
    ASSERT_TRUE(logLevelToChar(LogLevel::INFO) == 'I');
    ASSERT_TRUE(logLevelToChar(LogLevel::DEBUG) == 'D');
    ASSERT_TRUE(logLevelToChar(LogLevel::TRACE) == 'T');
    
    ASSERT_THROW(logLevelToChar(static_cast<LogLevel>(-1)), std::out_of_range);
}

