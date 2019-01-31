#ifndef EOLIAN_MONO_STRUCT_DEFINITION_HH
#define EOLIAN_MONO_STRUCT_DEFINITION_HH

#include "grammar/generator.hpp"
#include "grammar/klass_def.hpp"
#include "grammar/indentation.hpp"
#include "grammar/list.hpp"
#include "grammar/alternative.hpp"
#include "name_helpers.hh"
#include "helpers.hh"
#include "type.hh"
#include "using_decl.hh"
#include "documentation.hh"
#include "struct_fields.hh"
#include "blacklist.hh"

namespace eolian_mono {

inline std::string binding_struct_name(attributes::struct_def const& struct_)
{
    return name_helpers::typedecl_managed_name(struct_);
}

inline std::string binding_struct_internal_name(attributes::struct_def const& struct_)
{
   return binding_struct_name(struct_) + "_StructInternal";
}

struct struct_definition_generator
{
  template <typename OutputIterator, typename Context>
  bool generate(OutputIterator sink, attributes::struct_def const& struct_, Context const& context) const
  {
     EINA_CXX_DOM_LOG_DBG(eolian_mono::domain) << "struct_definition_generator: " << struct_.cxx_name << std::endl;
     if(!as_generator(documentation).generate(sink, struct_, context))
       return false;
     if(!as_generator
        (
         "[StructLayout(LayoutKind.Sequential)]\n"
         "public struct " << string << "\n{\n"
         )
        .generate(sink, binding_struct_name(struct_), context))
       return false;

     // iterate struct fields
     for (auto const& field : struct_.fields)
       {
          auto field_name = field.name;
          field_name[0] = std::toupper(field_name[0]); // Hack to allow 'static' as a field name
          if (!as_generator
              (
               documentation(1)
               << scope_tab(1) << "public " << type << " " << string << ";\n"
              )
              .generate(sink, std::make_tuple(field, field.type, name_helpers::to_field_name(field.name)), context))
            return false;
       }

      auto struct_name = binding_struct_name(struct_);

     // Check whether this is an extern struct without declared fields in .eo file and generate a
     // placeholder field if positive.
     // Mono's JIT is picky when generating function pointer for delegates with empty structs, leading to
     // those 'mini-amd64.c condition fields not met' crashes.
     if (struct_.fields.size() == 0)
       {
           if (!as_generator("///<summary>Placeholder field</summary>\npublic IntPtr field;\n").generate(sink, nullptr, context))
             return false;
       }
     else
       {
          // Constructor with default parameters for easy struct initialization
          if(!as_generator(
                      scope_tab << "///<summary>Constructor for " << string << ".</summary>\n"
                      << scope_tab << "public " << string << "(\n"
                      << ((scope_tab << scope_tab << field_argument_default) % ",\n")
                      << scope_tab << ")\n"
                      << scope_tab << "{\n"
                      << *(scope_tab << scope_tab << field_argument_assignment << ";\n")
                      << scope_tab << "}\n")
             .generate(sink, std::make_tuple(struct_name, struct_name, struct_.fields, struct_.fields), context))
              return false;
       }

     if(!as_generator(
            "public static implicit operator " << struct_name << "(IntPtr ptr)\n"
            << scope_tab << "{\n"
            << scope_tab << scope_tab << "var tmp = (" << struct_name << "_StructInternal)Marshal.PtrToStructure(ptr, typeof(" << struct_name << "_StructInternal));\n"
            << scope_tab << scope_tab << "return " << struct_name << "_StructConversion.ToManaged(tmp);\n"
            << scope_tab << "}\n"
            ).generate(sink, attributes::unused, context))
       return false;


     if(!as_generator("}\n").generate(sink, attributes::unused, context)) return false;

     return true;
  }
} const struct_definition {};


struct struct_internal_definition_generator
{
  template <typename OutputIterator, typename Context>
  bool generate(OutputIterator sink, attributes::struct_def const& struct_, Context const& context) const
  {
     if (!as_generator
         (
          "///<summary>Internal wrapper for struct " << string << ".</summary>\n"
          "[StructLayout(LayoutKind.Sequential)]\n"
          "public struct " << string << "\n{\n"
         )
         .generate(sink, std::make_tuple<>(binding_struct_name(struct_), binding_struct_internal_name(struct_)), context))
       return false;

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
                              || regular->base_type == "any_value_ptr")))
            {
               if (!as_generator("///<summary>Internal wrapper for field " << field_name << "</summary>\n"
                                 << "public System.IntPtr " << field_name << ";\n")
                   .generate(sink, nullptr, context))
                 return false;
            }
          else if (regular && !(regular->base_qualifier & efl::eolian::grammar::attributes::qualifier_info::is_ref)
                   && regular->base_type == "bool")
            {
               if (!as_generator("///<summary>Internal wrapper for field " << field_name << "</summary>\n"
                                 "public System.Byte " << field_name << ";\n")
                   .generate(sink, nullptr, context))
                 return false;
            }
          else if (regular && !(regular->base_qualifier & efl::eolian::grammar::attributes::qualifier_info::is_ref)
                   && regular->base_type == "char")
            {
               if (!as_generator("///<summary>Internal wrapper for field " << field_name << "</summary>\n"
                                 "public System.Byte " << field_name << ";\n")
                   .generate(sink, nullptr, context))
                 return false;
            }
          else if (!as_generator(scope_tab << eolian_mono::marshall_annotation(false) << "\n"
                                 << scope_tab << "public " << eolian_mono::marshall_type(false) << " " << string << ";\n")
                   .generate(sink, std::make_tuple(field.type, field.type, field_name), context))
            return false;
       }

     // Check whether this is an extern struct without declared fields in .eo file and generate a
     // placeholder field if positive.
     // Mono's JIT is picky when generating function pointer for delegates with empty structs, leading to
     // those 'mini-amd64.c condition fields not met' crashes.
     if (struct_.fields.size() == 0)
       {
           if (!as_generator("internal IntPtr field;\n").generate(sink, nullptr, context))
             return false;
       }

     auto external_name = binding_struct_name(struct_);
     auto internal_name = binding_struct_internal_name(struct_);

     if(!as_generator(
                 scope_tab << "///<summary>Implicit conversion to the internal/marshalling representation.</summary>\n"
                 << scope_tab << "public static implicit operator " << string << "(" << string << " struct_)\n"
                 << scope_tab << "{\n"
                 << scope_tab << scope_tab << "return " << string << "_StructConversion.ToManaged(struct_);\n"
                 << scope_tab << "}\n"
                 << scope_tab << "///<summary>Implicit conversion to the managed representation.</summary>\n"
                 << scope_tab << "public static implicit operator " << string << "(" << string << " struct_)\n"
                 << scope_tab << "{\n"
                 << scope_tab << scope_tab << "return " << string << "_StructConversion.ToInternal(struct_);\n"
                 << scope_tab << "}\n"
                 ).generate(sink, std::make_tuple(external_name, internal_name, external_name,
                                                  internal_name, external_name, external_name), context))
         return false;

     if(!as_generator("}\n").generate(sink, attributes::unused, context)) return false;

     return true;
  }
} const struct_internal_definition {};


