#ifndef EOLIAN_MONO_TYPE_HH
#define EOLIAN_MONO_TYPE_HH

#include "grammar/generator.hpp"
#include "grammar/klass_def.hpp"
#include "grammar/case.hpp"

namespace eolian_mono {

namespace attributes = efl::eolian::grammar::attributes;

template <typename OutputIterator, typename Context>
struct visitor_generate;
      
struct type_generator
{
   type_generator(bool is_return = false)
     : is_return(is_return) {}
  
   template <typename OutputIterator, typename Context>
   bool generate(OutputIterator sink, attributes::type_def const& type, Context const& context) const
   {
      return type.original_type.visit(visitor_generate<OutputIterator, Context>{sink, &context, type.c_type, false, is_return});
   }
   template <typename OutputIterator, typename Context>
   bool generate(OutputIterator sink, attributes::parameter_def const& param, Context const& context) const
   {
      return param.type.original_type.visit(visitor_generate<OutputIterator, Context>{sink, &context, param.c_type
            , param.direction != attributes::parameter_direction::in, false});
   }

   bool is_return;
};

struct type_terminal
{
  type_generator const operator()(bool is_return) const
  {
    return type_generator(is_return);
  }
} const type = {};

type_generator const as_generator(type_terminal)
{
  return type_generator{};
}

}

namespace efl { namespace eolian { namespace grammar {

template <>
struct is_eager_generator< ::eolian_mono::type_generator> : std::true_type {};
template <>
struct is_generator< ::eolian_mono::type_generator> : std::true_type {};
template <>
struct is_generator< ::eolian_mono::type_terminal> : std::true_type {};

namespace type_traits {
template <>
struct attributes_needed< ::eolian_mono::type_generator> : std::integral_constant<int, 1> {};  
template <>
struct attributes_needed< ::eolian_mono::type_terminal> : std::integral_constant<int, 1> {};  
}

} } }

#endif
