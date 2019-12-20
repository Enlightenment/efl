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
#ifndef EOLIAN_CXX_BASE_CLASS_DEFINITION_HH
#define EOLIAN_CXX_BASE_CLASS_DEFINITION_HH

#include "grammar/generator.hpp"
#include "grammar/klass_def.hpp"

#include "grammar/indentation.hpp"
#include "grammar/list.hpp"
#include "grammar/alternative.hpp"
#include "grammar/type.hpp"
#include "grammar/parameter.hpp"
#include "grammar/function_declaration.hpp"
#include "grammar/namespace.hpp"
#include "grammar/case.hpp"
#include "grammar/address_of.hpp"
#include "grammar/attribute_reorder.hpp"
#include "grammar/part_declaration.hpp"

namespace efl { namespace eolian { namespace grammar {

struct base_class_definition_generator
{
   template <typename OutputIterator, typename Context>
   bool generate(OutputIterator sink, attributes::klass_def const& cls, Context const& context) const
   {
     std::vector<std::string> cpp_namespaces = attributes::cpp_namespaces(cls.namespaces);
     // static_assert(std::is_same<OutputIterator, void>::value, "");
     auto open_namespace = *("namespace " << string << " { ") << "\n";
     if(!as_generator(open_namespace).generate
        (sink, cpp_namespaces, add_lower_case_context(context))) return false;

     if(!as_generator
        (
         "struct " << string << " {\n"
         ).generate(sink, cls.cxx_name, context)) return false;

     if(!as_generator(*(function_declaration(get_klass_name(cls))))
        .generate(sink, cls.functions, context)) return false;

     if(!as_generator(*(part_declaration << ";\n"))
        .generate(sink, cls.parts, context)) return false;

     // static Efl_Class const* _eo_class();
     std::string suffix;
     switch(cls.type)
       {
       case attributes::class_type::regular:
       case attributes::class_type::abstract_:
         suffix = "CLASS";
         break;
       case attributes::class_type::mixin:
         suffix = "MIXIN";
         break;
       case attributes::class_type::interface_:
         suffix = "INTERFACE";
         break;
       }

     if(!as_generator
        (
            scope_tab << "static Efl_Class const* _eo_class()\n"
            << scope_tab << "{\n"
            << scope_tab << scope_tab << "return "
        ).generate(sink,  attributes::unused, context)) return false;
     if(!as_generator
        (*(lower_case[string] << "_") << string << "_" << string)
        .generate(sink, std::make_tuple(cls.namespaces, cls.eolian_name, suffix), add_upper_case_context(context)))
       return false;
     if(!as_generator(";\n" << scope_tab << "}\n").generate(sink, attributes::unused, context)) return false;

     if(!as_generator(
         scope_tab << "Eo* _eo_ptr() const { return *(reinterpret_cast<Eo **>"
              << "(const_cast<" << string << " *>(this))); }\n"
        ).generate(sink, cls.cxx_name, context))
       return false;

     // operator ::ns::Class_Name() const;
     // operator ::ns::Class_Name&();
     // operator ::ns::Class_Name const&() const;
     auto class_name = *(lit("::") << lower_case[string]) << "::" << string;
     if(!as_generator
        (
         attribute_reorder<0, 1, 0, 1, 0, 1, 0, 1>
         (
            scope_tab << "operator " << class_name << "() const;\n"
            << scope_tab << "operator " << class_name << "&();\n"
            << scope_tab << "operator " << class_name << " const&() const;\n"
         )).generate(sink, std::make_tuple(cpp_namespaces, cls.cxx_name), context))
       return false;

     // /// @cond LOCAL
     if(!as_generator(scope_tab << "/// @cond LOCAL\n").generate(sink, attributes::unused, context)) return false;
     
     if(!as_generator(address_of).generate(sink, cls, context)) return false;

     // /// @endcond
     if(!as_generator(scope_tab << "/// @endcond\n").generate(sink, attributes::unused, context)) return false;

     if(!as_generator("};\n").generate(sink, attributes::unused, context)) return false;
     auto close_namespace = *(lit("} ")) << "\n";
     if(!as_generator(close_namespace).generate(sink, cpp_namespaces, context)) return false;
     
     return true;
   }
};

template <>
struct is_eager_generator<base_class_definition_generator> : std::true_type {};
template <>
struct is_generator<base_class_definition_generator> : std::true_type {};

namespace type_traits {
template <>
struct attributes_needed<base_class_definition_generator> : std::integral_constant<int, 1> {};
}
      
base_class_definition_generator const base_class_definition = {};
      
} } }

#endif
