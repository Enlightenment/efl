
#ifndef EOLIAN_CXX_EO_GENERATE_HH
#define EOLIAN_CXX_EO_GENERATE_HH

#include "eo_types.hh"

#include <iosfwd>
#include "grammar/eo_header_generator.hh"

namespace efl { namespace eolian {

inline void
generate(std::ostream& out, eo_class cls, eo_generator_options const& opts)
{
   grammar::eo_header_generator(out, cls, opts);
}

} }

#endif // EOLIAN_CXX_EO_GENERATE_HH
