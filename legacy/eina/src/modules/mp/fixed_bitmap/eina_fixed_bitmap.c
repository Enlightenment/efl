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

#include <stdint.h>
#include <string.h>
#include <assert.h>

#ifdef HAVE_EVIL
# include <Evil.h>
#endif

#include "eina_inlist.h"
#include "eina_rbtree.h"
#include "eina_error.h"

#include "eina_mempool.h"

#include "eina_private.h"

typedef struct _Eina_Fixed_Bitmap Eina_Fixed_Bitmap;
typedef struct _Eina_Fixed_Bitmap_Pool Eina_Fixed_Bitmap_Pool;

struct _Eina_Fixed_Bitmap
{
   Eina_Rbtree *lookup;
   Eina_Inlist *head;

   int item_size;
};

struct _Eina_Fixed_Bitmap_Pool
{
   EINA_RBTREE;
   EINA_INLIST;

   uint32_t bitmask;
};

static inline int
_eina_rbtree_inlist_delta(void)
{
   Eina_Fixed_Bitmap_Pool tmp;
   void *a = &tmp.__rbtree;
   void *b = &tmp.__in_list;

   return a - b;
}

static Eina_Rbtree_Direction
_eina_fixed_cmp(const Eina_Rbtree *left, const Eina_Rbtree *right, __UNUSED__ void *data)
{
   if (left - right < 0)
     return EINA_RBTREE_LEFT;
   return EINA_RBTREE_RIGHT;
}

static int
_eina_fixed_cmp_key(const Eina_Rbtree *node, const void *key, __UNUSED__ int length, Eina_Fixed_Bitmap *mp)
{
   const void *a = node;
   const void *b = key;
   int delta;
   int limit;

   limit = sizeof (Eina_Fixed_Bitmap_Pool) + mp->item_size * 32;
   delta = a - b;

   if (delta > 0)
     return 1;
   if (delta + limit < 0)
     return -1;
   return 0;
}

static void
_eina_fixed_bitmap_pool_free(Eina_Fixed_Bitmap_Pool *pool, __UNUSED__ void *data)
{
   free(pool);
}

static void *
eina_fixed_bitmap_malloc(void *data, __UNUSED__ unsigned int size)
{
   Eina_Fixed_Bitmap *mp = data;
   Eina_Fixed_Bitmap_Pool *pool = NULL;
   void *ptr;
   int index;

   if (mp->head)
     {
	pool = (Eina_Fixed_Bitmap_Pool*) ((unsigned char*) mp->head + _eina_rbtree_inlist_delta());

	if (pool->bitmask == 0) pool = NULL;
     }

   if (!pool)
     {
	eina_error_set(0);
	pool = malloc(sizeof (Eina_Fixed_Bitmap_Pool) + mp->item_size * 32);
	if (!pool)
	  {
	     eina_error_set(EINA_ERROR_OUT_OF_MEMORY);
	     return NULL;
	  }

	pool->bitmask = 0xFFFFFFFF;

	mp->head = eina_inlist_prepend(mp->head, EINA_INLIST_GET(pool));
	mp->lookup = eina_rbtree_inline_insert(mp->lookup, EINA_RBTREE_GET(pool), EINA_RBTREE_CMP_NODE_CB(_eina_fixed_cmp), NULL);
     }

   index = ffs(pool->bitmask) - 1;
   pool->bitmask &= ~(1 << index);
   ptr = (unsigned char*) (pool + 1) + index * mp->item_size;

   if (pool->bitmask == 0)
     mp->head = eina_inlist_demote(mp->head, EINA_INLIST_GET(pool));

   return ptr;
}

static void
eina_fixed_bitmap_free(void *data, void *ptr)
{
   Eina_Fixed_Bitmap *mp = data;
   Eina_Fixed_Bitmap_Pool *pool;
   void *a;
   Eina_Bool push_front = EINA_FALSE;
   int delta;

   pool = (Eina_Fixed_Bitmap_Pool*) eina_rbtree_inline_lookup(mp->lookup,
							      ptr, 0,
							      EINA_RBTREE_CMP_KEY_CB(_eina_fixed_cmp_key), mp);
   if (!pool) return ;
   if (pool->bitmask != 0xFFFFFFFF) push_front = EINA_TRUE;

   a = pool;
   delta = (ptr - a - sizeof (Eina_Fixed_Bitmap_Pool)) / mp->item_size;

   assert(delta >= 0 && delta < 32);

   pool->bitmask |= (1 << (delta & 0x1F));

   if (pool->bitmask == 0xFFFFFFFF)
     {
	mp->head = eina_inlist_remove(mp->head, EINA_INLIST_GET(pool));
	mp->lookup = eina_rbtree_inline_remove(mp->lookup, EINA_RBTREE_GET(pool), EINA_RBTREE_CMP_NODE_CB(_eina_fixed_cmp), NULL);
	free(pool);
     }
   else if (push_front)
     mp->head = eina_inlist_promote(mp->head, EINA_INLIST_GET(pool));
}

static void *
eina_fixed_bitmap_realloc(__UNUSED__ void *data, __UNUSED__ void *element, __UNUSED__ unsigned int size)
{
   return NULL;
}

static void*
eina_fixed_bitmap_init(__UNUSED__ const char *context, __UNUSED__ const char *option, va_list args)
{
   Eina_Fixed_Bitmap *mp;

   mp = malloc(sizeof (Eina_Fixed_Bitmap));
   if (!mp) return NULL;

   mp->item_size = va_arg(args, int);
   mp->lookup = NULL;
   mp->head = NULL;

   return mp;
}

static void
eina_fixed_bitmap_shutdown(void *data)
{
   Eina_Fixed_Bitmap *mp = data;

   eina_rbtree_delete(mp->lookup, EINA_RBTREE_FREE_CB(_eina_fixed_bitmap_pool_free), NULL);
   free(mp);
}

static Eina_Mempool_Backend mp_backend = {
  .name ="fixed_bitmap",
  .init = &eina_fixed_bitmap_init,
  .shutdown = &eina_fixed_bitmap_shutdown,
  .realloc = &eina_fixed_bitmap_realloc,
  .alloc = &eina_fixed_bitmap_malloc,
  .free = &eina_fixed_bitmap_free
};

Eina_Bool fixed_bitmap_init(void)
{
   return eina_mempool_register(&mp_backend);
}

void fixed_bitmap_shutdown(void)
{
   eina_mempool_unregister(&mp_backend);
}

#ifndef EINA_STATIC_BUILD_FIXED_BITMAP

EINA_MODULE_INIT(fixed_bitmap_init);
EINA_MODULE_SHUTDOWN(fixed_bitmap_shutdown);

#endif /* ! EINA_STATIC_BUILD_FIXED_BITMAP */

