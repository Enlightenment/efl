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

#ifdef EFL_HAVE_POSIX_THREADS
#include <pthread.h>

# ifdef EFL_DEBUG_THREADS
#  include <assert.h>
# endif
#endif

#ifdef EFL_HAVE_WIN32_THREADS
# define WIN32_LEAN_AND_MEAN
# include <windows.h>
# undef WIN32_LEAN_AND_MEAN
#endif

#include "eina_inlist.h"
#include "eina_error.h"
#include "eina_module.h"
#include "eina_mempool.h"
#include "eina_trash.h"
#include "eina_rbtree.h"

#include "eina_private.h"

#ifndef NVALGRIND
# include <valgrind/memcheck.h>
#endif

#ifdef DEBUG
#include <assert.h>
#include "eina_log.h"

static int _eina_chained_mp_log_dom = -1;

#ifdef INF
#undef INF
#endif
#define INF(...) EINA_LOG_DOM_INFO(_eina_chained_mp_log_dom, __VA_ARGS__)
#endif

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
#ifdef EFL_HAVE_THREADS
# ifdef EFL_HAVE_POSIX_THREADS
#  ifdef EFL_DEBUG_THREADS
   pthread_t self;
#  endif
   pthread_mutex_t mutex;
# else
   HANDLE mutex;
# endif
#endif
};

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

static inline Eina_Rbtree_Direction
_eina_chained_mp_pool_cmp(const Eina_Rbtree *left, const Eina_Rbtree *right, __UNUSED__ void *data)
{
   if (left < right) return EINA_RBTREE_LEFT;
   return EINA_RBTREE_RIGHT;
}

static inline int
_eina_chained_mp_pool_key_cmp(const Eina_Rbtree *node, const void *key,
                              __UNUSED__ int length, __UNUSED__ void *data)
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
   unsigned int alignof;

   eina_error_set(0);
   p = malloc(pool->alloc_size);
   if (!p)
     {
        eina_error_set(EINA_ERROR_OUT_OF_MEMORY);
        return NULL;
     }

   alignof = eina_mempool_alignof(sizeof(Chained_Pool));
   ptr = (unsigned char *)p + alignof;
   p->usage = 0;
   p->base = NULL;

   p->last = ptr;
   p->limit = ptr + pool->item_alloc * pool->pool_size;

#ifndef NVALGRIND
   VALGRIND_MAKE_MEM_NOACCESS(ptr, pool->alloc_size - alignof);
#endif

   return p;
}

static inline void
_eina_chained_mp_pool_free(Chained_Pool *p)
{
   free(p);
}

static void *
eina_chained_mempool_malloc(void *data, __UNUSED__ unsigned int size)
{
   Chained_Mempool *pool = data;
   Chained_Pool *p = NULL;
   void *mem;

#ifdef EFL_HAVE_THREADS
   if (_threads_activated)
     {
# ifdef EFL_HAVE_POSIX_THREADS
        pthread_mutex_lock(&pool->mutex);
# else
        WaitForSingleObject(pool->mutex, INFINITE);
# endif
     }
#ifdef EFL_DEBUG_THREADS
   else
     assert(pthread_equal(pool->self, pthread_self()));
#endif
#endif

   // Either we have some free space in the first one, or there is no free space.
   p = EINA_INLIST_CONTAINER_GET(pool->first, Chained_Pool);

   // base is not NULL - has a free slot
   if (p && !p->base && !p->last)
     p = NULL;

#ifdef DEBUG
   if (p == NULL)
     EINA_INLIST_FOREACH(pool->first, p)
       assert(!p->base && !p->last);
#endif

   // we have reached the end of the list - no free pools
   if (!p)
     {
        p = _eina_chained_mp_pool_new(pool);
        if (!p)
          {
#ifdef EFL_HAVE_PTHREAD
             if (_threads_activated)
               {
# ifdef EFL_HAVE_POSIX_THREADS
                  pthread_mutex_unlock(&pool->mutex);
# else
                  ReleaseMutex(pool->mutex);
# endif
               }
#endif
             return NULL;
          }

        pool->first = eina_inlist_prepend(pool->first, EINA_INLIST_GET(p));
        pool->root = eina_rbtree_inline_insert(pool->root, EINA_RBTREE_GET(p),
                                               _eina_chained_mp_pool_cmp, NULL);
     }

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

#ifdef EFL_HAVE_THREADS
   if (_threads_activated)
     {
# ifdef EFL_HAVE_POSIX_THREADS
        pthread_mutex_unlock(&pool->mutex);
# else
        ReleaseMutex(pool->mutex);
# endif
     }
#endif

#ifndef NVALGRIND
   VALGRIND_MEMPOOL_ALLOC(pool, mem, pool->item_alloc);
#endif

   return mem;
}

