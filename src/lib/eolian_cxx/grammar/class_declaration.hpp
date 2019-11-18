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
#ifndef EOLIAN_CXX_CLASS_DECLARATION_HH
#define EOLIAN_CXX_CLASS_DECLARATION_HH

#include "grammar/generator.hpp"
#include "grammar/klass_def.hpp"

#include "grammar/indentation.hpp"
#include "grammar/list.hpp"
#include "grammar/alternative.hpp"
#include "grammar/type.hpp"
#include "grammar/parameter.hpp"
#include "grammar/function_declaration.hpp"

namespace efl { namespace eolian { namespace grammar {

struct class_declaration_generator
{
   bool type_traits;
  
   template <typename OutputIterator, typename Context>
   bool generate(OutputIterator sink, attributes::klass_def const& cls, Context const& context) const
   {
     std::vector<std::string> cpp_namespaces = attributes::cpp_namespaces(cls.namespaces);

     std::string guard_symbol;

     if(!as_generator(*(string << "_") << string << "_FWD_GUARD")
        .generate(std::back_inserter(guard_symbol)
                  , std::make_tuple(cpp_namespaces, cls.cxx_name), add_upper_case_context(context)))
       return false;

     if(!as_generator(   "#ifndef " << guard_symbol << "\n"
                      << "#define " << guard_symbol << "\n")
        .generate(sink, std::make_tuple(), context))
       return false;
     
     auto open_namespace = *("namespace " << string << " { ") << "\n";
     if(!as_generator(open_namespace).generate(sink, cpp_namespaces, add_lower_case_context(context))) return false;

     if(!as_generator
        (
         "struct " << string << ";\n"
        ).generate(sink, cls.cxx_name, context)) return false;

     auto close_namespace = *(lit("} ")) << "\n";
     if(!as_generator(close_namespace).generate(sink, cpp_namespaces, context)) return false;

     // if(type_traits)
       if(!as_generator
          (
           "namespace efl { namespace eo { template<> struct is_eolian_object< "
           "::" << *(lower_case[string] << "::") << string << "> : ::std::true_type {}; } }\n"
           "namespace efl { namespace eo { template<> struct is_eolian_object< "
           "::" << *(lower_case[string] << "::") << string << "&> : ::std::true_type {}; } }\n"
           "namespace efl { namespace eo { template<> struct is_eolian_object< "
           "::" << *(lower_case[string] << "::") << string << " const> : ::std::true_type {}; } }\n"
           "namespace efl { namespace eo { template<> struct is_eolian_object< "
           "::" << *(lower_case[string] << "::") << string << " const&> : ::std::true_type {}; } }\n"
           ).generate(sink, std::make_tuple
                      (
                       cpp_namespaces, cls.cxx_name, cpp_namespaces, cls.cxx_name
                       , cpp_namespaces, cls.cxx_name, cpp_namespaces, cls.cxx_name
                      ), context)) return false;

     if(!as_generator("#endif\n")
        .generate(sink, std::make_tuple(), context))
       return false;

     
     return true;
   }
};

template <>
struct is_eager_generator<class_declaration_generator> : std::true_type {};
template <>
struct is_generator<class_declaration_generator> : std::true_type {};

namespace type_traits {
template <>
struct attributes_needed<class_declaration_generator> : std::integral_constant<int, 1> {};
}
      
class_declaration_generator const class_declaration = {true};
class_declaration_generator const class_forward_declaration = {false};
      
} } }

#endif
