#ifndef EOLIAN_CXX_HEADER_HH
#define EOLIAN_CXX_HEADER_HH

#include "header_guards.hpp"
#include "eps.hpp"
#include "string.hpp"
#include "sequence.hpp"
#include "kleene.hpp"
#include "header_include_directive.hpp"
#include "base_class_definition.hpp"
#include "class_definition.hpp"
#include "class_declaration.hpp"
#include "implementation_include_directive.hpp"

namespace efl { namespace eolian { namespace grammar {

auto class_header =
  header_guards // class name
  [
       "#include <Eo.h>\n"
       "\nextern \"C\" {\n"
    << *header_include_directive // sequence<string>
    << "}\n"
    << "#include <Eina.hh>\n"
       "#include <Eo.hh>\n"
    << *header_include_directive // sequence<string>
    << *class_declaration          // sequence<class> | class
    << "\nnamespace eo_cxx {\n"
    << *base_class_definition      // sequence<class> | class
    << "}\n"
    << *class_definition           // sequence<class> | class
    << *implementation_include_directive
  ]
  ;

} } }

#endif
