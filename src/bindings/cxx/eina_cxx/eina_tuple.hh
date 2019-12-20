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
#ifndef EFL_EINA_EINA_TUPLE_HH_
#define EFL_EINA_EINA_TUPLE_HH_

#include <eina_integer_sequence.hh>
#include <eina_logical.hh>

#include <tuple>

namespace efl { namespace eina { namespace _mpl {

template <typename A, typename... Args>
struct push_back;

template <template <typename... Args> class C, typename... Args, typename... AArgs>
struct push_back<C<Args...>, AArgs...>
{
  typedef C<Args..., AArgs...> type;
};

template <typename A, typename... Args>
struct push_front;

template <template <typename... Args> class C, typename... Args, typename... AArgs>
struct push_front<C<Args...>, AArgs...>
{
  typedef C<Args..., AArgs...> type;
};

template <typename A, std::size_t N = 1>
struct pop_front;
      
template <template <typename...> class C, typename T, typename... Args>
struct pop_front<C<T, Args...>, 1>
{
  typedef C<Args...> type;
};

template <typename A, std::size_t N>
struct pop_front : pop_front<typename pop_front<A, 1>::type, N-1>
{
};

template <typename T, typename F, std::size_t... I>
void for_each_impl(T&& t, F&& f, eina::index_sequence<I...>)
{
  std::initializer_list<int> l = { (f(std::get<I>(t)), 0)...};
  static_cast<void>(l);
}

template <typename T, typename F>
void for_each(T&& t, F&& f)
{
  _mpl::for_each_impl(t, f, eina::make_index_sequence
                      <std::tuple_size<typename std::remove_reference<T>::type>::value>());
}

template <typename T, typename Transform>
struct tuple_transform;

template <typename...T, typename Transform>
struct tuple_transform<std::tuple<T...>, Transform>
{
  typedef std::tuple<typename Transform::template apply<T>::type...> type;
};

template <typename T, typename Tuple>
struct tuple_contains;


      
template <typename T, typename...Ts>
struct tuple_contains<T, std::tuple<Ts...> >
  : _mpl::or_<std::is_same<T, Ts>::value...>
{
};

template <typename T, typename Tuple>
struct tuple_find : std::integral_constant<int, -1> {};

template <typename T, typename... Ts>
struct tuple_find<T, std::tuple<T, Ts...> > : std::integral_constant<std::size_t, 0> {};

template <typename T, typename T1, typename... Ts>
struct tuple_find<T, std::tuple<T1, Ts...> > : std::integral_constant
 <std::size_t, 1 + tuple_find<T, std::tuple<Ts...> >::value> {};
      
} } }

#endif
