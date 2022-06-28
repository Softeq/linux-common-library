#include "softeq/common/log.hh"
#include "softeq/common/system_logger.hh"

#include <chrono>
#include <ctime>
#include <fstream>
#include <gtest/gtest.h>
#include <iostream>
#include <syslog.h>
#include <thread>

using namespace softeq::common;

namespace
{

const char *temporaryFile = "TemporaryLoggerTest.txt";    /* name of temporary file for analyze */

const std::vector<LogLevel> cLogLevels =
{
    LogLevel::NONE,
    LogLevel::FATAL,
    LogLevel::CRITICAL,
    LogLevel::ERROR,
    LogLevel::WARNING,
    LogLevel::INFO,
    LogLevel::DEBUG,
    LogLevel::TRACE,
};

/* content blocks of the console logger, system logger */
const std::vector<std::string> cConsoleLogWithBadEnvVar =
{
    {"Logger filter error: stoi"},
    {"fix your SC_LOG_FILTER envvar"}
};

const std::vector<std::pair<std::string, std::string>> cConsoleLogWithEnvVar =
{
    {"[W]", "/Warning/ msg test with env SC_LOG_FILTER"},
    {"[E]", "/Error/ msg test with env SC_LOG_FILTER"},
};

const std::vector<std::pair<std::string, std::string>> cConsoleLog =
{
    {"[\000]", "Console msg test with level in []"},
    {"[F]", "Console msg test with level in []"},
    {"[C]", "Console msg test with level in []"},
    {"[E]", "Console msg test with level in []"},
    {"[W]", "Console msg test with level in []"},
    {"[I]", "Console msg test with level in []"},
    {"[D]", "Console msg test with level in []"},
    {"[T]", "Console msg test with level in []"},
};

const std::vector<std::pair<std::string, std::string>> cSysLog =
{
    {"unittest.syslogger", "debug log test"},
    {"unittest.syslogger", "info log test"},
    {"unittest.syslogger", "warning log test"},
    {"unittest.syslogger", "error log test"},
    {"unittest.syslogger", "critical log test"},
    {"unittest.syslogger", "fatal log test"},
};

const std::vector<std::pair<std::string, std::string>> cSysLogNoPrefix =
{
    {"common_tests", "debug log test without prefix"},
    {"common_tests", "info log test without prefix"},
    {"common_tests", "System error (Permission denied), "},
};

/* class for redirection messages */
class LogInterception final
{
public:

    /* Redirect filestream (stdout, stdin, stderr) to temporary file */
    void streamToFile(const char *modeopen, FILE *filestream)
    {
        std::fgetpos(filestream, &_pos);
        _fd = dup(fileno(filestream));
        std::freopen(temporaryFile, modeopen, filestream);
    }

    /* Redirect filestream (stdout, stdin, stderr) back to the terminal */
    void streamBackToTerminal()
    {
        std::fflush(stdout);
        dup2(_fd, fileno(stdout));
        close(_fd);
        std::clearerr(stdout);
        std::fsetpos(stdout, &_pos);
    }

    /* Remove temporary file with test terminal content */
    void removeTemporaryFile()
    {
        std::remove(temporaryFile);
    }

private:

    int _fd;        /* file descriptor */
    fpos_t _pos;    /* position in file */
};  /* class Interception */

class LogAnalysis final
{
public:

    /* check correctness of the log message */
    void consoleLogCheck(const std::vector<std::pair<std::string, std::string>> *testContent, const char *domain)
    {
        if (testContent == nullptr)
        {
            return;
        }

        std::string s;                      /* string for compare */
        std::ifstream file(temporaryFile);  /* file for compare */

        /* run and test the content of the log */
        /* check log time */
        for (std::pair<std::string, std::string> i : *testContent)
        {
            getline(file, s, ' ');
            /* check message time */
            EXPECT_GE(static_cast<unsigned long>(time(0)), static_cast<unsigned long>(std::stoi(s)));
            /* chek that thread id valid */
            getline(file, s, ' ');
            EXPECT_STREQ(convertThreadIdFormat(std::this_thread::get_id()).c_str(), s.c_str());
            /* check log level */
            getline(file, s, ' ');
            EXPECT_STREQ(s.c_str(), i.first.c_str());
            /* check context name */
            getline(file, s, ':');
            EXPECT_STREQ(s.c_str(), domain);
            /* check message */
            getline(file, s, ' ');
            getline(file, s);
            EXPECT_STREQ(s.c_str(), i.second.c_str());
        }
        file.close();
    }

