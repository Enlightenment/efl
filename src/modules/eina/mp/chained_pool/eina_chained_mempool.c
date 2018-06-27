/* EINA - EFL data type library
 * Copyright (C) 2008-2010 Cedric BAIL, Vincent Torri
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

#ifdef EINA_HAVE_DEBUG_THREADS
# include <assert.h>
#endif

#ifdef EINA_DEBUG_MALLOC
# ifdef __linux__
#  include <malloc.h>
# endif
# ifdef __FreeBSD__
#  include <malloc_np.h>
# endif
#endif

#include "eina_config.h"
#include "eina_inlist.h"
#include "eina_module.h"
#include "eina_mempool.h"
#include "eina_trash.h"
#include "eina_rbtree.h"
#include "eina_lock.h"
#include "eina_thread.h"
#include "eina_cpu.h"

#include "eina_private.h"

#ifndef NVALGRIND
# include <memcheck.h>
#endif

#if defined DEBUG || defined EINA_DEBUG_MALLOC
#include <assert.h>
#include "eina_log.h"

static int _eina_chained_mp_log_dom = -1;

#ifdef INF
#undef INF
#endif
#define INF(...) EINA_LOG_DOM_INFO(_eina_chained_mp_log_dom, __VA_ARGS__)

#ifdef ERR
#undef ERR
#endif
#define ERR(...) EINA_LOG_DOM_ERR(_eina_chained_mp_log_dom, __VA_ARGS__)

#endif

static int aligned_chained_pool = 0;
static int page_size = 0;

typedef struct _Chained_Pool Chained_Pool;
struct _Chained_Pool
{
   EINA_INLIST;
   EINA_RBTREE;
   Eina_Trash *base;
   int usage;

   unsigned char *last;
   unsigned char *limit;
};

typedef struct _Chained_Mempool Chained_Mempool;
struct _Chained_Mempool
{
   Eina_Inlist *first;
   Eina_Rbtree *root;
   const char *name;
   int item_alloc;
   int pool_size;
   int alloc_size;
   int group_size;
   int usage;
   Chained_Pool* first_fill; //All allocation will happen in this chain,unless it is filled
#ifdef EINA_DEBUG_MALLOC
   int minimal_size;
#endif
#ifdef EINA_HAVE_DEBUG_THREADS
   Eina_Thread self;
#endif
   Eina_Spinlock mutex;
};


static inline Eina_Rbtree_Direction
_eina_chained_mp_pool_cmp(const Eina_Rbtree *left, const Eina_Rbtree *right, EINA_UNUSED void *data)
{
   if (left < right) return EINA_RBTREE_LEFT;
   return EINA_RBTREE_RIGHT;
}

static inline int
_eina_chained_mp_pool_key_cmp(const Eina_Rbtree *node, const void *key,
                              EINA_UNUSED int length, EINA_UNUSED void *data)
{
   const Chained_Pool *r = EINA_RBTREE_CONTAINER_GET(node, const Chained_Pool);

   if (key > (void *) r->limit) return -1;
   if (key < (void *) r) return 1;
   return 0;
}

static inline Chained_Pool *
_eina_chained_mp_pool_new(Chained_Mempool *pool)
{
   Chained_Pool *p;
   unsigned char *ptr;

   p = malloc(pool->alloc_size);
   if (!p) return NULL;

#if defined(EINA_DEBUG_MALLOC) && defined (HAVE_MALLOC_USABLE_SIZE)
   {
      size_t sz;
      sz = malloc_usable_size(p);
      if (sz - pool->minimal_size > 0)
        INF("Just allocated %0.2f%% to much memory in '%s' for one block of size %i that means %lu bytes to much.",
            ((float)(sz - pool->minimal_size) * 100) / (float) (pool->alloc_size),
            pool->name,
            pool->alloc_size,
            (unsigned long) sz - pool->minimal_size);
   }
#endif

   ptr = (unsigned char *)(p + 1);
   p->usage = 0;
   p->base = NULL;

   p->last = ptr;
   p->limit = ptr + pool->item_alloc * pool->pool_size;

#ifndef NVALGRIND
   VALGRIND_MAKE_MEM_NOACCESS(ptr, pool->alloc_size - aligned_chained_pool);
#endif

   return p;
}

static inline void
_eina_chained_mp_pool_free(Chained_Pool *p)
{
   free(p);
}

static int
_eina_chained_mempool_usage_cmp(const Eina_Inlist *l1, const Eina_Inlist *l2)
{
  const Chained_Pool *p1;
  const Chained_Pool *p2;

  p1 = EINA_INLIST_CONTAINER_GET(l1, const Chained_Pool);
  p2 = EINA_INLIST_CONTAINER_GET(l2, const Chained_Pool);

  return p2->usage - p1->usage;
}

static void *
_eina_chained_mempool_alloc_in(Chained_Mempool *pool, Chained_Pool *p)
{
  void *mem;

  if (p->last)
    {
      mem = p->last;
      p->last += pool->item_alloc;
      if (p->last >= p->limit)
      p->last = NULL;
    }
  else
    {
#ifndef NVALGRIND
      VALGRIND_MAKE_MEM_DEFINED(p->base, pool->item_alloc);
#endif
      // Request a free pointer
      mem = eina_trash_pop(&p->base);
    }
  
  // move to end - it just filled up
  if (!p->base && !p->last)
    pool->first = eina_inlist_demote(pool->first, EINA_INLIST_GET(p));
  
  p->usage++;
  pool->usage++;

#ifndef NVALGRIND
   VALGRIND_MEMPOOL_ALLOC(pool, mem, pool->item_alloc);
#endif

  return mem;
}

static Eina_Bool
_eina_chained_mempool_free_in(Chained_Mempool *pool, Chained_Pool *p, void *ptr)
{
#ifdef DEBUG
   void *pmem;
  
   // pool mem base
   pmem = (void *)(((unsigned char *)p) + sizeof(Chained_Pool));

   // is it in pool mem?
   if (ptr < pmem)
     {
        ERR("%p is inside the private part of %p pool from %p '%s' Chained_Mempool (could be the sign of a buffer underrun).", ptr, p, pool, pool->name);
        return EINA_FALSE;
     }

   // is it really a pointer returned by malloc
   if ((((unsigned char *)ptr) - (unsigned char *)(p + 1)) % pool->item_alloc)
     {
        ERR("%p is %lu bytes inside a pointer served by %p '%s' Chained_Mempool (You are freeing the wrong pointer man !).",
            ptr, ((((unsigned char *)ptr) - (unsigned char *)(p + 1)) % pool->item_alloc), pool, pool->name);
        return EINA_FALSE;
     }
#endif

   // freed node points to prev free node
   eina_trash_push(&p->base, ptr);
   // next free node is now the one we freed
   p->usage--;
   pool->usage--;
   if (p->usage == 0)
     {
        // free bucket
        pool->first = eina_inlist_remove(pool->first, EINA_INLIST_GET(p));
        pool->root = eina_rbtree_inline_remove(pool->root, EINA_RBTREE_GET(p),
                                               _eina_chained_mp_pool_cmp, NULL);
        if (pool->first_fill == p)
          {
             pool->first_fill = NULL;
             pool->first_fill = EINA_INLIST_CONTAINER_GET(pool->first, Chained_Pool);
          }
        _eina_chained_mp_pool_free(p);

       return EINA_TRUE;
     }
   else
     {
        // move to front
        pool->first = eina_inlist_promote(pool->first, EINA_INLIST_GET(p));
     }

   return EINA_FALSE;
}

static void *
eina_chained_mempool_malloc(void *data, EINA_UNUSED unsigned int size)
{
   Chained_Mempool *pool = data;
   Chained_Pool *p = NULL;
   void *mem;

   if (!eina_spinlock_take(&pool->mutex))
     {
#ifdef EINA_HAVE_DEBUG_THREADS
        assert(eina_thread_equal(pool->self, eina_thread_self()));
#endif
     }

   //we have some free space in first fill chain
   if (pool->first_fill) p = pool->first_fill;

   // base is not NULL - has a free slot
   if (p && !p->base && !p->last)
     {
       //Current pointed chain is filled , so point it to first one
       pool->first_fill = EINA_INLIST_CONTAINER_GET(pool->first, Chained_Pool);
       //Either first one has some free space or every chain is filled
       if (pool->first_fill && !pool->first_fill->base && !pool->first_fill->last)
        {
           p = NULL;
           pool->first_fill = NULL;
        }
     }

#ifdef DEBUG
   if (p == NULL)
     EINA_INLIST_FOREACH(pool->first, p)
       assert(!p->base && !p->last);
#endif

   // we have reached the end of the list - no free pools
   if (!p)
     {
       //new chain created ,point it to be the first_fill chain
        pool->first_fill = _eina_chained_mp_pool_new(pool);
        if (!pool->first_fill)
          {
             eina_spinlock_release(&pool->mutex);
             return NULL;
          }

        pool->first = eina_inlist_prepend(pool->first, EINA_INLIST_GET(pool->first_fill));
        pool->root = eina_rbtree_inline_insert(pool->root, EINA_RBTREE_GET(pool->first_fill),
                                               _eina_chained_mp_pool_cmp, NULL);
     }

   mem = _eina_chained_mempool_alloc_in(pool, pool->first_fill);

   eina_spinlock_release(&pool->mutex);

   return mem;
}

static void
eina_chained_mempool_free(void *data, void *ptr)
{
   Chained_Mempool *pool = data;
   Eina_Rbtree *r;
   Chained_Pool *p;

   // look 4 pool
   if (!eina_spinlock_take(&pool->mutex))
     {
#ifdef EINA_HAVE_DEBUG_THREADS
        assert(eina_thread_equal(pool->self, eina_thread_self()));
#endif
     }

   // searching for the right mempool
   r = eina_rbtree_inline_lookup(pool->root, ptr, 0, _eina_chained_mp_pool_key_cmp, NULL);

   // related mempool not found
   if (!r)
     {
#ifdef DEBUG
        ERR("%p is not the property of %p Chained_Mempool", ptr, pool);
#endif
        goto on_error;
     }

   p = EINA_RBTREE_CONTAINER_GET(r, Chained_Pool);

   _eina_chained_mempool_free_in(pool, p, ptr);

 on_error:
#ifndef NVALGRIND
   if (ptr)
     {
        VALGRIND_MEMPOOL_FREE(pool, ptr);
     }
#endif

   eina_spinlock_release(&pool->mutex);
   return;
}

static Eina_Bool
eina_chained_mempool_from(void *data, void *ptr)
{
   Chained_Mempool *pool = data;
   Eina_Rbtree *r;
   Chained_Pool *p;
   Eina_Trash *t;
#ifndef NVALGRIND
   Eina_Trash *last = NULL;
#endif
   void *pmem;
   Eina_Bool ret = EINA_FALSE;

   // look 4 pool
   if (!eina_spinlock_take(&pool->mutex))
     {
#ifdef EINA_HAVE_DEBUG_THREADS
        assert(eina_thread_equal(pool->self, eina_thread_self()));
#endif
     }

   // searching for the right mempool
   r = eina_rbtree_inline_lookup(pool->root, ptr, 0, _eina_chained_mp_pool_key_cmp, NULL);

   // related mempool not found
   if (!r) goto end;

   p = EINA_RBTREE_CONTAINER_GET(r, Chained_Pool);

   // pool mem base
   pmem = (void *)(((unsigned char *)p) + sizeof(Chained_Pool));

   // is it in pool mem?
   if (ptr < pmem)
     {
#ifdef DEBUG
        ERR("%p is inside the private part of %p pool from %p '%s' Chained_Mempool (could be the sign of a buffer underrun).", ptr, p, pool, pool->name);
#endif
        goto end;
     }

   // is it really a pointer returned by malloc
   if ((((unsigned char *)ptr) - (unsigned char *)(p + 1)) % pool->item_alloc)
     {
#ifdef DEBUG
        ERR("%p is %lu bytes inside a pointer served by %p '%s' Chained_Mempool (You are freeing the wrong pointer man !).",
            ptr, ((((unsigned char *)ptr) - (unsigned char *)(p + 1)) % pool->item_alloc), pool, pool->name);
#endif
        goto end;
     }

   // Check if the pointer was freed
   for (t = p->base; t != NULL; t = t->next)
     {
#ifndef NVALGRIND
        VALGRIND_MAKE_MEM_DEFINED(t, pool->item_alloc);
        if (last) VALGRIND_MAKE_MEM_NOACCESS(last, pool->item_alloc);
        last = t;
#endif

        if (t == ptr) goto end;
     }
#ifndef NVALGRIND
     if (last) VALGRIND_MAKE_MEM_NOACCESS(last, pool->item_alloc);
#endif

   // Seems like we have a valid pointer actually
   ret = EINA_TRUE;

 end:
   eina_spinlock_release(&pool->mutex);
   return ret;
}

static void
eina_chained_mempool_repack(void *data,
			    Eina_Mempool_Repack_Cb cb,
			    void *cb_data)
{
  Chained_Mempool *pool = data;
  Chained_Pool *start;
  Chained_Pool *tail;

  /* FIXME: Improvement - per Chained_Pool lock */
   if (!eina_spinlock_take(&pool->mutex))
     {
#ifdef EINA_HAVE_DEBUG_THREADS
        assert(eina_thread_equal(pool->self, eina_thread_self()));
#endif
     }

   pool->first = eina_inlist_sort(pool->first,
				  (Eina_Compare_Cb) _eina_chained_mempool_usage_cmp);

   /*
     idea : remove the almost empty pool at the beginning of the list by
     moving data in the last pool with empty slot
    */
   tail = EINA_INLIST_CONTAINER_GET(pool->first->last, Chained_Pool);
   while (tail && tail->usage == pool->pool_size)
     tail = EINA_INLIST_CONTAINER_GET((EINA_INLIST_GET(tail)->prev), Chained_Pool);

   while (tail)
     {
       unsigned char *src;
       unsigned char *dst;

       start = EINA_INLIST_CONTAINER_GET(pool->first, Chained_Pool);

       if (start == tail || start->usage == pool->pool_size)
	 break;

       for (src = start->limit - pool->group_size;
	    src != start->limit;
	    src += pool->item_alloc)
	 {
	   Eina_Bool is_free = EINA_FALSE;
	   Eina_Bool is_dead;

	   /* Do we have something inside that piece of memory */
	   if (start->last != NULL && src >= start->last)
	     {
	       is_free = EINA_TRUE;
	     }
	   else
	     {
	       Eina_Trash *over = start->base;

	       while (over != NULL && (unsigned char*) over != src)
		 over = over->next;

	       if (over == NULL)
		 is_free = EINA_TRUE;
	     }

	   if (is_free) continue ;

	   /* get a new memory pointer from the latest most occuped pool */
	   dst = _eina_chained_mempool_alloc_in(pool, tail);
	   /* move data from one to another */
	   memcpy(dst, src, pool->item_alloc);
	   /* notify caller */
	   cb(dst, src, cb_data);
	   /* destroy old pointer */
	   is_dead = _eina_chained_mempool_free_in(pool, start, src);

	   /* search last tail with empty slot */
	   while (tail && tail->usage == pool->pool_size)
	     tail = EINA_INLIST_CONTAINER_GET((EINA_INLIST_GET(tail)->prev),
					      Chained_Pool);
	   /* no more free space */
	   if (!tail || tail == start) break;
	   if (is_dead) break;
	 }
     }

   /* FIXME: improvement - reorder pool so that the most used one get in front */
   eina_spinlock_release(&pool->mutex);
}

