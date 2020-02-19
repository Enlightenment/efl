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
#ifndef EOLIAN_MONO_STRUCT_DEFINITION_HH
#define EOLIAN_MONO_STRUCT_DEFINITION_HH

#include "grammar/generator.hpp"
#include "grammar/klass_def.hpp"
#include "grammar/indentation.hpp"
#include "grammar/list.hpp"
#include "grammar/alternative.hpp"
#include "grammar/attribute_reorder.hpp"
#include "name_helpers.hh"
#include "helpers.hh"
#include "type.hh"
#include "using_decl.hh"
#include "documentation.hh"
#include "struct_fields.hh"
#include "blacklist.hh"
#include "culture_info.hh"

namespace eolian_mono {

inline std::string binding_struct_name(attributes::struct_def const& struct_)
{
    return name_helpers::typedecl_managed_name(struct_);
}

inline std::string struct_internal_decl_name()
{
   return  "NativeStruct";
}

inline std::string binding_struct_internal_name(attributes::struct_def const& struct_)
{
   return binding_struct_name(struct_) + "." + struct_internal_decl_name();
}

// Conversors generation //

struct to_internal_field_convert_generator
{
   template <typename OutputIterator, typename Context>
   bool generate(OutputIterator sink, attributes::struct_field_def const& field, Context const& context) const
   {
      auto const& indent = current_indentation(context);
      auto field_name = name_helpers::to_field_name(field.name);
      // FIXME Replace need_struct_conversion(regular) with need_struct_conversion(type)
      auto regular = efl::eina::get<attributes::regular_type_def>(&field.type.original_type);
      auto klass = efl::eina::get<attributes::klass_name>(&field.type.original_type);
      auto complex = efl::eina::get<attributes::complex_type_def>(&field.type.original_type);

      if (klass)
        {
           if (!as_generator(
                 indent << scope_tab << scope_tab << "_internal_struct." << string << " = _external_struct." << string << "?.NativeHandle ?? System.IntPtr.Zero;\n")
               .generate(sink, std::make_tuple(field_name, field_name), context))
             return false;
        }
      else if ((complex && (complex->outer.base_type == "array")))
        {
           if (!as_generator(
                 indent << scope_tab << scope_tab << "_internal_struct." << string << " = Efl.Eo.Globals.IListToNativeArray(_external_struct." << string << ", " << (field.type.has_own ? "true" : "false") << ");\n")
               .generate(sink, std::make_tuple(field_name, field_name), context))
             return false;
        }
      else if ((complex && (complex->outer.base_type == "list")))
        {
           if (!as_generator(
                 indent << scope_tab << scope_tab << "_internal_struct." << string << " = Efl.Eo.Globals.IListToNativeList(_external_struct." << string << ", " << (field.type.has_own ? "true" : "false") << ");\n")
               .generate(sink, std::make_tuple(field_name, field_name), context))
             return false;
        }
      else if ((complex && (complex->outer.base_type == "iterator")))
        {
           if (!as_generator(
                 indent << scope_tab << scope_tab << "_internal_struct." << string << " = Efl.Eo.Globals.IEnumerableToIterator(_external_struct." << string << ", " << (field.type.has_own ? "true" : "false")  << ");\n")
               .generate(sink, std::make_tuple(field_name, field_name), context))
             return false;
        }
      else if ((complex && (complex->outer.base_type == "accessor")))
        {
           if (!as_generator(
                 indent << scope_tab << scope_tab << "_internal_struct." << string << " = Efl.Eo.Globals.IEnumerableToAccessor(_external_struct." << string << ", " << (field.type.has_own ? "true" : "false")  << ");\n")
               .generate(sink, std::make_tuple(field_name, field_name), context))
             return false;
        }
      else if ((complex && (complex->outer.base_type == "hash"))
            || field.type.c_type == "Eina_Binbuf *" || field.type.c_type == "const Eina_Binbuf *")
        {
           // Always assumes pointer
           if (!as_generator(
                 indent << scope_tab << scope_tab << "_internal_struct." << string << " = _external_struct." << string << ".Handle;\n")
               .generate(sink, std::make_tuple(field_name, field_name), context))
             return false;
        }
      else if (field.type.is_ptr && helpers::need_pointer_conversion(regular) && !helpers::need_struct_conversion(regular))
        {
           if (!as_generator(
                 indent << scope_tab << scope_tab << "_internal_struct." << string << " = Eina.PrimitiveConversion.ManagedToPointerAlloc(_external_struct." << string << ");\n")
               .generate(sink, std::make_tuple(field_name, field_name), context))
             return false;
        }
      else if (helpers::need_struct_conversion(regular))
        {
           if (!as_generator(
                 indent << scope_tab << scope_tab << "_internal_struct." << string << " = _external_struct." << string << ";\n")
               .generate(sink, std::make_tuple(field_name, field_name), context))
             return false;
        }
      else if (regular && (regular->base_type == "string" || regular->base_type == "mstring"))
        {
           if (!as_generator(
                 indent << scope_tab << scope_tab << "_internal_struct." << string << " = Eina.MemoryNative.StrDup(_external_struct." << string << ");\n")
               .generate(sink, std::make_tuple(field_name, field_name), context))
             return false;
        }
      else if (regular && regular->base_type == "stringshare")
        {
           if (!as_generator(
                 indent << scope_tab << scope_tab << "_internal_struct." << string << " = Eina.MemoryNative.AddStringshare(_external_struct." << string << ");\n")
               .generate(sink, std::make_tuple(field_name, field_name), context))
             return false;
        }
      else if (field.type.c_type == "Eina_Slice" || field.type.c_type == "const Eina_Slice"
               || field.type.c_type == "Eina_Rw_Slice" || field.type.c_type == "const Eina_Rw_Slice")
        {
           if (!as_generator(
                 "\n" <<
                 indent << scope_tab << scope_tab << "_internal_struct." << field_name << ".Len = _external_struct." << field_name << ".Len;\n" <<
                 indent << scope_tab << scope_tab << "_internal_struct." << field_name << ".Mem = _external_struct." << field_name << ".Mem;\n")
               .generate(sink, attributes::unused, context))
             return false;
        }
      else if (field.type.c_type == "Eina_Value" || field.type.c_type == "const Eina_Value")
        {
           if (!as_generator(
                 indent << scope_tab << scope_tab << "_internal_struct." << string << " = _external_struct." << string << ".GetNative();\n"
               ).generate(sink, std::make_tuple(field_name, field_name), context))
             return false;
        }
      else if (field.type.c_type == "Eina_Value *" || field.type.c_type == "const Eina_Value *")
        {
           if (!as_generator(
                 indent << scope_tab << scope_tab << "_internal_struct." << string << " = _external_struct." << string << "?.NativeHandle ?? System.IntPtr.Zero;\n"
               ).generate(sink, std::make_tuple(field_name, field_name), context))
             return false;
        }
      else if (!field.type.is_ptr && regular && regular->base_type == "bool")
        {
           if (!as_generator(
                 indent << scope_tab << scope_tab << "_internal_struct." << string << " = _external_struct." << string << " ? (byte)1 : (byte)0;\n")
               .generate(sink, std::make_tuple(field_name, field_name), context))
             return false;
        }
      else if (!field.type.is_ptr && regular && regular->base_type == "char")
        {
           if (!as_generator(
                 indent << scope_tab << scope_tab << "_internal_struct." << string << " = (byte)_external_struct." << string << ";\n")
               .generate(sink, std::make_tuple(field_name, field_name), context))
             return false;
        }
      else // primitives and enums
        {
           if (!as_generator(
                 indent << scope_tab << scope_tab << "_internal_struct." << string << " = _external_struct." << string << ";\n")
               .generate(sink, std::make_tuple(field_name, field_name), context))
             return false;
        }

      return true;
   }
} const to_internal_field_convert {};

struct to_external_field_convert_generator
{
   template <typename OutputIterator, typename Context>
   bool generate(OutputIterator sink, attributes::struct_field_def const& field, Context const& context) const
   {
      auto field_name = name_helpers::to_field_name(field.name);
      auto regular = efl::eina::get<attributes::regular_type_def>(&field.type.original_type);
      auto klass = efl::eina::get<attributes::klass_name>(&field.type.original_type);
      auto complex = efl::eina::get<attributes::complex_type_def>(&field.type.original_type);

      if (klass)
        {
           auto interface_name = name_helpers::klass_full_interface_name(*klass);
           if (!as_generator(
                 "(" << interface_name << ") Efl.Eo.Globals.CreateWrapperFor(" << string << ");"
                 ).generate(sink, std::make_tuple(field_name, field_name), context))
             return false;
        }
      else if (field.type.c_type == "Eina_Binbuf *" || field.type.c_type == "const Eina_Binbuf *")
        {
           if (!as_generator(
                 "new " << type << "(" << string << ", false);")
               .generate(sink, std::make_tuple(field.type, field_name), context))
             return false;
        }
      else if (complex && (complex->outer.base_type == "array"))
        {
           // Always assumes pointer
           if (!as_generator(
                 "Efl.Eo.Globals.NativeArrayTo" << type << "(" << string << ");")
               .generate(sink, std::make_tuple(field.type, field_name), context))
             return false;
        }
      else if (complex && (complex->outer.base_type == "list"))
        {
           // Always assumes pointer
           if (!as_generator(
                 "Efl.Eo.Globals.NativeListTo" << type << "(" << string << ");")
               .generate(sink, std::make_tuple(field.type, field_name), context))
             return false;
        }
      else if (complex && complex->outer.base_type == "hash")
        {
           if (!as_generator(
                 "new " << type << "(" << string << ", false, false, false);")
               .generate(sink, std::make_tuple(field.type, field_name), context))
             return false;
        }
      else if (complex && complex->outer.base_type == "iterator")
        {
           if (!as_generator(
                 "Efl.Eo.Globals.IteratorTo" << type << "(" << string << ");")
               .generate(sink, std::make_tuple(field.type, field_name), context))
             return false;
        }
      else if (complex && complex->outer.base_type == "accessor")
        {
           if (!as_generator(
                 "Efl.Eo.Globals.AccessorTo" << type << "(" << string << ");")
               .generate(sink, std::make_tuple(field.type, field_name), context))
             return false;
        }
      else if (field.type.is_ptr && helpers::need_pointer_conversion(regular) && !helpers::need_struct_conversion(regular))
        {
           if (!as_generator(
                 "Eina.PrimitiveConversion.PointerToManaged<" << type << ">(" << string << ");")
               .generate(sink, std::make_tuple(field.type, field_name), context))
             return false;
        }
      else if (helpers::need_struct_conversion(regular))
        {
           if (!as_generator(
                 string << ";")
               .generate(sink, field_name, context))
             return false;
        }
      else if (regular && (regular->base_type == "string" || regular->base_type == "mstring" || regular->base_type == "stringshare"))
        {
           if (!as_generator(
                 "Eina.StringConversion.NativeUtf8ToManagedString(" << string << ");")
               .generate(sink, std::make_tuple(field_name, field_name), context))
             return false;
        }
      else if (field.type.c_type == "Eina_Slice" || field.type.c_type == "const Eina_Slice"
               || field.type.c_type == "Eina_Rw_Slice" || field.type.c_type == "const Eina_Rw_Slice")
        {
           if (!as_generator(field_name << ";")
               .generate(sink, attributes::unused, context))
             return false;
        }
      else if (field.type.c_type == "Eina_Value" || field.type.c_type == "const Eina_Value")
        {
           if (!as_generator(
                 "new Eina.Value(" << string << ");"
               ).generate(sink, std::make_tuple(field_name), context))
             return false;
        }
      else if (field.type.c_type == "Eina_Value *" || field.type.c_type == "const Eina_Value *")
        {
           if (!as_generator(
                 "new Eina.Value(" << string << ", Eina.Ownership.Unmanaged);"
               ).generate(sink, std::make_tuple(field_name), context))
             return false;
        }
      else if (!field.type.is_ptr && regular && regular->base_type == "bool")
        {
           if (!as_generator(
                 string << " != 0;"
               ).generate(sink, std::make_tuple(field_name), context))
             return false;
        }
      else if (!field.type.is_ptr && regular && regular->base_type == "char")
        {
           if (!as_generator(
                 "(char)" << string << ";"
               ).generate(sink, std::make_tuple(field_name), context))
             return false;
        }
      else // primitives and enums
        {
           if (!as_generator(
                 field_name << ";"
               ).generate(sink, attributes::unused, context))
             return false;
        }
      return true;
   }
} const to_external_field_convert {};

// Internal Struct //

struct struct_private_property_generator
{
  template <typename OutputIterator, typename Context>
  bool generate(OutputIterator sink, attributes::struct_def const& struct_, Context const& context) const
  {
     auto const& indent = current_indentation(context);

     // iterate struct fields
     for (auto const& field : struct_.fields)
       {
          auto field_name = name_helpers::to_field_name(field.name);
          auto klass = efl::eina::get<attributes::klass_name>(&field.type.original_type);
          auto regular = efl::eina::get<attributes::regular_type_def>(&field.type.original_type);

          if (klass
              || (regular && (regular->base_type == "string"
                              || regular->base_type == "mstring"
                              || regular->base_type == "stringshare"
                              || regular->base_type == "any_value_ref")))
            {
               if (!as_generator(indent << scope_tab << "/// <summary>Internal wrapper for field " << field_name << "</summary>\n"
                                 << indent << scope_tab << "private System.IntPtr " << field_name << ";\n")
                   .generate(sink, nullptr, context))
                 return false;
            }
          else if (regular && !(regular->base_qualifier & efl::eolian::grammar::attributes::qualifier_info::is_ref)
                   && regular->base_type == "bool")
            {
               if (!as_generator(indent << scope_tab << "/// <summary>Internal wrapper for field " << field_name << "</summary>\n"
                                 << indent << scope_tab << "private System.Byte " << field_name << ";\n")
                   .generate(sink, nullptr, context))
                 return false;
            }
          else if (regular && !(regular->base_qualifier & efl::eolian::grammar::attributes::qualifier_info::is_ref)
                   && regular->base_type == "char")
            {
               if (!as_generator(indent << scope_tab << "/// <summary>Internal wrapper for field " << field_name << "</summary>\n"
                                 << indent << scope_tab << "private System.Byte " << field_name << ";\n")
                   .generate(sink, nullptr, context))
                 return false;
            }
          else if (!as_generator(indent << scope_tab << eolian_mono::marshall_annotation(false) << "\n"
                                 << indent << scope_tab << "private " << eolian_mono::marshall_type(false) << " " << string << ";\n")
                   .generate(sink, std::make_tuple(field.type, field.type, field_name), context))
            return false;
       }

     // Check whether this is an extern struct without declared fields in .eo file and generate a
     // placeholder field if positive.
     // Mono's JIT is picky when generating function pointer for delegates with empty structs, leading to
     // those 'mini-amd64.c condition fields not met' crashes.
     if (struct_.fields.size() == 0)
       {
           if (!as_generator(indent << scope_tab << "/// <summary>Placeholder field</summary>\n"
                             << indent << scope_tab << "private IntPtr field;\n").generate(sink, nullptr, context))
             return false;
       }

     if(!as_generator("\n")
        .generate(sink, attributes::unused, context))
       return false;

     return true;
  }
} const struct_private_property {};

// Managed Struct //

struct struct_definition_generator
{
  /**
   * Generates an implicit operator for packing only if the struct has more
   * than one attribute. Then operator will receive a tuple with the same of
   * each attribute's type in the same order they were declared.
   *
   * Remarks: due to the MCS compiler's limitations, no operator is generated
   * for structs with more than 4 fields.
   */
  template <typename OutputIterator, typename Context>
  bool generate_implicit_operator(attributes::struct_def const& struct_
                              , OutputIterator sink
                              , Context const& context) const
  {
     if (struct_.fields.size() <= 1 || struct_.fields.size() > 4)
       return true;

     auto struct_name = binding_struct_name(struct_);
     auto const& indent = current_indentation(context).inc();

     if (!as_generator(
           indent << scope_tab << "/// <summary>Packs tuple into " << struct_name << " object.\n"
           << indent << scope_tab << "///<para>Since EFL 1.24.</para>\n"
           << indent << scope_tab << "///</summary>\n"
        ).generate(sink, attributes::unused, context))
       return false;

     if (!as_generator(
          indent << scope_tab << "public static implicit operator " << struct_name << "(("
          << (field_argument_decl % ", ")
          << ") tuple)\n"
        ).generate(sink, struct_.fields, context))
       return false;

     // object constructor
     if (!as_generator(
          indent << scope_tab << scope_tab << "=> new " << struct_name << "("
        ).generate(sink, attributes::unused, context))
       return false;

     if (!as_generator(
          (("tuple." << struct_field_name) % ", ")
          << ");\n\n"
        ).generate(sink, struct_.fields, context))
       return false;

     return true;
  }

