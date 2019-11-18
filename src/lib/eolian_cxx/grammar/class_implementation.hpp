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
#ifndef EOLIAN_CXX_CLASS_IMPLEMENTATION_HH
#define EOLIAN_CXX_CLASS_IMPLEMENTATION_HH

#include "grammar/generator.hpp"
#include "grammar/klass_def.hpp"

#include "grammar/string.hpp"
#include "grammar/indentation.hpp"
#include "grammar/list.hpp"
#include "grammar/alternative.hpp"
#include "grammar/type.hpp"
#include "grammar/parameter.hpp"
#include "grammar/function_definition.hpp"
#include "grammar/namespace.hpp"
#include "grammar/type_impl.hpp"
#include "grammar/attribute_reorder.hpp"
#include "grammar/part_implementation.hpp"
#include "grammar/ignore_warning.hpp"

namespace efl { namespace eolian { namespace grammar {

struct class_implementation_generator
{
   template <typename OutputIterator, typename Context>
   bool generate(OutputIterator sink, attributes::klass_def const& cls, Context const& ctx) const
   {
     std::vector<std::string> cpp_namespaces = attributes::cpp_namespaces(cls.namespaces);
     auto base_class_name = *(lower_case[string] << "::") << string;
     auto class_name = *(lit("::") << lower_case[string]) << "::" << string;
     std::string guard_symbol;

     if(!as_generator(*(string << "_") << string << "_IMPL_HH")
        .generate(std::back_inserter(guard_symbol)
                  , std::make_tuple(cpp_namespaces, cls.cxx_name), add_upper_case_context(ctx)))
       return false;

     if(!as_generator(   "#ifndef " << guard_symbol << "\n"
                      << "#define " << guard_symbol << "\n")
        .generate(sink, std::make_tuple(), ctx))
       return false;

     if(!as_generator("\n#include \"" << *(string << "_") << string << ".eo.hh\"\n\n")
        .generate(sink, std::make_tuple(cls.namespaces, cls.cxx_name), add_lower_case_context(ctx)))
       return false;

     if(!as_generator(ignore_warning_begin).generate(sink, nullptr, ctx))
       return false;

#ifndef USE_EOCXX_INHERIT_ONLY
     if(!as_generator(
        (namespaces
         [*function_definition(get_klass_name(cls))
         << *part_implementation(cls.cxx_name)]
         << "\n"
         )).generate(sink, std::make_tuple(cls.namespaces, cls.functions, cls.parts), ctx))
       return false;
#endif

     if(!as_generator(
        attribute_reorder<0, 1, 4, 2, 3, 2, 3, 2, 3, 2, 3, 2, 3, 2, 3, 2, 3, 2, 3, 2, 3>
        (
         "namespace eo_cxx {\n"
         << namespaces
         [
          *function_definition(get_klass_name(cls))
          << *part_implementation(cls.cxx_name)
          << "inline " << base_class_name << "::operator " << class_name << "() const { return *static_cast< "
            << class_name << " const*>(static_cast<void const*>(this)); }\n"
          << "inline " << base_class_name << "::operator " << class_name << "&() { return *static_cast< "
            << class_name << "*>(static_cast<void*>(this)); }\n"
          << "inline " << base_class_name << "::operator " << class_name << " const&() const { return *static_cast< "
            << class_name << " const*>(static_cast<void const*>(this)); }\n"
         ]
         << "}\n"
         )).generate(sink, std::make_tuple(cls.namespaces, cls.functions, cpp_namespaces, cls.cxx_name, cls.parts), ctx))
       return false;

     if(!as_generator(ignore_warning_end).generate(sink, nullptr, ctx))
       return false;

     if(!as_generator("#endif\n").generate(sink, std::make_tuple(), ctx))
       return false;

     return true;
   }
};

template <>
struct is_eager_generator<class_implementation_generator> : std::true_type {};
template <>
struct is_generator<class_implementation_generator> : std::true_type {};

namespace type_traits {
template <>
struct attributes_needed<class_implementation_generator> : std::integral_constant<int, 1> {};
}
      
class_implementation_generator const class_implementation = {};
      
} } }

#endif
