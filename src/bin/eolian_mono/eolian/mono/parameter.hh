#ifndef EOLIAN_MONO_PARAMETER_HH
#define EOLIAN_MONO_PARAMETER_HH

#include "grammar/generator.hpp"
#include "grammar/klass_def.hpp"
#include "grammar/case.hpp"
#include "marshall_type.hh"
#include "type.hh"
#include "using_decl.hh"
#include "keyword.hh"


namespace eolian_mono {
    struct parameter_generator;
    struct marshall_parameter_generator;
    struct argument_generator;
    struct argument_invocation_generator;
    struct native_argument_invocation_generator;
    struct native_convert_in_variable_generator;
    struct convert_in_variable_generator;
    struct native_convert_out_variable_generator;
    struct convert_out_variable_generator;
    struct convert_out_assign_generator;
    struct native_convert_out_assign_parameterized;
    struct native_convert_out_assign_generator;
    struct convert_return_generator;
    struct convert_return_variable_generator;
    struct native_convert_return_generator;
    struct native_convert_return_parameterized;
    struct native_convert_return_variable_generator;
    struct convert_function_pointer_generator;
    struct native_convert_function_pointer_generator;
}

namespace efl { namespace eolian { namespace grammar {

template <>
struct is_eager_generator< ::eolian_mono::parameter_generator> : std::true_type {};
template <>
struct is_generator< ::eolian_mono::parameter_generator> : std::true_type {};

namespace type_traits {
template <>
struct attributes_needed< ::eolian_mono::parameter_generator> : std::integral_constant<int, 1> {};  
}

template <>
struct is_eager_generator< ::eolian_mono::marshall_parameter_generator> : std::true_type {};
template <>
struct is_generator< ::eolian_mono::marshall_parameter_generator> : std::true_type {};

namespace type_traits {
template <>
struct attributes_needed< ::eolian_mono::marshall_parameter_generator> : std::integral_constant<int, 1> {};  
}
      
template <>
struct is_eager_generator< ::eolian_mono::argument_generator> : std::true_type {};
template <>
struct is_generator< ::eolian_mono::argument_generator> : std::true_type {};

namespace type_traits {
template <>
struct attributes_needed< ::eolian_mono::argument_generator> : std::integral_constant<int, 1> {};  
}
      
template <>
struct is_eager_generator< ::eolian_mono::argument_invocation_generator> : std::true_type {};
template <>
struct is_generator< ::eolian_mono::argument_invocation_generator> : std::true_type {};

namespace type_traits {
template <>
struct attributes_needed< ::eolian_mono::argument_invocation_generator> : std::integral_constant<int, 1> {};
}

template <>
struct is_eager_generator< ::eolian_mono::native_argument_invocation_generator> : std::true_type {};
template <>
struct is_generator< ::eolian_mono::native_argument_invocation_generator> : std::true_type {};

namespace type_traits {
template <>
struct attributes_needed< ::eolian_mono::native_argument_invocation_generator> : std::integral_constant<int, 1> {};
}

template <>
struct is_eager_generator< ::eolian_mono::native_convert_in_variable_generator> : std::true_type {};
template <>
struct is_generator< ::eolian_mono::native_convert_in_variable_generator> : std::true_type {};

namespace type_traits {
template <>
struct attributes_needed< ::eolian_mono::native_convert_in_variable_generator> : std::integral_constant<int, 1> {};
}

template <>
struct is_eager_generator< ::eolian_mono::convert_in_variable_generator> : std::true_type {};
template <>
struct is_generator< ::eolian_mono::convert_in_variable_generator> : std::true_type {};

namespace type_traits {
template <>
struct attributes_needed< ::eolian_mono::convert_in_variable_generator> : std::integral_constant<int, 1> {};
}

template <>
struct is_eager_generator< ::eolian_mono::convert_out_variable_generator> : std::true_type {};
template <>
struct is_generator< ::eolian_mono::convert_out_variable_generator> : std::true_type {};

namespace type_traits {
template <>
struct attributes_needed< ::eolian_mono::convert_out_variable_generator> : std::integral_constant<int, 1> {};
}

template <>
struct is_eager_generator< ::eolian_mono::native_convert_out_variable_generator> : std::true_type {};
template <>
struct is_generator< ::eolian_mono::native_convert_out_variable_generator> : std::true_type {};

namespace type_traits {
template <>
struct attributes_needed< ::eolian_mono::native_convert_out_variable_generator> : std::integral_constant<int, 1> {};
}

template <>
struct is_eager_generator< ::eolian_mono::convert_out_assign_generator> : std::true_type {};
template <>
struct is_generator< ::eolian_mono::convert_out_assign_generator> : std::true_type {};

namespace type_traits {
template <>
struct attributes_needed< ::eolian_mono::convert_out_assign_generator> : std::integral_constant<int, 1> {};
}

template <>
struct is_eager_generator< ::eolian_mono::convert_return_variable_generator> : std::true_type {};
template <>
struct is_generator< ::eolian_mono::convert_return_variable_generator> : std::true_type {};

namespace type_traits {
template <>
struct attributes_needed< ::eolian_mono::convert_return_variable_generator> : std::integral_constant<int, 1> {};
}

template <>
struct is_eager_generator< ::eolian_mono::convert_return_generator> : std::true_type {};
template <>
struct is_generator< ::eolian_mono::convert_return_generator> : std::true_type {};

namespace type_traits {
template <>
struct attributes_needed< ::eolian_mono::convert_return_generator> : std::integral_constant<int, 1> {};
}

template <>
struct is_eager_generator< ::eolian_mono::native_convert_out_assign_generator> : std::true_type {};
template <>
struct is_generator< ::eolian_mono::native_convert_out_assign_generator> : std::true_type {};
template <>
struct is_generator< ::eolian_mono::native_convert_out_assign_parameterized> : std::true_type {};

namespace type_traits {
template <>
struct attributes_needed< ::eolian_mono::native_convert_out_assign_generator> : std::integral_constant<int, 1> {};
}

template <>
struct is_eager_generator< ::eolian_mono::native_convert_return_variable_generator> : std::true_type {};
template <>
struct is_generator< ::eolian_mono::native_convert_return_variable_generator> : std::true_type {};

namespace type_traits {
template <>
struct attributes_needed< ::eolian_mono::native_convert_return_variable_generator> : std::integral_constant<int, 1> {};
}

template <>
struct is_eager_generator< ::eolian_mono::native_convert_return_generator> : std::true_type {};
template <>
struct is_generator< ::eolian_mono::native_convert_return_generator> : std::true_type {};
template <>
struct is_generator< ::eolian_mono::native_convert_return_parameterized> : std::true_type {};

namespace type_traits {
template <>
struct attributes_needed< ::eolian_mono::native_convert_return_generator> : std::integral_constant<int, 1> {};
}

template <>
struct is_eager_generator< ::eolian_mono::convert_function_pointer_generator> : std::true_type {};
template <>
struct is_generator< ::eolian_mono::convert_function_pointer_generator> : std::true_type {};

namespace type_traits {
template <>
struct attributes_needed< ::eolian_mono::convert_function_pointer_generator> : std::integral_constant<int, 1> {};
}

template <>
struct is_eager_generator< ::eolian_mono::native_convert_function_pointer_generator> : std::true_type {};
template <>
struct is_generator< ::eolian_mono::native_convert_function_pointer_generator> : std::true_type {};

namespace type_traits {
template <>
struct attributes_needed< ::eolian_mono::native_convert_function_pointer_generator> : std::integral_constant<int, 1> {};
}

} } }