// Conversors generation //

struct to_internal_field_convert_generator
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
           if (!as_generator(
                 scope_tab << scope_tab << "_internal_struct." << string << " = _external_struct." << string << ".NativeHandle;\n")
               .generate(sink, std::make_tuple(field_name, field_name), context))
             return false;
        }
      else if ((complex && (complex->outer.base_type == "array"
                         || complex->outer.base_type == "inarray"
                         || complex->outer.base_type == "list"
                         || complex->outer.base_type == "inlist"
                         || complex->outer.base_type == "iterator"
                         || complex->outer.base_type == "hash"))
            || field.type.c_type == "Eina_Binbuf *" || field.type.c_type == "const Eina_Binbuf *")
        {
           // Always assumes pointer
           if (!as_generator(
                 scope_tab << scope_tab << "_internal_struct." << string << " = _external_struct." << string << ".Handle;\n")
               .generate(sink, std::make_tuple(field_name, field_name), context))
             return false;
        }
      else if (field.type.is_ptr && helpers::need_pointer_conversion(regular) && !helpers::need_struct_conversion(regular))
        {
           if (!as_generator(
                 scope_tab << scope_tab << "_internal_struct." << string << " = Eina.PrimitiveConversion.ManagedToPointerAlloc(_external_struct." << string << ");\n")
               .generate(sink, std::make_tuple(field_name, field_name), context))
             return false;
        }
      else if (helpers::need_struct_conversion(regular))
        {
           if (!as_generator(
                 scope_tab << scope_tab << "_internal_struct." << string << " = " << type << "_StructConversion.ToInternal(_external_struct." << string << ");\n")
               .generate(sink, std::make_tuple(field_name, field.type, field_name), context))
             return false;
        }
      else if (regular && (regular->base_type == "string" || regular->base_type == "mstring"))
        {
           if (!as_generator(
                 scope_tab << scope_tab << "_internal_struct." << string << " = Eina.MemoryNative.StrDup(_external_struct." << string << ");\n")
               .generate(sink, std::make_tuple(field_name, field_name), context))
             return false;
        }
      else if (regular && regular->base_type == "stringshare")
        {
           if (!as_generator(
                 scope_tab << scope_tab << "_internal_struct." << string << " = Eina.Stringshare.eina_stringshare_add(_external_struct." << string << ");\n")
               .generate(sink, std::make_tuple(field_name, field_name), context))
             return false;
        }
      else if (field.type.c_type == "Eina_Slice" || field.type.c_type == "const Eina_Slice"
               || field.type.c_type == "Eina_Rw_Slice" || field.type.c_type == "const Eina_Rw_Slice")
        {
           if (!as_generator(
                 "\n" <<
                 scope_tab << scope_tab << "_internal_struct." << field_name << ".Len = _external_struct." << field_name << ".Len;\n" <<
                 scope_tab << scope_tab << "_internal_struct." << field_name << ".Mem = _external_struct." << field_name << ".Mem;\n\n")
               .generate(sink, attributes::unused, context))
             return false;
        }
      else if (field.type.c_type == "Eina_Value" || field.type.c_type == "const Eina_Value")
        {
           if (!as_generator(
                 scope_tab << scope_tab << "_internal_struct." << string << " = _external_struct." << string << ".GetNative();\n"
               ).generate(sink, std::make_tuple(field_name, field_name), context))
             return false;
        }
      else if (field.type.c_type == "Eina_Value *" || field.type.c_type == "const Eina_Value *")
        {
           if (!as_generator(
                 scope_tab << scope_tab << "_internal_struct." << string << " = _external_struct." << string << ".NativeHandle;\n"
               ).generate(sink, std::make_tuple(field_name, field_name), context))
             return false;
        }
      else if (!field.type.is_ptr && regular && regular->base_type == "bool")
        {
           if (!as_generator(
                 scope_tab << scope_tab << "_internal_struct." << string << " = _external_struct." << string << " ? (byte)1 : (byte)0;\n")
               .generate(sink, std::make_tuple(field_name, field_name), context))
             return false;
        }
      else if (!field.type.is_ptr && regular && regular->base_type == "char")
        {
           if (!as_generator(
                 scope_tab << scope_tab << "_internal_struct." << string << " = (byte)_external_struct." << string << ";\n")
               .generate(sink, std::make_tuple(field_name, field_name), context))
             return false;
        }
      else // primitives and enums
        {
           if (!as_generator(
                 scope_tab << scope_tab << "_internal_struct." << string << " = _external_struct." << string << ";\n")
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
           auto concrete_name = name_helpers::klass_full_concrete_name(*klass);
           if (!as_generator(
                 "\n"
                 << scope_tab << scope_tab << "_external_struct." << string
                 << " = (" << concrete_name << ") System.Activator.CreateInstance(typeof("
                 << concrete_name << "), new System.Object[] {_internal_struct." << string << "});\n"
                 << scope_tab << scope_tab << "Efl.Eo.Globals.efl_ref(_internal_struct." << string << ");\n\n")
               .generate(sink, std::make_tuple(field_name, field_name, field_name), context))
             return false;
        }
      else if (field.type.c_type == "Eina_Binbuf *" || field.type.c_type == "const Eina_Binbuf *")
        {
           if (!as_generator(
                 scope_tab << scope_tab << "_external_struct." << string << " = new " << type << "(_internal_struct." << string << ", false);\n")
               .generate(sink, std::make_tuple(field_name, field.type, field_name), context))
             return false;
        }
      else if (complex && (complex->outer.base_type == "array"
                        || complex->outer.base_type == "inarray"
                        || complex->outer.base_type == "list"
                        || complex->outer.base_type == "inlist"
                        || complex->outer.base_type == "iterator"))
        {
           // Always assumes pointer
           if (!as_generator(
                 scope_tab << scope_tab << "_external_struct." << string << " = new " << type << "(_internal_struct." << string << ", false, false);\n")
               .generate(sink, std::make_tuple(field_name, field.type, field_name), context))
             return false;
        }
      else if (complex && complex->outer.base_type == "hash")
        {
           if (!as_generator(
                 scope_tab << scope_tab << "_external_struct." << string << " = new " << type << "(_internal_struct." << string << ", false, false, false);\n")
               .generate(sink, std::make_tuple(field_name, field.type, field_name), context))
             return false;
        }
      else if (field.type.is_ptr && helpers::need_pointer_conversion(regular) && !helpers::need_struct_conversion(regular))
        {
           if (!as_generator(
                 scope_tab << scope_tab << "_external_struct." << string << " = Eina.PrimitiveConversion.PointerToManaged<" << type << ">(_internal_struct." << string << ");\n")
               .generate(sink, std::make_tuple(field_name, field.type, field_name), context))
             return false;
        }
      else if (helpers::need_struct_conversion(regular))
        {
           if (!as_generator(
                 scope_tab << scope_tab << "_external_struct." << string << " = " << type << "_StructConversion.ToManaged(_internal_struct." << string << ");\n")
               .generate(sink, std::make_tuple(field_name, field.type, field_name), context))
             return false;
        }
      else if (regular && (regular->base_type == "string" || regular->base_type == "mstring" || regular->base_type == "stringshare"))
        {
           if (!as_generator(
                 scope_tab << scope_tab << "_external_struct." << string << " = Eina.StringConversion.NativeUtf8ToManagedString(_internal_struct." << string << ");\n")
               .generate(sink, std::make_tuple(field_name, field_name), context))
             return false;
        }
      else if (field.type.c_type == "Eina_Slice" || field.type.c_type == "const Eina_Slice"
               || field.type.c_type == "Eina_Rw_Slice" || field.type.c_type == "const Eina_Rw_Slice")
        {
           if (!as_generator(
                 "\n" <<
                 scope_tab << scope_tab << "_external_struct." << field_name << ".Len = _internal_struct." << field_name << ".Len;\n" <<
                 scope_tab << scope_tab << "_external_struct." << field_name << ".Mem = _internal_struct." << field_name << ".Mem;\n\n")
               .generate(sink, attributes::unused, context))
             return false;
        }
      else if (field.type.c_type == "Eina_Value" || field.type.c_type == "const Eina_Value")
        {
           if (!as_generator(
                 scope_tab << scope_tab << "_external_struct." << string << " = new Eina.Value(_internal_struct." << string << ");\n"
               ).generate(sink, std::make_tuple(field_name, field_name), context))
             return false;
        }
      else if (field.type.c_type == "Eina_Value *" || field.type.c_type == "const Eina_Value *")
        {
           if (!as_generator(
                 scope_tab << scope_tab << "_external_struct." << string << " = new Eina.Value(_internal_struct." << string << ", Eina.Ownership.Unmanaged);\n"
               ).generate(sink, std::make_tuple(field_name, field_name), context))
             return false;
        }
      else if (!field.type.is_ptr && regular && regular->base_type == "bool")
        {
           if (!as_generator(
                 scope_tab << scope_tab << "_external_struct." << string << " = _internal_struct." << string << " != 0;\n"
               ).generate(sink, std::make_tuple(field_name, field_name), context))
             return false;
        }
      else if (!field.type.is_ptr && regular && regular->base_type == "char")
        {
           if (!as_generator(
                 scope_tab << scope_tab << "_external_struct." << string << " = (char)_internal_struct." << string << ";\n"
               ).generate(sink, std::make_tuple(field_name, field_name), context))
             return false;
        }
      else // primitives and enums
        {
           if (!as_generator(
                 scope_tab << scope_tab << "_external_struct." << string << " = _internal_struct." << string << ";\n")
               .generate(sink, std::make_tuple(field_name, field_name), context))
             return false;
        }
      return true;
   }
} const to_external_field_convert {};

