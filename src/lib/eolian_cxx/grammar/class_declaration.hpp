#ifndef EOLIAN_CXX_CLASS_DECLARATION_HH
#define EOLIAN_CXX_CLASS_DECLARATION_HH

#include "grammar/generator.hpp"
#include "grammar/klass_def.hpp"

#include "grammar/indentation.hpp"
#include "grammar/list.hpp"
#include "grammar/alternative.hpp"
#include "grammar/type.hpp"
#include "grammar/parameter.hpp"
#include "grammar/function_declaration.hpp"

namespace efl { namespace eolian { namespace grammar {

struct class_declaration_generator
{
   template <typename OutputIterator, typename Context>
   bool generate(OutputIterator sink, attributes::klass_def const& cls, Context const& context) const
   {
     std::vector<std::string> cpp_namespaces = attributes::cpp_namespaces(cls.namespaces);
     auto open_namespace = *("namespace " << string << " { ") << "\n";
     if(!as_generator(open_namespace).generate(sink, cpp_namespaces, add_lower_case_context(context))) return false;

     if(!as_generator
        (
         "struct " << string << ";\n"
        ).generate(sink, cls.cxx_name, context)) return false;

     auto close_namespace = *(lit("} ")) << "\n";
     if(!as_generator(close_namespace).generate(sink, cpp_namespaces, context)) return false;

     if(!as_generator
        (
         "namespace efl { namespace eo { template<> struct is_eolian_object< "
         "::" << *(lower_case[string] << "::") << string << "> : ::std::true_type {}; } }\n"
        ).generate(sink, std::make_tuple(cpp_namespaces, cls.cxx_name), context)) return false;

     
     return true;
   }
};

template <>
struct is_eager_generator<class_declaration_generator> : std::true_type {};

namespace type_traits {
template <>
struct attributes_needed<class_declaration_generator> : std::integral_constant<int, 1> {};
}
      
class_declaration_generator const class_declaration;
      
} } }

#endif
