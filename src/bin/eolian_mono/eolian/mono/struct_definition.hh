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
      else if ((complex && (complex->outer.base_type == "array"
                         || complex->outer.base_type == "list"
                         || complex->outer.base_type == "iterator"
                         || complex->outer.base_type == "hash"))
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
                 indent << scope_tab << scope_tab << "_internal_struct." << string << " = Eina.Stringshare.eina_stringshare_add(_external_struct." << string << ");\n")
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
      auto const& indent = current_indentation(context);
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
                 << indent << scope_tab << scope_tab << "_external_struct." << string
                 << " = (" << concrete_name << ") Efl.Eo.Globals.CreateWrapperFor(_internal_struct." << string << ");\n"
                 ).generate(sink, std::make_tuple(field_name, field_name), context))
             return false;
        }
      else if (field.type.c_type == "Eina_Binbuf *" || field.type.c_type == "const Eina_Binbuf *")
        {
           if (!as_generator(
                 indent << scope_tab << scope_tab << "_external_struct." << string << " = new " << type << "(_internal_struct." << string << ", false);\n")
               .generate(sink, std::make_tuple(field_name, field.type, field_name), context))
             return false;
        }
      else if (complex && (complex->outer.base_type == "array"
                        || complex->outer.base_type == "list"
                        || complex->outer.base_type == "iterator"))
        {
           // Always assumes pointer
           if (!as_generator(
                 indent << scope_tab << scope_tab << "_external_struct." << string << " = new " << type << "(_internal_struct." << string << ", false, false);\n")
               .generate(sink, std::make_tuple(field_name, field.type, field_name), context))
             return false;
        }
      else if (complex && complex->outer.base_type == "hash")
        {
           if (!as_generator(
                 indent << scope_tab << scope_tab << "_external_struct." << string << " = new " << type << "(_internal_struct." << string << ", false, false, false);\n")
               .generate(sink, std::make_tuple(field_name, field.type, field_name), context))
             return false;
        }
      else if (field.type.is_ptr && helpers::need_pointer_conversion(regular) && !helpers::need_struct_conversion(regular))
        {
           if (!as_generator(
                 indent << scope_tab << scope_tab << "_external_struct." << string << " = Eina.PrimitiveConversion.PointerToManaged<" << type << ">(_internal_struct." << string << ");\n")
               .generate(sink, std::make_tuple(field_name, field.type, field_name), context))
             return false;
        }
      else if (helpers::need_struct_conversion(regular))
        {
           if (!as_generator(
                 indent << scope_tab << scope_tab << "_external_struct." << string << " = _internal_struct." << string << ";\n")
               .generate(sink, std::make_tuple(field_name, field_name), context))
             return false;
        }
      else if (regular && (regular->base_type == "string" || regular->base_type == "mstring" || regular->base_type == "stringshare"))
        {
           if (!as_generator(
                 indent << scope_tab << scope_tab << "_external_struct." << string << " = Eina.StringConversion.NativeUtf8ToManagedString(_internal_struct." << string << ");\n")
               .generate(sink, std::make_tuple(field_name, field_name), context))
             return false;
        }
      else if (field.type.c_type == "Eina_Slice" || field.type.c_type == "const Eina_Slice"
               || field.type.c_type == "Eina_Rw_Slice" || field.type.c_type == "const Eina_Rw_Slice")
        {
           if (!as_generator(
                 "\n" <<
                 indent << scope_tab << scope_tab << "_external_struct." << field_name << ".Len = _internal_struct." << field_name << ".Len;\n" <<
                 indent << scope_tab << scope_tab << "_external_struct." << field_name << ".Mem = _internal_struct." << field_name << ".Mem;\n")
               .generate(sink, attributes::unused, context))
             return false;
        }
      else if (field.type.c_type == "Eina_Value" || field.type.c_type == "const Eina_Value")
        {
           if (!as_generator(
                 indent << scope_tab << scope_tab << "_external_struct." << string << " = new Eina.Value(_internal_struct." << string << ");\n"
               ).generate(sink, std::make_tuple(field_name, field_name), context))
             return false;
        }
      else if (field.type.c_type == "Eina_Value *" || field.type.c_type == "const Eina_Value *")
        {
           if (!as_generator(
                 indent << scope_tab << scope_tab << "_external_struct." << string << " = new Eina.Value(_internal_struct." << string << ", Eina.Ownership.Unmanaged);\n"
               ).generate(sink, std::make_tuple(field_name, field_name), context))
             return false;
        }
      else if (!field.type.is_ptr && regular && regular->base_type == "bool")
        {
           if (!as_generator(
                 indent << scope_tab << scope_tab << "_external_struct." << string << " = _internal_struct." << string << " != 0;\n"
               ).generate(sink, std::make_tuple(field_name, field_name), context))
             return false;
        }
      else if (!field.type.is_ptr && regular && regular->base_type == "char")
        {
           if (!as_generator(
                 indent << scope_tab << scope_tab << "_external_struct." << string << " = (char)_internal_struct." << string << ";\n"
               ).generate(sink, std::make_tuple(field_name, field_name), context))
             return false;
        }
      else // primitives and enums
        {
           if (!as_generator(
                 indent << scope_tab << scope_tab << "_external_struct." << string << " = _internal_struct." << string << ";\n")
               .generate(sink, std::make_tuple(field_name, field_name), context))
             return false;
        }
      return true;
   }
} const to_external_field_convert {};

