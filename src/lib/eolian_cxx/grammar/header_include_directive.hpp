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
#ifndef EOLIAN_CXX_HEADER_INCLUDE_DIRECTIVE_HH
#define EOLIAN_CXX_HEADER_INCLUDE_DIRECTIVE_HH

#include "generator.hpp"

namespace efl { namespace eolian { namespace grammar {

struct header_include_directive_generator
{
   template <typename OutputIterator, typename Attribute, typename Context>
   bool generate(OutputIterator sink, Attribute const& attribute, Context const&) const
   {
      const char include_directive[] = "#include \"";
      std::copy(include_directive, include_directive + sizeof(include_directive)-1, sink);
      std::copy(std::begin(attribute), std::end(attribute), sink);
      *sink++ = '\"';
      *sink++ = '\n';
      return true;
   }
};

template <>
struct is_eager_generator<header_include_directive_generator> : std::true_type {};
template <>
struct is_generator<header_include_directive_generator> : std::true_type {};

namespace type_traits {
template <>
struct attributes_needed<header_include_directive_generator> : std::integral_constant<int, 1> {};
}

header_include_directive_generator const header_include_directive = {};

} } }

#endif
