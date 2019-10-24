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
#ifndef EOLIAN_CXX_PART_DECLARATION_HH
#define EOLIAN_CXX_PART_DECLARATION_HH

#include "grammar/generator.hpp"

#include "grammar/string.hpp"
#include "grammar/indentation.hpp"
#include "grammar/list.hpp"
#include "grammar/alternative.hpp"
#include "grammar/type.hpp"
#include "grammar/parameter.hpp"
#include "grammar/keyword.hpp"

namespace efl { namespace eolian { namespace grammar {

struct part_declaration_generator
{
   template <typename OutputIterator, typename Context>
   bool generate(OutputIterator sink, attributes::part_def const& part, Context const& ctx) const
   {
      if(!as_generator(scope_tab << "::efl::eolian::return_traits<::" << *(string << "::"))
            .generate(sink, part.klass.namespaces, add_lower_case_context(ctx)))
        return false;
      if(!as_generator(string << ">::type " << string << "() const")
            .generate(sink, std::make_tuple(part.klass.eolian_name, part.name), ctx))
        return false;

      return true;
   }
};

template <>
struct is_eager_generator<part_declaration_generator> : std::true_type {};
template <>
struct is_generator<part_declaration_generator> : std::true_type {};
namespace type_traits {
template <>
struct attributes_needed<part_declaration_generator> : std::integral_constant<int, 1> {};
}

part_declaration_generator const part_declaration = {};

} } }

#endif