    /* check correctness of the syslog message */
    void sysLogCheck(const char *domain)
    {
        std::string wholeString;            /* string for prepareing */
        std::string partString;             /* string for compare */
        std::ifstream file(temporaryFile);  /* file for compare */

        for (std::pair<std::string, std::string> i : cSysLog)
        {
            getline(file, wholeString);
            wholeString.erase(std::remove_if(wholeString.begin(),
                                             wholeString.end(),
                                             [](const char c) { return c == '[' || c == ']' || c == ':'; }),
                              wholeString.end());

            /* checking of prefix */
            EXPECT_STREQ(getPart(&wholeString, " ").c_str(), i.first.c_str());

            /* checking of pid */
            EXPECT_EQ(static_cast<unsigned long>(std::stoi(getPart(&wholeString, " ").c_str())),
                      static_cast<unsigned long>(getpid()));

            /* checking of time */
            EXPECT_GE(static_cast<unsigned long>(time(0)),
                      static_cast<unsigned long>(std::stoi(getPart(&wholeString, " ").c_str())));

            /* checking of message */
            EXPECT_STREQ(getPart(&wholeString, "\n").c_str(), i.second.c_str());
        }

        for (std::pair<std::string, std::string> i : cSysLogNoPrefix)
        {
            getline(file, wholeString);
            wholeString.replace(wholeString.find('[', 1), 1, " ");
            wholeString.erase(std::remove_if(wholeString.begin(), wholeString.end(),
                                             [](const char c) {return c == '[' || c == ']' || c == ':';}),
                              wholeString.end());

            /* checking of prefix */
            EXPECT_STREQ(getPart(&wholeString, " ").c_str(), i.first.c_str());

            /* checking of pid */
            EXPECT_EQ(static_cast<unsigned long>(std::stoi(getPart(&wholeString, " ").c_str())),
                      static_cast<unsigned long>(getpid()));

            /* checking of time */
            EXPECT_GE(static_cast<unsigned long>(time(0)),
                      static_cast<unsigned long>(std::stoi(getPart(&wholeString, " ").c_str())));

            /* checking of message */
            EXPECT_STREQ(getPart(&wholeString, "\n").c_str(), i.second.c_str());
        }
    }

private:

    /* get part of string from the whole string during syslog string parsing */
    std::string getPart(std::string *wholeString, const char *separator)
    {
        int position = 0;
        std::string partString;
        position = wholeString->find(separator);
        partString = wholeString->substr(0, position);
        wholeString->erase(wholeString->begin(), wholeString->begin() + position + 1);
        return partString;
    }

    /* make thread id in format, like syslog format */
    std::string convertThreadIdFormat(const std::thread::id &id)
    {
        std::stringstream stream;
        stream << "0x" << std::uppercase << std::setfill('0') << std::setw(4) << std::hex << id;
        return stream.str();
    }
};  /* class LogAnalysis */
}   /* namespace */

TEST(Logger, ConsoleLogWithBadFilter)
{
    /* set incorrect environment variable */
    setenv("SC_LOG_FILTER", "empty", true);

    std::stringstream file;
    std::streambuf *buffer = std::cerr.rdbuf(file.rdbuf());
    std::string s;                                      /* string for compare */
    EXPECT_NO_THROW(log().level(LogLevel::WARNING));    /* call log constructor with incorrect value of SC_LOG_FILTER */

    /* run and test the content of the log */
    for (std::string i : cConsoleLogWithBadEnvVar)
    {
        EXPECT_TRUE(getline(file, s));
        EXPECT_STREQ(i.c_str(), s.c_str());
    }

    /* no more lines in this case */
    EXPECT_FALSE(getline(file, s));
    std::cerr.rdbuf(buffer);

    /* remove incorrect environment variable */
    unsetenv("SC_LOG_FILTER");
}