static void *
eina_chained_mempool_realloc(EINA_UNUSED void *data,
                             EINA_UNUSED void *element,
                             EINA_UNUSED unsigned int size)
{
   return NULL;
}

static void *
eina_chained_mempool_init(const char *context,
                          EINA_UNUSED const char *option,
                          va_list args)
{
   Chained_Mempool *mp;
   int item_size;
   size_t length;

   length = context ? strlen(context) + 1 : 0;

   mp = calloc(1, sizeof(Chained_Mempool) + length);
   if (!mp)
      return NULL;

   item_size = va_arg(args, int);
   mp->pool_size = va_arg(args, int);

   if (length)
     {
        mp->name = (const char *)(mp + 1);
        memcpy((char *)mp->name, context, length);
     }

   mp->item_alloc = MAX(eina_mempool_alignof(item_size), sizeof(void *));

   mp->pool_size = (((((mp->item_alloc * mp->pool_size + aligned_chained_pool) / page_size)
		      + 1) * page_size)
		    - aligned_chained_pool) / mp->item_alloc;

#ifdef EINA_DEBUG_MALLOC
   mp->minimal_size = item_size * mp->pool_size + sizeof(Chained_Pool);
#endif

   mp->group_size = mp->item_alloc * mp->pool_size;
   mp->alloc_size = mp->group_size + aligned_chained_pool;

#ifndef NVALGRIND
   VALGRIND_CREATE_MEMPOOL(mp, 0, 1);
#endif

#ifdef EINA_HAVE_DEBUG_THREADS
   mp->self = eina_thread_self();
#endif
   mp->first_fill = NULL;
   eina_spinlock_new(&mp->mutex);

   return mp;
}

