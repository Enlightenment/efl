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
#ifndef EOLIAN_MONO_FUNCTION_DECLARATION_HH
#define EOLIAN_MONO_FUNCTION_DECLARATION_HH

#include "grammar/generator.hpp"
#include "grammar/klass_def.hpp"

#include "grammar/indentation.hpp"
#include "grammar/list.hpp"
#include "grammar/alternative.hpp"
#include "type.hh"
#include "parameter.hh"
#include "name_helpers.hh"
#include "using_decl.hh"
#include "blacklist.hh"

namespace eolian_mono {

struct function_declaration_generator
{
  template <typename OutputIterator, typename Context>
  bool generate(OutputIterator sink, attributes::function_def const& f, Context const& context) const
  {
    if(blacklist::is_function_blacklisted(f, context) || f.is_static)
      return true;

    // C# interfaces can't have non-public members
    if(f.scope != attributes::member_scope::scope_public)
      return true;

    if(!as_generator(documentation(2)).generate(sink, f, context))
      return false;

    return as_generator
      (scope_tab(2) << eolian_mono::type(true) << " " << string << "(" << (parameter % ", ") << ");\n\n")
      .generate(sink, std::make_tuple(f.return_type, name_helpers::managed_method_name(f), f.parameters), context);
  }
};

function_declaration_generator const function_declaration = {};

}

namespace efl { namespace eolian { namespace grammar {

template <>
struct is_eager_generator< ::eolian_mono::function_declaration_generator> : std::true_type {};
template <>
struct is_generator< ::eolian_mono::function_declaration_generator> : std::true_type {};

namespace type_traits {
template <>
struct attributes_needed< ::eolian_mono::function_declaration_generator> : std::integral_constant<int, 1> {};
}
      
} } }

#endif
