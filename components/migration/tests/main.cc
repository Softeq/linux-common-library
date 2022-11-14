#include <gtest/gtest.h>

class TestGlobal : public ::testing::Environment
{
protected:
    void SetUp()
    {
    }

    void TearDown()
    {
    }
};

int main(int argc, char *argv[])
{
    ::testing::InitGoogleTest(&argc, argv);
    // gtest takes ownership of the TestEnvironment ptr - we don't delete it.
    ::testing::AddGlobalTestEnvironment(new TestGlobal());
    return RUN_ALL_TESTS();
}