namespace eolian_mono {

namespace attributes = efl::eolian::grammar::attributes;

// Helper function to query parameter attributes
const static bool WANT_OWN = true;
const static bool WANT_OUT = true;
inline bool param_is_acceptable(attributes::parameter_def const &param, std::string c_type, bool want_own, bool want_out)
{
   return (param.type.c_type == c_type)
          && ((param.direction != attributes::parameter_direction::in) == want_out)
          && (param.type.has_own == want_own);
}

inline bool param_should_use_out_var(attributes::parameter_def const& param)
{
   if (param_is_acceptable(param, "const char *", !WANT_OWN, WANT_OUT)
           || param_is_acceptable(param, "Eina_Stringshare *", !WANT_OWN, WANT_OUT)
           || param_is_acceptable(param, "Eina_Binbuf *", !WANT_OWN, WANT_OUT)
           || param_is_acceptable(param, "Eina_Binbuf *", WANT_OWN, WANT_OUT)
           || param_is_acceptable(param, "const Eina_Binbuf *", !WANT_OWN, WANT_OUT)
           || param_is_acceptable(param, "const Eina_Binbuf *", WANT_OWN, WANT_OUT)
           || param_is_acceptable(param, "Eina_Array *", !WANT_OWN, WANT_OUT)
           || param_is_acceptable(param, "Eina_Array *", WANT_OWN, WANT_OUT)
           || param_is_acceptable(param, "const Eina_Array *", !WANT_OWN, WANT_OUT)
           || param_is_acceptable(param, "const Eina_Array *", WANT_OWN, WANT_OUT)
           || param_is_acceptable(param, "Eina_List *", !WANT_OWN, WANT_OUT)
           || param_is_acceptable(param, "Eina_List *", WANT_OWN, WANT_OUT)
           || param_is_acceptable(param, "const Eina_List *", !WANT_OWN, WANT_OUT)
           || param_is_acceptable(param, "const Eina_List *", WANT_OWN, WANT_OUT)
           || param_is_acceptable(param, "Eina_Hash *", !WANT_OWN, WANT_OUT)
           || param_is_acceptable(param, "Eina_Hash *", WANT_OWN, WANT_OUT)
           || param_is_acceptable(param, "const Eina_Hash *", !WANT_OWN, WANT_OUT)
           || param_is_acceptable(param, "const Eina_Hash *", WANT_OWN, WANT_OUT)
      )
     return true;

   return false;
}

inline bool param_should_use_in_var(attributes::parameter_def const& param)
{
    if (param_is_acceptable(param, "Eina_Binbuf *", !WANT_OWN, !WANT_OUT)
        || param_is_acceptable(param, "Eina_Binbuf *", WANT_OWN, !WANT_OUT)
        || param_is_acceptable(param, "const Eina_Binbuf *", !WANT_OWN, !WANT_OUT)
        || param_is_acceptable(param, "const Eina_Binbuf *", WANT_OWN, !WANT_OUT)
        || param_is_acceptable(param, "Eina_Array *", !WANT_OWN, !WANT_OUT)
        || param_is_acceptable(param, "Eina_Array *", WANT_OWN, !WANT_OUT)
        || param_is_acceptable(param, "const Eina_Array *", !WANT_OWN, !WANT_OUT)
        || param_is_acceptable(param, "const Eina_Array *", WANT_OWN, !WANT_OUT)
        || param_is_acceptable(param, "Eina_List *", !WANT_OWN, !WANT_OUT)
        || param_is_acceptable(param, "Eina_List *", WANT_OWN, !WANT_OUT)
        || param_is_acceptable(param, "const Eina_List *", !WANT_OWN, !WANT_OUT)
        || param_is_acceptable(param, "const Eina_List *", WANT_OWN, !WANT_OUT)
        || param_is_acceptable(param, "Eina_Hash *", !WANT_OWN, !WANT_OUT)
        || param_is_acceptable(param, "Eina_Hash *", WANT_OWN, !WANT_OUT)
        || param_is_acceptable(param, "const Eina_Hash *", !WANT_OWN, !WANT_OUT)
        || param_is_acceptable(param, "const Eina_Hash *", WANT_OWN, !WANT_OUT)
       )
        return true;

    return false;
}

inline std::string out_variable_name(std::string const& param_name)
{
   return "_out_" + escape_keyword(param_name);
}

inline std::string in_variable_name(std::string const& param_name)
{
   return "_in_" + escape_keyword(param_name);
}

inline std::string direction_modifier(attributes::parameter_def const& param)
{
   if (param.direction != attributes::parameter_direction::in)
     {
        if (param.type.c_type == "Eina_Slice" || param.type.c_type == "Eina_Rw_Slice")
           return " ref ";
        else
           return " out ";
     }
   return " ";
}

struct is_fp_visitor
{
   typedef is_fp_visitor visitor_type;
   typedef bool result_type;

