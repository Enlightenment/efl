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
#ifndef EOLIAN_CXX_ATTRIBUTES_HH
#define EOLIAN_CXX_ATTRIBUTES_HH

#include <tuple>
#include <type_traits>
#include "grammar/type_traits.hpp"

namespace efl { namespace eolian { namespace grammar {

namespace attributes {

struct unused_type {
  unused_type() = default;
  unused_type(std::nullptr_t) {}
};
unused_type const unused = {};
        
template <int N, typename Tuple, typename Enable = void>
struct tuple_element;

template <int N, typename Tuple>
struct tuple_element<N, Tuple, typename std::enable_if
                     <type_traits::is_std_tuple<Tuple>::value>::type> : std::tuple_element<N, Tuple>
{};
        
template <int N, typename Tuple>
typename tuple_element<N, typename std::remove_const<Tuple>::type>::type get_adl(Tuple& tuple)
{
  using std::get;
  return get<N>(tuple);
}
  
template <int I, typename Tuple>
struct lazy_offset_tuple
{
   lazy_offset_tuple(Tuple& tuple)
     : tuple(tuple) {}

   template <int N>
   typename tuple_element<N+I, typename std::remove_const<Tuple>::type>::type get() const
   {
      return attributes::get_adl<N+I>(tuple);
   }
  
   Tuple& tuple;
};

}
namespace type_traits {
template <int N, typename T>
struct is_tuple<attributes::lazy_offset_tuple<N, T>> : std::true_type {};
template <int N,  typename T>
struct is_explicit_tuple<attributes::lazy_offset_tuple<N, T>> : std::true_type {};
}
namespace attributes {
  
template <typename T>
struct infinite_tuple
{
  T object;
};

template <typename T>
infinite_tuple<T> make_infinite_tuple(T&& object)
{
  return infinite_tuple<T>{std::forward<T>(object)};
}

template <int N, typename T>
struct tuple_element<N, infinite_tuple<T>>
{
  typedef T type;
  static type& get(infinite_tuple<T>& tuple) { return tuple.object; }
  static type const& get(infinite_tuple<T> const& tuple) { return tuple.object; }
};
template <int N, typename T>
struct tuple_element<N, infinite_tuple<T> const> : tuple_element<N, infinite_tuple<T>> {};
template <int N, typename T>
typename tuple_element<N, infinite_tuple<T>>::type&
get(infinite_tuple<T>& tuple)
{
  return tuple_element<N, infinite_tuple<T>>::get(tuple);
}
template <int N, typename T>
typename tuple_element<N, infinite_tuple<T>>::type const&
get(infinite_tuple<T> const& tuple)
{
  return tuple_element<N, infinite_tuple<T>>::get(tuple);
}
  
}

namespace type_traits {
template <typename T>
struct is_tuple<attributes::infinite_tuple<T>> : std::true_type {};
template <typename T>
struct is_explicit_tuple<attributes::infinite_tuple<T>> : std::true_type {};
}
namespace attributes {

template <int N, int I, typename Tuple>
struct tuple_element<N, lazy_offset_tuple<I, Tuple>> : tuple_element<N+I, typename std::remove_const<Tuple>::type>
{};
template <int N, int I, typename Tuple>
struct tuple_element<N, lazy_offset_tuple<I, Tuple> const> : tuple_element<N+I, typename std::remove_const<Tuple>::type>
{};

template <int N, int I, typename Tuple>
typename tuple_element<N, lazy_offset_tuple<I, Tuple> const>::type
 get(lazy_offset_tuple<I, Tuple> const& tuple)
{
   return tuple.template get<N>();
}
        
template <typename Tuple>
lazy_offset_tuple<1, Tuple> pop_front(Tuple& tuple, typename std::enable_if<type_traits::is_tuple<Tuple>::value>::type* = 0)
{
   return lazy_offset_tuple<1, Tuple>(tuple);
}

template <int N, typename Tuple>
lazy_offset_tuple<N + 1, Tuple> pop_front(lazy_offset_tuple<N, Tuple> tuple, typename std::enable_if<type_traits::is_tuple<Tuple>::value>::type* = 0)
{
   return lazy_offset_tuple<N + 1, Tuple>(tuple.tuple);
}

template <int I, typename Tuple>
lazy_offset_tuple<I, Tuple> pop_front_n(Tuple& tuple, typename std::enable_if<type_traits::is_tuple<Tuple>::value && I>::type* = 0)
{
   return lazy_offset_tuple<I, Tuple>(tuple);
}
template <int I, typename Tuple>
Tuple& pop_front_n(Tuple& tuple, typename std::enable_if<I == 0>::type* = 0)
{
   return tuple;
}

template <int I, int N, typename Tuple>
lazy_offset_tuple<N + I, Tuple> pop_front_n(lazy_offset_tuple<N, Tuple> tuple, typename std::enable_if<type_traits::is_tuple<Tuple>::value>::type* = 0)
{
   return lazy_offset_tuple<N + I, Tuple>(tuple.tuple);
}

template <typename Generator, typename OutputIterator, typename Attribute, typename Context>
bool generate(Generator const& gen, OutputIterator sink, Attribute const& attribute, Context const& context
              , typename std::enable_if
              <type_traits::is_explicit_tuple<Attribute>::value
              && !type_traits::accepts_tuple<Generator>::value
              && type_traits::attributes_needed<Generator>::value != 0
              >::type* = 0)
{
   return gen.generate(sink, get<0>(attribute), context);
}

template <typename Generator, typename OutputIterator, typename Attribute, typename Context>
bool generate(Generator const& gen, OutputIterator sink, Attribute const& attribute, Context const& context
              , typename std::enable_if
              <type_traits::is_explicit_tuple<Attribute>::value
              && type_traits::accepts_tuple<Generator>::value
              && type_traits::attributes_needed<Generator>::value != 0
              >::type* = 0)
{
   return gen.generate(sink, attribute, context);
}
  
template <typename Generator, typename OutputIterator, typename Attribute, typename Context>
bool generate(Generator const& gen, OutputIterator sink, Attribute const&
              , Context const& context
              , typename std::enable_if
              <type_traits::attributes_needed<Generator>::value == 0
              >::type* = 0)
{
   return gen.generate(sink, unused, context);
}

template <typename Generator, typename OutputIterator, typename Attribute, typename Context>
bool generate(Generator const& gen, OutputIterator sink, Attribute const& attribute, Context const& context
              , typename std::enable_if
              <!type_traits::is_explicit_tuple<Attribute>::value
              && type_traits::attributes_needed<Generator>::value != 0
              >::type* = 0)
{
   return gen.generate(sink, attribute, context);
}
  
} } } }

#endif
