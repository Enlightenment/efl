#ifndef EOLIAN_CXX_PARAMETER_HH
#define EOLIAN_CXX_PARAMETER_HH

#include "grammar/generator.hpp"
#include "grammar/klass_def.hpp"
#include "grammar/type.hpp"

namespace efl { namespace eolian { namespace grammar {

struct add_reference_visitor
{
   typedef void result_type;
   template <typename T>
   void operator()(T& object) const
   {
      object.pointers.insert(object.pointers.begin(), {{attributes::qualifier_info::is_none}, true});
   }
   void operator()(attributes::complex_type_def& complex) const
   {
     (*this)(complex.outer);
   }
};
      
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
      return as_generator
        (
         " ::efl::eolian::" << string << "_traits<"
         << type << ">::type"
        ).generate(sink, std::make_tuple(dir, param), context);
   }
};

template <>
struct is_eager_generator<parameter_type_generator> : std::true_type {};
namespace type_traits {
template <>
struct attributes_needed<parameter_type_generator> : std::integral_constant<int, 1> {};  
}

parameter_type_generator const parameter_type;

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
namespace type_traits {
template <>
struct attributes_needed<parameter_generator> : std::integral_constant<int, 1> {};  
}
parameter_generator const parameter;
      
} } }

#endif