   bool operator()(grammar::attributes::regular_type_def const &type) const
   {
      return type.is_function_pointer;
   }

   template<typename T>
   bool operator()(T const &) const
   {
      return false;
   }
};

struct parameter_generator
{
   template <typename OutputIterator, typename Context>
   bool generate(OutputIterator sink, attributes::parameter_def const& param, Context const& context) const
   {
     std::string param_name = escape_keyword(param.param_name);

     return as_generator(
               direction_modifier(param) << type << " " << string
          ).generate(sink, std::make_tuple(param, param_name), context);

   }
} const parameter {};

struct marshall_parameter_generator
{
   template <typename OutputIterator, typename Context>
   bool generate(OutputIterator sink, attributes::parameter_def const& param, Context const& context) const
   {
      std::string param_name = escape_keyword(param.param_name);

      if (!param.type.original_type.visit(is_fp_visitor{}))
         return as_generator(
                 direction_modifier(param) << marshall_type << " " << string
            ).generate(sink, std::make_tuple(param, param_name), context);

      return as_generator(
               type << "Internal " << param_name << ", IntPtr " << param_name << "_data, Eina_Free_Cb "
               << param_name << "_free_cb"
           ).generate(sink, param, context);
   }
} const marshall_parameter {};
  
struct argument_generator
{
   template <typename OutputIterator, typename Context>
   bool generate(OutputIterator sink, attributes::parameter_def const& param, Context const& context) const
   {
     std::string param_name = escape_keyword(param.param_name);
     std::string direction = direction_modifier(param);

     if (!param.type.original_type.visit(is_fp_visitor{}))
       return as_generator(
                direction << param_name
           ).generate(sink, attributes::unused, context);

    return as_generator(
            param_name << ", " << param_name << "_data, " << param_name << "_free_cb"
          ).generate(sink, attributes::unused, context);

   }

} const argument {};

struct native_argument_invocation_generator
{
   template <typename OutputIterator, typename Context>
   bool generate(OutputIterator sink, attributes::parameter_def const& param, Context const& context) const
   {
     std::string arg = direction_modifier(param);

     if (param_should_use_out_var(param))
       arg += out_variable_name(param.param_name);
     else if (param_should_use_in_var(param))
       arg += in_variable_name(param.param_name);
     else if (param.type.original_type.visit(is_fp_visitor{}))
       {
          arg += escape_keyword(param.param_name) + "_wrapper.ManagedCb";
       }
     else // FIXME Wrap data and C function pointers into some kind of structure.
       arg += escape_keyword(param.param_name);

     return as_generator(arg).generate(sink, attributes::unused, context);
   }
} const native_argument_invocation {};

// Generates the correct parameter name when invoking a function
struct argument_invocation_generator
{
   template <typename OutputIterator, typename Context>
   bool generate(OutputIterator sink, attributes::parameter_def const& param, Context const& context) const
   {
     std::string arg = direction_modifier(param);

     if (param_should_use_out_var(param))
       arg += out_variable_name(param.param_name);
     else if (param_should_use_in_var(param))
       arg += in_variable_name(param.param_name);
     else if (param.type.original_type.visit(is_fp_visitor{}))
       {
          std::string param_name = escape_keyword(param.param_name);
          return as_generator(type << "Wrapper.Cb, GCHandle.ToIntPtr(" << param_name << "_handle), efl.eo.Globals.free_gchandle")
             .generate(sink, param.type, context);
       }
     else
       arg += escape_keyword(param.param_name);

     return as_generator(arg).generate(sink, attributes::unused, context);
   }
} const argument_invocation {};

struct native_convert_in_variable_generator
{
   template <typename OutputIterator, typename Context>
   bool generate(OutputIterator sink, attributes::parameter_def const& param, Context const& context) const
   {
      if (param.direction != attributes::parameter_direction::in)
        return true;

      if (param.type.c_type == "Eina_Binbuf *" || param.type.c_type == "const Eina_Binbuf *")
        {
           return as_generator(
                "var " << string << " = new eina.Binbuf(" << escape_keyword(param.param_name) << ", " << (param.type.has_own ? "true" : "false") << ");\n"
             ).generate(sink, in_variable_name(param.param_name), context);
        }
     else if (param.type.c_type == "Eina_Hash *" || param.type.c_type == "const Eina_Hash *")
       {
          attributes::complex_type_def const* complex = efl::eina::get<attributes::complex_type_def>(&param.type.original_type);
          if (!complex || complex->subtypes.size() != 2)
            return false;
          return as_generator(
               "var " << string << " = new " << type << "(" << escape_keyword(param.param_name)
               << ", " << (param.type.has_own ? "true" : "false")
               << ", " << (complex->subtypes.front().has_own ? "true" : "false")
               << ", " << (complex->subtypes.back().has_own ? "true" : "false")
               << ");\n"
            ).generate(sink, std::make_tuple(in_variable_name(param.param_name), param.type), context);
       }
     else if (param.type.c_type == "Eina_Array *" || param.type.c_type == "const Eina_Array *"
              || param.type.c_type == "Eina_List *" || param.type.c_type == "const Eina_List *"
     )
       {
          attributes::complex_type_def const* complex = efl::eina::get<attributes::complex_type_def>(&param.type.original_type);
          if (!complex)
            return false;
          return as_generator(
               "var " << string << " = new " << type << "(" << escape_keyword(param.param_name)
               << ", " << (param.type.has_own ? "true" : "false")
               << ", " << (complex->subtypes.front().has_own ? "true" : "false")
               << ");\n"
            ).generate(sink, std::make_tuple(in_variable_name(param.param_name), param.type), context);
       }
      return true;
   }

} const native_convert_in_variable {};

struct convert_in_variable_generator
{
   template <typename OutputIterator, typename Context>
   bool generate(OutputIterator sink, attributes::parameter_def const& param, Context const& context) const
   {
      if (param.direction != attributes::parameter_direction::in)
        return true;

      if (param.type.c_type == "Eina_Binbuf *" || param.type.c_type == "const Eina_Binbuf *")
        {
           auto var_name = in_variable_name(param.param_name);
           if (!as_generator(
                 "var " << string << " = " << escape_keyword(param.param_name) << ".Handle;\n"
              ).generate(sink, var_name, context))
             return false;
           if (param.type.has_own)
             {
                return as_generator(
                     escape_keyword(param.param_name) << ".Own = false;\n"
                  ).generate(sink, attributes::unused, context);
             }
        }
      else if (param.type.c_type == "Eina_Hash *" || param.type.c_type == "const Eina_Hash *")
        {
           attributes::complex_type_def const* complex = efl::eina::get<attributes::complex_type_def>(&param.type.original_type);
           if (!complex || complex->subtypes.size() != 2)
             return false;
           auto var_name = in_variable_name(param.param_name);
           if (!as_generator(
                 "var " << string << " = " << escape_keyword(param.param_name) << ".Handle;\n"
              ).generate(sink, var_name, context))
             return false;
           if (param.type.has_own && !as_generator(
                     escape_keyword(param.param_name) << ".SetOwn(false);\n"
                  ).generate(sink, attributes::unused, context))
             return false;
           if (complex->subtypes.front().has_own && !as_generator(
                     escape_keyword(param.param_name) << ".SetOwnKey(false);\n"
                  ).generate(sink, attributes::unused, context))
             return false;
           if (complex->subtypes.back().has_own && !as_generator(
                     escape_keyword(param.param_name) << ".SetOwnValue(false);\n"
                     << escape_keyword(param.param_name) << ".UnSetFreeCb();\n"
                  ).generate(sink, attributes::unused, context))
             return false;
        }
      else if (param.type.c_type == "Eina_Array *" || param.type.c_type == "const Eina_Array *"
               || param.type.c_type == "Eina_List *" || param.type.c_type == "const Eina_List *"
      )
        {
           attributes::complex_type_def const* complex = efl::eina::get<attributes::complex_type_def>(&param.type.original_type);
           if (!complex)
             return false;
           auto var_name = in_variable_name(param.param_name);
           if (!as_generator(
                 "var " << string << " = " << escape_keyword(param.param_name) << ".Handle;\n"
              ).generate(sink, var_name, context))
             return false;
           if (param.type.has_own && !as_generator(
                     escape_keyword(param.param_name) << ".Own = false;\n"
                  ).generate(sink, attributes::unused, context))
             return false;
           if (complex->subtypes.front().has_own && !as_generator(
                     escape_keyword(param.param_name) << ".OwnContent = false;\n"
                  ).generate(sink, attributes::unused, context))
             return false;
        }
      return true;
   }

} const convert_in_variable {};
  
struct convert_out_variable_generator
{
   template <typename OutputIterator, typename Context>
   bool generate(OutputIterator sink, attributes::parameter_def const& param, Context const& context) const
   {
      if (param_is_acceptable(param, "Eina_Stringshare *", !WANT_OWN, WANT_OUT))
        {
           return as_generator(
               marshall_type << " " << string << " = default(" << marshall_type << ");\n"
             ).generate(sink, std::make_tuple(param, out_variable_name(param.param_name), param), context);
        }
      else if (param_is_acceptable(param, "const char *", !WANT_OWN, WANT_OUT))
        {
           return as_generator(
               marshall_type << " " << string << " = default(" << marshall_type << ");\n"
            ).generate(sink, std::make_tuple(param, out_variable_name(param.param_name), param), context);
        }
      else if (param_is_acceptable(param, "Eina_Binbuf *", WANT_OWN, WANT_OUT)
               || param_is_acceptable(param, "Eina_Binbuf *", !WANT_OWN, WANT_OUT)
               || param_is_acceptable(param, "const Eina_Binbuf *", WANT_OWN, WANT_OUT)
               || param_is_acceptable(param, "const Eina_Binbuf *", !WANT_OWN, WANT_OUT)
              )
        {
           return as_generator(
               "System.IntPtr " << string << " = System.IntPtr.Zero;\n"
             ).generate(sink, out_variable_name(param.param_name), context);
        }
      else if (param_is_acceptable(param, "Eina_Array *", WANT_OWN, WANT_OUT)
               || param_is_acceptable(param, "Eina_Array *", !WANT_OWN, WANT_OUT)
               || param_is_acceptable(param, "const Eina_Array *", WANT_OWN, WANT_OUT)
               || param_is_acceptable(param, "const Eina_Array *", !WANT_OWN, WANT_OUT)
               || param_is_acceptable(param, "Eina_List *", WANT_OWN, WANT_OUT)
               || param_is_acceptable(param, "Eina_List *", !WANT_OWN, WANT_OUT)
               || param_is_acceptable(param, "const Eina_List *", WANT_OWN, WANT_OUT)
               || param_is_acceptable(param, "const Eina_List *", !WANT_OWN, WANT_OUT)
               || param_is_acceptable(param, "Eina_Hash *", WANT_OWN, WANT_OUT)
               || param_is_acceptable(param, "Eina_Hash *", !WANT_OWN, WANT_OUT)
               || param_is_acceptable(param, "const Eina_Hash *", WANT_OWN, WANT_OUT)
               || param_is_acceptable(param, "const Eina_Hash *", !WANT_OWN, WANT_OUT)
              )
        {
           return as_generator(
               "System.IntPtr " << string << " = System.IntPtr.Zero;\n"
             ).generate(sink, out_variable_name(param.param_name), context);
        }
      return true;
   }

} const convert_out_variable {};

struct native_convert_out_variable_generator
{
   template <typename OutputIterator, typename Context>
   bool generate(OutputIterator sink, attributes::parameter_def const& param, Context const& context) const
   {
      if (param_is_acceptable(param, "const char *", !WANT_OWN, WANT_OUT)
          || param_is_acceptable(param, "Eina_Stringshare *", !WANT_OWN, WANT_OUT))
        {
           return as_generator(
                  type << " " << string << " = default(" << type << ");\n"
             ).generate(sink, std::make_tuple(param, out_variable_name(param.param_name), param), context);
        }
      else if (param_is_acceptable(param, "Eina_Binbuf *", WANT_OWN, WANT_OUT)
               || param_is_acceptable(param, "Eina_Binbuf *", !WANT_OWN, WANT_OUT)
               || param_is_acceptable(param, "const Eina_Binbuf *", WANT_OWN, WANT_OUT)
               || param_is_acceptable(param, "const Eina_Binbuf *", !WANT_OWN, WANT_OUT)
              )
        {
           return as_generator(
               "eina.Binbuf " << string << ";\n"
             ).generate(sink, out_variable_name(param.param_name), context);
        }
      else if (param_is_acceptable(param, "Eina_Array *", WANT_OWN, WANT_OUT)
               || param_is_acceptable(param, "Eina_Array *", !WANT_OWN, WANT_OUT)
               || param_is_acceptable(param, "const Eina_Array *", WANT_OWN, WANT_OUT)
               || param_is_acceptable(param, "const Eina_Array *", !WANT_OWN, WANT_OUT)
               || param_is_acceptable(param, "Eina_List *", WANT_OWN, WANT_OUT)
               || param_is_acceptable(param, "Eina_List *", !WANT_OWN, WANT_OUT)
               || param_is_acceptable(param, "const Eina_List *", WANT_OWN, WANT_OUT)
               || param_is_acceptable(param, "const Eina_List *", !WANT_OWN, WANT_OUT)
               || param_is_acceptable(param, "Eina_Hash *", WANT_OWN, WANT_OUT)
               || param_is_acceptable(param, "Eina_Hash *", !WANT_OWN, WANT_OUT)
               || param_is_acceptable(param, "const Eina_Hash *", WANT_OWN, WANT_OUT)
               || param_is_acceptable(param, "const Eina_Hash *", !WANT_OWN, WANT_OUT)
              )
        {
           attributes::complex_type_def const* complex = efl::eina::get<attributes::complex_type_def>(&param.type.original_type);
           if (!complex)
             return false;
           return as_generator(
               type << " " << string << ";\n"
             ).generate(sink, std::make_tuple(param.type, out_variable_name(param.param_name)), context);
        }
      else if (param.direction == attributes::parameter_direction::out)
        {
           // Assign a default value to the out variable in case we end up in the catch clause.
           return as_generator(
                   string << " = default(" << type << ");"
                   ).generate(sink, std::make_tuple(param.param_name, param), context);
        }
      return true;
   }

} const native_convert_out_variable {};

struct convert_out_assign_generator
{
   template <typename OutputIterator, typename Context>
   bool generate(OutputIterator sink, attributes::parameter_def const& param, Context const& context) const
   {
      if (param_is_acceptable(param, "Eina_Stringshare *", !WANT_OWN, WANT_OUT))
        {
           if (!as_generator(
                escape_keyword(param.param_name) << " = Marshal.PtrToStringAnsi(" << out_variable_name(param.param_name) << ");\n"
             ).generate(sink, attributes::unused, context))
               return true;

           if (param.type.has_own)
             {
                if (!as_generator(
                    "eina.Stringshare.eina_stringshare_del(" << out_variable_name(param.param_name) << ");\n"
                  ).generate(sink, attributes::unused, context))
                    return false;
             }

        }
      else if (param_is_acceptable(param, "const char *", !WANT_OWN, WANT_OUT))
        {
            return as_generator(
                 string << " = Marshal.PtrToStringAuto(" << out_variable_name(param.param_name) << ");\n"
               ).generate(sink, escape_keyword(param.param_name), context);
        }
      else if (param_is_acceptable(param, "Eina_Binbuf *", WANT_OWN, WANT_OUT)
               || param_is_acceptable(param, "Eina_Binbuf *", !WANT_OWN, WANT_OUT)
               || param_is_acceptable(param, "const Eina_Binbuf *", WANT_OWN, WANT_OUT)
               || param_is_acceptable(param, "const Eina_Binbuf *", !WANT_OWN, WANT_OUT)
              )
        {
           return as_generator(
               string << " = new eina.Binbuf(" << string << ", " << (param.type.has_own ? "true" : "false") << ");\n"
             ).generate(sink, std::make_tuple(escape_keyword(param.param_name), out_variable_name(param.param_name)), context);
        }
      else if (param_is_acceptable(param, "Eina_Hash *", WANT_OWN, WANT_OUT)
               || param_is_acceptable(param, "Eina_Hash *", !WANT_OWN, WANT_OUT)
               || param_is_acceptable(param, "const Eina_Hash *", WANT_OWN, WANT_OUT)
               || param_is_acceptable(param, "const Eina_Hash *", !WANT_OWN, WANT_OUT)
              )
        {
           attributes::complex_type_def const* complex = efl::eina::get<attributes::complex_type_def>(&param.type.original_type);
           if (!complex || complex->subtypes.size() != 2)
             return false;
           return as_generator(
               string << " = new " << type << "(" << string
               << ", " << (param.type.has_own ? "true" : "false")
               << ", " << (complex->subtypes.front().has_own ? "true" : "false")
               << ", " << (complex->subtypes.back().has_own ? "true" : "false")
               << ");\n"
             ).generate(sink, std::make_tuple(escape_keyword(param.param_name), param.type, out_variable_name(param.param_name)), context);
        }
      else if (param_is_acceptable(param, "Eina_Array *", WANT_OWN, WANT_OUT)
               || param_is_acceptable(param, "Eina_Array *", !WANT_OWN, WANT_OUT)
               || param_is_acceptable(param, "const Eina_Array *", WANT_OWN, WANT_OUT)
               || param_is_acceptable(param, "const Eina_Array *", !WANT_OWN, WANT_OUT)
               || param_is_acceptable(param, "Eina_List *", WANT_OWN, WANT_OUT)
               || param_is_acceptable(param, "Eina_List *", !WANT_OWN, WANT_OUT)
               || param_is_acceptable(param, "const Eina_List *", WANT_OWN, WANT_OUT)
               || param_is_acceptable(param, "const Eina_List *", !WANT_OWN, WANT_OUT)
              )
        {
           attributes::complex_type_def const* complex = efl::eina::get<attributes::complex_type_def>(&param.type.original_type);
           if (!complex)
             return false;
           return as_generator(
               string << " = new " << type << "(" << string
               << ", " << (param.type.has_own ? "true" : "false")
               << ", " << (complex->subtypes.front().has_own ? "true" : "false")
               << ");\n"
             ).generate(sink, std::make_tuple(escape_keyword(param.param_name), param.type, out_variable_name(param.param_name)), context);
        }
      return true;
   }

} const convert_out_assign {};

struct convert_return_variable_generator
{
   template <typename OutputIterator, typename Context>
   bool generate(OutputIterator sink, attributes::type_def const& ret_type, Context const& context) const
   {
     if (ret_type.c_type != "void")
       return as_generator("var _ret_var = ").generate(sink, attributes::unused, context);
     return true;
   }

} const convert_return_variable {};


struct convert_return_generator
{
   template <typename OutputIterator, typename Context>
   bool generate(OutputIterator sink, attributes::type_def const& ret_type, Context const& context) const
   {
     if (ret_type.c_type == "Eina_Stringshare *" && !ret_type.has_own)
       {
         if (!as_generator("string _mng_str = Marshal.PtrToStringAuto(_ret_var);\n")
             .generate(sink, attributes::unused, context))
             return false;

         if (!as_generator(scope_tab << scope_tab << "return _mng_str;\n").generate(sink, attributes::unused, context))
             return false;
       }
     else if (ret_type.c_type == "const char *" && !ret_type.has_own)
       {

         if (!as_generator("string _mng_str = Marshal.PtrToStringAuto(_ret_var);\n")
             .generate(sink, attributes::unused, context))
             return false;

         if (!as_generator(scope_tab << scope_tab << "return _mng_str;\n").generate(sink, attributes::unused, context))
             return false;

       }
     else if (ret_type.c_type == "Eina_Binbuf *" || ret_type.c_type == "const Eina_Binbuf *")
       {
           if (!as_generator("var _binbuf_ret = new eina.Binbuf(_ret_var, " << std::string{ret_type.has_own ? "true" : "false"} << ");\n"
                             << scope_tab << scope_tab << "return _binbuf_ret;\n")
             .generate(sink, attributes::unused, context))
             return false;
       }
     else if (ret_type.c_type == "Eina_Hash *" || ret_type.c_type == "const Eina_Hash *")
       {
           attributes::complex_type_def const* complex = efl::eina::get<attributes::complex_type_def>(&ret_type.original_type);
           if (!complex || complex->subtypes.size() != 2)
             return false;
           if (!as_generator("return new " << type << "(_ret_var, " << std::string{ret_type.has_own ? "true" : "false"}
                   << ", " << (complex->subtypes.front().has_own ? "true" : "false")
                   << ", " << (complex->subtypes.back().has_own ? "true" : "false")
                   << ");\n")
             .generate(sink, ret_type, context))
             return false;
       }
     else if (ret_type.c_type == "Eina_Array *" || ret_type.c_type == "const Eina_Array *"
              || ret_type.c_type == "Eina_List *" || ret_type.c_type == "const Eina_List *"
     )
       {
           attributes::complex_type_def const* complex = efl::eina::get<attributes::complex_type_def>(&ret_type.original_type);
           if (!complex)
             return false;
           if (!as_generator("return new " << type << "(_ret_var, " << std::string{ret_type.has_own ? "true" : "false"}
                   << ", " << (complex->subtypes.front().has_own ? "true" : "false")
                   << ");\n")
             .generate(sink, ret_type, context))
             return false;
       }
     else if (ret_type.c_type != "void")
       {
         return as_generator("return _ret_var;\n").generate(sink, ret_type, context);
       }
     return true;
   }

} const convert_return {};

// Native (virtual wrappers) generators
struct native_convert_out_assign_generator
{
   attributes::klass_def const* klass;

