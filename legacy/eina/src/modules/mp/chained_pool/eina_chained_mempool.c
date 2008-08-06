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

#include "eina_inlist.h"
#include "eina_error.h"

#include "eina_private.h"

typedef struct _Chained_Mempool Chained_Mempool;
struct _Chained_Mempool
{
   Eina_Inlist  *first;
   const char *name;
   int item_size;
   int pool_size;
   int usage;
};

typedef struct _Chained_Pool Chained_Pool;
struct _Chained_Pool
{
   Eina_Inlist _list_data;
   void *base;
   int usage;
};

static Chained_Pool *
_eina_chained_mp_pool_new(Chained_Mempool *pool)
{
   Chained_Pool *p;
   void **ptr;
   int item_alloc, i;

   item_alloc = ((pool->item_size + sizeof(void *) - 1) / sizeof(void *)) * sizeof(void *);
   p = malloc(sizeof(Chained_Pool) + (pool->pool_size * item_alloc));
   ptr = (void **)(((unsigned char *)p) + sizeof(Chained_Pool));
   p->usage = 0;
   p->base = ptr;
   for (i = 0; i < pool->pool_size - 1; i++)
     {
	*ptr = (void **)(((unsigned char *)ptr) + item_alloc);
	ptr = *ptr;
     }
   *ptr = NULL;
   return p;
}

static void
_eina_chained_mp_pool_free(Chained_Pool *p)
{
   free(p);
}

static void *
eina_chained_mempool_malloc(void *data, __UNUSED__ unsigned int size)
{
   Chained_Mempool *pool = data;
   Chained_Pool *p;
   Eina_Inlist *item;
   void *mem;

   // look 4 pool from 2nd bucket on
   EINA_INLIST_ITER_NEXT(pool->first, item)
     {
	p = (Chained_Pool*) item;

	// base is not NULL - has a free slot
	if (p->base)
	  {
	     pool->first = eina_inlist_remove(pool->first, item);
	     break;
	  }
     }
   // we have reached the end of the list - no free pools
   if (!p)
     {
	p = _eina_chained_mp_pool_new(pool);
	if (!p) return NULL;
	pool->first = eina_inlist_prepend(pool->first, p);
     }
   // this points to the next free block - so take it
   mem = p->base;
   // base now points to the next free block
   p->base = *((void **)mem);
   // move to end - it just filled up
   if (!p->base)
     {
	pool->first = eina_inlist_remove(pool->first, p);
	pool->first = eina_inlist_append(pool->first, p);
     }
   p->usage++;
   pool->usage++;
   return mem;
}

static void
eina_chained_mempool_free(void *data, void *ptr)
{
   Chained_Mempool *pool = data;
   Chained_Pool *p;
   Eina_Inlist *item;
   void *pmem;
   int item_alloc, psize;

   item_alloc = ((pool->item_size + sizeof(void *) - 1) / sizeof(void *)) * sizeof(void *);
   psize = item_alloc * pool->pool_size;
   // look 4 pool
   EINA_INLIST_ITER_NEXT(pool->first, item)
     {
	p = (Chained_Pool*) item;

	// pool mem base
	pmem = (void *)(((unsigned char *)p) + sizeof(Chained_Pool));
	// is it in pool mem?
	if ((ptr >= pmem) && ((unsigned char *)ptr < (((unsigned char *)pmem) + psize)))
	  {
	     // freed node points to prev free node
	     *((void **)ptr) = p->base;
	     // next free node is now the one we freed
	     p->base = ptr;
	     p->usage--;
	     pool->usage--;
	     pool->first = eina_inlist_remove(pool->first, p);
	     if (p->usage == 0)
	       // free bucket
	       _eina_chained_mp_pool_free(p);
	     else
	       // move to front
	       pool->first = eina_inlist_prepend(pool->first, p);
	     break;
	  }
     }
}

static void*
eina_chained_mempool_realloc(__UNUSED__ void *data, __UNUSED__ void *element, __UNUSED__ unsigned int size)
{
   return NULL;
}

static void*
eina_chained_mempool_init(const char *context, __UNUSED__ const char *option, va_list args)
{
   Chained_Mempool *mp;
   int length;

   length = context ? strlen(context) + 1 : 0;

   mp = calloc(1, sizeof(Chained_Mempool) + length);
   if (!mp) return NULL;

   mp->item_size = va_arg(args, int);
   mp->pool_size = va_arg(args, int);

   if (length)
     {
	mp->name = (const char*) (mp + 1);
	memcpy((char*) mp->name, context, length);
     }

   return mp;
}

static void
eina_chained_mempool_shutdown(void *data)
{
   Chained_Mempool *mp;

   mp = (Chained_Mempool *) data;

   while (mp->first)
     {
	Chained_Pool *p = (Chained_Pool *) mp->first;

#ifdef DEBUG
	if (p->usage > 0)
	  EINA_ERROR_PINFO("Bad news we are destroying not an empty mempool [%s]\n", mp->name);
#endif

	mp->first = eina_inlist_remove(mp->first, mp->first);
	_eina_chained_mp_pool_free(p);
     }

   free(mp);
}

Eina_Mempool_Backend mp_backend = {
  .init = &eina_chained_mempool_init,
  .shutdown = &eina_chained_mempool_shutdown,
  .realloc = &eina_chained_mempool_realloc,
  .alloc = &eina_chained_mempool_malloc,
  .free = &eina_chained_mempool_free
};
