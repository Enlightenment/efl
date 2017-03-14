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
           || param_is_acceptable(param, "Eina_Stringshare *", WANT_OWN, WANT_OUT)
           || param_is_acceptable(param, "Eina_Stringshare *", !WANT_OWN, WANT_OUT))
     return true;

   return false;
}

inline std::string out_variable_name(std::string const& param_name)
{
   return "_out_" + escape_keyword(param_name);
}

struct parameter_generator
{
   template <typename OutputIterator, typename Context>
   bool generate(OutputIterator sink, attributes::parameter_def const& param, Context const& context) const
   {
     bool is_out = param.direction != attributes::parameter_direction::in;
     return as_generator(
             (is_out ? " out ": " ") << type << " " << string
        ).generate(sink, std::make_tuple(param, escape_keyword(param.param_name)), context);
   }
} const parameter {};

struct marshall_parameter_generator
{
   template <typename OutputIterator, typename Context>
   bool generate(OutputIterator sink, attributes::parameter_def const& param, Context const& context) const
   {
      bool is_out = param.direction != attributes::parameter_direction::in;
      return as_generator(
              (is_out ? " out " : " ") << marshall_type << " " << string
         ).generate(sink, std::make_tuple(param, escape_keyword(param.param_name)), context);
   }
} const marshall_parameter {};
  
inline std::string argument_forward(attributes::parameter_def const& param)
{
    //FIXME Support correct ownership
     return escape_keyword(param.param_name);
}

struct argument_generator
{
   template <typename OutputIterator, typename Context>
   bool generate(OutputIterator sink, attributes::parameter_def const& param, Context const& context) const
   {
     std::string direction =  param.direction != attributes::parameter_direction::in ? " out " : "";
     return as_generator(
             direction << argument_forward(param)
        ).generate(sink, attributes::unused, context);
   }

} const argument {};

// Generates the correct parameter name when invoking a function
struct argument_invocation_generator
{
   template <typename OutputIterator, typename Context>
   bool generate(OutputIterator sink, attributes::parameter_def const& param, Context const& context) const
   {
     std::string arg;

     if (param.direction != attributes::parameter_direction::in)
       arg = " out ";

     if (param_should_use_out_var(param))
       arg += out_variable_name(param.param_name);
     else
       arg += escape_keyword(param.param_name);

     return as_generator(arg).generate(sink, attributes::unused, context);
   }
} const argument_invocation {};
  
struct convert_out_variable_generator
{
   template <typename OutputIterator, typename Context>
   bool generate(OutputIterator sink, attributes::parameter_def const& param, Context const& context) const
   {
      if (param_is_acceptable(param, "Eina_Stringshare *", WANT_OWN, WANT_OUT)
          || param_is_acceptable(param, "Eina_Stringshare *", !WANT_OWN, WANT_OUT))
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
      return true;
   }

} const convert_out_variable {};

struct native_convert_out_variable_generator
{
   template <typename OutputIterator, typename Context>
   bool generate(OutputIterator sink, attributes::parameter_def const& param, Context const& context) const
   {
      if (param_is_acceptable(param, "const char *", !WANT_OWN, WANT_OUT)
          || param_is_acceptable(param, "Eina_Stringshare *", !WANT_OWN, WANT_OUT)
          || param_is_acceptable(param, "Eina_Stringshare *", WANT_OWN, WANT_OUT))
        {
           return as_generator(
                  type << " " << string << " = default(" << type << ");\n"
             ).generate(sink, std::make_tuple(param, out_variable_name(param.param_name), param), context);
        }
      return true;
   }

} const native_convert_out_variable {};

struct convert_out_assign_generator
{
   template <typename OutputIterator, typename Context>
   bool generate(OutputIterator sink, attributes::parameter_def const& param, Context const& context) const
   {
      if (param_is_acceptable(param, "Eina_Stringshare *", !WANT_OWN, WANT_OUT)
          || param_is_acceptable(param, "Eina_Stringshare *", WANT_OWN, WANT_OUT))
        {
           if (!as_generator(
                escape_keyword(param.param_name) << " = Marshal.PtrToStringAnsi(" << out_variable_name(param.param_name) << "); "
             ).generate(sink, attributes::unused, context))
               return true;

           if (param.type.has_own)
             {
                if (!as_generator(
                    "eina.Stringshare.eina_stringshare_del(" << out_variable_name(param.param_name) << ");"
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
     if (ret_type.c_type == "const char *" || ret_type.c_type == "Eina_Stringshare *")
       {
         if (!as_generator("string _mng_str = Marshal.PtrToStringAuto(_ret_var);\n")
             .generate(sink, attributes::unused, context))
             return false;

         if (ret_type.has_own)
           {
               if (ret_type.c_type == "const char *")
                 {
                     if (!as_generator("Marshal.FreeHGlobal(_ret_var);\n").generate(sink, attributes::unused, context))
                         return false;
                 }
               else if (ret_type.c_type == "Eina_Stringshare *")
                 {
                     if (!as_generator("eina.Stringshare.eina_stringshare_del(_ret_var);\n").generate(sink, attributes::unused, context))
                         return false;
                 }
           }

         if (!as_generator("return _mng_str;").generate(sink, attributes::unused, context))
             return false;
       }
     else if (ret_type.c_type != "void")
       {
         return as_generator("return _ret_var; ").generate(sink, ret_type, context);
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
      if (param_is_acceptable(param, "Eina_Stringshare *", WANT_OWN, WANT_OUT)
          || param_is_acceptable(param, "Eina_Stringshare *", !WANT_OWN, WANT_OUT))
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
     if (ret_type.c_type == "const char *" || ret_type.c_type == "Eina_Stringshare *") { // Correct check for string?
         if (!ret_type.has_own)
           {
               if (ret_type.c_type == "const char *")
                 {
                    return as_generator(
                         "return efl.eo.Globals.cached_string_to_intptr(((" << string << "Inherit)wrapper).cached_strings, _ret_var);"
                      ).generate(sink, klass->cxx_name, context);
                 }
               else if (ret_type.c_type == "Eina_Stringshare *")
                 {
                    return as_generator(
                         "return efl.eo.Globals.cached_stringshare_to_intptr(((" << string << "Inherit)wrapper).cached_stringshares, _ret_var);"
                      ).generate(sink, klass->cxx_name, context);
                 }
           }
         else
           {
               if (ret_type.c_type == "const char *")
                 {
                     return as_generator("return Marshal.StringToHGlobalAuto(_ret_var);")
                         .generate(sink, attributes::unused, context);
                 }
               else if (ret_type.c_type == "Eina_Stringshare *")
                 {
                     return as_generator("return eina.Stringshare.eina_stringshare_add(_ret_var);")
                         .generate(sink, attributes::unused, context);
                 }
           }
     }
     else if (ret_type.c_type != "void")
       return as_generator("return _ret_var; ").generate(sink, ret_type, context);
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
} } }

#endif