   template <typename OutputIterator, typename Context>
   bool generate(OutputIterator sink, attributes::parameter_def const& param, Context const& context) const
   {
      if (param_is_acceptable(param, "Eina_Stringshare *", !WANT_OWN, WANT_OUT))
        {
           return as_generator(
                string << "= efl.eo.Globals.cached_stringshare_to_intptr(((" << string << "Inherit)wrapper).cached_stringshares, " << string << ");\n"
              ).generate(sink, std::make_tuple(escape_keyword(param.param_name), klass->cxx_name, out_variable_name(param.param_name)), context);
        }
      else if (param_is_acceptable(param, "const char *", !WANT_OWN, WANT_OUT))
        {
           return as_generator(
                string << "= efl.eo.Globals.cached_string_to_intptr(((" << string << "Inherit)wrapper).cached_strings, " << string << ");\n"
              ).generate(sink, std::make_tuple(escape_keyword(param.param_name), klass->cxx_name, out_variable_name(param.param_name)), context);
        }
      else if (param_is_acceptable(param, "Eina_Binbuf *", WANT_OWN, WANT_OUT)
               || param_is_acceptable(param, "Eina_Binbuf *", !WANT_OWN, WANT_OUT)
               || param_is_acceptable(param, "const Eina_Binbuf *", WANT_OWN, WANT_OUT)
               || param_is_acceptable(param, "const Eina_Binbuf *", !WANT_OWN, WANT_OUT)
              )
        {
           if (!as_generator(
                string << " = " << string << ".Handle;\n"
              ).generate(sink, std::make_tuple(escape_keyword(param.param_name), out_variable_name(param.param_name)), context))
             return false;
           if (param.type.has_own)
             return as_generator(
                 string << ".Own = false;\n"
               ).generate(sink, out_variable_name(param.param_name), context);
        }
      else if (param_is_acceptable(param, "Eina_Hash *", WANT_OWN, WANT_OUT)
               || param_is_acceptable(param, "Eina_Hash *", !WANT_OWN, WANT_OUT)
               || param_is_acceptable(param, "const Eina_Hash *", WANT_OWN, WANT_OUT)
               || param_is_acceptable(param, "const Eina_Hash *", !WANT_OWN, WANT_OUT)
              )
        {
           attributes::complex_type_def const* complex = efl::eina::get<attributes::complex_type_def>(&param.type.original_type);
           if (!complex || complex->subtypes.size() != 2)
             return false;
           auto outvar = out_variable_name(param.param_name);
           if (!as_generator(
                string << " = " << string << ".Handle;\n"
              ).generate(sink, std::make_tuple(escape_keyword(param.param_name), outvar), context))
             return false;
           if (param.type.has_own && !as_generator(
                 string << ".SetOwn(false);\n"
               ).generate(sink, outvar, context))
             return false;
           if (complex->subtypes.front().has_own && !as_generator(
                 string << ".SetOwnKey(false);\n"
               ).generate(sink, outvar, context))
             return false;
           if (complex->subtypes.back().has_own && !as_generator(
                 string << ".SetOwnValue(false);\n"
                 << string << ".UnSetFreeCb();\n"
               ).generate(sink, std::make_tuple(outvar, outvar), context))
             return false;
        }
      else if (param_is_acceptable(param, "Eina_Array *", WANT_OWN, WANT_OUT)
               || param_is_acceptable(param, "Eina_Array *", !WANT_OWN, WANT_OUT)
               || param_is_acceptable(param, "const Eina_Array *", WANT_OWN, WANT_OUT)
               || param_is_acceptable(param, "const Eina_Array *", !WANT_OWN, WANT_OUT)
               || param_is_acceptable(param, "Eina_List *", WANT_OWN, WANT_OUT)
               || param_is_acceptable(param, "Eina_List *", !WANT_OWN, WANT_OUT)
               || param_is_acceptable(param, "const Eina_List *", WANT_OWN, WANT_OUT)
               || param_is_acceptable(param, "const Eina_List *", !WANT_OWN, WANT_OUT)
              )
        {
           attributes::complex_type_def const* complex = efl::eina::get<attributes::complex_type_def>(&param.type.original_type);
           if (!complex)
             return false;
           auto outvar = out_variable_name(param.param_name);
           if (!as_generator(
                string << " = " << string << ".Handle;\n"
              ).generate(sink, std::make_tuple(escape_keyword(param.param_name), outvar), context))
             return false;
           if (param.type.has_own && !as_generator(
                 string << ".Own = false;\n"
               ).generate(sink, outvar, context))
             return false;
           if (complex->subtypes.front().has_own && !as_generator(
                 string << ".OwnContent = false;\n"
               ).generate(sink, outvar, context))
             return false;
        }
      return true;
   }

};

struct native_convert_return_variable_generator
{
   template <typename OutputIterator, typename Context>
   bool generate(OutputIterator sink, attributes::type_def const& ret_type, Context const& context) const
   {
     if (ret_type.c_type != "void")
       return as_generator("var _ret_var = ").generate(sink, attributes::unused, context);
     return true;
   }

} const native_convert_return_variable {};

struct native_convert_return_generator
{
   attributes::klass_def const* klass;

