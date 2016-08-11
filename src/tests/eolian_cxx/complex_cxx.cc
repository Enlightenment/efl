
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <Eina.h>
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

test_param_type<typeof( & nonamespace::Complex::inclasscont ), efl::eina::range_list<efl::Object>> inclasscont;
test_param_type<typeof( & nonamespace::Complex::incontcont ), efl::eina::range_list<efl::eina::range_list<int>>> incontcont;
test_param_type<typeof( & nonamespace::Complex::incontcontown ), efl::eina::list<efl::eina::range_list<int>>const&> incontcontown;
test_param_type<typeof( & nonamespace::Complex::incontowncontown ), efl::eina::list<efl::eina::list<int>>const&> incontowncontown;
test_param_type<typeof( & nonamespace::Complex::incontowncont ), efl::eina::range_list<efl::eina::list<int>>> incontowncont;
test_param_type<typeof( & nonamespace::Complex::instringcont ), efl::eina::range_list<efl::eina::string_view>> instringcont;
test_param_type<typeof( & nonamespace::Complex::instringowncont ), efl::eina::range_list<efl::eina::string_view>> instringowncont;
test_param_type<typeof( & nonamespace::Complex::instringcontown ), efl::eina::list<efl::eina::string_view>const&> instringcontown;

test_param_type<typeof( & nonamespace::Complex::outclasscont ), efl::eina::range_list<efl::Object>&> outclasscont;
test_param_type<typeof( & nonamespace::Complex::outcontcont ), efl::eina::range_list<efl::eina::range_list<int>>&> outcontcont;
test_param_type<typeof( & nonamespace::Complex::outcontcontown ), efl::eina::list<efl::eina::range_list<int>>&> outcontcontown;
test_param_type<typeof( & nonamespace::Complex::outcontowncontown ), efl::eina::list<efl::eina::list<int>>&> outcontowncontown;
test_param_type<typeof( & nonamespace::Complex::outcontowncont ), efl::eina::range_list<efl::eina::list<int>>&> outcontowncont;
test_param_type<typeof( & nonamespace::Complex::outstringcont ), efl::eina::range_list<efl::eina::string_view>&> outstringcont;
test_param_type<typeof( & nonamespace::Complex::outstringowncont ), efl::eina::range_list<efl::eina::string_view>&> outstringowncont;
test_param_type<typeof( & nonamespace::Complex::outstringcontown ), efl::eina::list<efl::eina::string_view>&> outstringcontown;

test_param_type<typeof( & nonamespace::Complex::foo ), efl::eina::range_list<int>> foo;
test_return_type<typeof( & nonamespace::Complex::bar ), efl::eina::range_array<int>> bar;
test_return_type<typeof( & nonamespace::Complex::wrapper_r ), nonamespace::Complex> wrapper_r;
test_param_type<typeof( & nonamespace::Complex::wrapper_in ), nonamespace::Complex> wrapper_in;
test_param_type<typeof( & nonamespace::Complex::wrapper_inout ), nonamespace::Complex&> wrapper_inout;
test_param_type<typeof( & nonamespace::Complex::wrapper_out ), nonamespace::Complex&> wrapper_out;
