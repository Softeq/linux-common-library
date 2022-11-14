#include <gtest/gtest.h>

#include <common/stdutils/optional.hh>

#include <exception>

using namespace softeq::common::stdutils;

#define ARG(T) (static_cast< T const* >(0))

template<class T>
inline void check_initialized_const(Optional<T> const &opt)
{
  ASSERT_TRUE(opt);
  ASSERT_TRUE(!!opt);
  ASSERT_TRUE(opt.hasValue());
}

template<class T>
inline void check_initialized(Optional<T> &opt)
{
    T value;
    EXPECT_NO_THROW(value = !opt.cValue());

    ASSERT_TRUE(opt);

    ASSERT_TRUE(!!opt);
    ASSERT_TRUE(opt.hasValue());
    ASSERT_TRUE(opt.valueOr(value) == opt.cValue());

    check_initialized_const(opt);
}

template<class T>
inline void check_value_const(Optional<T> const &opt, T const &v, T const &z)
{
  ASSERT_TRUE(*opt == v);
  ASSERT_TRUE(*opt != z);
  ASSERT_TRUE(opt.cValue() == v);
  ASSERT_TRUE(opt.cValue() != z);
  ASSERT_TRUE((*(opt.operator->()) == v));
}

template<class T>
inline void check_value ( Optional<T>& opt, T const& v, T const& z )
{
  ASSERT_TRUE( *opt == v ) ;
  ASSERT_TRUE( *opt != z ) ;
  ASSERT_TRUE( opt.cValue() == v ) ;
  ASSERT_TRUE( opt.cValue() != z ) ;
  ASSERT_TRUE( (*(opt.operator->()) == v) ) ;
  check_value_const(opt,v,z);
}

template<class T>
inline void check_uninitialized(Optional<T>& opt)
{	
  ASSERT_TRUE(!opt.hasValue());
  EXPECT_THROW(opt.cValue(), std::exception);

  EXPECT_THROW(opt.operator*(), std::exception);
  EXPECT_THROW(opt.operator->(), std::exception);
}

template<class T>
void test_basics( T const* )
{
	T z(0);
	T a(1);
	T b(2);

	// Default construction.
	// 'def' state is Uninitialized.
	// T::T() is not called (and it is not even defined)
	Optional<T> def ;
  check_uninitialized(def);

	// Direct initialization.
	// 'oa' state is Initialized with 'a'
	// T::T( T const& x ) is used.;
	Optional<T> oa ( a ) ;
  check_initialized(oa);
	check_value(oa,a,z);

	// Value-Assignment upon Uninitialized optional.
	// T::T( T const& x ) is used.
	Optional<T> ob ;
	ob = a ;
  check_initialized(ob);
	check_value(ob,a,z);

	// Assignment initialization.
	// T::T ( T const& x ) is used to copy new value. ;
	Optional<T> const oa2 (oa);
  check_initialized_const(oa2);
	check_value_const(oa2,a,z);

  ob = b;

	// Assignment
	// T::operator= ( T const& x ) is used to copy new value.
	oa = ob ;
	check_value(oa,b,z);
}

//
// Test Uninitialized access assert
//
template<class T>
void test_uninitialized_access( T const* )
{
    Optional<T> def;

    // This should throw because 'def' is uninitialized
    EXPECT_THROW(def.cValue(), std::exception);
}

