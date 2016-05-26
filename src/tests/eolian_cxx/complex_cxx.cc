
#include <Eo.h>

#include "complex.eo.h"
#include "complex.eo.hh"

template <typename T, typename U>
struct test_return_type;

template <typename T, typename R, typename U>
struct test_return_type<R(T::*)(), U>
{
   static_assert(std::is_same<R, U>::value, "Wrong type");
};
template <typename T, typename R, typename U>
struct test_return_type<R(T::*)() const, U>
{
   static_assert(std::is_same<R, U>::value, "Wrong type");
};

template <typename T, typename U>
struct test_param_type;

template <typename T, typename P, typename U>
struct test_param_type<void(T::*)(P) const, U>
{
   static_assert(std::is_same<P, U>::value, "Wrong type");
};
template <typename T, typename P, typename U>
struct test_param_type<void(T::*)(P), U>
{
   static_assert(std::is_same<P, U>::value, "Wrong type");
};

test_param_type<typeof( & nonamespace::Complex::foo ), efl::eina::range_list<int>> foo;
test_return_type<typeof( & nonamespace::Complex::bar ), efl::eina::range_array<int>> bar;
test_return_type<typeof( & nonamespace::Complex::wrapper_r ), nonamespace::Complex> wrapper_r;
test_param_type<typeof( & nonamespace::Complex::wrapper_in ), nonamespace::Complex> wrapper_in;
test_param_type<typeof( & nonamespace::Complex::wrapper_inout ), nonamespace::Complex*> wrapper_inout;
test_param_type<typeof( & nonamespace::Complex::wrapper_out ), nonamespace::Complex*> wrapper_out;
