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
#ifndef EOLIAN_CXX_ALTERNATIVE_HH
#define EOLIAN_CXX_ALTERNATIVE_HH

#include "grammar/generator.hpp"
#include "grammar/meta.hpp"

namespace efl { namespace eolian { namespace grammar {

template <typename L, typename R>
struct alternative_generator
{
   template <typename OutputIterator, typename...Args, typename Context>
   bool generate(OutputIterator /*sink*/, eina::variant<Args...> const& /*attribute*/, Context const& /*context*/) const
   {
      // return grammar::alternative_sequence(left, right, sink, attribute);
      return false;
   }
   template <typename OutputIterator, typename Attribute, typename...Args, typename Context>
   bool generate(OutputIterator sink, Attribute const& attribute, Context const& context) const
   {
      if(!attributes::generate(as_generator(left), sink, attribute, context))
        return attributes::generate(as_generator(right), sink, attribute, context);
      else
        return true;
   }

   L left;
   R right;
};

template <typename L, typename R>
struct is_eager_generator<alternative_generator<L, R> > : std::true_type {};
template <typename L, typename R>
struct is_generator<alternative_generator<L, R> > : std::true_type {};

namespace type_traits {
template  <typename  L, typename R>
struct attributes_needed<alternative_generator<L, R> > : std::integral_constant
  <int, meta::max<attributes_needed<L>::value, attributes_needed<R>::value>::value> {};
template <typename L, typename R>
struct accepts_tuple<alternative_generator<L, R> > : std::true_type {};
}
      
template <typename L, typename R>
typename std::enable_if<grammar::is_generator<L>::value && grammar::is_generator<R>::value, alternative_generator<L, R>>::type
operator|(L l, R r)
{
   return alternative_generator<L, R>{l, r};
}

} } }

#endif
