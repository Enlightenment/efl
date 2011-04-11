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

#ifdef EFL_HAVE_POSIX_THREADS
# include <pthread.h>
#endif

#include <assert.h>

#ifdef EFL_HAVE_WIN32_THREADS
# define WIN32_LEAN_AND_MEAN
# include <windows.h>
# undef WIN32_LEAN_AND_MEAN
#endif

#include "eina_mempool.h"
#include "eina_trash.h"
#include "eina_inlist.h"
#include "eina_log.h"

#ifndef NVALGRIND
# include <valgrind/memcheck.h>
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

static int _eina_one_big_mp_log_dom = -1;

typedef struct _One_Big One_Big;
struct _One_Big
{
   const char *name;

   int item_size;

   int usage;
   int over;

   int served;
   int max;
   unsigned char *base;

   Eina_Trash *empty;
   Eina_Inlist *over_list;

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

static void *
eina_one_big_malloc(void *data, __UNUSED__ unsigned int size)
{
   One_Big *pool = data;
   unsigned char *mem = NULL;

#ifdef EFL_HAVE_THREADS
   if (_threads_activated)
     {
# ifdef EFL_HAVE_POSIX_THREADS
        pthread_mutex_lock(&pool->mutex);
# else
        WaitForSingleObject(pool->mutex, INFINITE);
# endif
     }
# ifdef EFL_HAVE_POSIX_THREADS
#  ifdef EFL_DEBUG_THREADS
   else
     assert(pthread_equal(pool->self, pthread_self()));
#  endif
# endif
#endif

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
	if (!pool->base)
	  {
	     eina_error_set(EINA_ERROR_OUT_OF_MEMORY);
	     goto retry_smaller;
	  }
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
   eina_error_set(0);
   mem = malloc(sizeof(Eina_Inlist) + pool->item_size);
   if (!mem)
      eina_error_set(EINA_ERROR_OUT_OF_MEMORY);
   else
     {
        pool->over++;
        memset(mem, 0, sizeof(Eina_Inlist));
        pool->over_list = eina_inlist_append(pool->over_list, 
                                             (Eina_Inlist *)mem);
        mem = ((unsigned char *)mem) + sizeof(Eina_Inlist);
     }
#ifndef NVALGRIND
   VALGRIND_MAKE_MEM_NOACCESS(mem, pool->item_size);
#endif

on_exit:
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
   VALGRIND_MEMPOOL_ALLOC(pool, mem, pool->item_size);
#endif
   return mem;
}

static void
eina_one_big_free(void *data, void *ptr)
{
   One_Big *pool = data;

#ifdef EFL_HAVE_THREADS
   if (_threads_activated)
     {
# ifdef EFL_HAVE_POSIX_THREADS
        pthread_mutex_lock(&pool->mutex);
# else
        WaitForSingleObject(pool->mutex, INFINITE);
# endif
     }
# ifdef EFL_HAVE_POSIX_THREADS
#  ifdef EFL_DEBUG_THREADS
   else
     assert(pthread_equal(pool->self, pthread_self()));
#  endif
# endif
#endif

   if ((void *)pool->base <= ptr
       && ptr < (void *)(pool->base + (pool->max * pool->item_size)))
     {
        eina_trash_push(&pool->empty, ptr);
        pool->usage--;
     }
   else
     {
#ifndef NDEBUG
        Eina_Inlist *it;
#endif
        Eina_Inlist *il;

        il = (Eina_Inlist *)(((unsigned char *)ptr) - sizeof(Eina_Inlist));

#ifndef NDEBUG
        for (it = pool->over_list; it != NULL; it = it->next)
          if (it == il) break;

        assert(it != NULL);
#endif

        pool->over_list = eina_inlist_remove(pool->over_list, il);
        free(il);
        pool->over--;
     }

#ifndef NVALGRIND
   VALGRIND_MEMPOOL_FREE(pool, ptr);
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
}

static void *
eina_one_big_realloc(__UNUSED__ void *data,
                     __UNUSED__ void *element,
                     __UNUSED__ unsigned int size)
{
   return NULL;
}

static void *
eina_one_big_init(const char *context,
                  __UNUSED__ const char *option,
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

   pool->item_size = eina_mempool_alignof(item_size);
   pool->max = va_arg(args, int);

   if (length)
     {
        pool->name = (const char *)(pool + 1);
        memcpy((char *)pool->name, context, length);
     }

#ifdef EFL_HAVE_THREADS
# ifdef EFL_HAVE_POSIX_THREADS
#  ifdef EFL_DEBUG_THREADS
   pool->self = pthread_self();
#  endif
   pthread_mutex_init(&pool->mutex, NULL);
# else
   pool->mutex = CreateMutex(NULL, FALSE, NULL);
# endif
#endif

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
#ifdef EFL_HAVE_THREADS
   if (_threads_activated)
     {
# ifdef EFL_HAVE_POSIX_THREADS
        pthread_mutex_lock(&pool->mutex);
# else
        WaitForSingleObject(pool->mutex, INFINITE);
# endif
     }
# ifdef EFL_HAVE_POSIX_THREADS
#  ifdef EFL_DEBUG_THREADS
   else
     assert(pthread_equal(pool->self, pthread_self()));
#  endif
# endif
#endif

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
             pool->over_list = eina_inlist_remove(pool->over_list, il);
             free(il);
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

#ifdef EFL_HAVE_THREADS
   if (_threads_activated)
     {
# ifdef EFL_HAVE_POSIX_THREADS
        pthread_mutex_unlock(&pool->mutex);
        pthread_mutex_destroy(&pool->mutex);
# else
        ReleaseMutex(pool->mutex);
        CloseHandle(pool->mutex);
# endif
     }
#endif
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

