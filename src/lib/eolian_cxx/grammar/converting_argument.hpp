#ifndef EOLIAN_CXX_CONVERTING_ARGUMENT_HH
#define EOLIAN_CXX_CONVERTING_ARGUMENT_HH

#include "grammar/generator.hpp"
#include "grammar/klass_def.hpp"

#include "grammar/string.hpp"
#include "grammar/indentation.hpp"
#include "grammar/list.hpp"
#include "grammar/alternative.hpp"
#include "grammar/type.hpp"
#include "grammar/parameter.hpp"
#include "grammar/function_definition.hpp"
#include "grammar/namespace.hpp"
#include "grammar/c_type.hpp"
#include "grammar/attribute_reorder.hpp"

namespace efl { namespace eolian { namespace grammar {

struct converting_argument_generator
{
   struct
   {
     typedef bool result_type;
     template <typename T>
     bool operator()(T const&) const { return false;}
     bool operator()(attributes::regular_type_def const& r) const
     {
       return r.is_function_ptr();
     }
   } static const is_function_ptr;
   template <typename OutputIterator, typename Context>
   bool generate(OutputIterator sink, attributes::parameter_def const& param, Context const& ctx) const
   {
     attributes::qualifier_def qualifier = param.type.original_type.visit(attributes::get_qualifier_visitor{});
     if (param.type.original_type.visit(this->is_function_ptr))
       {
          return as_generator("fw_" << param.param_name << "->data_to_c(), fw_" << param.param_name << "->func_to_c(), fw_"
                              << param.param_name << "->free_to_c()")
                .generate(sink, param, ctx);
       }
     else
       return as_generator
       (
        attribute_reorder<-1, -1, 2>
        (
         " ::efl::eolian::convert_to_c<" << c_type
         << ", " << parameter_type
         << (qualifier & qualifier_info::is_own
             ? ", true" : "")
         << ">(" << string << ")"
        )
       ).generate(sink, param, ctx);
   }
};

template <>
struct is_eager_generator<converting_argument_generator> : std::true_type {};
template <>
struct is_generator<converting_argument_generator> : std::true_type {};

namespace type_traits {
template <>
struct attributes_needed<converting_argument_generator> : std::integral_constant<int, 1> {};
}
      
converting_argument_generator const converting_argument = {};
      
} } }

#endif
