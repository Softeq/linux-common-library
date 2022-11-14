#ifndef SOFTEQ_COMMON_FSUTILS_H_
#define SOFTEQ_COMMON_FSUTILS_H_

#include <ctime>
#include <cstdint>
#include <list>
#include <memory>
#include <string>

namespace softeq
{
namespace common
{
namespace system
{
/**< This type represents file access permissions */
enum class Permissions : uint32_t
{
    None = 0,
    OwnerRead = 0400,
    OwnerWrite = 0200,
    OwnerExec = 0100,
    OwnerAll = 0700,
    GroupRead = 040,
    GroupWrite = 020,
    GroupExec = 010,
    GroupAll = 070,
    OthersRead = 04,
    OthersWrite = 02,
    OthersExec = 01,
    OthersAll = 07,
    All = 0777,
    SetUid = 04000,
    SetGid = 02000,
    StickyBit = 01000,
    Mask = 07777,
    Unknown = 0xFFFF,
    AddPerms = 0x10000,
    RemovePerms = 0x20000,
    SymlinkNofollow = 0x40000
};

/*!
 * \brief Binary operator OR for file persissions type
 * \return Result of binary OR on corresponding pairs of lhs and rhs permissions
 * \param[in] lhs Permissions on the left-hand side of operator
 * \param[in] rhs Permissions on the right-hand side of operator
 */
Permissions operator|(Permissions lhs, Permissions rhs);

/*!
  \brief List of functions for manipulate with "file name", "file path", "full path to file".
*/
class Path
{
public:
    /**< details of file type */
    enum class Type
    {
        NONE = 0,
        REGULAR = 1,
        DIRECTORY = 2,
        SYMLINK = 3,
        UNKNOWN = 8
    };

private:
    std::time_t _aTime = 0; ///< last time, when entry was accessed (the same as st_atime in struct stat)
    std::time_t _mTime = 0; ///< last time, when entry was modified (the same as st_mtime in struct stat)
    std::string _value;
    Type _type = Type::NONE;
    void updateInternals();

public:
    Path() = default;
    explicit Path(const std::string &value);
    std::time_t aTime() const
    {
        return _aTime;
    }
    std::time_t mTime() const
    {
        return _mTime;
    }
    Type type() const
    {
        return _type;
    }
    operator std::string() const
    {
        return _value;
    }
    bool exist() const;
    std::string basename() const;
    std::string extension() const;
    Path parent() const;
    bool hasParent() const;
    Permissions permissions() const;

    /*!
      \brief Smart append to path
      The method appends path and fixes extra or missing slash, i.e. there will be only one slash
      between existed and appended parts, for example:
        1. /var/ + log = /var/log
        2. /var/ + /log = /var/log
        3. /var + log = /var/log
        3. /var + ///log = /var/log
        4. / + / = /
      If you do not need this functionality, use operators + and += which act as a simple string operators.

      \param[in] other - path to append
    */
    void append(const std::string &other);

    /*!
      \brief Simple operator += which acts the same way as string's operator
      \see Path::append() if you need to fix extra or missing slashes between parts
    */
    Path &operator+=(const std::string &other);
    /*!
      \brief Simple operator + which acts the same way as string's operator
      \see Path::append() if you need to fix extra or missing slashes between parts
    */
    friend Path operator+(const Path &lhs, const std::string &rhs);
};

/*!
  The function tries to get filename from "full path"
  \return Filename (without filepath)
  \param[in] filename - full path to file
*/
std::string basename(const std::string &filename);

/*!
  The function tries to get full path to file
  \return Full path to file as string
  \param[in] filename - full path to file
*/
std::string dirname(const std::string &filename);

/*!
  The function tries to get extension of the file
  \return File extension as string
  \param[in] filename - name of file for analyze
*/
std::string extension(const std::string &filename);

/*!
  The function checks if file provided has parent directory
  \return true if parent path is exist, false otherwise
  \param[in] filename - full path to file
*/
bool hasParent(const std::string &filename);

/*!
  The function tries to get parent path of the file
  \return Full path to file as string
  \param[in] filename - full path to file
*/
std::string parent(const std::string &filename);

/*!
   The function tries to create directory like "mkdir -p"
   \return Operation status: TRUE - success, FALSE - if any error during create directory
   \param[in] path - full path with dir name
*/
bool mkdirs(const std::string &path, Permissions permissions = Permissions::OwnerAll | Permissions::GroupAll);

/*!
   The function returns access permissions of the file or directory
   \return permissions - permissions of given file or directory
   \param[in] path - full path to file or directory
   \throw std::runtime_error in case of stat() failure
*/
Permissions getPermissions(const std::string &path);

bool isdir(const std::string &path);

/*!
   The function tries to check path to directory, if exists
   \return Operation status: TRUE - if dir path exists, FALSE - if doesn't
   \param[in] path - full directory path
*/
bool exist(const std::string &path);

std::string workingDir();

int64_t filesize(const std::string &path);

time_t fileAtime(const std::string &path);
time_t fileMtime(const std::string &path);

/*!
   The function tries to remove file from directory
   \return Operation status: TRUE if file was deleted, FALSE - if any errors
   \param[in] path - full path to file
*/
bool remove(const std::string &path);

/*!
   The function tries to change name or location of the file
   \return Operation status: TRUE if file was deleted, FALSE - if any errors
   \param[in] oldname - initial name of the file
   \param[in] newname - required name of the file
*/
bool rename(const std::string &oldname, const std::string &newname);

/*!
   The function deletes specified directory, even it isn't empty. In such case function
   traverses directory in deep (recursively) and deletes each entry, which it encounters.
   \return Operation status: TRUE - directory was removed with all child items, FALSE - if any errors
   \param[in] path_to_remove - path of the directory to remove
 */
bool removeNonemptyDirectory(const std::string &path_to_remove);

/*!
   The function tries to copy file between two directories
   \return Operation status: TRUE - file was copied, FALSE - if any errors
   \param[in] srcpath - full path to source file
   \param[in] dstpath - full path to destination file
*/
bool copy(const std::string &srcpath, const std::string &dstpath);

/*!
   The function tries to get filename from "full path"
   \return Operation status: TRUE - if symLink was created without errors, FALSE - if any errors
   \param[in] srcpath - full path to source file
   \param[in] dstpath - name of destination symlink (full path with filename)
*/
bool symlink(const std::string &srcpath, const std::string &dstpath);

/*!
   The function tries to scan directory content with applied filename's filter.
   The function scans directory lineary, i.e. it doesn't go to nested levels.
   \return List of filenames
   \param[in] path - full path to directory (will be scan...)
   \param[in] filter - filter string to separate filenames
*/
std::list<std::string> dirContent(const std::string &path, const std::string &filter = "*");

bool isValidFilename(const std::string &value);

/*!
   The function gets filesize, allocates memory enough to read it and reads file content into buffer.
   \return Operation status: TRUE - content of the file was read successfully into buffer, FALSE - if any error.
   \param[in] filepath - full path to file to read
   \param[out] buffer - target buffer to read file
*/
bool readBinaryFileIntoBuffer(const std::string &filepath, std::unique_ptr<char[]> &buffer) noexcept;

/*!
   The function returns number of bytes on the file system available to non-privileged process.
   \return Number of bytes or -1 if any error.
   \param[in] path - any file path on the file system
*/
int64_t fsAvail(const std::string &path);

} // namespace system
} // namespace common
} // namespace softeq

#endif // SOFTEQ_COMMON_FSUTILS_H_
