/* EIO - EFL data type library
 * Copyright (C) 2010 Enlightenment Developers:
 *           Cedric Bail <cedric.bail@free.fr>
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
#include "eio_private.h"
#include "Eio.h"

/*============================================================================*
 *                                  Local                                     *
 *============================================================================*/

/**
 * @cond LOCAL
 */

#ifdef EFL_HAVE_POSIX_THREADS
# define EIO_MUTEX_TYPE         pthread_mutex_t
# define EIO_MUTEX_INITIALIZER  PTHREAD_MUTEX_INITIALIZER
# define EIO_MUTEX_INIT(Pool)
# define EIO_MUTEX_LOCK(Pool)   pthread_mutex_lock(&Pool->lock)
# define EIO_MUTEX_UNLOCK(Pool) pthread_mutex_unlock(&Pool->lock)
# define EIO_MUTEX_DESTROY(Pool)
#endif

#ifdef EFL_HAVE_WIN32_THREADS
# define EIO_MUTEX_TYPE          HANDLE
# define EIO_MUTEX_INITIALIZER   NULL
# define EIO_MUTEX_INIT(Pool)    Pool.lock = CreateMutex(NULL, FALSE, NULL)
# define EIO_MUTEX_LOCK(Pool)    WaitForSingleObject(Pool->lock, INFINITE)
# define EIO_MUTEX_UNLOCK(Pool)  ReleaseMutex(Pool->lock)
# define EIO_MUTEX_DESTROY(Pool) CloseHandle(Pool.lock)
#endif

/* Progress pool */
typedef struct _Eio_Alloc_Pool Eio_Alloc_Pool;

struct _Eio_Alloc_Pool
{
   int count;
   Eina_Trash *trash;

   EIO_MUTEX_TYPE lock;
};

static int _eio_count = 0;

static Eio_Alloc_Pool progress_pool = { 0, NULL, EIO_MUTEX_INITIALIZER };
static Eio_Alloc_Pool direct_info_pool = { 0, NULL, EIO_MUTEX_INITIALIZER };
static Eio_Alloc_Pool char_pool = { 0, NULL, EIO_MUTEX_INITIALIZER };
static Eio_Alloc_Pool associate_pool = { 0, NULL, EIO_MUTEX_INITIALIZER };

static void *
_eio_pool_malloc(Eio_Alloc_Pool *pool, size_t sz)
{
   void *result = NULL;

   if (pool->count)
     {
        EIO_MUTEX_LOCK(pool);
        result = eina_trash_pop(&pool->trash);
        if (result) pool->count--;
        EIO_MUTEX_UNLOCK(pool);
     }

   if (!result) result = malloc(sz);
   return result;
}

static void
_eio_pool_free(Eio_Alloc_Pool *pool, void *data)
{
   if (pool->count >= EIO_PROGRESS_LIMIT)
     {
        free(data);
     }
   else
     {
        EIO_MUTEX_LOCK(pool);
        eina_trash_push(&pool->trash, data);
        pool->count++;
        EIO_MUTEX_UNLOCK(pool);
     }
}

/**
 * @endcond
 */

/*============================================================================*
 *                                 Global                                     *
 *============================================================================*/

/**
 * @cond LOCAL
 */

Eio_Progress *
eio_progress_malloc(void)
{
   return _eio_pool_malloc(&progress_pool, sizeof (Eio_Progress));
}

void
eio_progress_free(Eio_Progress *data)
{
   eina_stringshare_del(data->source);
   eina_stringshare_del(data->dest);

   _eio_pool_free(&progress_pool, data);
}

void
eio_progress_send(Ecore_Thread *thread, Eio_File_Progress *op, long long current, long long max)
{
   Eio_Progress *progress;

   if (op->progress_cb == NULL)
     return ;

   progress = eio_progress_malloc();
   if (!progress) return ;

   progress->op = op->op;
   progress->current = current;
   progress->max = max;
   progress->percent = (float) current * 100.0 / (float) max;
   progress->source = eina_stringshare_ref(op->source);
   progress->dest = eina_stringshare_ref(op->dest);

   ecore_thread_feedback(thread, progress);
}

Eio_File_Direct_Info *
eio_direct_info_malloc(void)
{
   return _eio_pool_malloc(&direct_info_pool, sizeof (Eio_File_Direct_Info));
}

void
eio_direct_info_free(Eio_File_Direct_Info *data)
{
   _eio_pool_free(&direct_info_pool, data);
}

Eio_File_Char *
eio_char_malloc(void)
{
  return _eio_pool_malloc(&char_pool, sizeof (Eio_File_Char));
}

void
eio_char_free(Eio_File_Char *data)
{
  _eio_pool_free(&char_pool, data);
}

Eio_File_Associate *
eio_associate_malloc(void *data, Eina_Free_Cb free_cb)
{
  Eio_File_Associate *tmp;

  tmp = _eio_pool_malloc(&associate_pool, sizeof (Eio_File_Associate));
  if (!tmp) return tmp;

  tmp->data = data;
  tmp->free_cb = free_cb;

  return tmp;
}

void
eio_associate_free(void *data)
{
  Eio_File_Associate *tmp;

  if (!data) return ;

  tmp = data;
  if (tmp->free_cb)
    tmp->free_cb(tmp->data);
  _eio_pool_free(&associate_pool, tmp);
}


/**
 * @endcond
 */


/*============================================================================*
 *                                   API                                      *
 *============================================================================*/

/**
 * @addtogroup Eio_Group Eio Reference API
 *
 * @{
 */

/**
 * @brief Initialize eio and all it's required submodule.
 * @return the current number of eio users.
 */
EAPI int
eio_init(void)
{
   _eio_count++;

   if (_eio_count > 1) return _eio_count;

   eina_init();
   ecore_init();

   EIO_MUTEX_INIT(progress_pool);
   EIO_MUTEX_INIT(direct_info_pool);
   EIO_MUTEX_INIT(char_pool);
   EIO_MUTEX_INIT(associate_pool);

   eio_monitor_init();

   return _eio_count;
}

/**
 * @brief Shutdown eio and all it's submodule if possible.
 * @return the number of pending users of eio.
 */
EAPI int
eio_shutdown(void)
{
   Eio_File_Direct_Info *info;
   Eio_File_Char *cin;
   Eio_Progress *pg;
   Eio_File_Associate *asso;

   _eio_count--;

   if (_eio_count > 0) return _eio_count;

   EIO_MUTEX_DESTROY(direct_info);
   EIO_MUTEX_DESTROY(progress_pool);
   EIO_MUTEX_DESTROY(char_pool);
   EIO_MUTEX_DESTROY(associate_pool);

   /* Cleanup pool */
   EINA_TRASH_CLEAN(&progress_pool.trash, pg)
     free(pg);
   progress_pool.count = 0;

   EINA_TRASH_CLEAN(&direct_info_pool.trash, info)
     free(info);
   direct_info_pool.count = 0;

   EINA_TRASH_CLEAN(&char_pool.trash, cin)
     free(cin);
   char_pool.count = 0;

   EINA_TRASH_CLEAN(&associate_pool.trash, asso)
     free(asso);
   associate_pool.count = 0;

   ecore_shutdown();
   eina_shutdown();
   return _eio_count;
}

/**
 * @}
 */
