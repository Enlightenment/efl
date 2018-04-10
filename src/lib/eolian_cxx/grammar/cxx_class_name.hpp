#ifndef EOLIAN_CXX_CLASS_NAME_HH
#define EOLIAN_CXX_CLASS_NAME_HH

#include "grammar/case.hpp"
#include "grammar/string.hpp"

namespace efl { namespace eolian { namespace grammar {

struct cxx_class_name_generator
{
   template <typename OutputIterator, typename Context>
   bool generate(OutputIterator sink, std::string name, Context const& context) const
   {
     if(name == "Class")
       {
         name = "efl_class";
       }
     
     return as_generator(lower_case[string]).generate(sink, name, context);
   }
};

cxx_class_name_generator const cxx_class_name{};

template <>
struct is_eager_generator<cxx_class_name_generator> : std::true_type {};
template <>
struct is_generator<cxx_class_name_generator> : std::true_type {};

namespace type_traits {
template <>
struct attributes_needed<cxx_class_name_generator> : std::integral_constant<int, 1> {};
}
      
} } }

#endif
