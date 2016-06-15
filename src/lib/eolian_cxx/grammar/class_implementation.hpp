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
     std::vector<std::string> cpp_namespaces = attributes::cpp_namespaces(cls.namespaces);
     auto base_class_name = *(lower_case[string] << "::") << string;
     auto class_name = *(lit("::") << lower_case[string]) << "::" << string;
     return as_generator
       (
        (namespaces
         [*function_definition(get_klass_name(cls))]
         << "\n"
       )).generate(sink, std::make_tuple(cls.namespaces, cls.functions), ctx)
       && as_generator
       (
        attribute_reorder<0, 1, 2, 3, 2, 3, 2, 3, 2, 3, 2, 3, 2, 3, 2, 3, 2, 3, 2, 3>
        (
         "namespace eo_cxx {\n"
         << namespaces
         [
          *function_definition(get_klass_name(cls))
          << "inline " << base_class_name << "::operator " << class_name << "() const { return *static_cast< "
            << class_name << " const*>(static_cast<void const*>(this)); }\n"
          << "inline " << base_class_name << "::operator " << class_name << "&() { return *static_cast< "
            << class_name << "*>(static_cast<void*>(this)); }\n"
          << "inline " << base_class_name << "::operator " << class_name << " const&() const { return *static_cast< "
            << class_name << " const*>(static_cast<void const*>(this)); }\n"
         ]
         << "}\n\n"
         )).generate(sink, std::make_tuple(cls.namespaces, cls.functions, cpp_namespaces, cls.cxx_name), ctx);
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
