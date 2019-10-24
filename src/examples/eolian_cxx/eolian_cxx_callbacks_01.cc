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

// Disabled test while Ecore Eo API is still rapidly changing
#if 0
#include <iostream>
#include <thread>
#include <cassert>
#include <sstream>

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <Eo.h>
#include <Ecore.h>
#include <Evas.h>
#include <Ecore_Evas.h>

#include <Eina.hh>
#include <Eo.hh>
#include <Ecore.hh>
#include <Evas.hh>

#include <ecore_poller.eo.hh>

namespace efl { namespace ecore {
using ::ecore::poller;
} }

void
example_callbacks()
{
   int count = 0;
   efl::ecore::poller poller(
     poller.constructor(ECORE_POLLER_CORE, 1,
       [&count, &poller]
       {
          if (++count == 5)
            {
               std::cout << std::endl;
               ecore_main_loop_quit();
               return false;
            }
          poller.interval_set(2*poller.interval_get());
          std::cout << "." << std::flush;
          return true;
       })
   );
   ::ecore_main_loop_begin();
}

int main()
{
   efl::eina::eina_init eina_;
   efl::eo::eo_init eo_;
   efl::ecore::ecore_init ecore_;

   std::cerr << "[+] Running EFL++ example: Callbacks" << std::endl;
   example_callbacks();

   return 0;
}
#else
int main()
{
}
#endif
