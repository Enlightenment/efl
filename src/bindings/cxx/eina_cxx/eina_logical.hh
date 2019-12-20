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
#ifndef EFL_EINA_LOGICAL_HH
#define EFL_EINA_LOGICAL_HH

#include <type_traits>

namespace efl { namespace eina { namespace _mpl {

template <bool... N>
struct or_;

template <>
struct or_<> : std::integral_constant<bool, false> {};

template <bool B>
struct or_<B> : std::integral_constant<bool, B> {};

template <bool B1, bool B2, bool... Bs>
struct or_<B1, B2, Bs...> : std::integral_constant<bool, B1 || B2 || or_<Bs...>::value> {};

template <bool... N>
struct and_;

template <>
struct and_<> : std::integral_constant<bool, true> {};

template <bool B>
struct and_<B> : std::integral_constant<bool, B> {};

template <bool B1, bool B2, bool... Bs>
struct and_<B1, B2, Bs...> : std::integral_constant<bool, B1 && B2 && and_<Bs...>::value> {};

} } }

#endif