struct struct_binding_conversion_functions_generator
{
  template <typename OutputIterator, typename Context>
  bool generate(OutputIterator sink, attributes::struct_def const& struct_, Context const& context) const
  {
     // Open conversion class
     if (!as_generator
         (
          "/// <summary>Conversion class for struct " << name_helpers::typedecl_managed_name(struct_) << "</summary>\n"
          "public static class " << name_helpers::typedecl_managed_name(struct_) << "_StructConversion\n{\n"
         )
         .generate(sink, nullptr, context))
       return false;

     // to internal
     if (!as_generator
         (
          scope_tab << "internal static " << string << " ToInternal(" << string << " _external_struct)\n"
          << scope_tab << "{\n"
          << scope_tab << scope_tab << "var _internal_struct = new " << string << "();\n\n"
         )
         .generate(sink, std::make_tuple(binding_struct_internal_name(struct_)
           , binding_struct_name(struct_)
           , binding_struct_internal_name(struct_)
           ), context))
       return false;

     for (auto const& field : struct_.fields)
       {
          if (!to_internal_field_convert.generate(sink, field, context))
            return false;
       }

     if (!as_generator
         (
          "\n"
          << scope_tab << scope_tab << "return _internal_struct;\n"
          << scope_tab << "}\n\n"
         )
         .generate(sink, attributes::unused, context))
       return false;

     // to external
     if (!as_generator
         (
          scope_tab << "internal static " << string << " ToManaged(" << string << " _internal_struct)\n"
          << scope_tab << "{\n"
          << scope_tab << scope_tab << "var _external_struct = new " << string << "();\n\n"
         )
         .generate(sink, std::make_tuple(binding_struct_name(struct_)
           , binding_struct_internal_name(struct_)
           , binding_struct_name(struct_)
           ), context))
       return false;

     for (auto const& field : struct_.fields)
       {
          if (!to_external_field_convert.generate(sink, field, context))
            return false;
       }

     if (!as_generator
         (
          "\n"
          << scope_tab << scope_tab << "return _external_struct;\n"
          << scope_tab << "}\n\n"
         )
         .generate(sink, attributes::unused, context))
       return false;

     // Close conversion class
     if (!as_generator("}\n").generate(sink, attributes::unused, context))
       return false;

     return true;
  }
} const struct_binding_conversion_functions {};

