/*
 * Copyright (C) 2013, 2014 Mike Blumenkrantz
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

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <time.h>

#ifdef _WIN32
# include <evil_private.h> /* strptime */
#endif

#include "eina_safety_checks.h"
#include "eina_value.h"
#include "eina_stringshare.h"


typedef struct _Eina_Value_Util_Struct_Desc
{
   Eina_Value_Struct_Desc base;
   int refcount;
} Eina_Value_Util_Struct_Desc;

static void *
_ops_malloc(const Eina_Value_Struct_Operations *ops EINA_UNUSED, const Eina_Value_Struct_Desc *desc)
{
   Eina_Value_Util_Struct_Desc *edesc = (Eina_Value_Util_Struct_Desc*)desc;
   edesc->refcount++;
   //DBG("%p refcount=%d", edesc, edesc->refcount);
   return malloc(desc->size);
}

static void
_ops_free(const Eina_Value_Struct_Operations *ops EINA_UNUSED, const Eina_Value_Struct_Desc *desc, void *memory)
{
   Eina_Value_Util_Struct_Desc *edesc = (Eina_Value_Util_Struct_Desc*) desc;
   edesc->refcount--;
   free(memory);
   //DBG("%p refcount=%d", edesc, edesc->refcount);
   if (edesc->refcount <= 0)
     {
        unsigned i;
        for (i = 0; i < edesc->base.member_count; i++)
          eina_stringshare_del((char *)edesc->base.members[i].name);
        free((Eina_Value_Struct_Member *)edesc->base.members);
        free(edesc);
     }
}

static Eina_Value_Struct_Operations operations =
{
   EINA_VALUE_STRUCT_OPERATIONS_VERSION,
   _ops_malloc,
   _ops_free,
   NULL,
   NULL,
   NULL
};

EAPI Eina_Value_Struct_Desc *
eina_value_util_struct_desc_new(void)
{
   Eina_Value_Util_Struct_Desc *st_desc;

   st_desc = calloc(1, sizeof(Eina_Value_Util_Struct_Desc));
   EINA_SAFETY_ON_NULL_RETURN_VAL(st_desc, NULL);
   st_desc->base.version = EINA_VALUE_STRUCT_DESC_VERSION;
   st_desc->base.ops = &operations;
   return (Eina_Value_Struct_Desc*)st_desc;
}

EAPI Eina_Value *
eina_value_util_time_string_new(const char *timestr)
{
   Eina_Value *v;
   struct tm tm;
   time_t t;

   if (!strptime(timestr, "%Y%m%dT%H:%M:%S", &tm)) return NULL;
   t = mktime(&tm);
   v = eina_value_new(EINA_VALUE_TYPE_TIMESTAMP);
   if (v) eina_value_set(v, t);
   return v;
}
