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
#ifndef EOLIAN_CXX_EPS_HH
#define EOLIAN_CXX_EPS_HH

#include "grammar/generator.hpp"

namespace efl { namespace eolian { namespace grammar {

struct eps_generator
{
   eps_generator () : r(true) {}
   eps_generator (bool r) : r(r) {}
   bool r;
  
   template <typename OutputIterator, typename Attribute, typename Context>
   bool generate(OutputIterator, Attribute const&, Context const&) const
   {
      return r;
   }

   eps_generator operator()(bool r) const { return {r}; }
};

template <>
struct is_eager_generator<eps_generator> : std::true_type {};
template <>
struct is_generator<eps_generator> : std::true_type {};

eps_generator const eps = {};

} } }

#endif