// Internal Struct //

struct struct_internal_definition_generator
{
  template <typename OutputIterator, typename Context>
  bool generate(OutputIterator sink, attributes::struct_def const& struct_, Context const& context) const
  {
     auto const& indent = current_indentation(context);
     if (!as_generator
         (
          indent << "#pragma warning disable CS1591\n\n"
          << indent << "///<summary>Internal wrapper for struct " << string << ".</summary>\n"
          << indent << "[StructLayout(LayoutKind.Sequential)]\n"
          << indent << "public struct " << string << "\n"
          << indent << "{\n"
         )
         .generate(sink, std::make_tuple<>(binding_struct_name(struct_), struct_internal_decl_name()), context))
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
               if (!as_generator(indent << scope_tab << "///<summary>Internal wrapper for field " << field_name << "</summary>\n"
                                 << indent << scope_tab << "public System.IntPtr " << field_name << ";\n")
                   .generate(sink, nullptr, context))
                 return false;
            }
          else if (regular && !(regular->base_qualifier & efl::eolian::grammar::attributes::qualifier_info::is_ref)
                   && regular->base_type == "bool")
            {
               if (!as_generator(indent << scope_tab << "///<summary>Internal wrapper for field " << field_name << "</summary>\n"
                                 << indent << scope_tab << "public System.Byte " << field_name << ";\n")
                   .generate(sink, nullptr, context))
                 return false;
            }
          else if (regular && !(regular->base_qualifier & efl::eolian::grammar::attributes::qualifier_info::is_ref)
                   && regular->base_type == "char")
            {
               if (!as_generator(indent << scope_tab << "///<summary>Internal wrapper for field " << field_name << "</summary>\n"
                                 << indent << scope_tab << "public System.Byte " << field_name << ";\n")
                   .generate(sink, nullptr, context))
                 return false;
            }
          else if (!as_generator(indent << scope_tab << eolian_mono::marshall_annotation(false) << "\n"
                                 << indent << scope_tab << "public " << eolian_mono::marshall_type(false) << " " << string << ";\n")
                   .generate(sink, std::make_tuple(field.type, field.type, field_name), context))
            return false;
       }

     // Check whether this is an extern struct without declared fields in .eo file and generate a
     // placeholder field if positive.
     // Mono's JIT is picky when generating function pointer for delegates with empty structs, leading to
     // those 'mini-amd64.c condition fields not met' crashes.
     if (struct_.fields.size() == 0)
       {
           if (!as_generator(indent << scope_tab << "internal IntPtr field;\n").generate(sink, nullptr, context))
             return false;
       }

     auto external_name = binding_struct_name(struct_);
     auto internal_name = binding_struct_internal_name(struct_);

     // to internal
     if (!as_generator(
           indent << scope_tab << "///<summary>Implicit conversion to the internal/marshalling representation.</summary>\n"
           << indent << scope_tab << "public static implicit operator " << string << "(" << string << " _external_struct)\n"
           << indent << scope_tab << "{\n"
           << indent << scope_tab << scope_tab << "var _internal_struct = new " << string << "();\n"
           ).generate(sink, std::make_tuple(internal_name, external_name, internal_name), context))
       return false;

     for (auto const& field : struct_.fields)
       {
          if (!to_internal_field_convert.generate(sink, field, context))
            return false;
       }

     if (!as_generator(indent << scope_tab << scope_tab << "return _internal_struct;\n"
                       << indent << scope_tab << "}\n\n").generate(sink, nullptr, context))
       return false;

     // to managed
     if (!as_generator(
           indent << scope_tab << "///<summary>Implicit conversion to the managed representation.</summary>\n"
           << indent << scope_tab << "public static implicit operator " << string << "(" << string << " _internal_struct)\n"
           << indent << scope_tab << "{\n"
           << indent << scope_tab << scope_tab << "var _external_struct = new " << string << "();\n"
           ).generate(sink, std::make_tuple(external_name, internal_name, external_name), context))
       return false;

     for (auto const& field : struct_.fields)
       {
          if (!to_external_field_convert.generate(sink, field, context))
            return false;
       }

     if (!as_generator(indent << scope_tab << scope_tab << "return _external_struct;\n"
                       << indent << scope_tab << "}\n\n").generate(sink, nullptr, context))
       return false;

     // close internal class
     if(!as_generator(indent << "}\n\n"
                      << indent << "#pragma warning restore CS1591\n\n"
                 ).generate(sink, attributes::unused, context)) return false;

     return true;
  }
} const struct_internal_definition {};

