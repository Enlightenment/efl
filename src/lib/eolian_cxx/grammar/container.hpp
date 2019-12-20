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
