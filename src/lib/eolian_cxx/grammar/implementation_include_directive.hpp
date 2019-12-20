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
#ifndef EOLIAN_CXX_IMPLEMENTATION_INCLUDE_DIRECTIVE_HH
#define EOLIAN_CXX_IMPLEMENTATION_INCLUDE_DIRECTIVE_HH

#include "grammar/generator.hpp"
#include "grammar/klass_def.hpp"

#include "grammar/string.hpp"
#include "grammar/indentation.hpp"
#include "grammar/list.hpp"
#include "grammar/alternative.hpp"
#include "grammar/type.hpp"
#include "grammar/parameter.hpp"
#include "grammar/function_declaration.hpp"

namespace efl { namespace eolian { namespace grammar {

struct implementation_include_directive_generator
{
   template <typename OutputIterator, typename Context>
   bool generate(OutputIterator sink, attributes::klass_def const& cls, Context const& ctx) const
   {
     return as_generator("#include \"" << string << ".impl.hh\"\n")
       .generate(sink, std::string(eolian_object_file_get((const Eolian_Object *)get_klass(get_klass_name(cls, unit_from_context(ctx))))), add_lower_case_context(ctx));
   }
   template <typename OutputIterator, typename Context>
   bool generate(OutputIterator sink, std::string const& cls, Context const& ctx) const
   {
     return as_generator("#include \"" << string << ".impl.hh\"\n")
       .generate(sink, cls, add_lower_case_context(ctx));
   }
};

template <>
struct is_eager_generator<implementation_include_directive_generator> : std::true_type {};
template <>
struct is_generator<implementation_include_directive_generator> : std::true_type {};

namespace type_traits {
template <>
struct attributes_needed<implementation_include_directive_generator> : std::integral_constant<int, 1> {};
}
      
implementation_include_directive_generator const implementation_include_directive = {};
      
} } }

#endif
