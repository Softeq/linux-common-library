#include <gtest/gtest.h>

#include <common/system/fsutils.hh>

#include <fcntl.h>
#include <climits>
#include <cstdlib>
#include <cstdio>
#include <climits>
#include <unistd.h>

using namespace softeq::common::system;

namespace
{

static std::string getTempDir()
{
    return std::string(P_tmpdir);
}
} /* end namespace */

TEST(Path, Null)
{
    EXPECT_THROW(Path(nullptr), std::logic_error);
}

TEST(Path, Exist)
{
    Path usrPath("/usr");
    Path nullPath("");

    ASSERT_TRUE(usrPath.exist());
    ASSERT_FALSE(nullPath.exist());
}

TEST(Path, Types)
{
    // Try to create unknown type
    Path unknownPath("/dev/null");

    // Try to create Path for symlink
    const std::string testDir(getTempDir() + std::string("/test_link"));
    std::string cmd(std::string("mkdir -p ") + testDir);
    int rc = std::system(cmd.c_str());
    ASSERT_EQ(rc, 0);

    cmd = std::string("touch " + testDir + "/1");
    rc = std::system(cmd.c_str());
    ASSERT_EQ(rc, 0);
    cmd = std::string("ln -s " + testDir + "/1" + " " + testDir + "/2");
    rc = std::system(cmd.c_str());
    ASSERT_EQ(rc, 0);

    Path linkPath(testDir + "/2");
    ASSERT_TRUE(linkPath.exist());

    Path filePath(testDir + "/1");
    ASSERT_TRUE(linkPath.exist());

    cmd = std::string("rm -rf " + getTempDir() + std::string("/test_link"));
    rc = std::system(cmd.c_str());
    ASSERT_EQ(rc, 0);
}

TEST(Path, Basename)
{
    Path testPath("/");
    Path dir1Path("/dir");
    Path subdir1Path("/dir/subdir1");
    Path subdir2Path("/dir/subdir1/subdir2");
    Path subdir3Path("/dir/subdir1/subdir2/subdir3/");

    ASSERT_STREQ(testPath.basename().c_str(), "/");
    ASSERT_STREQ(dir1Path.basename().c_str(), "dir");
    ASSERT_STREQ(subdir1Path.basename().c_str(), "subdir1");
    ASSERT_STREQ(subdir2Path.basename().c_str(), "subdir2");
    ASSERT_STREQ(subdir3Path.basename().c_str(), "subdir3");
}

TEST(Path, Extension)
{
    Path filePath("file.ext");
    Path dir1Path("dir.");
    Path dir2Path("dir");

    ASSERT_STREQ(filePath.extension().c_str(), "ext");
    ASSERT_STREQ(dir1Path.extension().c_str(), "");
    ASSERT_STREQ(dir2Path.extension().c_str(), "");
}

TEST(Path, OperatorPlusEqual)
{
    Path dirPath("dir");

    dirPath += "";
    ASSERT_STREQ(((std::string)dirPath).c_str(), "dir");

    dirPath += "/add1";
    ASSERT_STREQ(((std::string)dirPath).c_str(), "dir/add1");

    dirPath += "add2";
    ASSERT_STREQ(((std::string)dirPath).c_str(), "dir/add1add2");

    dirPath += "/";
    ASSERT_STREQ(((std::string)dirPath).c_str(), "dir/add1add2/");
}

