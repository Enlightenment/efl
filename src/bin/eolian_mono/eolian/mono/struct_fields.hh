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
      auto field_name = to_field_name(field.name);
      // FIXME Replace need_struct_conversion(regular) with need_struct_conversion(type)
      auto regular = efl::eina::get<attributes::regular_type_def>(&field.type.original_type);
      auto klass = efl::eina::get<attributes::klass_name>(&field.type.original_type);
      auto complex = efl::eina::get<attributes::complex_type_def>(&field.type.original_type);

      if (klass)
        {
           if (!as_generator(
                 "this." << string << " = " << string << "?.NativeHandle ?? System.IntPtr.Zero;\n")
               .generate(sink, std::make_tuple(field_name, field_name), context))
             return false;
        }
      else if ((complex && (complex->outer.base_type == "array")))
        {
           if (!as_generator(
                 "this." << string << " = Efl.Eo.Globals.IListToNativeArray(" << string << ", " << (field.type.has_own ? "true" : "false") << ");\n")
               .generate(sink, std::make_tuple(field_name, field_name), context))
             return false;
        }
      else if ((complex && (complex->outer.base_type == "list")))
        {
           if (!as_generator(
                 "this." << string << " = Efl.Eo.Globals.IListToNativeList(" << string << ", " << (field.type.has_own ? "true" : "false") << ");\n")
               .generate(sink, std::make_tuple(field_name, field_name), context))
             return false;
        }
      else if ((complex && (complex->outer.base_type == "iterator")))
        {
           if (!as_generator(
                 "this." << string << " = Efl.Eo.Globals.IEnumerableToIterator(" << string << ", " << (field.type.has_own ? "true" : "false")  << ");\n")
               .generate(sink, std::make_tuple(field_name, field_name), context))
             return false;
        }
      else if ((complex && (complex->outer.base_type == "accessor")))
        {
           if (!as_generator(
                 "this." << string << " = Efl.Eo.Globals.IEnumerableToAccessor(" << string << ", " << (field.type.has_own ? "true" : "false")  << ");\n")
               .generate(sink, std::make_tuple(field_name, field_name), context))
             return false;
        }
      else if ((complex && (complex->outer.base_type == "hash"))
            || field.type.c_type == "Eina_Binbuf *" || field.type.c_type == "const Eina_Binbuf *")
        {
           // Always assumes pointer
           if (!as_generator(
                 "this." << string << " = " << string << ".Handle;\n")
               .generate(sink, std::make_tuple(field_name, field_name), context))
             return false;
        }
      else if (field.type.is_ptr && helpers::need_pointer_conversion(regular) && !helpers::need_struct_conversion(regular))
        {
           if (!as_generator(
                 "this." << string << " = Eina.PrimitiveConversion.ManagedToPointerAlloc(" << string << ");\n")
               .generate(sink, std::make_tuple(field_name, field_name), context))
             return false;
        }
      else if (helpers::need_struct_conversion(regular))
        {
           if (!as_generator(
                 "this." << string << " = " << string << ";\n")
               .generate(sink, std::make_tuple(field_name, field_name), context))
             return false;
        }
      else if (regular && (regular->base_type == "string" || regular->base_type == "mstring"))
        {
           if (!as_generator(
                 "this." << string << " = Eina.MemoryNative.StrDup(" << string << ");\n")
               .generate(sink, std::make_tuple(field_name, field_name), context))
             return false;
        }
      else if (regular && regular->base_type == "stringshare")
        {
           if (!as_generator(
                 "this." << string << " = Eina.MemoryNative.AddStringshare(" << string << ");\n")
               .generate(sink, std::make_tuple(field_name, field_name), context))
             return false;
        }
      else if (field.type.c_type == "Eina_Slice" || field.type.c_type == "const Eina_Slice"
               || field.type.c_type == "Eina_Rw_Slice" || field.type.c_type == "const Eina_Rw_Slice")
        {
           if (!as_generator(
                 "this." << string << " = " << string << ";\n")
               .generate(sink, std::make_tuple(field_name, field_name), context))
             return false;
        }
      else if (field.type.c_type == "Eina_Value" || field.type.c_type == "const Eina_Value")
        {
           if (!as_generator(
                 "this." << string << " = " << string << ".GetNative();\n"
               ).generate(sink, std::make_tuple(field_name, field_name), context))
             return false;
        }
      else if (field.type.c_type == "Eina_Value *" || field.type.c_type == "const Eina_Value *")
        {
           if (!as_generator(
                 "this." << string << " = " << string << "?.NativeHandle ?? System.IntPtr.Zero;\n"
               ).generate(sink, std::make_tuple(field_name, field_name), context))
             return false;
        }
      else if (!field.type.is_ptr && regular && regular->base_type == "bool")
        {
           if (!as_generator(
                 "this." << string << " = " << string << " ? (byte)1 : (byte)0;\n")
               .generate(sink, std::make_tuple(field_name, field_name), context))
             return false;
        }
      else if (!field.type.is_ptr && regular && regular->base_type == "char")
        {
           if (!as_generator(
                 "this." << string << " = (byte)" << string << ";\n")
               .generate(sink, std::make_tuple(field_name, field_name), context))
             return false;
        }
      else // primitives and enums
        {
           if (!as_generator(
                 "this." << string << " = " << string << ";\n")
               .generate(sink, std::make_tuple(field_name, field_name), context))
             return false;
        }
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

