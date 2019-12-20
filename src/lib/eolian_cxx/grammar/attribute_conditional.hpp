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
#ifndef EOLIAN_CXX_ATTRIBUTE_CONDITIONAL_HH
#define EOLIAN_CXX_ATTRIBUTE_CONDITIONAL_HH

#include "grammar/generator.hpp"

namespace efl { namespace eolian { namespace grammar {

template <typename F, typename G>
struct functional_attribute_conditional_generator
{
   template <typename OutputIterator, typename Attribute, typename Context>
   bool generate(OutputIterator sink, Attribute const& attribute, Context const& ctx) const
   {
     if(f(attribute))
       return as_generator(g).generate(sink, attribute, ctx);
     else
       return false;
   }

   F f;
   G g;
};

template <typename F>
struct functional_attribute_conditional_directive
{
  template <typename G>
  functional_attribute_conditional_generator<F, G> operator[](G g) const
  {
    return {f, g};
  }

  template <typename OutputIterator, typename Attribute, typename Context>
  bool generate(OutputIterator, Attribute const& attribute, Context const&) const
  {
    return f(attribute);
  }
  
  F f;
};

struct attribute_conditional_terminal
{
  template <typename F>
  functional_attribute_conditional_directive<F> operator()(F f) const
  {
    return {f};
  }
} const attribute_conditional = {};

template <typename F, typename G>
struct is_eager_generator<functional_attribute_conditional_generator<F, G>> : std::true_type {};
template <typename F>
struct is_eager_generator<functional_attribute_conditional_directive<F>> : std::true_type {};
template <typename F, typename G>
struct is_generator<functional_attribute_conditional_generator<F, G>> : std::true_type {};
template <typename F>
struct is_generator<functional_attribute_conditional_directive<F>> : std::true_type {};
      
namespace type_traits {
template <typename F, typename G>
struct attributes_needed<functional_attribute_conditional_generator<F, G>>
  : std::conditional<(attributes_needed<G>::value >= 1)
                     , attributes_needed<G>
                     , std::integral_constant<int, 1>>::type {};  
template <typename F>
struct attributes_needed<functional_attribute_conditional_directive<F>> : std::integral_constant<int, 1> {};  
}
      
} } }

#endif
