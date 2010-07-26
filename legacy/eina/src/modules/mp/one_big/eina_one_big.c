/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */
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
#include <pthread.h>
#endif

#ifdef EFL_HAVE_WIN32_THREADS
# define WIN32_LEAN_AND_MEAN
# include <windows.h>
# undef WIN32_LEAN_AND_MEAN
#endif

#include "eina_mempool.h"
#include "eina_trash.h"

#ifdef DEBUG
#include "eina_private.h"
#include "eina_log.h"

static int _eina_mempool_log_dom = -1;

#ifdef INF
#undef INF
#endif
#define INF(...) EINA_LOG_DOM_INFO(_eina_mempool_log_dom, __VA_ARGS__)
#endif

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

#ifdef EFL_HAVE_THREADS
# ifdef EFL_HAVE_POSIX_THREADS
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
   unsigned char *mem;

#ifdef EFL_HAVE_THREADS
# ifdef EFL_HAVE_POSIX_THREADS
   pthread_mutex_lock(&pool->mutex);
# else
   WaitForSingleObject(pool->mutex, INFINITE);
# endif
#endif

   if (pool->base)
     {
	mem = eina_trash_pop(&pool->empty);
	pool->usage++;
	goto on_exit;
     }

   if (pool->served < pool->max)
     {
	mem = pool->base + (pool->served++ * pool->item_size);
	pool->usage++;
	goto on_exit;
     }

   eina_error_set(0);
   mem = malloc(pool->item_size);
   if (!mem)
     {
	eina_error_set(EINA_ERROR_OUT_OF_MEMORY);
     }
   else
     {
	pool->over++;
     }

 on_exit:
#ifdef EFL_HAVE_THREADS
# ifdef EFL_HAVE_POSIX_THREADS
   pthread_mutex_unlock(&pool->mutex);
# else
   ReleaseMutex(pool->mutex);
# endif
#endif
   return mem;
}

static void
eina_one_big_free(void *data, void *ptr)
{
   One_Big *pool = data;

#ifdef EFL_HAVE_THREADS
# ifdef EFL_HAVE_POSIX_THREADS
   pthread_mutex_lock(&pool->mutex);
# else
   WaitForSingleObject(pool->mutex, INFINITE);
# endif
#endif

   if ((void*) pool->base <= ptr
       && ptr < (void*) (pool->base + (pool->max * pool->item_size)))
     {
	eina_trash_push(&pool->empty, ptr);
	pool->usage--;
     }
   else
     {
	free(ptr);
	pool->over--;
     }

#ifdef EFL_HAVE_THREADS
# ifdef EFL_HAVE_POSIX_THREADS
   pthread_mutex_unlock(&pool->mutex);
# else
   ReleaseMutex(pool->mutex);
# endif
#endif
}

static void*
eina_one_big_realloc(__UNUSED__ void *data, __UNUSED__ void *element, __UNUSED__ unsigned int size)
{
   return NULL;
}

static void*
eina_one_big_init(const char *context, __UNUSED__ const char *option, va_list args)
{
   One_Big *pool;
   int item_size;
   size_t length;

   length = context ? strlen(context) + 1 : 0;

   pool = calloc(1, sizeof (One_Big) + length);
   if (!pool) return NULL;

   item_size = va_arg(args, int);

   pool->item_size = eina_mempool_alignof(item_size);
   pool->max = va_arg(args, int);

   if (length)
     {
	pool->name = (const char*) (pool + 1);
	memcpy((char*) pool->name, context, length);
     }

   pool->base = malloc(pool->item_size * pool->max);
   if (!pool->base)
     {
	free(pool);
	return NULL;
     }

#ifdef EFL_HAVE_THREADS
# ifdef EFL_HAVE_POSIX_THREADS
   pthread_mutex_init(&pool->mutex, NULL);
# else
   pool->mutex = CreateMutex(NULL, FALSE, NULL);
# endif
#endif

   return pool;
}

static void
eina_one_big_shutdown(void *data)
{
   One_Big *pool;

   pool = data;

#ifdef DEBUG
   if (pool->usage > 0)
     INF("Bad news we are destroying memory still referenced in mempool [%s]\n", pool->name);

   if (pool->over > 0)
     INF("Bad news we are loosing track of pointer from mempool [%s]\n", pool->name);
#endif

   free(pool->base);
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
  &eina_one_big_shutdown
};

Eina_Bool one_big_init(void)
{
#ifdef DEBUG
   _eina_mempool_log_dom = eina_log_domain_register("eina_one_big_mempool", EINA_LOG_COLOR_DEFAULT);
   if (_eina_mempool_log_dom < 0)
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
   eina_log_domain_unregister(_eina_mempool_log_dom);
   _eina_mempool_log_dom = -1;
#endif
}

#ifndef EINA_STATIC_BUILD_ONE_BIG

EINA_MODULE_INIT(one_big_init);
EINA_MODULE_SHUTDOWN(one_big_shutdown);

#endif /* ! EINA_STATIC_BUILD_ONE_BIG */

