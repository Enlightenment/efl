/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */
/* EINA - EFL data type library
 * Copyright (C) 2008 Cedric BAIL
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

#include <stdlib.h>
#include <string.h>
#include <ememoa_mempool_fixed.h>

#include "eina_inlist.h"
#include "eina_error.h"
#include "eina_module.h"

#include "eina_private.h"

typedef struct _Eina_Ememoa_Fixed_Mempool Eina_Ememoa_Fixed_Mempool;
struct _Eina_Ememoa_Fixed_Mempool
{
   struct ememoa_mempool_desc_s *desc;
   int pool;
};

static void *
eina_ememoa_fixed_malloc(void *data, __UNUSED__ unsigned int size)
{
   Eina_Ememoa_Fixed_Mempool *efm = data;

   return ememoa_mempool_fixed_pop_object(efm->pool);
}

static void
eina_ememoa_fixed_free(void *data, void *ptr)
{
   Eina_Ememoa_Fixed_Mempool *efm = data;

   ememoa_mempool_fixed_push_object(efm->pool, ptr);
}

static void*
eina_ememoa_fixed_realloc(__UNUSED__ void *data, __UNUSED__ void *element, __UNUSED__ unsigned int size)
{
   return NULL;
}

static void
eina_ememoa_fixed_gc(void *data)
{
   Eina_Ememoa_Fixed_Mempool *efm = data;

   ememoa_mempool_fixed_garbage_collect(efm->pool);
}

static void
eina_ememoa_fixed_statistics(void *data)
{
   Eina_Ememoa_Fixed_Mempool *efm = data;

   ememoa_mempool_fixed_display_statistic(efm->pool);
}

static void*
eina_ememoa_fixed_init(const char *context, __UNUSED__ const char *option, va_list args)
{
   struct ememoa_mempool_desc_s *desc = NULL;
   Eina_Ememoa_Fixed_Mempool *efm = NULL;
   Eina_Bool thread_protect;
   int context_length;
   int item_size;
   int pool_size;

   if (context)
     {
	context_length = strlen(context) + 1;

	desc = calloc(1, sizeof (struct ememoa_mempool_desc_s) + context_length);
	if (!desc) goto on_error;

	desc->name = (char*) (desc + 1);
	memcpy((char*) desc->name, context, context_length);
     }

   item_size = va_arg(args, int);
   pool_size = va_arg(args, int);
   thread_protect = va_arg(args, int);

   efm = malloc(sizeof (Eina_Ememoa_Fixed_Mempool));
   if (!efm) goto on_error;

   efm->desc = desc;
   efm->pool = ememoa_mempool_fixed_init(item_size,
					 pool_size,
					 thread_protect ? EMEMOA_THREAD_PROTECTION : 0,
					 efm->desc);
   if (efm->pool < 0) goto on_error;

   return efm;

 on_error:
   if (desc) free(desc);
   if (efm) free(efm);
   return NULL;
}

static void
eina_ememoa_fixed_shutdown(void *data)
{
   Eina_Ememoa_Fixed_Mempool *efm = data;

   if (efm->desc) free(efm->desc);
   ememoa_mempool_fixed_clean(efm->pool);
   free(efm);
}

static Eina_Mempool_Backend mp_backend = {
  .init = &eina_ememoa_fixed_init,
  .shutdown = &eina_ememoa_fixed_shutdown,
  .realloc = &eina_ememoa_fixed_realloc,
  .alloc = &eina_ememoa_fixed_malloc,
  .free = &eina_ememoa_fixed_free,
  .garbage_collect = &eina_ememoa_fixed_gc,
  .statistics = &eina_ememoa_fixed_statistics
};

EINA_MODULE("ememoa_fixed", "mp", NULL, &mp_backend);
