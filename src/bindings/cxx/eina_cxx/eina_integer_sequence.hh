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
#ifndef EINA_CXX_EINA_INTEGER_SEQUENCE_HH
#define EINA_CXX_EINA_INTEGER_SEQUENCE_HH

#include <cstdlib>

/**
 * @addtogroup Eina_Cxx_Data_Types_Group
 *
 * @{
 */

namespace efl { namespace eina {

/**
 * @defgroup Eina_Cxx_Integer_Sequence_Group Integer Sequence
 * @ingroup Eina_Cxx_Data_Types_Group
 *
 * @{
 */

/**
 * Compile-time sequence of integers.
 */
template <typename T, T... Ints>
struct integer_sequence
{
  typedef T value_type; /**< Type of the integers. */

  /**
   * @brief Get the number of elements in the sequence.
   * @return <tt>std::size_t</tt> representing the sequence size.
   */
  static constexpr std::size_t size() { return sizeof...(Ints); }
  typedef integer_sequence<T, Ints...> type; /**< Type for the sequence instantiation. */
};

template<class S1, class S2> struct concat;

/**
 * Compile-time concatenation of two integer sequences.
 */
template<typename T, T... I1, T... I2>
struct concat<integer_sequence<T, I1...>, integer_sequence<T, I2...> >
  : integer_sequence<T, I1..., (sizeof...(I1)+I2)...> {};

template<class S1, class S2>
using Concat = typename concat<S1, S2>::type;

template<typename T, T N> struct gen_seq;

/**
 * Make a compile time sequence of integers from @c 0 to <tt>N-1</tt>.
 */
template<typename T, T N> using make_integer_sequence = typename gen_seq<T, N>::type;

template<typename T, T N>
struct gen_seq : Concat<make_integer_sequence<T, N/2>
                        , make_integer_sequence<T, N - N/2>>{};

template<> struct gen_seq<std::size_t, 0> : integer_sequence<std::size_t>{};
template<> struct gen_seq<std::size_t, 1> : integer_sequence<std::size_t, 0>{};

/**
 * Compile time sequence of indexes.
 */
template <std::size_t... I>
using index_sequence = integer_sequence<std::size_t, I...>;

/**
 * Make a compile time sequence of indexes from @c 0 to <tt>N-1</tt>.
 */
template <std::size_t I>
using make_index_sequence = make_integer_sequence<std::size_t, I>;

template <typename T, typename U>
struct pop_integer_sequence_t;

template <typename T>
struct pop_integer_sequence_t<integer_sequence<T>, integer_sequence<T> >
{
  typedef integer_sequence<T> type;
};

template <typename T, T S0, T... S>
struct pop_integer_sequence_t<integer_sequence<T>, integer_sequence<T, S0, S...> >
{
  typedef integer_sequence<T, S...> type;
};

template <typename T, T S0, T... S>
struct pop_integer_sequence_t<integer_sequence<T, S0, S...>, integer_sequence<T> >
{
  typedef integer_sequence<T> type;
};

template <typename T, T S, T... Ss1, T... Ss2>
struct pop_integer_sequence_t<integer_sequence<T, S, Ss1...>, integer_sequence<T, S, Ss2...> >
  : pop_integer_sequence_t<integer_sequence<T, Ss1...>, integer_sequence<T, Ss2...> >
{
};

template <typename T, typename U>
using pop_integer_sequence = typename pop_integer_sequence_t<T, U>::type;

/**
 * @}
 */

} }

/**
 * @}
 */

#endif