//
// This verifies relational operators.
//
template<class T>
void test_relops( T const* )
{
  T v0(0);
  T v1(1);
  T v2(1);

  Optional<T> def0 ;
  Optional<T> def1 ;
  Optional<T> opt0(v0);
  Optional<T> opt1(v1);
  Optional<T> opt2(v2);

  // Check identity
  ASSERT_TRUE ( def0 == def0 ) ;
  ASSERT_TRUE ( opt0 == opt0 ) ;
  ASSERT_TRUE ( !(def0 != def0) ) ;
  ASSERT_TRUE ( !(opt0 != opt0) ) ;

  // Check when both are uininitalized.
  ASSERT_TRUE (   def0 == def1  ) ; // both uninitialized compare equal
  ASSERT_TRUE ( !(def0 <  def1) ) ; // uninitialized is never less    than uninitialized
  ASSERT_TRUE ( !(def0 >  def1) ) ; // uninitialized is never greater than uninitialized
  ASSERT_TRUE ( !(def0 != def1) ) ;
  ASSERT_TRUE (   def0 <= def1  ) ;
  ASSERT_TRUE (   def0 >= def1  ) ;

  // Check when only lhs is uninitialized.
  ASSERT_TRUE (   def0 != opt0  ) ; // uninitialized is never equal to initialized
  ASSERT_TRUE ( !(def0 == opt0) ) ;
  ASSERT_TRUE (   def0 <  opt0  ) ; // uninitialized is always less than initialized
  ASSERT_TRUE ( !(def0 >  opt0) ) ;
  ASSERT_TRUE (   def0 <= opt0  ) ;
  ASSERT_TRUE ( !(def0 >= opt0) ) ;

  // Check when only rhs is uninitialized.
  ASSERT_TRUE (   opt0 != def0  ) ; // initialized is never equal to uninitialized
  ASSERT_TRUE ( !(opt0 == def0) ) ;
  ASSERT_TRUE ( !(opt0 <  def0) ) ; // initialized is never less than uninitialized
  ASSERT_TRUE (   opt0 >  def0  ) ;
  ASSERT_TRUE ( !(opt0 <= def0) ) ;
  ASSERT_TRUE (   opt0 >= opt0  ) ;

  // If both are initialized, values are compared
  ASSERT_TRUE ( opt0 != opt1 ) ;
  ASSERT_TRUE ( opt1 == opt2 ) ;
  ASSERT_TRUE ( opt0 <  opt1 ) ;
  ASSERT_TRUE ( opt1 >  opt0 ) ;
  ASSERT_TRUE ( opt1 <= opt2 ) ;
  ASSERT_TRUE ( opt1 >= opt0 ) ;

  // Compare against a value directly
  ASSERT_TRUE ( opt0 == v0 ) ;
  ASSERT_TRUE ( opt0 != v1 ) ;
  ASSERT_TRUE ( opt1 == v2 ) ;
  ASSERT_TRUE ( opt0 <  v1 ) ;
  ASSERT_TRUE ( opt1 >  v0 ) ;
  ASSERT_TRUE ( opt1 <= v2 ) ;
  ASSERT_TRUE ( opt1 >= v0 ) ;
  ASSERT_TRUE ( v0 != opt1 ) ;
  ASSERT_TRUE ( v1 == opt2 ) ;
  ASSERT_TRUE ( v0 <  opt1 ) ;
  ASSERT_TRUE ( v1 >  opt0 ) ;
  ASSERT_TRUE ( v1 <= opt2 ) ;
  ASSERT_TRUE ( v1 >= opt0 ) ;
  ASSERT_TRUE (   def0 != v0  ) ;
  ASSERT_TRUE ( !(def0 == v0) ) ;
  ASSERT_TRUE (   def0 <  v0  ) ;
  ASSERT_TRUE ( !(def0 >  v0) ) ;
  ASSERT_TRUE (   def0 <= v0  ) ;
  ASSERT_TRUE ( !(def0 >= v0) ) ;
  ASSERT_TRUE (   v0 != def0  ) ;
  ASSERT_TRUE ( !(v0 == def0) ) ;
  ASSERT_TRUE ( !(v0 <  def0) ) ;
  ASSERT_TRUE (   v0 >  def0  ) ;
  ASSERT_TRUE ( !(v0 <= def0) ) ;
  ASSERT_TRUE (   v0 >= opt0  ) ;
}

