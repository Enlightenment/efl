/* EINA - EFL data type library
 * Copyright (C) 2010 Cedric BAIL, Vincent Torri
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

#include <assert.h>

#include "eina_config.h"
#include "eina_mempool.h"
#include "eina_trash.h"
#include "eina_inlist.h"
#include "eina_log.h"
#include "eina_lock.h"
#include "eina_thread.h"
#include "eina_cpu.h"

#ifndef NVALGRIND
# include <memcheck.h>
#endif

#include "eina_private.h"

#ifdef INF
#undef INF
#endif
#define INF(...) EINA_LOG_DOM_INFO(_eina_mempool_log_dom, __VA_ARGS__)

#ifdef WRN
#undef WRN
#endif
#define WRN(...) EINA_LOG_DOM_WARN(_eina_one_big_mp_log_dom, __VA_ARGS__)

#define OVER_MEM_TO_LIST(_pool, _over_mem)                      \
  ((Eina_Inlist *)(((char *)_over_mem) + (_pool)->offset_to_item_inlist))

#define OVER_MEM_FROM_LIST(_pool, _node)        \
  ((void *)(((char *)_node) - (_pool)->offset_to_item_inlist))

static int _eina_one_big_mp_log_dom = -1;

typedef struct _One_Big One_Big;
struct _One_Big
{
   const char *name;

   unsigned int item_size;
   int offset_to_item_inlist;

   int usage;
   int over;

   unsigned int served;
   unsigned int max;
   unsigned char *base;

   Eina_Trash *empty;
   Eina_Inlist *over_list;

#ifdef EINA_HAVE_DEBUG_THREADS
   Eina_Thread self;
#endif
   Eina_Lock mutex;
};

static void *
eina_one_big_malloc(void *data, EINA_UNUSED unsigned int size)
{
   One_Big *pool = data;
   unsigned char *mem = NULL;

   if (!eina_lock_take(&pool->mutex))
     {
#ifdef EINA_HAVE_DEBUG_THREADS
        assert(eina_thread_equal(pool->self, eina_thread_self()));
#endif
     }

   if (pool->empty)
     {
#ifndef NVALGRIND
        VALGRIND_MAKE_MEM_DEFINED(pool->empty, pool->item_size);
#endif
        mem = eina_trash_pop(&pool->empty);
        pool->usage++;
        goto on_exit;
     }

   if (!pool->base)
     {
	pool->base = malloc(pool->item_size * pool->max);
	if (!pool->base) goto retry_smaller;
#ifndef NVALGRIND
        VALGRIND_MAKE_MEM_NOACCESS(pool->base, pool->item_size * pool->max);
#endif
     }

   if (pool->served < pool->max)
     {
        mem = pool->base + (pool->served++ *pool->item_size);
        pool->usage++;
        goto on_exit;
     }

 retry_smaller:
   mem = malloc(sizeof(Eina_Inlist) + pool->offset_to_item_inlist);
   if (mem)
     {
        Eina_Inlist *node = OVER_MEM_TO_LIST(pool, mem);
        pool->over++;
        /* Only need to zero list elements and not the payload here */
        memset(node, 0, sizeof(Eina_Inlist));
        pool->over_list = eina_inlist_append(pool->over_list, node);
     }
#ifndef NVALGRIND
   VALGRIND_MAKE_MEM_NOACCESS(mem, pool->item_size);
#endif

on_exit:
   eina_lock_release(&pool->mutex);

#ifndef NVALGRIND
   VALGRIND_MEMPOOL_ALLOC(pool, mem, pool->item_size);
#endif
   return mem;
}

static void
eina_one_big_free(void *data, void *ptr)
{
   One_Big *pool = data;

   if (!eina_lock_take(&pool->mutex))
     {
#ifdef EINA_HAVE_DEBUG_THREADS
        assert(eina_thread_equal(pool->self, eina_thread_self()));
#endif
     }

   if ((void *)pool->base <= ptr
       && ptr < (void *)(pool->base + (pool->max * pool->item_size)))
     {
        eina_trash_push(&pool->empty, ptr);
        pool->usage--;

#ifndef NVALGRIND
        VALGRIND_MAKE_MEM_NOACCESS(ptr, pool->item_size);
#endif
     }
   else
     {
#ifndef NDEBUG
        Eina_Inlist *it;
#endif
        Eina_Inlist *il;

        il = OVER_MEM_TO_LIST(pool, ptr);

#ifndef NDEBUG
        for (it = pool->over_list; it != NULL; it = it->next)
          if (it == il) break;

        assert(it != NULL);
#endif

        pool->over_list = eina_inlist_remove(pool->over_list, il);

#ifndef NVALGRIND
        VALGRIND_MEMPOOL_FREE(pool, ptr);
#endif
        free(ptr);
        pool->over--;
     }

   eina_lock_release(&pool->mutex);
}