TEST(Path, Append)
{
    Path dirPath("");

    dirPath.append("");
    ASSERT_STREQ(((std::string)dirPath).c_str(), "");
    dirPath.append("dir");
    ASSERT_STREQ(((std::string)dirPath).c_str(), "dir");
    dirPath.append("");
    ASSERT_STREQ(((std::string)dirPath).c_str(), "dir");

    dirPath.append("/add1");
    ASSERT_STREQ(((std::string)dirPath).c_str(), "dir/add1");

    dirPath.append("add2");
    ASSERT_STREQ(((std::string)dirPath).c_str(), "dir/add1/add2");

    dirPath.append("/");
    ASSERT_STREQ(((std::string)dirPath).c_str(), "dir/add1/add2/");

    // Not a copy-paste: check that no double slash at the end of the path
    dirPath.append("/");
    ASSERT_STREQ(((std::string)dirPath).c_str(), "dir/add1/add2/");

    dirPath.append("///");
    ASSERT_STREQ(((std::string)dirPath).c_str(), "dir/add1/add2/");

    dirPath = Path("dir");
    dirPath.append("///");
    ASSERT_STREQ(((std::string)dirPath).c_str(), "dir/");

    dirPath.append("/add1/");
    ASSERT_STREQ(((std::string)dirPath).c_str(), "dir/add1/");

    dirPath = Path("/");
    dirPath.append("//");
    ASSERT_STREQ(((std::string)dirPath).c_str(), "/");

    dirPath = Path("//");
    dirPath.append("//");
    ASSERT_STREQ(((std::string)dirPath).c_str(), "/");

    dirPath = Path("");
    dirPath.append("/");
    ASSERT_STREQ(((std::string)dirPath).c_str(), "/");

    dirPath = Path("");
    dirPath.append("/dir");
    ASSERT_STREQ(((std::string)dirPath).c_str(), "/dir");

    dirPath.append("/dir1/dir2");
    ASSERT_STREQ(((std::string)dirPath).c_str(), "/dir/dir1/dir2");

    dirPath = Path("");
    dirPath.append("dir/dir");
    ASSERT_STREQ(((std::string)dirPath).c_str(), "dir/dir");
}

TEST(Path, OperatorPlus)
{
    Path lhPath("lh"), rhPath("rh");
    std::string resultString = lhPath + rhPath;

    Path resultPath = lhPath + rhPath;
    ASSERT_STREQ(((std::string)resultPath).c_str(), resultString.c_str());
}

TEST(Path, Parent)
{
    ASSERT_STREQ(((std::string)Path("").parent()).c_str(), "");
    ASSERT_STREQ(((std::string)Path("/").parent()).c_str(), "/");
    ASSERT_STREQ(((std::string)Path("//").parent()).c_str(), "/");

    ASSERT_STREQ(((std::string)Path("/var").parent()).c_str(), "/");
    ASSERT_STREQ(((std::string)Path("/var/").parent()).c_str(), "/var");
    ASSERT_STREQ(((std::string)Path("/var/log").parent()).c_str(), "/var");
    ASSERT_STREQ(((std::string)Path("/var/log/").parent()).c_str(), "/var/log");
}

TEST(Path, HasParent)
{
    ASSERT_TRUE(Path("").hasParent() == false);
    ASSERT_TRUE(Path("/").hasParent() == false);
    ASSERT_TRUE(Path("/var").hasParent() == false);

    ASSERT_TRUE(Path("/var/").hasParent() == true);
    ASSERT_TRUE(Path("/var/log").hasParent() == true);
    ASSERT_TRUE(Path("/var/log/").hasParent() == true);
}

TEST(Path, permissions)
{
    const std::string testDir(getTempDir() + std::string("/test_permissions"));
    EXPECT_TRUE(softeq::common::system::mkdirs(testDir, Permissions::OwnerAll) == true);

    EXPECT_TRUE(Path(testDir).permissions() == Permissions::OwnerAll);

    // remove test dir
    std::string cmd = std::string("rm -Rf ") + testDir;
    int rc = std::system(cmd.c_str());
    ASSERT_EQ(rc, 0);
}

TEST(FsUtils, Basename)
{
    ASSERT_TRUE(softeq::common::system::basename("/") == "/");
    ASSERT_TRUE(softeq::common::system::basename("/usr") == "usr");
    ASSERT_TRUE(softeq::common::system::basename("/usr/") == "usr");
    ASSERT_TRUE(softeq::common::system::basename("usr") == "usr");
    ASSERT_TRUE(softeq::common::system::basename("usr/") == "usr");
    ASSERT_TRUE(softeq::common::system::basename("/usr/folder") == "folder");
    ASSERT_TRUE(softeq::common::system::basename("/usr/folder/") == "folder");
    ASSERT_TRUE(softeq::common::system::basename("usr/folder") == "folder");
    ASSERT_TRUE(softeq::common::system::basename("usr/folder/") == "folder");
    ASSERT_TRUE(softeq::common::system::basename("") == "");
}

