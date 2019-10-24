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
#ifndef EOLIAN_CXX_TYPES_DEFINITION_HH
#define EOLIAN_CXX_TYPES_DEFINITION_HH

#include "header_guards.hpp"
#include "eps.hpp"
#include "string.hpp"
#include "sequence.hpp"
#include "kleene.hpp"
#include "header_include_directive.hpp"
#include "type_function_declaration.hpp"

namespace efl { namespace eolian { namespace grammar {

struct types_definition_generator
{
   template <typename OutputIterator, typename Context>
   bool generate(OutputIterator sink, std::vector<attributes::function_def> const& functions, Context const& ctx) const
   {
      if(!as_generator(*(type_function_declaration()))
            .generate(sink, functions, ctx))
        return false;
      return true;
   }
};

template <>
struct is_eager_generator<types_definition_generator> : std::true_type {};

namespace type_traits {
template <>
struct attributes_needed<types_definition_generator> : std::integral_constant<int, 1> {};
}

types_definition_generator const types_definition = {};

} } }

#endif
