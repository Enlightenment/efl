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

#include <iostream>
#include <iomanip>

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "Eo.hh"
#include "Eina.hh"

extern "C"
{
#include "colourable_stub.h"
#include "ns_colourable.eo.h"
}

#define MY_CLASS NS_COLOURABLE_CLASS

static efl::eina::log_domain domain("colourable");

Eo *
_colourable_eo_base_constructor(Eo *obj, Colourable_Data *self)
{
   EINA_CXX_DOM_LOG_DBG(domain) << __func__ << std::endl;
   self->r = self->g = self->b = 0;
   return efl_constructor(efl_super(obj, MY_CLASS));
}

void
_colourable_eo_base_destructor(Eo *obj, Colourable_Data *self EINA_UNUSED)
{
   EINA_CXX_DOM_LOG_DBG(domain) << __func__ << std::endl;
   efl_destructor(efl_super(obj, MY_CLASS));
}

void
_colourable_rgb_24bits_constructor(Eo *obj, Colourable_Data *self, int rgb)
{
   EINA_CXX_DOM_LOG_DBG(domain) << std::showbase << std::hex
     << __func__ << " [ rgb = " << rgb <<" ]" << std::endl;
   self->r = (rgb & 0x00ff0000) >> 16;
   self->g = (rgb & 0x0000ff00) >> 8;
   self->b = rgb & 0x000000ff;
}

void
_colourable_print_colour(Eo *obj EINA_UNUSED, Colourable_Data *self EINA_UNUSED)
{
   EINA_CXX_DOM_LOG_DBG(domain) << __func__ << std::endl;
}

int
_colourable_colour_mask(Eo *obj EINA_UNUSED, Colourable_Data *self, int mask)
{
   int masked_rgb =
     (((self->r << 16)& 0x00ff0000) |
      ((self->g << 8) & 0x0000ff00) |
      (self->b & 0x000000ff)) & mask;
   EINA_CXX_DOM_LOG_DBG(domain) << std::showbase << std::hex
     << __func__ << " [ mask = "  << masked_rgb << " ]" << std::endl;
   return masked_rgb;
}

void
_colourable_composite_colour_get(Eo *obj EINA_UNUSED, Colourable_Data *self, int* r, int* g, int* b)
{
   *r = self->r;
   *g = self->g;
   *b = self->b;
   EINA_CXX_DOM_LOG_DBG(domain) << std::showbase << std::hex
     << __func__ << " [ (r, g, b) = (" << *r << ", " << *g << ", " << *b << ") ]" << std::endl;
   return;
}

void
_colourable_composite_colour_set(Eo *obj EINA_UNUSED, Colourable_Data *self, int r, int g, int b)
{
   EINA_CXX_DOM_LOG_DBG(domain) << std::showbase << std::hex
     << __func__ << " [ (r, g, b) = (" << r << ", " << g << ", " << b << ") ]" << std::endl;
   self->r = r;
   self->g = g;
   self->b = b;
   return;
}

int
_colourable_colour_get(Eo *obj EINA_UNUSED, Colourable_Data *self)
{
   int rgb =
     ((self->r << 16)& 0x00ff0000) |
     ((self->g << 8) & 0x0000ff00) |
     (self->b & 0x000000ff);
   EINA_CXX_DOM_LOG_DBG(domain) << std::showbase << std::hex
     << __func__ << " [ rgb = " << rgb <<" ]" << std::endl;
   return rgb;
}

void
_colourable_colour_set(Eo *obj EINA_UNUSED, Colourable_Data *self, int rgb)
{
   EINA_CXX_DOM_LOG_DBG(domain) << std::showbase << std::hex
     << __func__ << " [ rgb = " << rgb <<" ]" << std::endl;
   self->r = (rgb & 0x00ff0000) >> 16;
   self->g = (rgb & 0x0000ff00) >> 8;
   self->b = rgb & 0x000000ff;
   return;
}