TEST(FsUtils, Extension)
{
    std::string dir = getTempDir();

    // create files for tests
    std::string filename_noext = dir + "/filename_noext";
    std::FILE *file_noext = std::fopen(filename_noext.c_str(), "a");
    ASSERT_NE(file_noext, nullptr);
    unlink(filename_noext.c_str());

    std::string filename_ext = dir + "/filename_with_ext.ext";
    std::FILE *file_ext = std::fopen(filename_ext.c_str(), "a");
    EXPECT_NE(file_ext, nullptr);
    unlink(filename_ext.c_str());

    EXPECT_TRUE(softeq::common::system::extension("/tmp") == "");
    EXPECT_TRUE(softeq::common::system::extension(filename_ext.c_str()) == "ext");
    EXPECT_TRUE(softeq::common::system::extension(filename_noext.c_str()) == "");

    std::fclose(file_ext);
    std::fclose(file_noext);
}

TEST(FsUtils, Dirname)
{
    ASSERT_TRUE(softeq::common::system::dirname("/") == "/");
    ASSERT_TRUE(softeq::common::system::dirname("/tmp") == "/");
    ASSERT_TRUE(softeq::common::system::dirname("/tmp/") == "/");
    ASSERT_TRUE(softeq::common::system::dirname("/tmp/folder") == "/tmp");
    ASSERT_TRUE(softeq::common::system::dirname("/tmp/folder/") == "/tmp");

    ASSERT_TRUE(softeq::common::system::dirname("") == "");
    ASSERT_TRUE(softeq::common::system::dirname("tmp/folder") == "tmp");
}

TEST(FsUtils, Parent)
{
    ASSERT_TRUE(softeq::common::system::parent("") == "");
    ASSERT_TRUE(softeq::common::system::parent("/") == "/");
    ASSERT_TRUE(softeq::common::system::parent("//") == "/");

    ASSERT_TRUE(softeq::common::system::parent("/var") == "/");
    ASSERT_TRUE(softeq::common::system::parent("/var/") == "/var");
    ASSERT_TRUE(softeq::common::system::parent("/var/log") == "/var");
    ASSERT_TRUE(softeq::common::system::parent("/var/log/") == "/var/log");
}

TEST(FsUtils, HasParent)
{
    ASSERT_TRUE(softeq::common::system::hasParent("") == false);
    ASSERT_TRUE(softeq::common::system::hasParent("/") == false);
    ASSERT_TRUE(softeq::common::system::hasParent("/var") == false);

    ASSERT_TRUE(softeq::common::system::hasParent("/var/") == true);
    ASSERT_TRUE(softeq::common::system::hasParent("/var/log") == true);
    ASSERT_TRUE(softeq::common::system::hasParent("/var/log/") == true);
}

