
#ifndef EOLIAN_CXX_EO_GENERATE_HH
#define EOLIAN_CXX_EO_GENERATE_HH

#include <iosfwd>

#include "eo_types.hh"
#include "grammar/eo_header_generator.hh"

namespace efl { namespace eolian {

inline void
generate(std::ostream& header_decl,
         std::ostream& header_impl,
         eo_class const& cls,
         eo_generator_options const& opts)
{
   grammar::eo_headers_generator(header_decl, header_impl, cls, opts);
}

} }

#endif // EOLIAN_CXX_EO_GENERATE_HH
