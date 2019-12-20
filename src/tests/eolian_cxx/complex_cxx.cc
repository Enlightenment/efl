/*
 * Copyright 2019 by its authors. See AUTHORS.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

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

test_param_type<decltype( & nonamespace::Complex::inclasscont ), efl::eina::range_list<efl::Object>> inclasscont;
test_param_type<decltype( & nonamespace::Complex::incontcont ), efl::eina::range_list<efl::eina::range_list<::efl::eina::string_view> >> incontcont;
test_param_type<decltype( & nonamespace::Complex::incontcontown ), efl::eina::list<efl::eina::range_list<::efl::eina::string_view> >const&> incontcontown;
test_param_type<decltype( & nonamespace::Complex::incontowncontown ), efl::eina::list<efl::eina::list<::efl::eina::string_view> >const&> incontowncontown;
test_param_type<decltype( & nonamespace::Complex::incontowncont ), efl::eina::range_list<efl::eina::list<::efl::eina::string_view> >> incontowncont;
test_param_type<decltype( & nonamespace::Complex::instringcont ), efl::eina::range_list<efl::eina::string_view>> instringcont;
test_param_type<decltype( & nonamespace::Complex::instringowncont ), efl::eina::range_list<efl::eina::string_view>> instringowncont;
test_param_type<decltype( & nonamespace::Complex::instringcontown ), efl::eina::list<efl::eina::string_view>const&> instringcontown;

test_param_type<decltype( & nonamespace::Complex::outclasscont ), efl::eina::range_list<efl::Object>&> outclasscont;
test_param_type<decltype( & nonamespace::Complex::outcontcont ), efl::eina::range_list<efl::eina::range_list<::efl::eina::string_view> >&> outcontcont;
test_param_type<decltype( & nonamespace::Complex::outcontcontown ), efl::eina::list<efl::eina::range_list<::efl::eina::string_view> >&> outcontcontown;
test_param_type<decltype( & nonamespace::Complex::outcontowncontown ), efl::eina::list<efl::eina::list<::efl::eina::string_view> >&> outcontowncontown;
test_param_type<decltype( & nonamespace::Complex::outcontowncont ), efl::eina::range_list<efl::eina::list<::efl::eina::string_view> >&> outcontowncont;
test_param_type<decltype( & nonamespace::Complex::outstringcont ), efl::eina::range_list<efl::eina::string_view>&> outstringcont;
test_param_type<decltype( & nonamespace::Complex::outstringowncont ), efl::eina::range_list<efl::eina::string_view>&> outstringowncont;
test_param_type<decltype( & nonamespace::Complex::outstringcontown ), efl::eina::list<efl::eina::string_view>&> outstringcontown;

test_param_type<decltype( & nonamespace::Complex::foo ), efl::eina::range_list<::efl::eina::string_view> > foo;
test_return_type<decltype( & nonamespace::Complex::bar ), efl::eina::range_array<::efl::eina::string_view> > bar;
test_return_type<decltype( & nonamespace::Complex::wrapper_r ), nonamespace::Complex> wrapper_r;
test_param_type<decltype( & nonamespace::Complex::wrapper_in ), nonamespace::Complex> wrapper_in;
test_param_type<decltype( & nonamespace::Complex::wrapper_inout ), nonamespace::Complex&> wrapper_inout;
test_param_type<decltype( & nonamespace::Complex::wrapper_out ), nonamespace::Complex&> wrapper_out;