TEST(FsUtils, Mkdirs)
{
    const std::string testDir(getTempDir() + std::string("/test_mkdirs"));
    std::string cmd(std::string("mkdir -p ") + testDir);
    int rc = std::system(cmd.c_str());
    ASSERT_EQ(rc, 0);

    EXPECT_TRUE(softeq::common::system::mkdirs("") == true);
    EXPECT_TRUE(softeq::common::system::mkdirs(testDir + "/created_dir") == true);
    // not copypaste. we expect true for already created dir
    EXPECT_TRUE(softeq::common::system::mkdirs(testDir + "/created_dir") == true);

    EXPECT_TRUE(softeq::common::system::mkdirs(testDir + "/created_dir/created_dir_2") == true);
    EXPECT_TRUE(softeq::common::system::mkdirs(testDir + "/created_dir/created_dir_2") == true);

    EXPECT_TRUE(softeq::common::system::mkdirs(testDir + "/dir1/dir2/dir3/dir4/dir5") == true);
    EXPECT_TRUE(softeq::common::system::mkdirs(testDir + "/dir1/dir2/dir3/dir4/dir5") == true);

    EXPECT_TRUE(softeq::common::system::mkdirs(testDir + "/dir1/dir2/dir3") == true);

    EXPECT_TRUE(softeq::common::system::mkdirs(testDir + "/dir1/dir2/dir3/") == true);
    EXPECT_TRUE(softeq::common::system::mkdirs(testDir + "/dir1/dir2/dir3/dir4/dir5/dir6/") == true);

    cmd = std::string("touch ") + testDir + "/test_file.txt";
    rc = std::system(cmd.c_str());
    ASSERT_EQ(rc, 0);
    EXPECT_TRUE(softeq::common::system::mkdirs(testDir + std::string("/test_file.txt")) == false);
    EXPECT_TRUE(softeq::common::system::mkdirs("/proc/dir1") == false);
    EXPECT_TRUE(softeq::common::system::mkdirs("/proc/dir1/dir2") == false);

    // remove test dir
    cmd = std::string("rm -Rf ") + testDir;
    rc = std::system(cmd.c_str());
    ASSERT_EQ(rc, 0);
}

TEST(FsUtils, Permissions)
{
    std::string testDir(getTempDir() + std::string("/test_perms"));
    std::string cmd(std::string("mkdir -p ") + testDir);
    int rc = std::system(cmd.c_str());
    ASSERT_EQ(rc, 0);

    Permissions perm;

    // set umask to 0 to allow to create dirs with any permissions
    int oldUmask = umask(0);

    // check various possible values
    EXPECT_TRUE(softeq::common::system::mkdirs(testDir + "/created_dir", Permissions::OwnerAll) == true);
    perm = softeq::common::system::getPermissions(testDir + "/created_dir");
    EXPECT_EQ(perm, (Permissions::OwnerWrite | Permissions::OwnerRead | Permissions::OwnerExec));
    EXPECT_EQ(perm, Permissions::OwnerAll);

    testDir += "/1";
    // OwnerWrite
    cmd = std::string("rm -Rf ") + testDir;
    EXPECT_TRUE(softeq::common::system::mkdirs(testDir, (Permissions::OwnerWrite)) == true);
    perm = softeq::common::system::getPermissions(testDir);
    EXPECT_EQ(perm, (Permissions::OwnerWrite));
    rc = std::system(cmd.c_str());
    ASSERT_EQ(rc, 0);

    // OwnerRead
    EXPECT_TRUE(softeq::common::system::mkdirs(testDir, (Permissions::OwnerRead)) == true);
    perm = softeq::common::system::getPermissions(testDir);
    EXPECT_EQ(perm, (Permissions::OwnerRead));
    rc = std::system(cmd.c_str());
    ASSERT_EQ(rc, 0);

    // None
    EXPECT_TRUE(softeq::common::system::mkdirs(testDir, (Permissions::None)) == true);
    perm = softeq::common::system::getPermissions(testDir);
    EXPECT_EQ(perm, (Permissions::None));
    rc = std::system(cmd.c_str());
    ASSERT_EQ(rc, 0);

    // GroupAll
    EXPECT_TRUE(softeq::common::system::mkdirs(testDir, (Permissions::GroupWrite | Permissions::GroupRead | Permissions::GroupExec)) == true);
    perm = softeq::common::system::getPermissions(testDir);
    EXPECT_EQ(perm, (Permissions::GroupWrite | Permissions::GroupRead | Permissions::GroupExec));
    EXPECT_EQ(perm, (Permissions::GroupAll));
    rc = std::system(cmd.c_str());
    ASSERT_EQ(rc, 0);

    // All
    EXPECT_TRUE(softeq::common::system::mkdirs(testDir, (Permissions::All)) == true);
    perm = softeq::common::system::getPermissions(testDir);
    EXPECT_EQ(perm, (Permissions::All));
    rc = std::system(cmd.c_str());
    ASSERT_EQ(rc, 0);

    // Sticky
    EXPECT_TRUE(softeq::common::system::mkdirs(testDir, (Permissions::StickyBit)) == true);
    perm = softeq::common::system::getPermissions(testDir);
    EXPECT_EQ(perm, (Permissions::StickyBit));
    rc = std::system(cmd.c_str());
    ASSERT_EQ(rc, 0);

    // recursive mkdirs and check all nested dirs
    EXPECT_TRUE(softeq::common::system::mkdirs(testDir + "/dir1/dir2/", Permissions::OwnerAll) == true);
    EXPECT_EQ(softeq::common::system::getPermissions(testDir + "/dir1"), Permissions::OwnerAll);
    EXPECT_EQ(softeq::common::system::getPermissions(testDir + "/dir1/dir2"), Permissions::OwnerAll);

    // test file permissions
    cmd = std::string("install -m 777 /dev/null ") + testDir + "/test_file.txt";
    rc = std::system(cmd.c_str());
    ASSERT_EQ(rc, 0);
    perm = softeq::common::system::getPermissions(testDir + "/test_file.txt");
    EXPECT_EQ(perm, Permissions::All);

    // negative (not exists)
    EXPECT_THROW({
        try
        {
            perm = softeq::common::system::getPermissions(testDir + "/dir-not-exists");
        }
        catch(const std::system_error &e)
        {
            EXPECT_EQ(e.code().value(), ENOENT);
            throw;
        }
    }, std::runtime_error);

    // remove test dir
    cmd = std::string("rm -Rf ") + testDir;
    rc = std::system(cmd.c_str());
    ASSERT_EQ(rc, 0);
    umask(oldUmask);
}