static void
eina_chained_mempool_free(void *data, void *ptr)
{
   Chained_Mempool *pool = data;
   Eina_Rbtree *r;
   Chained_Pool *p;
   void *pmem;

   // look 4 pool

#ifdef EFL_HAVE_THREADS
   if (_threads_activated)
     {
# ifdef EFL_HAVE_POSIX_THREADS
        pthread_mutex_lock(&pool->mutex);
# else
        WaitForSingleObject(pool->mutex, INFINITE);
# endif
     }
#ifdef EFL_DEBUG_THREADS
   else
     assert(pthread_equal(pool->self, pthread_self()));
#endif
#endif

   // searching for the right mempool
   r = eina_rbtree_inline_lookup(pool->root, ptr, 0, _eina_chained_mp_pool_key_cmp, NULL);

   // related mempool not found
   if (!r)
     {
#ifdef DEBUG
        INF("%p is not the property of %p Chained_Mempool", ptr, pool);
#endif
        goto on_error;
     }

   p = EINA_RBTREE_CONTAINER_GET(r, Chained_Pool);

   // pool mem base
   pmem = (void *)(((unsigned char *)p) + sizeof(Chained_Pool));

   // is it in pool mem?
   if (ptr < pmem)
     {
#ifdef DEBUG
        INF("%p is inside the private part of %p pool from %p Chained_Mempool (could be the sign of a buffer underrun).", ptr, p, pool);
#endif
        goto on_error;
     }

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
        _eina_chained_mp_pool_free(p);
     }
   else
     {
        // move to front
        pool->first = eina_inlist_promote(pool->first, EINA_INLIST_GET(p));
     }

 on_error:
#ifndef NVALGRIND
   if (ptr)
     {
        VALGRIND_MEMPOOL_FREE(pool, ptr);
     }
#endif

#ifdef EFL_HAVE_THREADS
   if (_threads_activated)
     {
# ifdef EFL_HAVE_POSIX_THREADS
        pthread_mutex_unlock(&pool->mutex);
# else
        ReleaseMutex(pool->mutex);
# endif
     }
#endif
   return;
}

static void *
eina_chained_mempool_realloc(__UNUSED__ void *data,
                             __UNUSED__ void *element,
                             __UNUSED__ unsigned int size)
{
   return NULL;
}

static void *
eina_chained_mempool_init(const char *context,
                          __UNUSED__ const char *option,
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

   mp->item_alloc = eina_mempool_alignof(item_size);
   mp->group_size = mp->item_alloc * mp->pool_size;
   mp->alloc_size = mp->group_size + eina_mempool_alignof(sizeof(Chained_Pool));

#ifndef NVALGRIND
   VALGRIND_CREATE_MEMPOOL(mp, 0, 1);
#endif

#ifdef EFL_HAVE_THREADS
# ifdef EFL_HAVE_POSIX_THREADS
#  ifdef EFL_DEBUG_THREADS
   mp->self = pthread_self();
#  endif
   pthread_mutex_init(&mp->mutex, NULL);
# else
   mp->mutex = CreateMutex(NULL, FALSE, NULL);
# endif
#endif

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
           INF("Bad news we are destroying not an empty mempool [%s]\n",
               mp->name);

#endif

        mp->first = eina_inlist_remove(mp->first, mp->first);
        mp->root = eina_rbtree_inline_remove(mp->root, EINA_RBTREE_GET(p),
                                             _eina_chained_mp_pool_cmp, NULL);
        _eina_chained_mp_pool_free(p);
     }

#ifdef DEBUG
   if (mp->root)
     INF("Bad news, list of pool and rbtree are out of sync for %p !", mp);
#endif

#ifndef NVALGRIND
   VALGRIND_DESTROY_MEMPOOL(mp);
#endif

#ifdef EFL_HAVE_THREADS
# ifdef EFL_HAVE_POSIX_THREADS
#  ifdef EFL_DEBUG_THREADS
   assert(pthread_equal(mp->self, pthread_self()));
#  endif
   pthread_mutex_destroy(&mp->mutex);
# else
   CloseHandle(mp->mutex);
# endif
#endif

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
   &eina_chained_mempool_shutdown
};

Eina_Bool chained_init(void)
{
#ifdef DEBUG
   _eina_chained_mp_log_dom = eina_log_domain_register("eina_mempool",
                                                       EINA_LOG_COLOR_DEFAULT);
   if (_eina_chained_mp_log_dom < 0)
     {
        EINA_LOG_ERR("Could not register log domain: eina_mempool");
        return EINA_FALSE;
     }

#endif
   return eina_mempool_register(&_eina_chained_mp_backend);
}

void chained_shutdown(void)
{
   eina_mempool_unregister(&_eina_chained_mp_backend);
#ifdef DEBUG
   eina_log_domain_unregister(_eina_chained_mp_log_dom);
   _eina_chained_mp_log_dom = -1;
#endif
}

#ifndef EINA_STATIC_BUILD_CHAINED_POOL

EINA_MODULE_INIT(chained_init);
EINA_MODULE_SHUTDOWN(chained_shutdown);

#endif /* ! EINA_STATIC_BUILD_CHAINED_POOL */
