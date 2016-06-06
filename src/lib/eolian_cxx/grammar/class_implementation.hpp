#ifndef EOLIAN_CXX_CLASS_IMPLEMENTATION_HH
#define EOLIAN_CXX_CLASS_IMPLEMENTATION_HH

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
#include "grammar/type_impl.hpp"
#include "grammar/attribute_reorder.hpp"

namespace efl { namespace eolian { namespace grammar {

struct class_implementation_generator
{
   template <typename OutputIterator, typename Context>
   bool generate(OutputIterator sink, attributes::klass_def const& cls, Context const& ctx) const
   {
     return as_generator
       (
        (namespaces
         [*function_definition(get_klass_name(cls))]
         // << "namespace eo_cxx {\n"
         // << namespaces
         // [*function_definition(get_klass_name(cls))]
         // << "}\n\n"
       )).generate(sink, std::make_tuple(cls.namespaces, cls.functions), ctx)
       && as_generator
       (
        "namespace eo_cxx {\n"
        << namespaces
        [*function_definition(get_klass_name(cls))]
        << "}\n\n"
       ).generate(sink, std::make_tuple(cls.namespaces, cls.functions), ctx);
   }
};

template <>
struct is_eager_generator<class_implementation_generator> : std::true_type {};

namespace type_traits {
template <>
struct attributes_needed<class_implementation_generator> : std::integral_constant<int, 1> {};
}
      
class_implementation_generator const class_implementation = {};
      
} } }

#endif
