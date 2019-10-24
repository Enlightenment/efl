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
#ifndef EOLIAN_CXX_HEADER_GUARDS_HH
#define EOLIAN_CXX_HEADER_GUARDS_HH

#include <utility>
#include <tuple>
#include <algorithm>

#include "grammar/generator.hpp"
#include "grammar/attributes.hpp"
#include "grammar/string.hpp"

namespace efl { namespace eolian { namespace grammar {

template <typename Generator>
struct header_guards_generator
{
   header_guards_generator(Generator generator)
     : generator(std::move(generator))
   {}

   template <typename OutputIterator, typename Attribute, typename Context>
   bool generate(OutputIterator sink, Attribute const& attribute, Context const& context) const
   {
      using std::get;
      auto&& v = get<0>(attribute);
      const char ifndef_directive[] = "#ifndef ";
      const char define_directive[] = "#define ";
      const char endif_directive[] = "#endif\n";
      std::copy(&ifndef_directive[0],
                &ifndef_directive[0] + sizeof(ifndef_directive)-1,
                sink);

      std::transform(std::begin(v), std::end(v), sink, ::toupper);
      *sink++ = '\n';

      std::copy(&define_directive[0],
                &define_directive[0] + sizeof(define_directive)-1,
                sink);
      std::transform(std::begin(v), std::end(v), sink, ::toupper);
      *sink++ = '\n';

      bool b = as_generator(generator).generate(sink, attributes::pop_front(attribute), context);
      if(!b)
        return false;
      else
        {
          std::copy(&endif_directive[0],
                    &endif_directive[0] + sizeof(endif_directive)-1,
                    sink);
          return true;
        }
   }

   Generator generator;
};

template <typename G>
struct is_eager_generator<header_guards_generator<G> > : std::true_type {};
template <typename G>
struct is_generator<header_guards_generator<G> > : std::true_type {};

namespace type_traits {
template  <typename G>
struct attributes_needed<header_guards_generator<G> >
  : std::integral_constant<int, 1 + attributes_needed<G>::value> {};
}
      
struct header_guards_directive
{
   template <typename Generator>
   header_guards_generator<Generator> operator[](Generator generator) const
   {
      return header_guards_generator<Generator>(generator);
   }
};
      
header_guards_directive const header_guards = {};
      
} } }

#endif
