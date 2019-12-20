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
#ifndef EOLIAN_CXX_GENERATOR_HH
#define EOLIAN_CXX_GENERATOR_HH

#include <type_traits>

// EXPERIMENTAL
//#define USE_EOCXX_INHERIT_ONLY
//#define USE_EOCXX_MANUAL_OVERRIDES

namespace efl { namespace eolian { namespace grammar {

template <typename T, typename Enable = void>
struct is_generator : std::false_type {};
template <typename T, typename Enable = void>
struct is_eager_generator : std::false_type {};

template <typename T>
struct is_generator<T&> : is_generator<T> {};
template <typename T>
struct is_eager_generator<T&> : is_eager_generator<T> {};
template <typename T>
struct is_generator<T const&> : is_generator<T> {};
template <typename T>
struct is_eager_generator<T const&> : is_eager_generator<T> {};
template <typename T>
struct is_generator<T const> : is_generator<T> {};
template <typename T>
struct is_eager_generator<T const> : is_eager_generator<T> {};
template <typename T>
struct is_generator<T volatile> : is_generator<T> {};
template <typename T>
struct is_eager_generator<T volatile> : is_eager_generator<T> {};
template <typename T>
struct is_generator<T volatile&> : is_generator<T> {};
template <typename T>
struct is_eager_generator<T volatile&> : is_eager_generator<T> {};
template <typename T>
struct is_generator<T volatile const> : is_generator<T> {};
template <typename T>
struct is_eager_generator<T volatile const> : is_eager_generator<T> {};
template <typename T>
struct is_generator<T volatile const&> : is_generator<T> {};
template <typename T>
struct is_eager_generator<T volatile const&> : is_eager_generator<T> {};
// template <typename T>
// struct is_generator<T> : is_eager_generator<T> {};
      
template <typename G, typename Enable = typename std::enable_if<is_eager_generator<G>::value>::type>
G as_generator(G g) { return g; }


} } }

#endif