  template <typename OutputIterator, typename Context>
  bool generate_deconstruct_method(OutputIterator sink, attributes::struct_def const& struct_, Context const& context) const
  {
     auto const& indent = current_indentation(context).inc();
     auto struct_name = binding_struct_name(struct_);

     if (!as_generator(
         indent << scope_tab << "/// <summary>Unpacks " << struct_name << " into tuple.\n"
         << indent << scope_tab << "/// <para>Since EFL 1.24.</para>\n"
         << indent << scope_tab << "/// </summary>\n"
         << indent << scope_tab << "public void Deconstruct(\n"
         ).generate(sink, attributes::unused, context))
       return false;

     // parameters
     {
        auto i = 0u;
        for (auto const& field : struct_.fields)
          {
             auto field_name = name_helpers::to_field_name(field.name);

             auto suffix = i == struct_.fields.size() - 1 ? "\n" : ",\n";

             if (!as_generator(
                 indent << scope_tab << scope_tab << "out " << type << " " << field_name << suffix
                 ).generate(sink, std::make_tuple(field.type), context))
               return false;

             ++i;
          }
     }

     if (!as_generator(
         indent << scope_tab << ")\n"
         << indent << scope_tab << "{\n"
         ).generate(sink, attributes::unused, context))
       return false;

     // assigments
     for (auto const& field : struct_.fields)
       {
          auto field_name = name_helpers::managed_name(field.name);
          auto param_name = name_helpers::to_field_name(field.name);

          if (!as_generator(
              indent << scope_tab << scope_tab << param_name << " = this." << field_name << ";\n"
              ).generate(sink, attributes::unused, context))
            return false;
       }

     // the end
     return as_generator(
             indent << scope_tab << "}\n"
             ).generate(sink, attributes::unused, context);
  }