TEST(Logger, ConsoleLogWithFilter)
{
    LogInterception interception;
    LogAnalysis logAnalysis;

    const char *domain = "console.logger with filter";

    interception.streamToFile("w", stdout);  /* write log to file */

    /* set environment variable */
    setenv("SC_LOG_FILTER", "warning", true);

    /* this string supposed to be written */
    EXPECT_NO_THROW(log().Message(LogLevel::WARNING, domain, "/Warning/ msg test with env SC_LOG_FILTER"));

    /*  this string supposed to be skipped */
    EXPECT_NO_THROW(log().Message(LogLevel::DEBUG, domain, "/Debug/ msg test with env SC_LOG_FILTER"));

    /* set environment variable */
    setenv("SC_LOG_FILTER", "SC_LOG_FILTER:error", true);

    /* this string supposed to be written */
    EXPECT_NO_THROW(log().Message(LogLevel::ERROR, domain, "/Error/ msg test with env SC_LOG_FILTER"));

    /*  this string supposed to be skipped */
    EXPECT_NO_THROW(log().Message(LogLevel::DEBUG, domain, "/Debug/ msg test with env SC_LOG_FILTER"));

    interception.streamBackToTerminal();    /* return logging to terminal */
    logAnalysis.consoleLogCheck(&cConsoleLogWithEnvVar, domain);
    interception.removeTemporaryFile();
    unsetenv("SC_LOG_FILTER");
}

TEST(Logger, ConsoleLog)
{
    LogInterception interception;
    LogAnalysis logAnalysis;

    const char *domain = "console.logger";

    interception.streamToFile("w", stdout);  /* write log to file */

    /* switching log level */
    for (uint8_t i = 0; i < cLogLevels.size(); i ++)
    {
        /* calling set level method, then get, then check with GTest */
        log().level(cLogLevels[i]);
        ASSERT_TRUE(log().level() == cLogLevels[i]);

        /* write console log message */
        ASSERT_NO_THROW(log().Message(cLogLevels[i], domain, cConsoleLog[i].second.c_str()));
    }

    /* log with domain = loglevel INFO */
    ASSERT_NO_THROW(log().Message(LogLevel::INFO, "info", "Console msg test with level in []"));

    /* try logging without domain */
    ASSERT_NO_THROW(log().Message(LogLevel::DEBUG, nullptr, "Test message without domain"));

    interception.streamBackToTerminal();    /* return logging to terminal */
    logAnalysis.consoleLogCheck(&cConsoleLog, domain);
    interception.removeTemporaryFile();
}

TEST(Logger, SysLog)
{
    LogInterception interception;
    LogAnalysis logAnalysis;

    const char *format = "%s: log test";
    std::string domain = std::to_string(time(0));
    std::string name("unittest.syslogger ");

    /* testing system logger with prefix and list of levels */
    ASSERT_NO_THROW(log().set(LoggerInterface::UPtr(new softeq::common::SystemLogger(name, LOG_PID | LOG_PERROR,
                                                                                                     LOG_USER))));
    interception.streamToFile("w", stderr);  /* write log to file */

    /* not logged, not proceed */
    LOGT(domain.c_str(), format, "trace");

    /* shall logged */
    LOGD(domain.c_str(), format, "debug");
    LOGI(domain.c_str(), format, "info");
    LOGW(domain.c_str(), format, "warning");
    LOGE(domain.c_str(), format, "error");
    LOGC(domain.c_str(), format, "critical");
    LOGF(domain.c_str(), format, "fatal");

    /* change format to test without prefix */
    format = "%s: log test without prefix";

    setenv("LOG_PREFIX", "unitTest.syslog.envTest", true);
    name = "";

    /* testing system logger without prefix */
    ASSERT_NO_THROW(log().set(LoggerInterface::UPtr(new softeq::common::SystemLogger(name, LOG_PID | LOG_PERROR,
                                                                                                     LOG_USER))));
    LOGD(domain.c_str(), format, "debug");

    setenv("LOG_PREFIX", "", true);
    ASSERT_NO_THROW(log().set(LoggerInterface::UPtr(new softeq::common::SystemLogger(name, LOG_PID | LOG_PERROR, 0))));
    LOGI(domain.c_str(), format, "info");
    unsetenv("LOG_PREFIX");

    /* testing system error method */
    errno = EACCES;
    LOGSYS(domain.c_str(), "");

    interception.streamBackToTerminal();    /* return logging to terminal */
    logAnalysis.sysLogCheck(domain.c_str());
    interception.removeTemporaryFile();
}
