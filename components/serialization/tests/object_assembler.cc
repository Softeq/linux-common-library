#include "serialization_test_fixture.hh"

#include "structures/basic_structures.hh"

using namespace softeq::common::serialization;

TEST_F(Serialization, RedeclarationErrors)
{
    bool thrown = false;
    try
    {
        ObjectAssembler<ErrorNameDefineObject>::accessor();
    }
    catch (const std::logic_error &err)
    {
        thrown = true;
        EXPECT_EQ(err.what(), std::string("redeclaration of memeber with name 'i1'"));
    }
    EXPECT_EQ(thrown, true);

    thrown = false;
    try
    {
        ObjectAssembler<ErrorMemberDefineObject>::accessor();
    }
    catch (const std::logic_error &err)
    {
        thrown = true;
        EXPECT_EQ(err.what(), std::string("redeclaration of pointer memeber"));
    }
    EXPECT_EQ(thrown, true);

    thrown = false;
    try
    {
        ObjectAssembler<EnumMemberErrorValues>::accessor();
    }
    catch (const std::logic_error &err)
    {
        thrown = true;
        EXPECT_EQ(err.what(), std::string("redeclaration of enum with name 'enumError1'"));
    }
    EXPECT_EQ(thrown, true);
}

// TODO: Create test cases for graph()
