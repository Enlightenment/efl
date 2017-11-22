#ifndef EOLIAN_CXX_PARAMETER_HH
#define EOLIAN_CXX_PARAMETER_HH

#include "grammar/generator.hpp"
#include "grammar/klass_def.hpp"
#include "grammar/type.hpp"

namespace efl { namespace eolian { namespace grammar {

struct parameter_type_generator
{
   template <typename OutputIterator, typename Context>
   bool generate(OutputIterator sink, attributes::parameter_def const& param, Context const& context) const
   {
      std::string dir;
      switch(param.direction)
        {
          case attributes::parameter_direction::out:
            dir = "out";
            break;
          case attributes::parameter_direction::inout:
            dir = "inout";
            break;
          case attributes::parameter_direction::in:
            dir = "in";
          break;
        }

      attributes::regular_type_def const* typ =
            efl::eina::get<attributes::regular_type_def>(&param.type.original_type);
      if (typ && typ->is_function_ptr)
        return as_generator("F").generate(sink, attributes::unused, context);

      return as_generator
        (
         " ::efl::eolian::" << string << "_traits<"
         << type << ">::type"
        ).generate(sink, std::make_tuple(dir, param), context);
   }
};

template <>
struct is_eager_generator<parameter_type_generator> : std::true_type {};
template <>
struct is_generator<parameter_type_generator> : std::true_type {};
namespace type_traits {
template <>
struct attributes_needed<parameter_type_generator> : std::integral_constant<int, 1> {};  
}

parameter_type_generator const parameter_type = {};


/* */
struct parameter_generator
{
   template <typename OutputIterator, typename Context>
   bool generate(OutputIterator sink, attributes::parameter_def const& param, Context const& context) const
   {
      return as_generator(parameter_type << " " << string).generate(sink, std::make_tuple(param, param.param_name), context);
   }
};

template <>
struct is_eager_generator<parameter_generator> : std::true_type {};
template <>
struct is_generator<parameter_generator> : std::true_type {};
namespace type_traits {
template <>
struct attributes_needed<parameter_generator> : std::integral_constant<int, 1> {};
}
parameter_generator const parameter = {};


/* */
struct parameter_as_argument_generator
{
   template <typename OutputIterator, typename Context>
   bool generate(OutputIterator sink, attributes::parameter_def const& param, Context const& context) const
   {
      return as_generator(parameter_type << "(" << string << ")").generate(sink, std::make_tuple(param, param.param_name), context);
   }
};

template <>
struct is_eager_generator<parameter_as_argument_generator> : std::true_type {};
namespace type_traits {
template <>
struct attributes_needed<parameter_as_argument_generator> : std::integral_constant<int, 1> {};
}
parameter_as_argument_generator const parameter_as_argument = {};
      
} } }

#endif