TEST(FsUtils, Exist)
{
    const std::string testDir = getTempDir();

    ASSERT_TRUE(softeq::common::system::exist(testDir) == true);
    ASSERT_TRUE(softeq::common::system::exist(testDir + "123456") == false);
}

TEST(FsUtils, WorkingDir)
{
    char *path = ::getcwd(NULL, 0);
    std::string workingDir(path ? path : "");
    if (path)
    {
        free(path);
    }

    ASSERT_TRUE(softeq::common::system::workingDir() == workingDir);
}

TEST(FsUtils, FileSize)
{
    std::string dir = getTempDir();

    // create file for tests
    std::string filename = dir + "/filesize.txt";
    std::FILE *file = std::fopen(filename.c_str(), "w");
    ASSERT_NE(file, nullptr);

    EXPECT_TRUE(softeq::common::system::filesize(filename) == 0);

    uint8_t buffer[10] = {0};
    EXPECT_TRUE(fwrite(buffer, sizeof(buffer), 1, file) == 1);
    fflush(file);

    EXPECT_TRUE(softeq::common::system::filesize(filename) == sizeof(buffer));

    EXPECT_TRUE(softeq::common::system::filesize(filename + "123456.txt") == 0);

    std::fclose(file);
}

TEST(FsUtils, FileAtime)
{
    std::string dir = getTempDir();

    // create file for tests
    std::string filename = dir + "/fileatime.txt";
    std::FILE *file = std::fopen(filename.c_str(), "w");
    ASSERT_NE(file, nullptr);

    struct stat sb;
    int retval = ::stat(filename.c_str(), &sb);
    EXPECT_TRUE(retval == 0);

    EXPECT_TRUE(softeq::common::system::fileAtime(filename) == sb.st_atime);
    EXPECT_TRUE(softeq::common::system::fileAtime(filename + "12345678.9") == 0);

    std::fclose(file);
}

TEST(FsUtils, FileMtime)
{
    std::string dir = getTempDir();

    // create file for tests
    std::string filename = dir + "/filemtime.txt";
    std::FILE *file = std::fopen(filename.c_str(), "w");
    ASSERT_NE(file, nullptr);

    struct stat sb;
    int retval = ::stat(filename.c_str(), &sb);
    EXPECT_TRUE(retval == 0);

    EXPECT_TRUE(softeq::common::system::fileMtime(filename) == sb.st_mtime);
    EXPECT_TRUE(softeq::common::system::fileMtime(filename + "12345678.9") == 0);

    std::fclose(file);
}

