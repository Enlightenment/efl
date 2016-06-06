#ifndef EOLIAN_CXX_IMPL_HEADER_HH
#define EOLIAN_CXX_IMPL_HEADER_HH

#include "grammar/generator.hpp"
#include "grammar/klass_def.hpp"

#include "grammar/indentation.hpp"
#include "grammar/list.hpp"
#include "grammar/alternative.hpp"
#include "grammar/kleene.hpp"
#include "grammar/class_implementation.hpp"

namespace efl { namespace eolian { namespace grammar {

auto impl_header =
  *(class_implementation)
  ;
      
} } }

#endif