static void
eina_chained_mempool_shutdown(void *data)
{
   Chained_Mempool *mp;

   mp = (Chained_Mempool *)data;

   while (mp->first)
     {
        Chained_Pool *p = (Chained_Pool *)mp->first;

#ifdef DEBUG
        if (p->usage > 0)
           INF("Bad news we are destroying a non-empty mempool [%s]\n",
               mp->name);

#endif

        mp->first = eina_inlist_remove(mp->first, mp->first);
        mp->root = eina_rbtree_inline_remove(mp->root, EINA_RBTREE_GET(p),
                                             _eina_chained_mp_pool_cmp, NULL);
        _eina_chained_mp_pool_free(p);
     }

#ifdef DEBUG
   if (mp->root)
     ERR("Bad news, list of pool and rbtree are out of sync for %p !", mp);
#endif

#ifndef NVALGRIND
   VALGRIND_DESTROY_MEMPOOL(mp);
#endif

   eina_spinlock_free(&mp->mutex);

   free(mp);
}

static Eina_Mempool_Backend _eina_chained_mp_backend = {
   "chained_mempool",
   &eina_chained_mempool_init,
   &eina_chained_mempool_free,
   &eina_chained_mempool_malloc,
   &eina_chained_mempool_realloc,
   NULL,
   NULL,
   &eina_chained_mempool_shutdown,
   &eina_chained_mempool_repack,
   &eina_chained_mempool_from
};

Eina_Bool chained_init(void)
{
#if defined DEBUG || defined EINA_DEBUG_MALLOC
   _eina_chained_mp_log_dom = eina_log_domain_register("eina_mempool",
                                                       EINA_LOG_COLOR_DEFAULT);
   if (_eina_chained_mp_log_dom < 0)
     {
        EINA_LOG_ERR("Could not register log domain: eina_mempool");
        return EINA_FALSE;
     }

#endif
   aligned_chained_pool = eina_mempool_alignof(sizeof(Chained_Pool));
   page_size = eina_cpu_page_size();

   return eina_mempool_register(&_eina_chained_mp_backend);
}

void chained_shutdown(void)
{
   eina_mempool_unregister(&_eina_chained_mp_backend);
#if defined DEBUG || defined EINA_DEBUG_MALLOC
   eina_log_domain_unregister(_eina_chained_mp_log_dom);
   _eina_chained_mp_log_dom = -1;
#endif
}

#ifndef EINA_STATIC_BUILD_CHAINED_POOL

EINA_MODULE_INIT(chained_init);
EINA_MODULE_SHUTDOWN(chained_shutdown);

#endif /* ! EINA_STATIC_BUILD_CHAINED_POOL */