  template <typename OutputIterator, typename Context>
  bool generate(OutputIterator sink, attributes::struct_def const& struct_, Context const& context) const
  {
     EINA_CXX_DOM_LOG_DBG(eolian_mono::domain) << "struct_definition_generator: " << struct_.cxx_name << std::endl;
     auto const& indent = current_indentation(context).inc();
     if(!as_generator(documentation(1)).generate(sink, struct_, context))
       return false;
     auto struct_managed_name = binding_struct_name(struct_);
     if(!as_generator
        (
            indent << "[StructLayout(LayoutKind.Sequential)]\n"
         << indent << "[Efl.Eo.BindingEntity]\n"
         << indent << "[SuppressMessage(\"Microsoft.Naming\", \"CA1724:TypeNamesShouldNotMatchNamespaces\")]\n"
         << indent << "public struct " << struct_managed_name << " : IEquatable<" << struct_managed_name << ">\n"
         << indent << "{\n"
         )
        .generate(sink, attributes::unused, context))
       return false;

     if (!struct_private_property.generate(sink, struct_, change_indentation(indent, context)))
       return false;

     // iterate struct fields
     for (auto const& field : struct_.fields)
       {
          auto docs = documentation(indent.n + 1);
          if (!as_generator(docs).generate(sink, field, context))
            return false;

          if (!field.type.doc_summary.empty())
            {
               if (!docs.generate_tag_value(sink, field.type.doc_summary, context))
                 return false;
            }

          if (!as_generator(indent << scope_tab << "public " << type << " " << name_helpers::managed_name(field.name) << " { get => " << to_external_field_convert << " }\n").generate(sink, std::make_tuple(field.type, field), context))
            return false;
       }

      auto struct_name = binding_struct_name(struct_);

     if (struct_.fields.size() != 0)
       {
          // Constructor with default parameters for easy struct initialization
          if(!as_generator(
                      indent << scope_tab << "/// <summary>Constructor for " << string << ".\n"
               ).generate(sink, struct_name, context))
            return false;

          if (!struct_.documentation.since.empty())
            if (!as_generator(indent << scope_tab << "/// <para>Since EFL " + struct_.documentation.since + ".</para>\n"
                    ).generate(sink, attributes::unused, context))
              return false;

          if (!as_generator(
                      indent << scope_tab << "/// </summary>\n"
                      << *(indent << scope_tab << field_argument_docs << "\n")
                      << indent << scope_tab << "public " << string << "(\n"
                      << ((indent << scope_tab << scope_tab << field_argument_default) % ",\n")
                      << ")\n"
                      << indent << scope_tab << "{\n"
                      << *(indent << scope_tab << scope_tab << field_argument_assignment)
                      << indent << scope_tab << "}\n\n")
             .generate(sink, std::make_tuple(struct_.fields, struct_name, struct_.fields, struct_.fields), context))
            return false;

          if (!generate_implicit_operator(struct_, sink, context))
            return false;

          if (!generate_deconstruct_method(sink, struct_, context))
            return false;
       }

     std::string since_line;
     if (!struct_.documentation.since.empty())
         if (!as_generator(indent << scope_tab << "/// <para>Since EFL " + struct_.documentation.since + ".</para>\n"
                 ).generate(std::back_inserter(since_line), attributes::unused, context))
           return false;

     // GetHashCode (needed by the equality comparisons)
     if (!as_generator(
             indent << scope_tab << "/// <summary>Get a hash code for this item.\n"
             << since_line
             << indent << scope_tab << "/// </summary>\n"
             << indent << scope_tab << "public override int GetHashCode()\n"
             << indent << scope_tab << "{\n"
          ).generate(sink, attributes::unused, context))
       return false;

     if (struct_.fields.size() == 1 )
       {
          if (!as_generator(
                indent << scope_tab << scope_tab  << "return " << name_helpers::managed_name(struct_.fields[0].name) << ".GetHashCode();\n"
              ).generate(sink, attributes::unused, context))
            return false;
       }
     else if (struct_.fields.size() != 0 )
       {
          // int hash = 17;
          // hash = 23 * fieldA.GetHashCode();
          // hash = 23 * fieldB.GetHashCode();
          // hash = 23 * fieldC.GetHashCode();
          // return hash
          if (!as_generator(
                indent << scope_tab << scope_tab << "int hash = 17;\n"
                << *(grammar::attribute_reorder<-1, -1>(indent << scope_tab << scope_tab << "hash = hash * 23 + " << name_helpers::struct_property_name << ".GetHashCode(" << culture_info << ");\n"))
                << indent << scope_tab << scope_tab << "return hash;\n"
              ).generate(sink, struct_.fields, context))
            return false;
       }
     else
       {
          // Just compare the place holder pointers
          if (!as_generator(
                indent << scope_tab << scope_tab  << "return field.GetHashCode();\n"
              ).generate(sink, attributes::unused, context))
            return false;
       }

     if (!as_generator(
             indent << scope_tab << "}\n"
          ).generate(sink, attributes::unused, context))
       return false;

     // IEquatable<T> Equals
     if (!as_generator(
             indent << scope_tab << "/// <summary>Equality comparison.\n"
             << since_line
             << indent << scope_tab << "/// </summary>\n"
             << indent << scope_tab << "public bool Equals(" << struct_managed_name << " other)\n"
             << indent << scope_tab << "{\n"
             << indent << scope_tab << scope_tab << "return "
          ).generate(sink, attributes::unused, context))
       return false;

     if (struct_.fields.size() != 0 )
       {
          if (!as_generator(
                grammar::attribute_reorder<-1, -1>((name_helpers::struct_property_name << " == other." << name_helpers::struct_property_name)) % " && "
              ).generate(sink, struct_.fields, context))
            return false;
       }
     else
       {
          // Just compare the place holder pointers
          if (!as_generator(
                "field.Equals(other.field)"
              ).generate(sink, attributes::unused, context))
            return false;
       }


     if (!as_generator(
             ";\n"
             << indent << scope_tab << "}\n"
          ).generate(sink, attributes::unused, context))
      return false;

     // ValueType.Equals
     if (!as_generator(
           indent << scope_tab << "/// <summary>Equality comparison.\n"
           << since_line
           << indent << scope_tab << "/// </summary>\n"
           << indent << scope_tab << "public override bool Equals(object other)\n"
           << indent << scope_tab << scope_tab << "=> ((other is " << struct_managed_name  << ") ? Equals((" << struct_managed_name << ")other) : false);\n"
        ).generate(sink, attributes::unused, context))
       return false;

     // Equality operators
     if (!as_generator(
           indent << scope_tab << "/// <summary>Equality comparison.\n"
           << since_line
           << indent << scope_tab << "/// </summary>\n"
           << indent << scope_tab << "public static bool operator ==(" << struct_managed_name << " lhs, " << struct_managed_name << " rhs)\n"
           << indent << scope_tab << scope_tab << "=> lhs.Equals(rhs);"
        ).generate(sink, attributes::unused, context))
       return false;

     if (!as_generator(
           indent << scope_tab << "/// <summary>Equality comparison.\n"
           << since_line
           << indent << scope_tab << "/// </summary>\n"
           << indent << scope_tab << "public static bool operator !=(" << struct_managed_name << " lhs, " << struct_managed_name << " rhs)\n"
           << indent << scope_tab << scope_tab << "=> !lhs.Equals(rhs);"
        ).generate(sink, attributes::unused, context))
       return false;

     // Conversions from/to internal struct and IntPtrs
     if(!as_generator(
            indent << scope_tab << "/// <summary>Implicit conversion to the managed representation from a native pointer.\n"
            ).generate(sink, attributes::unused, context))
       return false;

     if (!struct_.documentation.since.empty())
       if (!as_generator(indent << scope_tab << "/// <para>Since EFL " + struct_.documentation.since + ".</para>\n"
            ).generate(sink, attributes::unused, context))
         return false;

     if (!as_generator(
            indent << scope_tab << "/// </summary>\n"
            << indent << scope_tab << "/// <param name=\"ptr\">Native pointer to be converted.</param>\n"
            << indent << scope_tab << "public static implicit operator " << struct_name << "(IntPtr ptr)\n"
            << indent << scope_tab << "{\n"
            << indent << scope_tab << scope_tab << "return (" << struct_name << ")Marshal.PtrToStructure(ptr, typeof(" << struct_name << "));\n"
            << indent << scope_tab << "}\n\n"
            ).generate(sink, attributes::unused, context))
       return false;

    if(!as_generator(
            indent << scope_tab << "/// <summary>Conversion to the managed representation from a native pointer.\n"
            ).generate(sink, attributes::unused, context))
       return false;

     if (!struct_.documentation.since.empty())
       if (!as_generator(indent << scope_tab << "/// <para>Since EFL " + struct_.documentation.since + ".</para>\n"
            ).generate(sink, attributes::unused, context))
         return false;

     if (!as_generator(
            indent << scope_tab << "/// </summary>\n"
            << indent << scope_tab << "/// <param name=\"ptr\">Native pointer to be converted.</param>\n"
            << indent << scope_tab << "public static " << struct_name << " FromIntPtr(IntPtr ptr)\n"
            << indent << scope_tab << "{\n"
            << indent << scope_tab << scope_tab << "return ptr;\n"
            << indent << scope_tab << "}\n\n"
            ).generate(sink, attributes::unused, context))
       return false;

     if(!as_generator(indent << "}\n\n").generate(sink, attributes::unused, context)) return false;

     return true;
  }
} const struct_definition {};

struct struct_entities_generator
{
  template <typename OutputIterator, typename Context>
  bool generate(OutputIterator sink, attributes::struct_def const& struct_, Context const& context) const
  {
     if (blacklist::is_struct_blacklisted(struct_, context))
       return true;

     if (!name_helpers::open_namespaces(sink, struct_.namespaces, context))
       return false;

     if (!struct_definition.generate(sink, struct_, context))
       return false;

     return name_helpers::close_namespaces(sink, struct_.namespaces, context);

  }
} const struct_entities {};

}

