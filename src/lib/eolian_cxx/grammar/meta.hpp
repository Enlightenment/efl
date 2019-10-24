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
#ifndef EOLIAN_CXX_META_HH_
#define EOLIAN_CXX_META_HH_

#include <utility>
#include <type_traits>

namespace efl { namespace eolian { namespace grammar { namespace meta {

template <std::size_t A0, std::size_t A1 = 0, std::size_t...args>
struct max : max<((A0 > A1) ? A0 : A1), args...> {};

template <std::size_t A0, std::size_t A1>
struct max<A0, A1> : std::integral_constant<std::size_t, (A0 > A1 ? A0 : A1)> {};

template <typename T>
struct identity { typedef T type; };

template <typename T, typename U, typename...Others>
struct is_one_of : std::conditional<std::is_same<T, U>::value
                                    , std::is_same<T, U>
                                    , is_one_of<T, Others...> >::type::type
{};

template <typename T, typename U>
struct is_one_of<T, U> : std::is_same<T, U>
{};


template <std::size_t size, typename T, typename U, typename...Args>
struct find_impl : find_impl<size+1, T, Args...>
{};

template <std::size_t size, typename T, typename...Args>
struct find_impl<size, T, T, Args...> : std::integral_constant<std::size_t, size> {};
  
template <typename T, typename U, typename...Args>
struct find : find_impl<0u, T, U, Args...>
{};

} } } }

#endif
