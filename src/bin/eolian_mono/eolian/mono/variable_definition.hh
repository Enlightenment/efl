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
#ifndef EOLIAN_MONO_VARIABLE_DEFINITION_HH
#define EOLIAN_MONO_VARIABLE_DEFINITION_HH

#include <Eina.hh>

#include "grammar/generator.hpp"
#include "grammar/klass_def.hpp"

#include "grammar/indentation.hpp"
#include "grammar/list.hpp"
#include "grammar/alternative.hpp"
#include "grammar/attribute_reorder.hpp"
#include "logging.hh"
#include "type.hh"
#include "name_helpers.hh"
#include "helpers.hh"
#include "function_helpers.hh"
#include "marshall_type.hh"
#include "parameter.hh"
#include "documentation.hh"
#include "using_decl.hh"
#include "generation_contexts.hh"
#include "blacklist.hh"

namespace eolian_mono {


struct constant_definition_generator
{
  template<typename OutputIterator, typename Context>
  bool generate(OutputIterator sink, attributes::constant_def constant, Context const& context) const
  {
    // Open static partial class
    if (!name_helpers::open_namespaces(sink, constant.namespaces, context))
      return false;

    if (!as_generator("public static partial class Constants\n{\n").generate(sink, attributes::unused, context))
      return false;

    std::string literal;
    if (constant.expression_value.type == EOLIAN_EXPR_NULL)
      literal = "null";
    else if (constant.expression_value.type == EOLIAN_EXPR_BOOL)
      literal = (constant.expression_value.value.b ? "true" : "false");
    else
      {
        auto lit = ::eolian_expression_value_to_literal(&constant.expression_value);
        if (!lit)
          return false;

        literal = lit;
        ::eina_stringshare_del(lit);

        // Cleanup suffix. Roslyn does not accept ULL/LL as it has only longs.
        if (utils::ends_with(literal, "LL") || utils::ends_with(literal, "ll"))
          literal = literal.substr(0, literal.size() -1);

      }

    // declare variable
    if (!as_generator(documentation(1)
                      << scope_tab(1)
                      << "public static readonly " << type
                      << " " << utils::remove_all(constant.name, '_')
                      << " = " << literal << ";\n")
          .generate(sink, std::make_tuple(constant, constant.base_type), context))
      return false;

    // FIXME missing documentation generator

    // Close static partial class
    if (!as_generator("}\n").generate(sink, attributes::unused, context))
      return false;

    if (!name_helpers::close_namespaces(sink, constant.namespaces, context))
      return false;

    return true;


  }
} const constant_definition;

}

namespace efl { namespace eolian { namespace grammar {

template <>
struct is_eager_generator< ::eolian_mono::constant_definition_generator> : std::true_type {};
template <>
struct is_generator< ::eolian_mono::constant_definition_generator> : std::true_type {};

namespace type_traits {
template <>
struct attributes_needed< ::eolian_mono::constant_definition_generator> : std::integral_constant<int, 1> {};
}

} } }

#endif
