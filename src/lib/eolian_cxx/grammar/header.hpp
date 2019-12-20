/*
 * Copyright 2019 by its authors. See AUTHORS.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
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
#include "ignore_warning.hpp"

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
    << *class_forward_declaration          // sequence<class> | class
    <<  string                   // extra header <string>
    << ignore_warning_begin
    << "\nnamespace eo_cxx {\n"
    << *base_class_definition      // sequence<class> | class
    << "}\n"
    << *class_definition           // sequence<class> | class
    // << *implementation_include_directive
    << ignore_warning_end
  ]
  ;

} } }

#endif
