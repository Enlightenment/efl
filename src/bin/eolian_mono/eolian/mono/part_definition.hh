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
#ifndef EOLIAN_MONO_PART_DEFINITION_HH
#define EOLIAN_MONO_PART_DEFINITION_HH

#include <Eina.hh>

#include "grammar/generator.hpp"
#include "grammar/klass_def.hpp"
#include "grammar/indentation.hpp"

#include "utils.hh"
#include "name_helpers.hh"
#include "documentation.hh"

namespace eolian_mono {

struct part_definition_generator
{
  template <typename OutputIterator, typename Context>
  bool generate(OutputIterator sink, attributes::part_def const& part, Context const& context) const
  {
     if (blacklist::is_class_blacklisted(part.klass, context))
       return true;

     auto part_klass_name = name_helpers::klass_full_concrete_or_interface_name(part.klass);
     return as_generator(documentation(2)
                       << scope_tab(2) << "public " << part_klass_name << " " << name_helpers::managed_part_name(part) << "\n"
                       << scope_tab(2) << "{\n"
                       << scope_tab(2) << scope_tab << "get\n"
                       << scope_tab(2) << scope_tab << "{\n"
                       << scope_tab(2) << scope_tab << scope_tab << "return GetPart(\"" << part.name << "\") as " << part_klass_name << ";\n"
                       << scope_tab(2) << scope_tab << "}\n"
                       << scope_tab(2) << "}\n"
            ).generate(sink, part.documentation, context);
  }

} const part_definition {};

struct part_extension_method_definition_generator
{
   template<typename OutputIterator, typename Context>
   bool generate(OutputIterator sink, attributes::part_def const& part, Context context) const
   {
      if (blacklist::is_class_blacklisted(part.klass, context))
        return true;

      auto part_klass_name = name_helpers::klass_full_concrete_or_interface_name(part.klass);
      /* auto unit = (const Eolian_Unit*) context_find_tag<eolian_state_context>(context).state; */
      /* auto klass = get_klass(part.klass, unit); */

      std::string bindableClass = "Efl.BindablePart";

      // Efl.Content parts should be bound only throught FactoryBind
      attributes::klass_def c(get_klass(part.klass, cls.unit), cls.unit);
      if (helpers::inherits_from(c, "Efl.IContent"))
        bindableClass = "Efl.BindableFactoryPart";

      if (!as_generator(
                scope_tab(2) << "public static " << bindableClass << "<" << part_klass_name << "> " << name_helpers::managed_part_name(part) << "<T>(this Efl.Ui.ItemFactory<T> fac, Efl.Csharp.ExtensionTag<"
                            << name_helpers::klass_full_concrete_or_interface_name(cls)
                            << ", T> x=null) where T : " << name_helpers::klass_full_concrete_or_interface_name(cls) << "\n"
                << scope_tab(2) << "{\n"
                << scope_tab(2) << scope_tab << "return new " << bindableClass << "<" << part_klass_name << ">(\"" << part.name << "\", fac);\n"
                << scope_tab(2) << "}\n\n"
            ).generate(sink, attributes::unused, context))
        return false;

      return true;
   }

   grammar::attributes::klass_def const& cls;
};

part_extension_method_definition_generator part_extension_method_definition (grammar::attributes::klass_def const& cls)
{
  return {cls};
}

}

namespace efl { namespace eolian { namespace grammar {

template <>
struct is_eager_generator< ::eolian_mono::part_definition_generator> : std::true_type {};
template <>
struct is_generator< ::eolian_mono::part_definition_generator> : std::true_type {};

template <>
struct is_eager_generator< ::eolian_mono::part_extension_method_definition_generator> : std::true_type {};
template <>
struct is_generator< ::eolian_mono::part_extension_method_definition_generator> : std::true_type {};

namespace type_traits {
template <>
struct attributes_needed< ::eolian_mono::part_definition_generator> : std::integral_constant<int, 1> {};

template <>
struct attributes_needed< ::eolian_mono::part_extension_method_definition_generator> : std::integral_constant<int, 1> {};
}
} } }

#endif

