
#ifndef EOLIAN_CXX_EO_GENERATE_HH
#define EOLIAN_CXX_EO_GENERATE_HH

#include <iosfwd>

#include "eo_types.hh"
#include "grammar/eo_header_generator.hh"

namespace efl { namespace eolian {

inline void
generate(std::ostream& out, eo_class const& cls, eo_generator_options const& opts)
{
   grammar::eo_header_generator(out, cls, opts);
}

} }

#endif // EOLIAN_CXX_EO_GENERATE_HH
