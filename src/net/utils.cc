#include "utils.hh"

int MHD_getParamsIter(void *cls, enum MHD_ValueKind kind, const char *key, const char *value)
{
    // DO TO: does the kind parameter make any sence here?
    (void)kind;
    ParamsMap &map = *static_cast<ParamsMap *>(cls);
    if (value)
    {
        map[key] = std::string(value);
    }
    else
    {
        map[key] = ParamsMap::mapped_type();
    }
    return MHD_YES; // continue iteration)
}

// TODO move somewhere
bool ParseRange(const std::string &str, Range &range) noexcept
{
    range = Range();
    try
    {
        std::size_t p = str.find('=');
        if (p != std::string::npos)
        {
            std::string s(str.substr(p + 1));
            if ((p = s.find('-')) != std::string::npos)
            {
                range._pos_begin = std::stol(s.substr(0, p));
                if (p + 1 < s.length())
                    range._pos_end = std::stol(s.substr(p + 1));
            }
        }
        else
        {
            throw std::invalid_argument("Range string without 'Range' prefix");
        }
    }
    catch (std::exception &ex)
    {
        LOGE(LOG_DOMAIN, "Parsing of range string '%s' failed.\nReason: %s", str.c_str(), ex.what());
        return false;
    }
    return true;
};
