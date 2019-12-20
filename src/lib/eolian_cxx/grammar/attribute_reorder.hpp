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
#ifndef EOLIAN_CXX_ATTRIBUTE_REORDER_HH
#define EOLIAN_CXX_ATTRIBUTE_REORDER_HH

#include "grammar/generator.hpp"
#include "grammar/attributes.hpp"

namespace efl { namespace eolian { namespace grammar {

template <typename Tuple, int...S>
struct reorder_tuple
{
  Tuple* tuple;
};

namespace impl {

template <int N, typename T>
struct index_calc;
  
template <int N, typename Tuple, int...S>
struct index_calc<0, reorder_tuple<Tuple, N, S...>> : std::integral_constant<int, N> {};

template <int I, int N, typename Tuple, int...S>
struct index_calc<I, reorder_tuple<Tuple, N, S...>> : index_calc<I-1, reorder_tuple<Tuple, S...>> {};
  
}
namespace attributes {      
template <int N, typename Tuple, int...S>
struct tuple_element<N, reorder_tuple<Tuple, S...>>
{
  template <typename T>
  struct identity { typedef T type; };
  typedef impl::index_calc<N, reorder_tuple<Tuple, S...>> index;
  typedef typename std::conditional
    <index::value == -1
     , identity<Tuple>
     , tuple_element<index::value, typename std::remove_const<Tuple>::type>>::type::type type;
  static type const& get_impl(reorder_tuple<Tuple, S...> const& t
                              , std::integral_constant<int, -1>)
  { return *t.tuple; }
  template <int I>
  static type const& get_impl(reorder_tuple<Tuple, S...> const& t
                              , std::integral_constant<int, I>)
  { using std::get; return get<index::value>(*t.tuple); }
  static type const& get(reorder_tuple<Tuple, S...> const& t)
  { return get_impl(t, index{}); }
};
}
template <int N, typename Tuple, int...S>
typename attributes::tuple_element<N, reorder_tuple<Tuple, S...>>::type const& get(reorder_tuple<Tuple, S...>const& t)
{
  return attributes::tuple_element<N, reorder_tuple<Tuple, S...>>::get(t);
}

template <typename G, int...S>
struct attribute_reorder_generator
{
   template <typename OutputIterator, typename Attribute, typename Context>
   bool generate(OutputIterator sink, Attribute const& attribute, Context const& ctx) const
   {
     return attributes::generate(as_generator(g), sink, reorder_tuple<Attribute const, S...>{&attribute}, ctx);
   }

   G g;
};

template <typename G, int...S>
struct is_eager_generator<attribute_reorder_generator<G, S...>> : std::true_type {};
template <typename G, int...S>
struct is_generator<attribute_reorder_generator<G, S...>> : std::true_type {};

template <int...S, typename G>
attribute_reorder_generator<G, S...> attribute_reorder(G g)
{
  return {g};
}

namespace type_traits {
template <typename G, int...S>
struct attributes_needed<attribute_reorder_generator<G, S...>> : attributes_needed<G> {};
template <int...S, typename Tuple>
struct is_explicit_tuple<reorder_tuple<Tuple, S...>> : std::true_type {};
template <int...S, typename Tuple>
struct is_tuple<reorder_tuple<Tuple, S...>> : std::true_type {};
}
      
} } }

#endif
