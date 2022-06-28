#include <gtest/gtest.h>

#include <softeq/common/scope_guard.hh>

using namespace softeq::common;
using std::vector;

double returnsDouble()
{
  return 0.0;
}

class MyFunctor
{
public:
    explicit MyFunctor(int* ptr) : ptr_(ptr) {}

    void operator()()
    {
        ++*ptr_;
    }

private:
    int* ptr_;
};

TEST(ScopeGuardTest, DifferentWaysToBind)
{
    {
        // There is implicit conversion from func pointer
        // double (*)() to function<void()>.
        scope_guard g = scope_guard(returnsDouble);
    }

    vector<int> v;
    void (vector<int>::*push_back)(int const&) = &vector<int>::push_back;

    v.push_back(1);
    {
        // binding to member function.
        scope_guard g = scope_guard(std::bind(&vector<int>::pop_back, &v));
    }
    EXPECT_EQ(0, v.size());

    {
        // bind member function with args. v is passed-by-value!
        scope_guard g = scope_guard(std::bind(push_back, v, 2));
    }
    EXPECT_EQ(0, v.size()); // push_back happened on a copy of v... fail!

    // pass in an argument by pointer so to avoid copy.
    {
        scope_guard g = scope_guard(std::bind(push_back, &v, 4));
    }
    EXPECT_EQ(1, v.size());

    {
        // pass in an argument by reference so to avoid copy.
        scope_guard g = scope_guard(std::bind(push_back, std::ref(v), 4));
    }
    EXPECT_EQ(2, v.size());

    // lambda with a reference to v
    {
        scope_guard g = scope_guard([&] { v.push_back(5); });
    }
    EXPECT_EQ(3, v.size());

    // lambda with a copy of v
    {
        scope_guard g = scope_guard([v] () mutable { v.push_back(6); });
    }
    EXPECT_EQ(3, v.size());

    // functor object
    int n = 0;
    {
        MyFunctor f(&n);
        scope_guard g = scope_guard(f);
    }
    EXPECT_EQ(1, n);

    // temporary functor object
    n = 0;
    {
        scope_guard g = scope_guard(MyFunctor(&n));
    }
    EXPECT_EQ(1, n);

    // Use auto instead of ScopeGuard
    n = 2;
    {
        auto g = scope_guard(MyFunctor(&n));
    }
    EXPECT_EQ(3, n);

    // Use const auto& instead of ScopeGuard
    n = 10;
    {
        const auto& g __attribute__((unused)) = scope_guard(MyFunctor(&n));
    }
    EXPECT_EQ(11, n);
}

/**
 * Add an integer to a vector if it was inserted
 * successfuly.
 */
void testUndoAction(bool failure)
{
    vector<int64_t> v;
    { // defines a "mini" scope

        // be optimistic and insert this into memory
        v.push_back(1);

        // The guard is triggered to undo the insertion unless dismiss() is called.
        scope_guard guard = scope_guard([&] { v.pop_back(); });

        // Do some action; Use the failure argument to pretend
        // if it failed or succeeded.

        // if there was no failure, dismiss the undo guard action.
        if (!failure)
        {
            guard.dismiss();
        }
    } // all stack allocated in the mini-scope will be destroyed here.

    if (failure)
    {
        EXPECT_EQ(0, v.size()); // the action failed => undo insertion
    }
    else
    {
        EXPECT_EQ(1, v.size()); // the action succeeded => keep insertion
    }
}

TEST(ScopeGuardTest, UndoAction)
{
    testUndoAction(true);
    testUndoAction(false);
}

/**
 * Sometimes in a try catch block we want to execute a piece of code
 * regardless if an exception happened or not. For example, you want
 * to close a db connection regardless if an exception was thrown during
 * insertion. In Java and other languages there is a finally clause that
 * helps accomplish this:
 *
 *   try {
 *     dbConn.doInsert(sql);
 *   } catch (const DbException& dbe) {
 *     dbConn.recordFailure(dbe);
 *   } catch (const CriticalException& e) {
 *     throw e; // re-throw the exception
 *   } finally {
 *     dbConn.closeConnection(); // executes no matter what!
 *   }
 *
 * We can approximate this behavior in C++ with ScopeGuard.
 */
enum class ErrorBehavior
{
  SUCCESS,
  HANDLED_ERROR,
  UNHANDLED_ERROR,
};

void testFinally(ErrorBehavior error)
{
    bool cleanupOccurred = false;

    try
    {
        scope_guard guard = scope_guard([&] { cleanupOccurred = true; });

        try
        {
            if (error == ErrorBehavior::HANDLED_ERROR)
            {
                throw std::runtime_error("throwing an expected error");
            }
            else if (error == ErrorBehavior::UNHANDLED_ERROR)
            {
                throw "never throw raw strings";
            }
        }
        catch (const std::runtime_error&)
        {
        }
    }
    catch (...)
    {
        // Outer catch to swallow the error for the UNHANDLED_ERROR behavior
    }

    EXPECT_TRUE(cleanupOccurred);
}

TEST(ScopeGuardTest, TryCatchFinally)
{
    testFinally(ErrorBehavior::SUCCESS);
    testFinally(ErrorBehavior::HANDLED_ERROR);
    testFinally(ErrorBehavior::UNHANDLED_ERROR);
}

TEST(ScopeGuardTest, Dual)
{
    vector<int64_t> v;
    { // defines a "mini" scope

        scope_guard guard;
        guard += ([&] { v.push_back(5); v.push_back(6); });

    } // all stack allocated in the mini-scope will be destroyed here.

    EXPECT_EQ(2, v.size());
    EXPECT_EQ(6, v.back());
}