//
// Test Direct Value Manipulation
//
template<class T>
void test_direct_value_manip( T const* )
{
  T x(3);

  Optional<T> const c_opt0(x) ;
  Optional<T>         opt0(x);

  ASSERT_TRUE( c_opt0.cValue().V() == x.V() ) ;
  ASSERT_TRUE(   opt0.cValue().V() == x.V() ) ;

  ASSERT_TRUE( c_opt0->V() == x.V() ) ;
  ASSERT_TRUE(   opt0->V() == x.V() ) ;

  ASSERT_TRUE( (*c_opt0).V() == x.V() ) ;
  ASSERT_TRUE( (*  opt0).V() == x.V() ) ;

  T y(4);
  opt0 = y ;
  ASSERT_TRUE(opt0->V() == y.V());
}

template<class T>
void test_arrow( T const* )
{
  T a(1234);
  Optional<T>        oa(a) ;
  Optional<T> const coa(a) ;

  ASSERT_TRUE ( coa->V() == 1234 ) ;

  oa->V() = 4321 ;

  ASSERT_TRUE (     a.V() = 1234 ) ;
  ASSERT_TRUE ( (*oa).V() = 4321 ) ;
}

int eat ( bool ) { return 1 ; }
int eat ( char ) { return 1 ; }
int eat ( int  ) { return 1 ; }
int eat ( void const* ) { return 1 ; }

template<class T> int eat ( T ) { return 0 ; }

//
// This verifies that operator safe_bool() behaves properly.
//
template<class T>
void test_no_implicit_conversions_impl( T const& )
{
  Optional<T> def ;
  ASSERT_TRUE ( eat(def) == 0 ) ;
}

class X
{
  public:
    X(int v):_v(v) {}
    int V() const { return _v; }
    int &V() { return _v; }
    X (X const &rhs) : _v(rhs._v) {}
    X &operator=(X const &rhs) { _v = rhs._v; return *this; }
  private:
    X();

  int _v;
};

TEST(Optional, CustomType)
{
  test_arrow(ARG(X));
  test_direct_value_manip(ARG(X));
}

TEST(Optional, Reset)
{
  bool b = true;
  int i = 42;
  void const* p = reinterpret_cast<void const*>(42);
  std::string s("test");

  Optional<bool> ob(b);
  Optional<int> oi(i);
  Optional<void const*> op(p);
  Optional<std::string> os(s);

  EXPECT_TRUE(ob.hasValue());
  EXPECT_EQ(ob.cValue(), true);

  EXPECT_TRUE(oi.hasValue());
  EXPECT_EQ(oi.cValue(), 42);

  EXPECT_EQ(op.cValue(), reinterpret_cast<void const*>(42));
  EXPECT_TRUE(op.hasValue());

  EXPECT_TRUE(os.hasValue());
  EXPECT_EQ(os.cValue(), "test");

  ob.reset();
  oi.reset();
  op.reset();
  os.reset();

  EXPECT_FALSE(ob.hasValue());
  EXPECT_FALSE(oi.hasValue());
  EXPECT_FALSE(op.hasValue());
  EXPECT_FALSE(os.hasValue());

  EXPECT_THROW(ob.cValue(), std::exception);
  EXPECT_THROW(oi.cValue(), std::exception);
  EXPECT_THROW(op.cValue(), std::exception);
  EXPECT_THROW(os.cValue(), std::exception);

  X x(42);
  Optional<X> ox(x);
  EXPECT_TRUE(ox.hasValue());
  ox.reset();
  EXPECT_FALSE(ox.hasValue());
  EXPECT_THROW(ox.cValue(), std::exception);
}

TEST(Optional, NoImplicitConvertions)
{
  bool b = false ;
  char c = 0 ;
  int i = 0 ;
  void const* p = 0 ;

  test_no_implicit_conversions_impl(b);
  test_no_implicit_conversions_impl(c);
  test_no_implicit_conversions_impl(i);
  test_no_implicit_conversions_impl(p);
}

TEST(Optional, BuiltinTypes)
{
  test_basics( ARG(double) );
  //test_conditional_ctor_and_get_valur_or( ARG(double) );
  test_uninitialized_access( ARG(double) );
  //test_no_throwing_swap( ARG(double) );
  test_relops( ARG(double) ) ;
  //test_none( ARG(double) ) ;
}