static Eina_Bool
eina_one_big_from(void *data, void *ptr)
{
   One_Big *pool = data;
   Eina_Bool r = EINA_FALSE;

   if (!eina_lock_take(&pool->mutex))
     {
#ifdef EINA_HAVE_DEBUG_THREADS
        assert(eina_thread_equal(pool->self, eina_thread_self()));
#endif
     }

   if ((void *)pool->base <= ptr
       && ptr < (void *)(pool->base + (pool->max * pool->item_size)))
     {
        Eina_Trash *t;
#ifndef NVALGRIND
        Eina_Trash *last = NULL;
#endif
        // Part of the bigger area

        // Check if it is a properly aligned element
        if (((unsigned char *)ptr - (unsigned char *) pool->base) % pool->item_size)
          {
#ifdef DEBUG
             ERR("%p is %lu bytes inside a pointer served by %p '%s' One_Big_Mempool (You are freeing the wrong pointer man !).",
                 ptr, ((unsigned char *)ptr - (unsigned char *) pool->base) % pool->item_size, pool, pool->name);
#endif
             goto end;
          }

        // Check if the pointer was freed
        for (t = pool->empty; t != NULL; t = t->next)
          {
#ifndef NVALGRIND
             VALGRIND_MAKE_MEM_DEFINED(t, pool->item_size);
             if (last) VALGRIND_MAKE_MEM_NOACCESS(last, pool->item_size);
             last = t;
#endif
             if (t == ptr) goto end;
          }
#ifndef NVALGRIND
        if (last) VALGRIND_MAKE_MEM_NOACCESS(last, pool->item_size);
#endif

        // Everything seems correct
        r = EINA_TRUE;
     }
   else
     {
        Eina_Inlist *it, *il;
        // Part of the smaller items inlist

        il = OVER_MEM_TO_LIST(pool, ptr);

        for (it = pool->over_list; it != NULL; it = it->next)
          if (it == il)
            {
               r = EINA_TRUE;
               break;
            }
     }

 end:
   eina_lock_release(&pool->mutex);
   return r;
}

typedef struct _Eina_Iterator_One_Big_Mempool Eina_Iterator_One_Big_Mempool;
struct _Eina_Iterator_One_Big_Mempool
{
   Eina_Iterator iterator;

   Eina_Iterator *walker;
   One_Big *pool;

   unsigned int offset;
};

static Eina_Bool
eina_mempool_iterator_next(Eina_Iterator_One_Big_Mempool *it, void **data)
{
   Eina_Inlist *il = NULL;

 retry:
   if (it->offset < (it->pool->max * it->pool->item_size))
     {
        unsigned char *ptr = (unsigned char *) (it->pool->base);

        ptr += it->offset;
        it->offset += it->pool->item_size;

        if (!eina_one_big_from(it->pool, ptr)) goto retry;

        if (data) *data = (void *) ptr;
        return EINA_TRUE;
     }

   if (!eina_iterator_next(it->walker, (void **) &il))
     return EINA_FALSE;

   if (data) *data = OVER_MEM_FROM_LIST(it->pool, il);
   return EINA_TRUE;
}

static One_Big *
eina_mempool_iterator_get_container(Eina_Iterator_One_Big_Mempool *it)
{
   return it->pool;
}

static void
eina_mempool_iterator_free(Eina_Iterator_One_Big_Mempool *it)
{
   eina_iterator_free(it->walker);
   free(it);
}

static Eina_Iterator *
eina_one_big_iterator_new(void *data)
{
   Eina_Iterator_One_Big_Mempool *it;
   One_Big *pool = data;

   it = calloc(1, sizeof (Eina_Iterator_One_Big_Mempool));
   if (!it) return NULL;

    it->walker = eina_inlist_iterator_new(pool->over_list);
    it->pool = pool;

    it->iterator.version = EINA_ITERATOR_VERSION;
    it->iterator.next = FUNC_ITERATOR_NEXT(eina_mempool_iterator_next);
    it->iterator.get_container = FUNC_ITERATOR_GET_CONTAINER(
                                                             eina_mempool_iterator_get_container);
    it->iterator.free = FUNC_ITERATOR_FREE(eina_mempool_iterator_free);

    EINA_MAGIC_SET(&it->iterator, EINA_MAGIC_ITERATOR);

    return &it->iterator;
}

