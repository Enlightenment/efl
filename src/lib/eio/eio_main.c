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

static Eio_Version _version = { VMAJ, VMIN, VMIC, VREV };
EAPI Eio_Version *eio_version = &_version;

/**
 * @cond LOCAL
 */

/* Progress pool */
typedef struct _Eio_Alloc_Pool Eio_Alloc_Pool;

struct _Eio_Alloc_Pool
{
   int count;
   Eina_Trash *trash;

   Eina_Lock lock;
};

static int _eio_init_count = 0;
int _eio_log_dom_global = -1;

static Eio_Alloc_Pool progress_pool;
static Eio_Alloc_Pool direct_info_pool;
static Eio_Alloc_Pool char_pool;
static Eio_Alloc_Pool associate_pool;

static void *
_eio_pool_malloc(Eio_Alloc_Pool *pool, size_t sz)
{
   void *result = NULL;

   if (pool->count)
     {
        eina_lock_take(&(pool->lock));
        result = eina_trash_pop(&pool->trash);
        if (result) pool->count--;
        eina_lock_release(&(pool->lock));
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
        eina_lock_take(&(pool->lock));
        eina_trash_push(&pool->trash, data);
        pool->count++;
        eina_lock_release(&(pool->lock));
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
     return;

   progress = eio_progress_malloc();
   if (!progress) return;

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
eio_associate_malloc(const void *data, Eina_Free_Cb free_cb)
{
  Eio_File_Associate *tmp;

  tmp = _eio_pool_malloc(&associate_pool, sizeof (Eio_File_Associate));
  if (!tmp) return tmp;

  tmp->data = (void*) data;
  tmp->free_cb = free_cb;

  return tmp;
}

void
eio_associate_free(void *data)
{
  Eio_File_Associate *tmp;

  if (!data) return;

  tmp = data;
  if (tmp->free_cb)
    tmp->free_cb(tmp->data);
  _eio_pool_free(&associate_pool, tmp);
}

Eina_List *
eio_pack_send(Ecore_Thread *thread, Eina_List *pack, double *start)
{
   double current;

   current = ecore_time_get();
   if (current - *start > EIO_PACKED_TIME)
     {
        *start = current;
        ecore_thread_feedback(thread, pack);
        return NULL;
     }

   return pack;
}

/**
 * @endcond
 */


/*============================================================================*
 *                                   API                                      *
 *============================================================================*/

EAPI int
eio_init(void)
{
   if (++_eio_init_count != 1)
     return _eio_init_count;

   if (!eina_init())
     {
        fprintf(stderr, "Eio can not initialize Eina\n");
        return --_eio_init_count;
     }

   _eio_log_dom_global = eina_log_domain_register("eio", EIO_DEFAULT_LOG_COLOR);
   if (_eio_log_dom_global < 0)
     {
        EINA_LOG_ERR("Eio can not create a general log domain.");
        goto shutdown_eina;
     }

   if (!ecore_init())
     {
        ERR("Can not initialize Ecore\n");
        goto unregister_log_domain;
     }

   memset(&progress_pool, 0, sizeof(progress_pool));
   memset(&direct_info_pool, 0, sizeof(direct_info_pool));
   memset(&char_pool, 0, sizeof(char_pool));
   memset(&associate_pool, 0, sizeof(associate_pool));

   eina_lock_new(&(progress_pool.lock));
   eina_lock_new(&(direct_info_pool.lock));
   eina_lock_new(&(char_pool.lock));
   eina_lock_new(&(associate_pool.lock));

   eio_monitor_init();

   eina_log_timing(_eio_log_dom_global,
		   EINA_LOG_STATE_STOP,
		   EINA_LOG_STATE_INIT);

   return _eio_init_count;

unregister_log_domain:
   eina_log_domain_unregister(_eio_log_dom_global);
   _eio_log_dom_global = -1;
shutdown_eina:
   eina_shutdown();
   return --_eio_init_count;
}

EAPI int
eio_shutdown(void)
{
   Eio_File_Direct_Info *info;
   Eio_File_Char *cin;
   Eio_Progress *pg;
   Eio_File_Associate *asso;

   if (_eio_init_count <= 0)
     {
        ERR("Init count not greater than 0 in shutdown.");
        return 0;
     }
   if (--_eio_init_count != 0)
     return _eio_init_count;

   eina_log_timing(_eio_log_dom_global,
		   EINA_LOG_STATE_START,
		   EINA_LOG_STATE_SHUTDOWN);

   eio_monitor_shutdown();

   eina_lock_free(&(direct_info_pool.lock));
   eina_lock_free(&(progress_pool.lock));
   eina_lock_free(&(char_pool.lock));
   eina_lock_free(&(associate_pool.lock));

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
   eina_log_domain_unregister(_eio_log_dom_global);
   _eio_log_dom_global = -1;
   eina_shutdown();

   return _eio_init_count;
}
