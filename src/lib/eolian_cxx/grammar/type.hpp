#ifndef EOLIAN_CXX_TYPE_HH
#define EOLIAN_CXX_TYPE_HH

#include "grammar/generator.hpp"
#include "grammar/klass_def.hpp"
#include "grammar/case.hpp"

namespace efl { namespace eolian { namespace grammar {

template <typename OutputIterator, typename Context>
struct visitor_generate;
      
struct type_generator
{
   template <typename OutputIterator, typename Context>
   bool generate(OutputIterator sink, attributes::type_def const& type, Context const& context) const
   {
      return type.original_type.visit(visitor_generate<OutputIterator, Context>{sink, &context, type.c_type, false});
   }
   template <typename OutputIterator, typename Context>
   bool generate(OutputIterator sink, attributes::parameter_def const& param, Context const& context) const
   {
      return param.type.original_type.visit(visitor_generate<OutputIterator, Context>{sink, &context, param.c_type
            , param.direction != attributes::parameter_direction::in});
   }
};

template <>
struct is_eager_generator<type_generator> : std::true_type {};

namespace type_traits {
template <>
struct attributes_needed<type_generator> : std::integral_constant<int, 1> {};  
}

type_generator const type = {};

} } }

#endif
