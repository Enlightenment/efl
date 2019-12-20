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
#include "colourablesquare_stub.h"
#include "ns_colourable.eo.h"
#include "ns_colourablesquare.eo.h"
}

#define MY_CLASS NS_COLOURABLESQUARE_CLASS

static efl::eina::log_domain domain("colourablesquare");

void
_colourablesquare_size_constructor(Eo *obj, ColourableSquare_Data *self, int size)
{
   self->size = size;
   EINA_CXX_DOM_LOG_DBG(domain) << __func__ << " [ size = " << size << " ]" << std::endl;
}

int
_colourablesquare_size_get(Eo *obj EINA_UNUSED, ColourableSquare_Data *self)
{
   EINA_CXX_DOM_LOG_DBG(domain) << __func__ << " [ size = " << self->size << " ]" << std::endl;
   return self->size;
}

void
_colourablesquare_size_print(Eo *obj EINA_UNUSED, ColourableSquare_Data *self)
{
   EINA_CXX_DOM_LOG_DBG(domain) << __func__ << " [ size = " << self->size << " ]" << std::endl;
}

void
_colourablesquare_size_set(Eo *obj EINA_UNUSED, ColourableSquare_Data *self EINA_UNUSED, int size)
{
   EINA_CXX_DOM_LOG_DBG(domain) << __func__ << " [ size = " << size << " ]" << std::endl;
}