TEST(FsUtils, IsDir)
{
    std::string dir = getTempDir();
    ASSERT_TRUE(softeq::common::system::isdir(dir) == true);

    std::string filename = dir + "/isdir_test.txt";
    std::FILE *file = std::fopen(filename.c_str(), "w");
    ASSERT_NE(file, nullptr);

    EXPECT_TRUE(softeq::common::system::isdir(filename) == false);
    EXPECT_TRUE(softeq::common::system::isdir("") == false);

    std::fclose(file);
}

TEST(FsUtils, Remove)
{
    std::string dir = getTempDir();
    std::string filename = dir + "/remove_test.txt";
    std::FILE *file = std::fopen(filename.c_str(), "w");
    ASSERT_NE(file, nullptr);

    EXPECT_TRUE(softeq::common::system::remove(filename) == true);
    EXPECT_TRUE(softeq::common::system::remove(filename) == false);
    EXPECT_TRUE(softeq::common::system::remove("") == false);

    std::fclose(file);
}

TEST(FsUtils, Rename)
{
    std::string dir = getTempDir();
    std::string filename = dir + "/rename_test.txt";
    std::FILE *file = std::fopen(filename.c_str(), "w");
    ASSERT_NE(file, nullptr);

    EXPECT_TRUE(softeq::common::system::rename(filename, filename + "_new") == true);
    EXPECT_TRUE(softeq::common::system::rename(filename, filename + "_new") == false);
    EXPECT_TRUE(softeq::common::system::rename("", "_new") == false);

    std::fclose(file);
}

TEST(FsUtils, RemoveNonEmptyDirectory)
{
    std::string dir = getTempDir();
    const std::string testDir(getTempDir() + std::string("/nonempty1/nonempty2"));
    std::string cmd(std::string("mkdir -p ") + testDir);
    int rc = std::system(cmd.c_str());
    ASSERT_EQ(rc, 0);

    std::string filename = testDir + "/removenonemptydir_test.txt";
    std::FILE *file = std::fopen(filename.c_str(), "a");
    ASSERT_NE(file, nullptr);
    std::fclose(file);

    ASSERT_TRUE(softeq::common::system::removeNonemptyDirectory(getTempDir() + "/nonempty1") == true);
    ASSERT_TRUE(softeq::common::system::removeNonemptyDirectory(getTempDir() + "/nonempty1") == false);
    ASSERT_TRUE(softeq::common::system::removeNonemptyDirectory("") == false);
    ASSERT_TRUE(softeq::common::system::removeNonemptyDirectory("/proc") == false);
}

TEST(FsUtils, Copy)
{

    // create src dir and file
    const std::string testSrcDir(getTempDir() + std::string("/test_copy_src"));
    std::string cmd(std::string("mkdir -p ") + testSrcDir);
    int rc = std::system(cmd.c_str());
    ASSERT_EQ(rc, 0);

    std::string filename = testSrcDir + "/test_srcdir.txt";
    std::FILE *file = std::fopen(filename.c_str(), "a");
    ASSERT_NE(file, nullptr);
    std::fclose(file);

    // create dst dir
    const std::string testDstDir(getTempDir() + std::string("/test_copy_dst"));
    cmd = std::string("mkdir -p ") + testDstDir;
    rc = std::system(cmd.c_str());
    ASSERT_EQ(rc, 0);

    EXPECT_TRUE(softeq::common::system::copy(filename, testDstDir + "/test_copy_dst.txt") == true);
    // not copypaste. check that we are able to overwrite dest file.
    EXPECT_TRUE(softeq::common::system::copy(filename, testDstDir + "/test_copy_dst.txt") == true);

    EXPECT_TRUE(softeq::common::system::copy(filename, testDstDir) == false);
    EXPECT_TRUE(softeq::common::system::copy(filename, testDstDir + "/abcd") == true);
    // not copypaste. check that we are able to overwrite dest file.
    EXPECT_TRUE(softeq::common::system::copy(filename, testDstDir + "/abcd") == true);
    EXPECT_TRUE(softeq::common::system::copy(filename + "123.123", testDstDir) == false);
    EXPECT_TRUE(softeq::common::system::copy("", testDstDir + "/test_copy_dst.txt") == false);
    EXPECT_TRUE(softeq::common::system::copy("", "") == false);
    EXPECT_TRUE(softeq::common::system::copy("", "/proc") == false);
    EXPECT_TRUE(softeq::common::system::copy(filename, "/proc/folder1/folder2/test_file.txt") == false);

    cmd = "rm -Rf " + testSrcDir;
    rc = std::system(cmd.c_str());
    ASSERT_EQ(rc, 0);
    cmd = "rm -Rf " + testDstDir;
    rc = std::system(cmd.c_str());
    ASSERT_EQ(rc, 0);
}

