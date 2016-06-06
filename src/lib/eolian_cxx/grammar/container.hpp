#ifndef EOLIAN_CXX_CONTAINER_HH
#define EOLIAN_CXX_CONTAINER_HH

#include "grammar/generator.hpp"
#include "grammar/klass_def.hpp"
#include "grammar/case.hpp"
#include "grammar/type.hpp"

namespace efl { namespace eolian { namespace grammar {

struct container_subtype_modify
{
  typedef void result_type;
  void operator()(attributes::complex_type_def& /*x*/) const
  {
  }

  void operator()(attributes::regular_type_def& x) const
  {
    if(x.base_type == "string")
      remove_own(x.base_qualifier);
    else if(!x.pointers.empty())
      x.pointers.pop_back();
  }
  
  template <typename T>
  void operator()(T& /*x*/) const
  {
  }
};
      
template <typename OutputIterator, typename Context>
void generate_container(OutputIterator sink, attributes::complex_type_def const& complex, Context const& context
                        , std::string const& name)
{
  if(!complex.subtypes.empty())
    {
      attributes::type_def subtype = complex.subtypes[0];
      subtype.original_type.visit(container_subtype_modify{});
      as_generator(" "<< name << "<" << type << ">").generate(sink, subtype, context);
    }
}

} } }

#endif
