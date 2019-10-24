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
#ifndef EOLIAN_CXX_SEQUENCE_HH
#define EOLIAN_CXX_SEQUENCE_HH

#include "grammar/generator.hpp"

namespace efl { namespace eolian { namespace grammar {

template <typename L, typename R>
struct sequence_generator;
      
template <typename T, typename Enable = void>
struct sequence_size : std::integral_constant<int, 0> {};

template <typename L, typename R>
struct sequence_size<sequence_generator<L, R>> : std::integral_constant<int, 1 + sequence_size<L>::value> {};
      
template <typename L, typename R, typename OutputIterator, typename Attribute, typename Context>
bool generate_sequence(L const& l, R const& r
                       , OutputIterator sink, Attribute const& attr, Context const& context
                       , typename std::enable_if<type_traits::is_tuple<Attribute>::value>::type* = 0)
{
   auto gen_left = as_generator(l);
   bool b = attributes::generate(gen_left, sink, attr, context);
   if(b)
     {
        return attributes::generate(as_generator(r), sink
                                    , attributes::pop_front_n<type_traits::attributes_needed<decltype(gen_left)>::value>
                                    (attr), context);
     }
   else
     return false;
}

template <typename L, typename R, typename OutputIterator, typename Attribute, typename Context>
bool generate_sequence(L const& l, R const& r
                       , OutputIterator sink, Attribute const& attr, Context const& context
                       , typename std::enable_if
                       <
                       !type_traits::is_tuple<Attribute>::value
                       && type_traits::attributes_needed<L>::value == 0
                       && type_traits::attributes_needed<R>::value == 1
                       >::type* = 0)
{
   bool b = as_generator(l).generate(sink, attributes::unused, context);
   if(b)
     {
        return as_generator(r).generate(sink, attr, context);
     }
   else
     return false;
}

template <typename L, typename R, typename OutputIterator, typename Attribute, typename Context>
bool generate_sequence(L const& l, R const& r
                       , OutputIterator sink, Attribute const&, Context const& context
                       , typename std::enable_if
                       <
                       !type_traits::is_tuple<Attribute>::value
                       && type_traits::attributes_needed<L>::value == 0
                       && type_traits::attributes_needed<R>::value == 0
                       >::type* = 0)
{
   bool b = as_generator(l).generate(sink, attributes::unused, context);
   if(b)
     {
        return as_generator(r).generate(sink, attributes::unused, context);
     }
   else
     return false;
}
      
template <typename L, typename R, typename OutputIterator, typename Attribute, typename Context>
bool generate_sequence(L const& l, R const& r
                       , OutputIterator sink, Attribute const& attr, Context const& context
                       , typename std::enable_if
                       <
                       !type_traits::is_tuple<Attribute>::value
                       && type_traits::attributes_needed<L>::value == 1
                       && type_traits::attributes_needed<R>::value == 0
                       >::type* = 0)
{
   bool b = as_generator(l).generate(sink, attr, context);
   if(b)
     {
        return as_generator(r).generate(sink, attributes::unused, context);
     }
   else
     return false;
}

template <typename L, typename R, typename OutputIterator, typename Context>
bool generate_sequence(L const& l, R const& r, OutputIterator sink, attributes::unused_type, Context const& context)
{
   bool b = as_generator(l).generate(sink, attributes::unused, context);
   if(b)
     {
        return as_generator(r).generate(sink, attributes::unused, context);
     }
   else
     return false;
}
      
template <typename L, typename R>
struct sequence_generator
{
   template <typename OutputIterator, typename Attribute, typename Context>
   bool generate(OutputIterator sink, Attribute const& attribute, Context const& context) const
   {
      return grammar::generate_sequence(left, right, sink, attribute, context);
   }

   L left;
   R right;
};

template <typename L, typename R>
struct is_eager_generator<sequence_generator<L, R> > : std::true_type {};
template <typename L, typename R>
struct is_generator<sequence_generator<L, R> > : std::true_type {};

namespace type_traits {
template  <typename  L, typename R>
struct attributes_needed<sequence_generator<L, R> > : std::integral_constant
  <int, attributes_needed<L>::value + attributes_needed<R>::value> {};
template <typename L, typename R>
struct accepts_tuple<sequence_generator<L, R> > : std::true_type {};
}
      
template <typename L, typename R>
typename std::enable_if<grammar::is_generator<L>::value && grammar::is_generator<R>::value, sequence_generator<L, R>>::type
operator<<(L l, R r)
{
   return sequence_generator<L, R>{l, r};
}

} } }

#endif
