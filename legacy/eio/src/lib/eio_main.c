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

static int _eio_count = 0;

/* Progress pool */
typedef struct _Eio_Alloc_Pool Eio_Alloc_Pool;

struct _Eio_Alloc_Pool
{
   int count;
   Eina_Trash *trash;

   pthread_mutex_t lock;
};

static Eio_Alloc_Pool progress = { 0, NULL, PTHREAD_MUTEX_INITIALIZER };
static Eio_Alloc_Pool direct_info = { 0, NULL, PTHREAD_MUTEX_INITIALIZER };


/**
 * @addtogroup Eio_Group Asynchronous Inout/Output library
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

   return _eio_count;
}

/**
 * @brief Shutdown eio and all it's submodule if possible.
 * @return the number of pending users of eio.
 */
EAPI int
eio_shutdown(void)
{
   Eina_File_Direct_Info *info;
   Eio_Progress *pg;

   _eio_count--;

   if (_eio_count > 0) return _eio_count;

   /* Cleanup pool */
   EINA_TRASH_CLEAN(&progress.trash, pg)
     free(pg);
   progress.count = 0;

   EINA_TRASH_CLEAN(&direct_info.trash, info)
     free(info);
   direct_info.count = 0;

   ecore_shutdown();
   eina_shutdown();
   return _eio_count;
}

static void *
_eio_pool_malloc(Eio_Alloc_Pool *pool, size_t sz)
{
   void *result = NULL;

   if (pool->count)
     {
        pthread_mutex_lock(&pool->lock);
        result = eina_trash_pop(&pool->trash);
        if (result) pool->count--;
        pthread_mutex_unlock(&pool->lock);
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
        pthread_mutex_lock(&pool->lock);
        eina_trash_push(&pool->trash, data);
        pool->count++;
        pthread_mutex_unlock(&pool->lock);
     }
}

Eio_Progress *
eio_progress_malloc(void)
{
   return _eio_pool_malloc(&progress, sizeof (Eio_Progress));
}

void
eio_progress_free(Eio_Progress *data)
{
   eina_stringshare_del(data->source);
   eina_stringshare_del(data->dest);

   _eio_pool_free(&progress, data);
}

void
eio_progress_send(Ecore_Thread *thread, Eio_File_Progress *op, off_t current, off_t max)
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

Eina_File_Direct_Info *
eio_direct_info_malloc(void)
{
   return _eio_pool_malloc(&direct_info, sizeof (Eina_File_Direct_Info) + sizeof (struct dirent));
}

void
eio_direct_info_free(Eina_File_Direct_Info *data)
{
   _eio_pool_free(&direct_info, data);
}

/**
 * @}
 */
