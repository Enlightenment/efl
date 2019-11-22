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
#ifndef EOLIAN_CXX_TYPE_TRAITS_HH
#define EOLIAN_CXX_TYPE_TRAITS_HH

#include <type_traits>

namespace efl { namespace eolian { namespace grammar { namespace type_traits {

template <typename G>
struct accepts_tuple : std::false_type {};
        
template <typename G>
struct attributes_needed : std::integral_constant<int, 0> {};
template <typename G>
struct attributes_needed<G const> : attributes_needed<G> {};
template <typename G>
struct attributes_needed<G&> : attributes_needed<G> {};
template <typename G>
struct attributes_needed<G const&> : attributes_needed<G> {};
template <typename G>
struct attributes_needed<G volatile> : attributes_needed<G> {};
template <typename G>
struct attributes_needed<G volatile&> : attributes_needed<G> {};
template <typename G>
struct attributes_needed<G const volatile> : attributes_needed<G> {};
template <typename G>
struct attributes_needed<G const volatile&> : attributes_needed<G> {};

template <typename T>
struct is_std_tuple : std::false_type {};

template <typename...Args>
struct is_std_tuple<std::tuple<Args...> > : std::true_type {};
template <typename...Args>
struct is_std_tuple<std::tuple<Args...>&> : std::true_type {};
template <typename...Args>
struct is_std_tuple<const std::tuple<Args...> > : std::true_type {};
template <typename...Args>
struct is_std_tuple<const std::tuple<Args...>&> : std::true_type {};

template <typename T, typename Enable = void>
struct is_explicit_tuple : std::false_type {};
template <typename T>
struct is_explicit_tuple<T, typename std::enable_if<is_std_tuple<T>::value>::type> : std::true_type {};
        
template <typename T, typename Enable = void>
struct is_tuple : std::false_type {};
template <typename T>
struct is_tuple<T const> : is_tuple<T> {};
template <typename T>
struct is_tuple<T const&> : is_tuple<T> {};
template <typename T>
struct is_tuple<T const volatile> : is_tuple<T> {};
template <typename T>
struct is_tuple<T const volatile&> : is_tuple<T> {};
template <typename T>
struct is_tuple<T volatile> : is_tuple<T> {};
template <typename T>
struct is_tuple<T volatile&> : is_tuple<T> {};

template <typename T>
struct is_tuple<T, typename std::enable_if<!std::is_const<T>::value && is_std_tuple<T>::value>::type> : std::true_type {};
  
} } } }

#endif