struct struct_entities_generator
{
  template <typename OutputIterator, typename Context>
  bool generate(OutputIterator sink, attributes::struct_def const& struct_, Context const& context) const
  {
     if (blacklist::is_struct_blacklisted(struct_))
       return true;


     if (!name_helpers::open_namespaces(sink, struct_.namespaces, context))
       return false;

     if (!struct_definition.generate(sink, struct_, context))
       return false;

     if (!struct_internal_definition.generate(sink, struct_, context))
       return false;

     if (!struct_binding_conversion_functions.generate(sink, struct_, context))
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
struct is_eager_generator< ::eolian_mono::struct_internal_definition_generator> : std::true_type {};
template <>
struct is_generator< ::eolian_mono::struct_internal_definition_generator> : std::true_type {};

template <>
struct is_eager_generator< ::eolian_mono::to_internal_field_convert_generator> : std::true_type {};
template <>
struct is_generator< ::eolian_mono::to_internal_field_convert_generator> : std::true_type {};

template <>
struct is_eager_generator< ::eolian_mono::to_external_field_convert_generator> : std::true_type {};
template <>
struct is_generator< ::eolian_mono::to_external_field_convert_generator> : std::true_type {};

template <>
struct is_eager_generator< ::eolian_mono::struct_binding_conversion_functions_generator> : std::true_type {};
template <>
struct is_generator< ::eolian_mono::struct_binding_conversion_functions_generator> : std::true_type {};

template <>
struct is_eager_generator< ::eolian_mono::struct_entities_generator> : std::true_type {};
template <>
struct is_generator< ::eolian_mono::struct_entities_generator> : std::true_type {};

namespace type_traits {
template <>
struct attributes_needed< ::eolian_mono::struct_definition_generator> : std::integral_constant<int, 1> {};

template <>
struct attributes_needed< ::eolian_mono::struct_internal_definition_generator> : std::integral_constant<int, 1> {};

template <>
struct attributes_needed< ::eolian_mono::to_internal_field_convert_generator> : std::integral_constant<int, 1> {};

template <>
struct attributes_needed< ::eolian_mono::to_external_field_convert_generator> : std::integral_constant<int, 1> {};

template <>
struct attributes_needed< ::eolian_mono::struct_binding_conversion_functions_generator> : std::integral_constant<int, 1> {};

template <>
struct attributes_needed< ::eolian_mono::struct_entities_generator> : std::integral_constant<int, 1> {};
}

} } }

#endif