TEST(FsUtils, Symlink)
{
    std::string testDir = getTempDir();
    std::string srcFilename = testDir + "/symlink_file.txt";
    std::system((std::string("rm ") + srcFilename).c_str());
    std::system((std::string("rm ") + testDir + "/target_symlink.txt").c_str());

    std::FILE *file = std::fopen(srcFilename.c_str(), "a");
    ASSERT_NE(file, nullptr);
    std::fclose(file);

    ASSERT_TRUE(softeq::common::system::symlink(srcFilename, testDir + "/target_symlink.txt") == true);
    ASSERT_TRUE(softeq::common::system::symlink(srcFilename, testDir + "/target_symlink.txt") == false);

    ASSERT_TRUE(softeq::common::system::symlink("", "") == false);
    ASSERT_TRUE(softeq::common::system::symlink("", "/proc/symlink.txt") == false);
    ASSERT_TRUE(softeq::common::system::symlink(srcFilename, "/proc/symlink.txt") == false);
}

TEST(FsUtils, IsValidFileName)
{
    const std::vector<std::string> cCorrectNames =
    {
        "a_1", "a-1", "a,1", "a.1", "a"
    };

    const std::vector<std::string> cIncorrectNames =
    {
        "", "/", ":", "*", "?", "<", ">", "|", "+", "a ", "@", "\0", "%", "^"
    };

    for(std::string i : cCorrectNames)
    {
        ASSERT_TRUE(softeq::common::system::isValidFilename(i.c_str()));
    }
    for(std::string i : cIncorrectNames)
    {
        ASSERT_FALSE(softeq::common::system::isValidFilename(i.c_str()));
    }
    ASSERT_THROW(softeq::common::system::isValidFilename(nullptr), std::logic_error);
}

TEST(FsUtils, ReadBinaryFileIntoBuffer)
{
    std::string content("123456789");
    std::string filename = getTempDir() + "/readbin";

    std::FILE *file = std::fopen(filename.c_str(), "w");
    ASSERT_NE(file, nullptr);
    EXPECT_NE(std::fwrite(content.c_str(), content.size(), 1, file), 0);
    ASSERT_EQ(std::fclose(file), 0);

    std::unique_ptr<char[]> buffer(new char[filesize(filename)]);

    ASSERT_TRUE(softeq::common::system::readBinaryFileIntoBuffer(filename.c_str(), buffer));
    ASSERT_TRUE(memcmp(buffer.get(), content.c_str(), content.size()) == 0);

    ASSERT_FALSE(softeq::common::system::readBinaryFileIntoBuffer("", buffer));
    ASSERT_THROW(softeq::common::system::readBinaryFileIntoBuffer(nullptr, buffer), std::logic_error);
}

TEST(FsUtils, FsAvail)
{
    std::string filename = getTempDir() + "/Temp.txt";

    std::FILE *file = std::fopen(filename.c_str(), "w");
    ASSERT_NE(file, nullptr);
    ASSERT_EQ(std::fclose(file), 0);

    ASSERT_NE(softeq::common::system::fsAvail(filename.c_str()), -1);
    ASSERT_EQ(softeq::common::system::fsAvail(""), -1);
    ASSERT_THROW(softeq::common::system::fsAvail(nullptr), std::logic_error);
}