namespace efl { namespace eolian { namespace grammar {

template <>
struct is_eager_generator< ::eolian_mono::struct_definition_generator> : std::true_type {};
template <>
struct is_generator< ::eolian_mono::struct_definition_generator> : std::true_type {};

template <>
struct is_eager_generator< ::eolian_mono::struct_private_property_generator> : std::true_type {};
template <>
struct is_generator< ::eolian_mono::struct_private_property_generator> : std::true_type {};

template <>
struct is_eager_generator< ::eolian_mono::to_internal_field_convert_generator> : std::true_type {};
template <>
struct is_generator< ::eolian_mono::to_internal_field_convert_generator> : std::true_type {};

template <>
struct is_eager_generator< ::eolian_mono::to_external_field_convert_generator> : std::true_type {};
template <>
struct is_generator< ::eolian_mono::to_external_field_convert_generator> : std::true_type {};

template <>
struct is_eager_generator< ::eolian_mono::struct_entities_generator> : std::true_type {};
template <>
struct is_generator< ::eolian_mono::struct_entities_generator> : std::true_type {};

namespace type_traits {
template <>
struct attributes_needed< ::eolian_mono::struct_definition_generator> : std::integral_constant<int, 1> {};

template <>
struct attributes_needed< ::eolian_mono::struct_private_property_generator> : std::integral_constant<int, 1> {};

template <>
struct attributes_needed< ::eolian_mono::to_internal_field_convert_generator> : std::integral_constant<int, 1> {};

template <>
struct attributes_needed< ::eolian_mono::to_external_field_convert_generator> : std::integral_constant<int, 1> {};

template <>
struct attributes_needed< ::eolian_mono::struct_entities_generator> : std::integral_constant<int, 1> {};
}

} } }

#endif
