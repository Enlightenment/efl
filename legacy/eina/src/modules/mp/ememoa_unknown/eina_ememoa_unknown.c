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
#include <ememoa_mempool_unknown_size.h>

#include "eina_types.h"
#include "eina_private.h"

typedef struct _Eina_Ememoa_Unknown_Size_Mempool Eina_Ememoa_Unknown_Size_Mempool;
struct _Eina_Ememoa_Unknown_Size_Mempool
{
   struct ememoa_mempool_desc_s *desc;
   int pool;
};

static void *
eina_ememoa_unknown_size_malloc(void *data, unsigned int size)
{
   Eina_Ememoa_Unknown_Size_Mempool *efm = data;

   return ememoa_mempool_unknown_size_pop_object(efm->pool, size);
}

static void
eina_ememoa_unknown_size_free(void *data, void *ptr)
{
   Eina_Ememoa_Unknown_Size_Mempool *efm = data;

   ememoa_mempool_unknown_size_push_object(efm->pool, ptr);
}

static void*
eina_ememoa_unknown_size_realloc(void *data, void *element, unsigned int size)
{
   Eina_Ememoa_Unknown_Size_Mempool *efm = data;

   return ememoa_mempool_unknown_size_resize_object(efm->pool, element, size);
}

static void
eina_ememoa_unknown_size_gc(void *data)
{
   Eina_Ememoa_Unknown_Size_Mempool *efm = data;

   ememoa_mempool_unknown_size_garbage_collect(efm->pool);
}

static void
eina_ememoa_unknown_size_statistics(void *data)
{
   Eina_Ememoa_Unknown_Size_Mempool *efm = data;

   ememoa_mempool_unknown_size_display_statistic(efm->pool);
}

static void*
eina_ememoa_unknown_size_init(const char *context, __UNUSED__ const char *option, va_list args)
{
   struct ememoa_mempool_desc_s *desc = NULL;
   Eina_Ememoa_Unknown_Size_Mempool *efm;
   Eina_Bool thread_protect;
   unsigned int *items_map;
   unsigned int items_count;
   unsigned int i;
   int context_length;

   if (context)
     {
	context_length = strlen(context) + 1;

	desc = calloc(1, sizeof (struct ememoa_mempool_desc_s) + context_length);
	if (!desc) goto on_error;

	desc->name = (char*) (desc + 1);
	memcpy((char*) desc->name, context, context_length);
     }

   thread_protect = va_arg(args, int);
   items_count = va_arg(args, unsigned int);

   items_map = malloc(sizeof (unsigned int) * 2 * items_count);

   for (i = 0; i < (items_count << 1); ++i)
     items_map[i] = va_arg(args, unsigned int);

   efm = malloc(sizeof (Eina_Ememoa_Unknown_Size_Mempool));
   if (!efm) goto on_error;

   efm->desc = desc;
   efm->pool = ememoa_mempool_unknown_size_init(items_count,
						items_map,
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
eina_ememoa_unknown_size_shutdown(void *data)
{
   Eina_Ememoa_Unknown_Size_Mempool *efm = data;

   if (efm->desc) free(efm->desc);
   ememoa_mempool_unknown_size_clean(efm->pool);
   free(efm);
}

Eina_Mempool_Backend mp_backend = {
  .init = &eina_ememoa_unknown_size_init,
  .shutdown = &eina_ememoa_unknown_size_shutdown,
  .realloc = &eina_ememoa_unknown_size_realloc,
  .alloc = &eina_ememoa_unknown_size_malloc,
  .free = &eina_ememoa_unknown_size_free,
  .garbage_collect = &eina_ememoa_unknown_size_gc,
  .statistics = &eina_ememoa_unknown_size_statistics
};

