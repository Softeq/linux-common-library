#include "softeq/common/fsutils.hh"

#include <sys/stat.h>
#include <sys/types.h>
#include <sys/sendfile.h>
#include <sys/statvfs.h>

#include <dirent.h>
#include <fcntl.h>
#include <fnmatch.h>
#include <string.h>
#include <unistd.h>

#include <cerrno>

#include <algorithm>
#include <fstream>
#include <stack>

#include "softeq/common/log.hh"
#include "softeq/common/scope_guard.hh"
#include "softeq/common/stdutils.hh"

namespace
{
const char *const LOG_DOMAIN = "FSUtil";
}

namespace softeq
{
namespace common
{
namespace fsutils
{

Path::Path(const std::string &value)
    : _value(value)

{
    updateInternals();
}

bool Path::exist() const
{
    return fsutils::exist(_value);
}

std::string Path::basename() const
{
    return fsutils::basename(_value);
}

std::string Path::extension() const
{
    return fsutils::extension(_value);
}

Path Path::parent() const
{
    return Path(fsutils::parent(_value));
}

Permissions Path::permissions() const
{
    return fsutils::getPermissions(_value);
}

bool Path::hasParent() const
{
    return fsutils::hasParent(_value);
}

void Path::updateInternals()
{
    struct stat sb;
    if (::stat(_value.c_str(), &sb) == 0)
    {
        _aTime = sb.st_atime;
        _mTime = sb.st_mtime;
        switch (sb.st_mode & S_IFMT)
        {
        case S_IFREG:
            _type = Type::REGULAR;
            break;
        case S_IFLNK:
            _type = Type::SYMLINK;
            break;
        case S_IFDIR:
            _type = Type::DIRECTORY;
            if (_value[_value.length() - 1] != '/')
            {
                _value.append("/");
            }
            break;
        default:
            _type = Type::UNKNOWN;
            break;
        }
    }
}

Path &Path::operator+=(const std::string &other)
{
    if (!other.empty())
    {
        if (other[0] == '/')
        {
            _value.append(other.substr(1));
        }
        else
        {
            _value.append(other);
        }
        updateInternals();
    }
    return *this;
}

Path operator+(const Path &lhs, const std::string &rhs)
{
    Path result_value(lhs);
    result_value += rhs;
    return result_value;
}

std::string basename(const std::string &filename)
{
    if (filename == "/")
    {
        return filename;
    }
    std::string f = filename;
    if (!f.empty() && f[f.size() - 1] == '/')
    {
        f.erase(f.size() - 1);
    }
    std::string::size_type pos = f.find_last_of('/');
    if (pos != std::string::npos)
    {
        return f.substr(pos + 1);
    }
    return f;
}

std::string extension(const std::string &filename)
{
    if (!isdir(filename))
    {
        std::string f = basename(filename);
        std::string::size_type pos = f.find_last_of('.');
        if (pos != std::string::npos)
        {
            return f.substr(pos + 1);
        }
    }
    return std::string();
}

std::string dirname(const std::string &filename)
{
    if (filename == "/")
    {
        return filename;
    }
    std::string f = filename;
    if (!f.empty() && f[f.size() - 1] == '/')
    {
        f.erase(f.size() - 1);
    }
    std::string::size_type pos = f.find_last_of('/');
    if (pos != std::string::npos)
    {
        if (pos != 0)
        {
            return f.substr(0, pos);
        }
        else
        {
            return "/";
        }
    }
    return std::string();
}

std::string parent(const std::string &filename)
{
    if (filename == "/")
    {
        return filename;
    }
    std::string f = filename;

    std::string::size_type pos = f.find_last_of('/');
    if (pos != std::string::npos)
    {
        if (pos != 0)
        {
            return f.substr(0, pos);
        }
        else
        {
            return "/";
        }
    }
    return std::string();
}

bool hasParent(const std::string &filename)
{
    std::string parentDir = parent(filename);

    return !(parentDir == "" || parentDir == "/");
}

Permissions getPermissions(const std::string &path)
{
    struct stat statbuf;
    const int res = stat(path.c_str(), &statbuf);

    if (res == -1)
    {
        throw std::system_error(errno, std::system_category(), path);
    }

    // clear bits responsible for file type
    return static_cast<Permissions>(statbuf.st_mode & ~S_IFMT);
}

bool mkdirs(const std::string &path, Permissions permissions)
{
    typedef std::stack<std::string> PathStack;
    PathStack stack;
    std::string p = path;
    while (!p.empty())
    {
        struct stat buf;
        const int res = stat(p.c_str(), &buf);
        if (res == -1)
        {
            if (errno != ENOENT)
            {
                return false;
            }
        }
        else
        {
            if ((buf.st_mode & S_IFMT) != S_IFDIR)
            {
                return false;
            }
            else
            {
                break;
            }
        }
        stack.push(p);
        p = dirname(p);
    }
    while (!stack.empty())
    {
        int res = mkdir(stack.top().c_str(), static_cast<uint32_t>(permissions));
        if (res == -1)
        {
            LOGE(LOG_DOMAIN, "mkdirs %s has failed", stack.top().c_str());
            return false;
        }
        stack.pop();
    }
    return true;
}

bool exist(const std::string &path)
{
    return ::access(path.c_str(), F_OK) == 0;
}

std::string workingDir()
{
	std::array<char, PATH_MAX> buffer;
    const char *path = ::getcwd(buffer.data(), buffer.size());
    std::string workingDir(path ? path : "");
    return workingDir;
}

int64_t filesize(const std::string &path)
{
    struct stat sb;
    const int res = ::stat(path.c_str(), &sb);
    if (res == -1)
    {
        LOGE(LOG_DOMAIN, "filesize %s has failed", path.c_str());
        return 0;
    }
    return sb.st_size;
}

time_t fileAtime(const std::string &path)
{
    struct stat sb;
    return (::stat(path.c_str(), &sb) == 0) ? sb.st_atime : 0;
}
time_t fileMtime(const std::string &path)
{
    struct stat sb;
    return (::stat(path.c_str(), &sb) == 0) ? sb.st_mtime : 0;
}

bool isdir(const std::string &path)
{
    struct stat sb;
    return (::stat(path.c_str(), &sb) == 0) && S_ISDIR(sb.st_mode);
}

bool remove(const std::string &path)
{
    if (::remove(path.c_str()) != 0)
    {
        LOGE(LOG_DOMAIN, "remove of %s has failed.", path.c_str());
        return false;
    }
    return true;
}

bool rename(const std::string &oldname, const std::string &newname)
{
    if (::rename(oldname.c_str(), newname.c_str()) != 0)
    {
        LOGE(LOG_DOMAIN, "rename %s to %s has failed.", oldname.c_str(), newname.c_str());
        return false;
    }
    return true;
}

bool removeNonemptyDirectory(const std::string &path_to_remove)
{
    std::list<std::string> dir_entries(dirContent(path_to_remove));
    if (!dir_entries.empty())
    {
        for (const std::string &entry : dir_entries)
        {
            if (entry == "." || entry == "..")
            {
                continue;
            }

            Path p(path_to_remove);
            p += entry;
            bool result = isdir(p) ? removeNonemptyDirectory(p) : remove(p);
            if (!result)
            {
                return false;
            }
        }
    }
    return remove(path_to_remove);
}

bool copy(const std::string &srcpath, const std::string &dstpath)
{
    const std::string dname(dirname(dstpath));
    if (dname.empty())
    {
        return false;
    }

    struct stat sb = {0};
    if ((::stat(dname.c_str(), &sb) == ENOENT) && (!mkdirs(dname)))
    {
        return false;
    }

    int src_fd = ::open(srcpath.c_str(), O_RDWR);
    if (src_fd > 0)
    {
        if (::lockf(src_fd, F_LOCK, 0) < 0)
        {
            LOGE(LOG_DOMAIN, " fs::copy() failed. ");
            ::close(src_fd);
            return false;
        }

        if (::fstat(src_fd, &sb) < 0)
        {
            LOGE(LOG_DOMAIN, " fs::copy() failed. ");
            ::lockf(src_fd, F_ULOCK, 0);
            ::close(src_fd);
            return false;
        }

        off_t src_size = sb.st_size;
        bool success = true;
        int dst_fd = ::open(dstpath.c_str(), O_WRONLY | O_CREAT, sb.st_mode);
        if (dst_fd < 0)
        {
            LOGE(LOG_DOMAIN, " fs::copy() failed. ");
            success = false;
        }

        if (success)
        {
            ssize_t send_cnt = ::sendfile(dst_fd, src_fd, NULL, sb.st_size);
            if (send_cnt < 0 || send_cnt != sb.st_size)
            {
                success = false;
            }
            /* details about format specifiers see here:
             http://pubs.opengroup.org/onlinepubs/009695399/functions/fprintf.html
             */
            LOGT(LOG_DOMAIN, "copying %s (%jd bytes) --> %s (%zd bytes)", srcpath.c_str(),
                 static_cast<intmax_t>(sb.st_size), dstpath.c_str(), send_cnt);

            if (::close(dst_fd) != 0)
            {
                LOGE(LOG_DOMAIN, " fs::copy() failed. ");
                success = false;
            }
        }

        if (::lockf(src_fd, F_ULOCK, 0) != 0)
        {
            LOGE(LOG_DOMAIN, " fs::copy() failed. ");
            success = false;
        }

        if (::close(src_fd) != 0)
        {
            LOGE(LOG_DOMAIN, " fs::copy() failed. ");
            success = false;
        }

        if (success)
        {
            if (::stat(dstpath.c_str(), &sb) != 0)
            {
                LOGE(LOG_DOMAIN, " fs::copy() failed. ");
                return false;
            }
            success = sb.st_size == src_size;
        }

        return success;
    }

    return false;
}

bool symlink(const std::string &srcpath, const std::string &dstpath)
{
    return ::symlink(srcpath.c_str(), dstpath.c_str()) == 0;
}

std::list<std::string> dirContent(const std::string &base_path, const std::string &filter /*="*"*/)
{
    std::list<std::string> result;
    scope_guard guard;

    DIR *dir = ::opendir(base_path.c_str());
    if (dir != nullptr)
    {
        guard += [&dir] {::closedir(dir);};

        struct dirent *entry = nullptr;
        while ((entry = ::readdir(dir)) != nullptr)
        {
            const char *fpath = entry->d_name;
            if (::fnmatch(filter.c_str(), fpath, FNM_CASEFOLD) == 0)
            {
                result.emplace_back(fpath);
            }
        }
    }
    return result;
}

bool isValidFilename(const std::string &value)
{
    if (value.empty())
    {
        return false;
    }
    for (std::size_t i = 0; i < value.length(); i++)
    {
        if (!std::isalnum(value[i]) && !(value[i] == '_' || value[i] == '-' || value[i] == ',' || value[i] == '.'))
        {
            return false;
        }
    }
    return true;
}

bool readBinaryFileIntoBuffer(const std::string &filepath, std::unique_ptr<char[]> &buffer) noexcept
{
    try
    {
        std::ifstream in_file(filepath.c_str(), std::ios::binary);
        if (!in_file)
        {
            throw std::runtime_error("Couldn't open file.");
        }

        uint64_t size = filesize(filepath);
        std::unique_ptr<char[]> buff(new char[size]);

        // assume, that std::streamsize is 64bit length
        in_file.read(buff.get(), size);

        buffer = std::move(buff);
        return true;
    }
    catch (const std::exception &ex)
    {
        LOGE(LOG_DOMAIN, "Exception when read binary file '%s' into buffer.\nReason: %s", filepath.c_str(), ex.what());
        return false;
    }
}

int64_t fsAvail(const std::string &path)
{
    struct statvfs s;
    if (statvfs(path.c_str(), &s) != 0)
    {
        LOGE(LOG_DOMAIN, "fsAvail %s has failed", path.c_str());
        return -1;
    }

    return int64_t(s.f_frsize) * s.f_bavail;
}

Permissions operator|(Permissions lhs, Permissions rhs)
{
    return static_cast<Permissions>(static_cast<uint32_t>(lhs) | static_cast<uint32_t>(rhs));
}

} // namespace fsutils
} // namespace common
} // namespace softeq
