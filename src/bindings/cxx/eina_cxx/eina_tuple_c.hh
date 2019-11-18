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
#ifndef EFL_EINA_EINA_TUPLE_C_HH_
#define EFL_EINA_EINA_TUPLE_C_HH_

#include <tuple>

namespace efl { namespace eina { namespace _mpl {

template <typename T, T... Ts>
using tuple_c = std::tuple<std::integral_constant<T, Ts>...>;

template <typename T, T... Ts>
constexpr std::size_t tuple_c_size(tuple_c<T, Ts...> const&)
{
  return sizeof...(Ts);
}

constexpr std::size_t tuple_c_size(std::tuple<> const&)
{
  return 0;
}

} } }

#endif
