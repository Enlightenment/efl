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

struct parameter_generator
{
   template <typename OutputIterator, typename Context>
   bool generate(OutputIterator sink, attributes::parameter_def const& param, Context const& context) const
   {
     return as_generator(type << " " << string).generate(sink, std::make_tuple(param, escape_keyword(param.param_name)), context);
   }
} const parameter {};

struct marshall_parameter_generator
{
   template <typename OutputIterator, typename Context>
   bool generate(OutputIterator sink, attributes::parameter_def const& param, Context const& context) const
   {
     return as_generator(marshall_type << " " << string).generate(sink, std::make_tuple(param, escape_keyword(param.param_name)), context);
   }
} const marshall_parameter {};
  
inline std::string out_variable_name(std::string const& param_name)
{
   return "_out_" + escape_keyword(param_name);
}

inline std::string argument_forward(attributes::parameter_def const& param)
{
   if (param.type.has_own && param.c_type == "Eina_Stringshare *")
     if (param.direction == attributes::parameter_direction::in)
       return "eina.Stringshare.eina_stringshare_add(" + escape_keyword(param.param_name) + ")";
     else
       return out_variable_name(param.param_name);
   else
     return escape_keyword(param.param_name);
}

struct argument_generator
{
   template <typename OutputIterator, typename Context>
   bool generate(OutputIterator sink, attributes::parameter_def const& param, Context const& context) const
   {
     return as_generator((param.direction != attributes::parameter_direction::in ? " out " : "")
       << argument_forward(param)).generate(sink, attributes::unused, context);
   }

} const argument {};
  
struct convert_out_variable_generator
{
   template <typename OutputIterator, typename Context>
   bool generate(OutputIterator sink, attributes::parameter_def const& param, Context const& context) const
   {
     if (param.type.has_own && param.type.c_type == "Eina_Stringshare *" && param.direction != attributes::parameter_direction::in)
       return as_generator(marshall_type << " " << string << " = default(" << marshall_type << "); ")
         .generate(sink, std::make_tuple(param.type, out_variable_name(param.param_name), param.type), context);
     return true;
   }

} const convert_out_variable {};

struct convert_out_assign_generator
{
   template <typename OutputIterator, typename Context>
   bool generate(OutputIterator sink, attributes::parameter_def const& param, Context const& context) const
   {
     if (param.type.has_own && param.type.c_type == "Eina_Stringshare *" && param.direction != attributes::parameter_direction::in)
       return as_generator(escape_keyword(param.param_name) << " = Marshal.PtrToStringAnsi(" << out_variable_name(param.param_name) << "); ")
         .generate(sink, attributes::unused, context);
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
     if (ret_type.has_own && ret_type.c_type == "Eina_Stringshare *")
       return as_generator("string _mng_str = Marshal.PtrToStringAnsi(_ret_var); eina.Stringshare.eina_stringshare_del(_ret_var); return _mng_str; ")
         .generate(sink, attributes::unused, context);
     else if (ret_type.c_type != "void")
       return as_generator("return _ret_var; ").generate(sink, ret_type, context);
     return true;
   }

} const convert_return {};

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
struct is_eager_generator< ::eolian_mono::convert_out_variable_generator> : std::true_type {};
template <>
struct is_generator< ::eolian_mono::convert_out_variable_generator> : std::true_type {};

namespace type_traits {
template <>
struct attributes_needed< ::eolian_mono::convert_out_variable_generator> : std::integral_constant<int, 1> {};
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
} } }

#endif
