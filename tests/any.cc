#include <gtest/gtest.h>

#include "softeq/common/any.hh"

using namespace softeq::common;

template <size_t N>
struct words
{
    void *w[N];
};

struct big_type
{
    char i_wanna_be_big[256];

    std::string value;

    big_type()
        : value(std::string(300, 'b'))
    {
        i_wanna_be_big[0] = i_wanna_be_big[50] = 'k';
    }

    bool check()
    {
        EXPECT_TRUE(value.size() == 300);

        EXPECT_TRUE(value.front() == 'b' && value.back() == 'b');

        EXPECT_TRUE(i_wanna_be_big[0] == 'k' && i_wanna_be_big[50] == 'k');

        return true;
    }
};

TEST(AnyTest, AnyType)
{
    EXPECT_TRUE(Any().type() == typeid(void));

    EXPECT_TRUE(Any(4).type() == typeid(int));

    EXPECT_TRUE(Any(big_type()).type() == typeid(big_type));

    EXPECT_TRUE(Any(1.5f).type() == typeid(float));

    EXPECT_TRUE(Any(1.5).type() == typeid(double));

    EXPECT_TRUE(Any(std::string("string")).type() == typeid(std::string));
}

TEST(AnyTest, BadCast)
{
    bool except1 = false;
    bool except2 = false;
    bool except3 = false;
    bool except4 = false;

    EXPECT_THROW(any_cast<std::string>(Any(1)), std::bad_cast);
    EXPECT_THROW(any_cast<int>(Any()), std::bad_cast);

    try
    {
        any_cast<int>(Any(4.0f));
    }
    catch (std::bad_cast &)
    {
        except1 = true;
    }

    try
    {
        any_cast<float>(Any(4.0f));
    }
    catch (std::bad_cast &)
    {
        except2 = true;
    }

    try
    {
        any_cast<float>(Any(big_type()));
    }
    catch (std::bad_cast &)
    {
        except3 = true;
    }

    try
    {
        any_cast<big_type>(Any(big_type()));
    }
    catch (std::bad_cast &)
    {
        except4 = true;
    }

    EXPECT_TRUE(except1 == true && except2 == false);

    EXPECT_TRUE(except3 == true && except4 == false);
}

TEST(AnyTest, AnyCast)
{
    Any i4 = 4;
    Any i5 = 5;
    Any f6 = 6.0f;
    Any const *pi4{&i4};
    Any big1 = big_type();
    Any big2 = big_type();
    Any big3 = big_type();

    EXPECT_TRUE(any_cast<int>(&i4) != nullptr);
    EXPECT_TRUE(any_cast<float>(&i4) == nullptr);

    EXPECT_TRUE(any_cast<int>(pi4) != nullptr);
    EXPECT_TRUE(any_cast<float>(pi4) == nullptr);

    EXPECT_TRUE(any_cast<int>(i5) == 5);

    EXPECT_TRUE(any_cast<float>(f6) == 6.0f);

    EXPECT_TRUE(any_cast<big_type>(big1).check() && any_cast<big_type>(big2).check() &&
                any_cast<big_type>(big3).check());
}

TEST(AnyTest, AnyConstruct)
{
    Any a("first");

    ASSERT_EQ(any_cast<std::string>(a), std::string("first"));
    ASSERT_EQ(a.type().name(), typeid(std::string).name());

    Any anyint(uint64_t(42));

    ASSERT_EQ(any_cast<uint64_t>(anyint), uint64_t(42));
    ASSERT_EQ(anyint.type().name(), typeid(uint64_t).name());
}

TEST(AnyTest, AnyCopy)
{
    Any a("first");
    Any b(int(1));

    ASSERT_EQ(any_cast<int>(b), 1);

    b = a;

    ASSERT_EQ(any_cast<std::string>(b), std::string("first"));
}

TEST(AnyTest, AnyGet)
{
    Any s(std::string("third"));
    std::string val = s;

    ASSERT_EQ(val, "third");
}

TEST(AnyTest, AnyAssign)
{
    Any a("first");
    a = "second";

    ASSERT_EQ(any_cast<std::string>(a), std::string("second"));
}

TEST(AnyTest, General)
{
    std::shared_ptr<int> ptr_count(new int);
    std::weak_ptr<int> weak = ptr_count;
    Any p0 = 0;

    EXPECT_EQ(weak.use_count(), 1);

    Any p1 = ptr_count;
    EXPECT_EQ(weak.use_count(), 2);

    Any p2 = p1;
    EXPECT_EQ(weak.use_count(), 3);

    p0 = p1;
    EXPECT_EQ(weak.use_count(), 4);

    p0 = 0;
    EXPECT_EQ(weak.use_count(), 3);

    p0 = std::move(p1);
    EXPECT_EQ(weak.use_count(), 3);

    std::swap(p0, p1);
    EXPECT_EQ(weak.use_count(), 3);

    p0 = 0;
    EXPECT_EQ(weak.use_count(), 3);

    p1.reset();
    EXPECT_EQ(weak.use_count(), 2);

    p2 = Any(big_type());
    EXPECT_EQ(weak.use_count(), 1);

    p1 = ptr_count;
    EXPECT_EQ(weak.use_count(), 2);

    ptr_count = nullptr;
    EXPECT_EQ(weak.use_count(), 1);

    p1 = Any();
    EXPECT_EQ(weak.use_count(), 0);
}

#if 0
TEST(AnyTest, Test)
{
    auto is_stack_allocated = [](const Any& a, const void* obj1)
    {
        uintptr_t a_ptr = (uintptr_t)(&a);
        uintptr_t obj   = (uintptr_t)(obj1);
        return (obj >= a_ptr && obj < a_ptr + sizeof(Any));
    };

    static_assert(sizeof(std::unique_ptr<big_type>) <= sizeof(void*) * 1, "unique_ptr too big");
    static_assert(sizeof(std::shared_ptr<big_type>) <= sizeof(void*) * 2, "shared_ptr too big");

    Any i = 400;
    Any f = 400.0f;
    // any unique = std::unique_ptr<big_type>(); -- must be copy constructible
    Any shared = std::shared_ptr<big_type>();
    Any rawptr = (void*)(nullptr);
    Any big = big_type();
    Any w2 = words<2>();
    Any w3 = words<3>();

    EXPECT_TRUE(is_stack_allocated(i, any_cast<int>(&i)));

    EXPECT_TRUE(is_stack_allocated(f, any_cast<float>(&f)));

    EXPECT_TRUE(is_stack_allocated(rawptr, any_cast<void*>(&rawptr)));

    EXPECT_TRUE(is_stack_allocated(shared, any_cast<std::shared_ptr<big_type>>(&shared)));

    EXPECT_TRUE(!is_stack_allocated(big, any_cast<big_type>(&big)));

    EXPECT_TRUE(is_stack_allocated(w2, any_cast<words<2>>(&w2)));

    EXPECT_TRUE(!is_stack_allocated(w3, any_cast<words<3>>(&w3)));

    // Regression test for GitHub Issue #1
    // Any r1 = regression1_type();
    // EXPECT_TRUE(is_stack_allocated(r1, any_cast<const regression1_type>(&r1)));
}
#endif
