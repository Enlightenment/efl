#ifndef EOLIAN_CXX_EPS_HH
#define EOLIAN_CXX_EPS_HH

#include "grammar/generator.hpp"

namespace efl { namespace eolian { namespace grammar {

struct eps_generator
{
   template <typename OutputIterator, typename Attribute, typename Context>
   bool generate(OutputIterator, Attribute const&, Context const&) const
   {
      return true;
   }
};

template <>
struct is_eager_generator<eps_generator> : std::true_type {};

eps_generator const eps;

} } }

#endif
