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
#ifndef EINA_CXX_EINA_TUPLE_UNWRAP_HH
#define EINA_CXX_EINA_TUPLE_UNWRAP_HH

#include <eina_integer_sequence.hh>

namespace efl { namespace eina {

template <typename Callable, typename T, std::size_t... S>
auto call_tuple_unwrap(Callable const& callable, T const& tuple
                       , eina::index_sequence<S...>)
  -> decltype(callable(std::get<S>(tuple)...))
{
  return callable(std::get<S>(tuple)...);
}

template <typename Callable, typename T, std::size_t... S
          , typename... Args>
auto call_tuple_unwrap_prefix(Callable const& callable, T const& tuple
                              , eina::index_sequence<S...>
                              , Args&&... args)
  -> decltype(callable(std::forward<Args>(args)..., std::get<S>(tuple)...))
{
  return callable(std::forward<Args>(args)..., std::get<S>(tuple)...);
}

template <typename Callable, typename T, std::size_t... S
          , typename... Args>
auto call_tuple_unwrap_suffix(Callable const& callable, T const& tuple
                              , eina::index_sequence<S...>
                              , Args&&... args)
  -> decltype(callable(std::get<S>(tuple)..., std::forward<Args>(args)...))
{
  return callable(std::get<S>(tuple)..., std::forward<Args>(args)...);
}

} }

#endif
