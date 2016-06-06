#ifndef EOLIAN_CXX_ADDRESS_OF_HH
#define EOLIAN_CXX_ADDRESS_OF_HH

#include "grammar/generator.hpp"
#include "grammar/klass_def.hpp"

#include "grammar/indentation.hpp"
#include "grammar/list.hpp"
#include "grammar/alternative.hpp"
#include "grammar/type.hpp"
#include "grammar/parameter.hpp"
#include "grammar/function_declaration.hpp"
#include "grammar/case.hpp"

namespace efl { namespace eolian { namespace grammar {

struct address_of_generator
{
   template <typename OutputIterator, typename Context>
   bool generate(OutputIterator sink, attributes::klass_def const& cls, Context const& context) const
   {
     std::vector<std::string> cpp_namespaces = attributes::cpp_namespaces(cls.namespaces);

     if(!as_generator
        (
         scope_tab << " ::efl::eolian::address_of_operator<" << string
        ).generate(sink, cls.cxx_name, context)) return false;

     for(auto&& i : cls.inherits)
       {
         if(!as_generator(",  " << *("::" << lower_case[string]) << "::" << string)
            .generate(sink, std::make_tuple(attributes::cpp_namespaces(i.namespaces), i.eolian_name), context))
           return false;
       }

      if(!as_generator
         (
           "> operator&() { return {this}; }\n"
         ).generate(sink, attributes::unused, context)) return false;

     if(!as_generator
        (
         scope_tab << " ::efl::eolian::address_of_operator<" << string << " const "
        ).generate(sink, cls.cxx_name, context)) return false;

     for(auto&& i : cls.inherits)
       {
         if(!as_generator(",  " << *("::" << lower_case[string]) << "::" << string << " const ")
            .generate(sink, std::make_tuple(attributes::cpp_namespaces(i.namespaces), i.eolian_name), context))
           return false;
       }

      if(!as_generator
         (
           "> operator&() const { return {this}; }\n"
         ).generate(sink, attributes::unused, context)) return false;
      
      return true;
   }
};

template <>
struct is_eager_generator<address_of_generator> : std::true_type {};

namespace type_traits {
template <>
struct attributes_needed<address_of_generator> : std::integral_constant<int, 1> {};
}
      
address_of_generator const address_of = {};
      
} } }

#endif
