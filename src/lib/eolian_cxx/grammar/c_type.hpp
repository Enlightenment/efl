#ifndef EOLIAN_CXX_C_TYPE_HH
#define EOLIAN_CXX_C_TYPE_HH

#include "grammar/generator.hpp"
#include "grammar/klass_def.hpp"
#include "grammar/string.hpp"

namespace efl { namespace eolian { namespace grammar { namespace attributes {

struct c_type_visitor
{
  std::string const* c_type;
  typedef std::string result_type;
  std::string operator()(attributes::klass_name const& name) const
  {
    std::string n;
    as_generator(" ::" << *(string << "_") << string << string << "*")
      .generate(std::back_insert_iterator<std::string>(n)
                , std::make_tuple(name.namespaces, name.eolian_name
                                  , std::string{name.base_qualifier & qualifier_info::is_const ? " const" : ""})
                , context_null {});
    return n;
  }
  template <typename T>
  std::string operator()(T const&) const
  {
    return *c_type;
  }
};
        
inline std::string c_type(parameter_def const& param)
{
   switch(param.direction)
     {
     case parameter_direction::in:
       return param.type.original_type.visit(c_type_visitor{&param.c_type});
     case parameter_direction::out:
     case parameter_direction::inout:
       return param.type.original_type.visit(c_type_visitor{&param.c_type}) + "*";
     default:
       throw std::runtime_error("Unknown parameter direction");
     };
}

}
      
struct c_type_generator
{
  template <typename OutputIterator, typename Context>
  bool generate(OutputIterator sink, attributes::parameter_def const& attribute, Context const& context) const
  {
    return as_generator(attributes::c_type(attribute)).generate(sink, attributes::unused, context);
  }
  template <typename OutputIterator, typename Context>
  bool generate(OutputIterator sink, attributes::type_def const& attribute, Context const& context) const
  {
    return as_generator(attribute.original_type.visit(attributes::c_type_visitor{&attribute.c_type}))
      .generate(sink, attributes::unused, context);
  }
};

template <>
struct is_eager_generator<c_type_generator> : std::true_type {};

namespace type_traits {
template <>
struct attributes_needed<c_type_generator> : std::integral_constant<int, 1> {};
}
      
c_type_generator const c_type = {};
      
} } }

#endif