// Managed Struct //

struct struct_definition_generator
{
  template <typename OutputIterator, typename Context>
  bool generate(OutputIterator sink, attributes::struct_def const& struct_, Context const& context) const
  {
     EINA_CXX_DOM_LOG_DBG(eolian_mono::domain) << "struct_definition_generator: " << struct_.cxx_name << std::endl;
     auto const& indent = current_indentation(context);
     if(!as_generator(documentation).generate(sink, struct_, context))
       return false;
     if(!as_generator
        (
            indent << "[StructLayout(LayoutKind.Sequential)]\n"
         << indent << "public struct " << string << "\n"
         << indent << "{\n"
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
                  indent << scope_tab << documentation
               << indent << scope_tab << "public " << type << " " << string << ";\n"
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
           if (!as_generator(indent << scope_tab << "///<summary>Placeholder field</summary>\n"
                             << indent << scope_tab << "public IntPtr field;\n").generate(sink, nullptr, context))
             return false;
       }
     else
       {
          // Constructor with default parameters for easy struct initialization
          if(!as_generator(
                      indent << scope_tab << "///<summary>Constructor for " << string << ".</summary>\n"
                      << indent << scope_tab << "public " << string << "(\n"
                      << ((indent << scope_tab << scope_tab << field_argument_default) % ",\n")
                      << indent << scope_tab << ")\n"
                      << indent << scope_tab << "{\n"
                      << *(indent << scope_tab << scope_tab << field_argument_assignment << ";\n")
                      << indent << scope_tab << "}\n\n")
             .generate(sink, std::make_tuple(struct_name, struct_name, struct_.fields, struct_.fields), context))
              return false;
       }

     if(!as_generator(
            indent << scope_tab << "///<summary>Implicit conversion to the managed representation from a native pointer.</summary>\n"
            << indent << scope_tab << "///<param name=\"ptr\">Native pointer to be converted.</param>\n"
            << indent << scope_tab << "public static implicit operator " << struct_name << "(IntPtr ptr)\n"
            << indent << scope_tab << "{\n"
            << indent << scope_tab << scope_tab << "var tmp = (" << struct_name << ".NativeStruct)Marshal.PtrToStructure(ptr, typeof(" << struct_name << ".NativeStruct));\n"
            << indent << scope_tab << scope_tab << "return tmp;\n"
            << indent << scope_tab << "}\n\n"
            ).generate(sink, attributes::unused, context))
       return false;

     if (!struct_internal_definition.generate(sink, struct_, change_indentation(indent.inc(), context)))
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
struct attributes_needed< ::eolian_mono::struct_entities_generator> : std::integral_constant<int, 1> {};
}

} } }

#endif
