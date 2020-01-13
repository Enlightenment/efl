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
#ifndef EOLIAN_MONO_ENUM_DEFINITION_HH
#define EOLIAN_MONO_ENUM_DEFINITION_HH

#include "grammar/generator.hpp"
#include "grammar/klass_def.hpp"
#include "grammar/indentation.hpp"
#include "grammar/list.hpp"
#include "grammar/alternative.hpp"
#include "type.hh"
#include "name_helpers.hh"
#include "using_decl.hh"

namespace eolian_mono {

struct enum_definition_generator
{
  template <typename OutputIterator, typename Context>
  bool generate(OutputIterator sink, attributes::enum_def const& enum_, Context const& context) const
  {
     auto options = efl::eolian::grammar::context_find_tag<options_context>(context);

     if(!options.want_beta && enum_.is_beta)
       return true;

     if(!name_helpers::open_namespaces(sink, enum_.namespaces, context))
       return false;

     std::string enum_name = name_helpers::typedecl_managed_name(enum_);
     std::string flags_attribute;

     // CA1717
     if (utils::ends_with(enum_name, "Flags") || utils::ends_with(enum_name, "InputHints")) // Special provision while Text api is revamped
       flags_attribute = "[Flags]";

     if(!as_generator
        (
         documentation(1)
         << scope_tab << flags_attribute << "\n"
         << scope_tab << "[Efl.Eo.BindingEntity]\n"
         << scope_tab << "public enum " << enum_name << "\n"
         << scope_tab << "{\n"
         )
        .generate(sink, enum_, context))
       return false;

     // iterate enum fiels
     for(auto first = std::begin(enum_.fields)
             , last = std::end(enum_.fields); first != last; ++first)
       {
          auto name = name_helpers::enum_field_managed_name((*first).name);
          auto literal = (*first).value.literal;
          if (!as_generator
              (
               documentation(2)
               << scope_tab(2) << string << " = " << string << ",\n"
              )
              .generate(sink, std::make_tuple(*first, name, literal), context))
            return false;
       }

     if(!as_generator(scope_tab << "}\n").generate(sink, attributes::unused, context)) return false;

     if(!name_helpers::close_namespaces(sink, enum_.namespaces, context))
       return false;

     return true;
  }
};

enum_definition_generator const enum_definition = {};

}

namespace efl { namespace eolian { namespace grammar {

template <>
struct is_eager_generator< ::eolian_mono::enum_definition_generator> : std::true_type {};
template <>
struct is_generator< ::eolian_mono::enum_definition_generator> : std::true_type {};

namespace type_traits {
template <>
struct attributes_needed< ::eolian_mono::enum_definition_generator> : std::integral_constant<int, 1> {};
}
      
} } }

#endif
