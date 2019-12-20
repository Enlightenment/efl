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
#ifndef EOLIAN_MONO_STRUCT_FIELDS_HH
#define EOLIAN_MONO_STRUCT_FIELDS_HH

#include "grammar/generator.hpp"
#include "grammar/klass_def.hpp"
#include "grammar/indentation.hpp"
#include "grammar/list.hpp"
#include "grammar/alternative.hpp"
#include "name_helpers.hh"
#include "type.hh"
#include "using_decl.hh"
#include "documentation.hh"

namespace eolian_mono {

struct field_argument_name_generator
{
   template<typename OutputIterator, typename Context>
   bool generate(OutputIterator sink, attributes::struct_field_def const& field, Context const& context) const
   {
       if (!as_generator(name_helpers::to_field_name(field.name))
               .generate(sink, attributes::unused, context))
           return false;
       return true;
   }
} const field_argument_name {};

struct field_argument_decl_generator
{
   template<typename OutputIterator, typename Context>
   bool generate(OutputIterator sink, attributes::struct_field_def const& field, Context const& context) const
   {
       if (!as_generator(type << " " << string)
               .generate(sink, std::make_tuple(field.type, name_helpers::to_field_name(field.name)), context))
           return false;
       return true;
   }
} const field_argument_decl {};

struct field_argument_default_generator
{
   template<typename OutputIterator, typename Context>
   bool generate(OutputIterator sink, attributes::struct_field_def const& field, Context const& context) const
   {
       if (!as_generator(type << " " << string << " = default(" << type << ")")
               .generate(sink, std::make_tuple(field.type, name_helpers::to_field_name(field.name), field.type), context))
           return false;
       return true;
   }
} const field_argument_default {};

struct field_argument_assignment_generator
{
   template<typename OutputIterator, typename Context>
   bool generate(OutputIterator sink, attributes::struct_field_def const& field, Context const& context) const
   {
       auto field_name = name_helpers::to_field_name(field.name);
       if (!as_generator("this." << field_name << " = " << field_name)
               .generate(sink, attributes::unused, context))
           return false;
       return true;
   }
} const field_argument_assignment {};

struct field_argument_docs_generator
{
   template<typename OutputIterator, typename Context>
   bool generate(OutputIterator sink, attributes::struct_field_def const& field, Context const& context) const
   {
      if (!as_generator(current_indentation(context) << "/// <param name=\"" << string << "\">" << documentation_string << "</param>")
               .generate(sink, std::make_tuple(name_helpers::to_field_name(field.name), field.documentation.summary), context))
           return false;
       return true;
   }
} const field_argument_docs {};

}

namespace efl { namespace eolian { namespace grammar {

template<>
struct is_eager_generator< ::eolian_mono::field_argument_name_generator> : std::true_type {};
template<>
struct is_generator< ::eolian_mono::field_argument_name_generator> : std::true_type {};

template<>
struct is_eager_generator< ::eolian_mono::field_argument_decl_generator> : std::true_type {};
template<>
struct is_generator< ::eolian_mono::field_argument_decl_generator> : std::true_type {};

template<>
struct is_eager_generator< ::eolian_mono::field_argument_default_generator> : std::true_type {};
template<>
struct is_generator< ::eolian_mono::field_argument_default_generator> : std::true_type {};

template<>
struct is_eager_generator< ::eolian_mono::field_argument_assignment_generator> : std::true_type {};
template<>
struct is_generator< ::eolian_mono::field_argument_assignment_generator> : std::true_type {};

template<>
struct is_eager_generator< ::eolian_mono::field_argument_docs_generator> : std::true_type {};
template<>
struct is_generator< ::eolian_mono::field_argument_docs_generator> : std::true_type {};

namespace type_traits {

template <>
struct attributes_needed< ::eolian_mono::field_argument_name_generator> : std::integral_constant<int, 1> {};

template <>
struct attributes_needed< ::eolian_mono::field_argument_decl_generator> : std::integral_constant<int, 1> {};

template <>
struct attributes_needed< ::eolian_mono::field_argument_default_generator> : std::integral_constant<int, 1> {};

template <>
struct attributes_needed< ::eolian_mono::field_argument_assignment_generator> : std::integral_constant<int, 1> {};

template <>
struct attributes_needed< ::eolian_mono::field_argument_docs_generator> : std::integral_constant<int, 1> {};

}

} } }

#endif

