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
#ifndef EOLIAN_CXX_KLEENE_HH
#define EOLIAN_CXX_KLEENE_HH

#include "grammar/generator.hpp"

namespace efl { namespace eolian { namespace grammar {

template <typename Generator>
struct kleene_generator
{
   kleene_generator(Generator g)
     : generator(g) {}
  
  template <typename OutputIterator, typename Attribute, typename Context>
  bool generate(OutputIterator sink, Attribute const& attribute, Context const& context) const
   {
      bool b;
      for(auto&& c : attribute)
        {
           b = as_generator(generator).generate(sink, c, context);
           if(!b)
             return false;
        }
      return true;
   }

   Generator generator;
};

template <typename Generator>
struct is_eager_generator<kleene_generator<Generator> > : std::true_type {};
template <typename Generator>
struct is_generator<kleene_generator<Generator> > : std::true_type {};

namespace type_traits {
template  <typename G>
struct attributes_needed<kleene_generator<G> > : std::integral_constant<int, 1> {};
}
      
template <typename Generator>
typename std::enable_if<grammar::is_generator<Generator>::value, kleene_generator<Generator>>::type
operator*(Generator g)
{
   return kleene_generator<Generator>{g};
}

} } }

#endif
