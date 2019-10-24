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
#ifndef _EINA_TYPE_TRAITS_HH
#define _EINA_TYPE_TRAITS_HH

#include <type_traits>

#include <string>
#include <vector>

/**
 * @addtogroup Eina_Cxx_Data_Types_Group
 *
 * @{
 */

namespace efl { namespace eina {

/**
 * @internal
 *
 * @{
 */

using std::enable_if;
using std::is_integral;
using std::is_pod;
using std::is_const;
using std::remove_cv;
using std::true_type;
using std::false_type;
using std::remove_pointer;
using std::remove_reference;

template <typename T>
struct indirect_is_contiguous_iterator : false_type
{};
template <>
struct indirect_is_contiguous_iterator<std::vector<char>::iterator> : std::true_type
{};
template <>
struct indirect_is_contiguous_iterator<std::vector<char>::const_iterator> : std::true_type
{};
    
template <typename T, typename Enable = void>
struct is_contiguous_iterator : indirect_is_contiguous_iterator<T> {};
template <>
struct is_contiguous_iterator<std::string::const_iterator> : true_type {};
template <>
struct is_contiguous_iterator<std::string::iterator> : true_type {};

template <bool, typename T, typename F>
struct if_c
{
  typedef T type;
};

template <typename T, typename F>
struct if_c<false, T, F>
{
  typedef F type;
};

template <typename U, typename T, typename F>
struct if_ : if_c<U::value, T, F>
{
};

template <typename T>
struct container_value_type
{
  typedef typename std::conditional<
    std::is_void<T>::value
    , std::add_pointer<T>, std::remove_reference<T>>::type::type type;
};

template <typename T>
struct nonconst_container_value_type
{
  typedef typename std::conditional<
    std::is_void<T>::value
    , std::add_pointer<T>, std::remove_cv<typename std::remove_reference<T>::type>>::type::type type;
};

/**
 * @}
 */

} }

/**
 * @}
 */

#endif