static void *
eina_one_big_realloc(EINA_UNUSED void *data,
                     EINA_UNUSED void *element,
                     EINA_UNUSED unsigned int size)
{
   return NULL;
}

static void *
eina_one_big_init(const char *context,
                  EINA_UNUSED const char *option,
                  va_list args)
{
   One_Big *pool;
   int item_size;
   size_t length;

   length = context ? strlen(context) + 1 : 0;

   pool = calloc(1, sizeof (One_Big) + length);
   if (!pool)
      return NULL;

   item_size = va_arg(args, int);
   if (item_size < 1) item_size = 1;

   pool->item_size = MAX(eina_mempool_alignof(item_size), sizeof(void*));
   pool->max = va_arg(args, int);
   if (pool->max < 1) pool->max = 1;

   pool->offset_to_item_inlist = pool->item_size;
   if (pool->offset_to_item_inlist % (int)sizeof(void *) != 0)
     {
        pool->offset_to_item_inlist =
          (((pool->offset_to_item_inlist / (int)sizeof(void *)) + 1) *
           (int)sizeof(void *));
     }

   if (length)
     {
        pool->name = (const char *)(pool + 1);
        memcpy((char *)pool->name, context, length);
     }

#ifdef EINA_HAVE_DEBUG_THREADS
   pool->self = eina_thread_self();
#endif
   eina_lock_new(&pool->mutex);

#ifndef NVALGRIND
   VALGRIND_CREATE_MEMPOOL(pool, 0, 1);
#endif

   return pool;
}

static void
eina_one_big_shutdown(void *data)
{
   One_Big *pool = data;

   if (!pool) return;
   if (!eina_lock_take(&pool->mutex))
     {
#ifdef EINA_HAVE_DEBUG_THREADS
        assert(eina_thread_equal(pool->self, eina_thread_self()));
#endif
     }

   if (pool->over > 0)
     {
// FIXME: should we warn here? one_big mempool exceeded its alloc and now
// mempool is cleaning up the mess created. be quiet for now as we were before
// but edje seems to be a big offender at the moment! bad cedric! :)
//        WRN(
//            "Pool [%s] over by %i. cleaning up for you",
//            pool->name, pool->over);
        while (pool->over_list)
          {
             Eina_Inlist *il = pool->over_list;
             void *ptr = OVER_MEM_FROM_LIST(pool, il);
             pool->over_list = eina_inlist_remove(pool->over_list, il);
             free(ptr);
             pool->over--;
          }
     }
   if (pool->over > 0)
     {
        WRN(
            "Pool [%s] still over by %i\n",
            pool->name, pool->over);
     }

#ifndef NVALGRIND
   VALGRIND_DESTROY_MEMPOOL(pool);
#endif

   if (pool->base) free(pool->base);

   eina_lock_release(&pool->mutex);
   eina_lock_free(&pool->mutex);
   free(pool);
}


static Eina_Mempool_Backend _eina_one_big_mp_backend = {
   "one_big",
   &eina_one_big_init,
   &eina_one_big_free,
   &eina_one_big_malloc,
   &eina_one_big_realloc,
   NULL,
   NULL,
   &eina_one_big_shutdown,
   NULL,
   &eina_one_big_from,
   &eina_one_big_iterator_new,
   NULL
};

Eina_Bool one_big_init(void)
{
#ifdef DEBUG
   _eina_one_big_mp_log_dom = eina_log_domain_register("eina_one_big_mempool",
                                                       EINA_LOG_COLOR_DEFAULT);
   if (_eina_one_big_mp_log_dom < 0)
     {
        EINA_LOG_ERR("Could not register log domain: eina_one_big_mempool");
        return EINA_FALSE;
     }

#endif
   return eina_mempool_register(&_eina_one_big_mp_backend);
}

void one_big_shutdown(void)
{
   eina_mempool_unregister(&_eina_one_big_mp_backend);
#ifdef DEBUG
   eina_log_domain_unregister(_eina_one_big_mp_log_dom);
   _eina_one_big_mp_log_dom = -1;
#endif
}

#ifndef EINA_STATIC_BUILD_ONE_BIG

EINA_MODULE_INIT(one_big_init);
EINA_MODULE_SHUTDOWN(one_big_shutdown);

#endif /* ! EINA_STATIC_BUILD_ONE_BIG */
