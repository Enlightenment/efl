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

#ifndef EOLIAN_CXX_LIB_HH
#define EOLIAN_CXX_LIB_HH

extern "C"
{
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif
#include <Eolian.h>
}

namespace efl { namespace eolian {

struct eolian_init
{
   eolian_init()
   {
      ::eolian_init();
   }
   ~eolian_init()
   {
      ::eolian_shutdown();
   }
};

struct eolian_state
{
   Eolian_State *value;
   eolian_state()
   {
       value = ::eolian_state_new();
   }
   ~eolian_state()
   {
     ::eolian_state_free(value);
   }

   inline Eolian_Unit const* as_unit() const
   {
       return (Eolian_Unit const*)value;
   }
};

} }

#endif // EOLIAN_CXX_LIB_HH