   template <typename OutputIterator, typename Context>
   bool generate(OutputIterator sink, attributes::type_def const& ret_type, Context const& context) const
   {
     if (ret_type.c_type == "const char *")
       {
          if(!ret_type.has_own)
            {
               return as_generator(
                    "return efl.eo.Globals.cached_string_to_intptr(((" << string << "Inherit)wrapper).cached_strings, _ret_var);\n"
                 ).generate(sink, klass->cxx_name, context);
            }
          else
            {
                return as_generator("return _ret_var;\n"
                    ).generate(sink, attributes::unused, context);
            }
       }
     else if (ret_type.c_type == "Eina_Stringshare *") { // Correct check for string?
         if (!ret_type.has_own)
           {
              return as_generator(
                   "return efl.eo.Globals.cached_stringshare_to_intptr(((" << string << "Inherit)wrapper).cached_stringshares, _ret_var);\n"
                ).generate(sink, klass->cxx_name, context);
           }
         else
           {
              return as_generator("return _ret_var;\n")
                 .generate(sink, attributes::unused, context);
           }
     }
     else if (ret_type.c_type == "Eina_Binbuf *" || ret_type.c_type == "const Eina_Binbuf *")
       {
          if (ret_type.has_own && !as_generator("_ret_var.Own = false; ")
              .generate(sink, attributes::unused, context))
            return false;

          return as_generator("return _ret_var.Handle;\n")
            .generate(sink, attributes::unused, context);
       }
     else if (ret_type.c_type == "Eina_Hash *" || ret_type.c_type == "const Eina_Hash *")
       {
          attributes::complex_type_def const* complex = efl::eina::get<attributes::complex_type_def>(&ret_type.original_type);
          if (!complex || complex->subtypes.size() != 2)
            return false;
          if (ret_type.has_own && !as_generator("_ret_var.SetOwn(false); ")
              .generate(sink, attributes::unused, context))
            return false;
          if (complex->subtypes.front().has_own && !as_generator("_ret_var.SetOwnKey(false); ")
              .generate(sink, attributes::unused, context))
            return false;
          if (complex->subtypes.back().has_own && !as_generator("_ret_var.SetOwnValue(false); _ret_var.UnSetFreeCb(); ")
              .generate(sink, attributes::unused, context))
            return false;

          return as_generator("return _ret_var.Handle;\n")
            .generate(sink, attributes::unused, context);
       }
     else if (ret_type.c_type == "Eina_Array *" || ret_type.c_type == "const Eina_Array *"
              || ret_type.c_type == "Eina_List *" || ret_type.c_type == "const Eina_List *"
     )
       {
          attributes::complex_type_def const* complex = efl::eina::get<attributes::complex_type_def>(&ret_type.original_type);
          if (!complex)
            return false;
          if (ret_type.has_own && !as_generator("_ret_var.Own = false; ")
              .generate(sink, attributes::unused, context))
            return false;
          if (complex->subtypes.front().has_own && !as_generator("_ret_var.OwnContent = false; ")
              .generate(sink, attributes::unused, context))
            return false;

          return as_generator("return _ret_var.Handle;\n")
            .generate(sink, attributes::unused, context);
       }
     else if (ret_type.c_type != "void")
       return as_generator("return _ret_var;\n").generate(sink, ret_type, context);
     return true;
   }

};

struct native_convert_out_assign_parameterized
{
    native_convert_out_assign_generator const operator()(attributes::klass_def const &klass) const
    {
        return {&klass};
    }
} const native_convert_out_assign;

struct native_convert_return_parameterized
{
    native_convert_return_generator const operator()(attributes::klass_def const &klass) const
    {
        return {&klass};
    }
} const native_convert_return;

struct convert_function_pointer_generator
{
   template <typename OutputIterator, typename Context>
   bool generate(OutputIterator sink, attributes::parameter_def const& param, Context const& context) const
   {
      if (param.direction != attributes::parameter_direction::in)
        return true;

      if (!param.type.original_type.visit(is_fp_visitor{}))
          return true;

      std::string param_name = escape_keyword(param.param_name);
      // Allocate GCHandle in "param_name"_handle for param;
      if (!as_generator(
                  "GCHandle " << param_name << "_handle = GCHandle.Alloc(" << param_name << ");\n"
                  ).generate(sink, attributes::unused, context))
      return false;

      return true;
   }

} const convert_function_pointer {};

struct native_convert_function_pointer_generator
{
   template <typename OutputIterator, typename Context>
   bool generate(OutputIterator sink, attributes::parameter_def const& param, Context const& context) const
   {
      if (param.direction != attributes::parameter_direction::in)
        return true;

      if (!param.type.original_type.visit(is_fp_visitor{}))
        return true;

      // Getting the type through C api
      const Eolian_Typedecl *tpd = ::eolian_typedecl_alias_get_by_name(NULL, param.type.c_type.c_str());
      if (eolian_typedecl_type_get(tpd) != EOLIAN_TYPEDECL_FUNCTION_POINTER)
        return true;

      const Eolian_Function *fd = eolian_typedecl_function_pointer_get(tpd);
      attributes::function_def f(fd, EOLIAN_FUNCTION_POINTER, NULL);

      std::string param_name = escape_keyword(param.param_name);
      // Allocate GCHandle in "param_name"_handle for param;
      if (!as_generator(
                  scope_tab << type << "Wrapper " << param_name << "_wrapper = new " << type << "Wrapper("
                      << param_name << ", " << param_name << "_data, " << param_name << "_free_cb);\n"
                  ).generate(sink, std::make_tuple(param.type, param.type), context))
      return false;
      // FIXME What about calling the free function?

      return true;
   }

} const native_convert_function_pointer {};

}

#endif
