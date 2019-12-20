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
#ifndef ELDBUS_CXX_ELDBUS_INTEGER_SEQUENCE_HH
#define ELDBUS_CXX_ELDBUS_INTEGER_SEQUENCE_HH

namespace efl { namespace eldbus { namespace _detail {

template <typename T, T... Ints>
struct integer_sequence
{
  typedef T value_type;
  static constexpr std::size_t size() { return sizeof...(Ints); }
  typedef integer_sequence<T, Ints...> type;
};

template<class S1, class S2> struct concat;

template<typename T, T... I1, T... I2>
struct concat<integer_sequence<T, I1...>, integer_sequence<T, I2...> >
  : integer_sequence<T, I1..., (sizeof...(I1)+I2)...> {};

template<class S1, class S2>
using Concat = typename concat<S1, S2>::type;

template<typename T, T N> struct gen_seq;
template<typename T, T N> using make_integer_sequence = typename gen_seq<T, N>::type;

template<typename T, T N>
struct gen_seq : Concat<make_integer_sequence<T, N/2>
                        , make_integer_sequence<T, N - N/2>>{};

template<> struct gen_seq<std::size_t, 0> : integer_sequence<std::size_t>{};
template<> struct gen_seq<std::size_t, 1> : integer_sequence<std::size_t, 0>{};

template <std::size_t... I>
using index_sequence = integer_sequence<std::size_t, I...>;

template <std::size_t I>
using make_index_sequence = make_integer_sequence<std::size_t, I>;

} } }

#endif
