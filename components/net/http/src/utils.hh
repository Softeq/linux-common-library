#pragma once

#include <common/stdutils/optional.hh>
#include <common/logging/log.hh>

#include <microhttpd.h>

#include <stdexcept>

const char *const LOG_DOMAIN = "HttpServerMHD";

constexpr int cHttpDaemonAddressReuse = 1;
constexpr int cHttpDaemonConnectionsLimit = 50;
constexpr int cHttpDaemonConnectionsPerIPLimit = cHttpDaemonConnectionsLimit / 2;

using ParamsMap = std::map<std::string, softeq::common::stdutils::Optional<std::string>>;

int MHD_getParamsIter(void *cls, enum MHD_ValueKind kind, const char *key, const char *value);

// TODO move somewhere
struct Range final
{
    Range()
    {
    }
    Range(int pos_begin, int pos_end)
        : _pos_begin(pos_begin)
        , _pos_end(pos_end)
    {
    }

    int _pos_begin = 0;
    int _pos_end = 0;

    int length() const noexcept
    {
        return (_pos_end >= _pos_begin) ? (_pos_end - _pos_begin + 1) : 0;
    }
};

// TODO move somewhere
bool ParseRange(const std::string &str, Range &range) noexcept;

class FileReaderContext final
{
    FILE *_file = nullptr;
    int64_t _offset = 0;

public:
    FileReaderContext(FILE *file, int64_t offset)
        : _file(file)
        , _offset(offset)
    {
        if (!file)
            throw std::invalid_argument(std::string(__func__) + "(): file");
    }

    ~FileReaderContext()
    {
        ::fclose(_file);
    }

    FILE *file() const noexcept
    {
        return _file;
    }
    int64_t offset() const noexcept
    {
        return _offset;
    }
};
