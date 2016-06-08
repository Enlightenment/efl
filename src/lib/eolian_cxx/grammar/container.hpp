#ifndef EOLIAN_CXX_CONTAINER_HH
#define EOLIAN_CXX_CONTAINER_HH

#include "grammar/generator.hpp"
#include "grammar/klass_def.hpp"
#include "grammar/case.hpp"
#include "grammar/type.hpp"

namespace efl { namespace eolian { namespace grammar {

template <typename OutputIterator, typename Context>
void generate_container(OutputIterator sink, attributes::complex_type_def const& complex, Context const& context
                        , std::string const& name)
{
  if(!complex.subtypes.empty())
    {
      attributes::type_def subtype = complex.subtypes[0];
      as_generator(" "<< name << "<" << type << ">").generate(sink, subtype, context);
    }
}

} } }

#endif
