/* EINA - EFL data type library
 * Copyright (C) 2010 Cedric Bail
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library;
 * if not, see <http://www.gnu.org/licenses/>.
 */

/* This file is here to preserve ABI compatibility, don't touch
   it unless you know what you are doing */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "eina_config.h"
#include "eina_private.h"
#include "eina_error.h"
#include "eina_log.h"
#include "eina_safety_checks.h"

typedef struct _Eina_Array Eina_Array;
struct _Eina_Array
{
#define EINA_ARRAY_VERSION 1
   int          version;

   void       **data;
   unsigned int total;
   unsigned int count;
   unsigned int step;
   EINA_MAGIC
};

EAPI void
eina_array_clean(Eina_Array *array)
{
   EINA_SAFETY_ON_NULL_RETURN(array);

   assert(array->version == EINA_ARRAY_VERSION);

   array->count = 0;
}

