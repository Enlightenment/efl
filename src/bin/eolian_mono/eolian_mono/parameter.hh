#ifndef EOLIAN_MONO_PARAMETER_HH
#define EOLIAN_MONO_PARAMETER_HH

#include "grammar/generator.hpp"
#include "grammar/klass_def.hpp"
#include "grammar/case.hpp"
#include "marshall_type.hh"
#include "type.hh"
#include "using_decl.hh"

namespace eolian_mono {

namespace attributes = efl::eolian::grammar::attributes;

struct parameter_generator
{
   template <typename OutputIterator, typename Context>
   bool generate(OutputIterator sink, attributes::parameter_def const& param, Context const& context) const
   {
     return as_generator(type << " " << string).generate(sink, std::make_tuple(param.type, param.param_name), context);
   }
} const parameter {};

struct marshall_parameter_generator
{
   template <typename OutputIterator, typename Context>
   bool generate(OutputIterator sink, attributes::parameter_def const& param, Context const& context) const
   {
     return as_generator(marshall_type << " " << string).generate(sink, std::make_tuple(param.type, param.param_name), context);
   }
} const marshall_parameter {};
  
struct argument_generator
{
   template <typename OutputIterator, typename Context>
   bool generate(OutputIterator sink, attributes::parameter_def const& param, Context const& context) const
   {
     return as_generator(param.param_name).generate(sink, attributes::unused, context);
   }

} const argument {};
  
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
      
} } }

#endif
