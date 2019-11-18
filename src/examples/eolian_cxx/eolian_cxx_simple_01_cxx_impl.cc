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
// EINA_LOG_LEVELS=colourable:4,colourablesquare:4 ./eolian_cxx_simple_01

#include <iostream>
#include <cassert>

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "colourable.eo.hh"
#include "colourablesquare.eo.hh"
#include "ns_colourable.eo.impl.hh"
#include "ns_colourablesquare.eo.impl.hh"

int
main()
{
   efl::eo::eo_init init;
   eina_log_domain_level_set("colourable", EINA_LOG_LEVEL_DBG);
   eina_log_domain_level_set("colourablesquare", EINA_LOG_LEVEL_DBG);

   int r, g, b;
   ::colourable obj1(
     obj1.colourable_rgb_24bits_constructor(0x123456)
   );
   obj1.colour_set(0xc0ffee);
   obj1.composite_colour_get(&r, &g, &b);

   ::colourablesquare obj2(
     obj2.colourablesquare_size_constructor(0x123456)
   );
   obj2.composite_colour_set(r, g, b);
   obj2.size_set(11);
   assert(obj1.colour_get() == obj2.colour_get());

   efl::eo::wref<::colourable> ref = obj1;
   efl::eina::optional<::colourable> obj3 = ref.lock();
   assert(obj3);
   obj3->colour_set(0x00);

   return 0;
}
