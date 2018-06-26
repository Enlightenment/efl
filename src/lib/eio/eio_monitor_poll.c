/* EIO - EFL data type library
 * Copyright (C) 2011 Enlightenment Developers:
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

typedef struct _Eio_Monitor_Stat Eio_Monitor_Stat;

struct _Eio_Monitor_Stat
{
   Eina_Stat buffer;
   int version;
};

struct _Eio_Monitor_Backend
{
   Eio_Monitor *parent;

   Eina_Stat self;
   Eina_Hash *children;

   Ecore_Timer *timer;
   Ecore_Thread *work;

   int version;

   Eina_Bool delete_me : 1;
   Eina_Bool initialised : 1;
   Eina_Bool destroyed : 1;
};

static double fallback_interval = 60.0;
static Eina_Hash *timer_hash;

static Eina_Bool _eio_monitor_fallback_timer_cb(void *data);

static void
_eio_monitor_fallback_heavy_cb(void *data, Ecore_Thread *thread)
{
   Eio_Monitor_Backend *backend = data;
   Eina_Iterator *it;
   Eina_Stat *est;
   Eina_File_Direct_Info *info;
   _eio_stat_t st;
   /* FIXME : copy ecore_file_monitor_poll here */

   if (!backend->initialised)
     est = &backend->self;
   else
     est = alloca(sizeof (Eina_Stat));

   if (!backend->parent)
     return;

   if (_eio_stat(backend->parent->path, &st))
     {
        if (backend->initialised && !backend->destroyed)
          {
             ecore_thread_main_loop_begin();
             _eio_monitor_send(backend->parent, backend->parent->path, EIO_MONITOR_SELF_DELETED);
             ecore_thread_main_loop_end();
             backend->destroyed = EINA_TRUE;
          }
        return;
     }

   backend->destroyed = EINA_FALSE;

   est->dev = st.st_dev;
   est->ino = st.st_ino;
   est->mode = st.st_mode;
   est->nlink = st.st_nlink;
   est->uid = st.st_uid;
   est->gid = st.st_gid;
   est->rdev = st.st_rdev;
   est->size = st.st_size;
#ifdef _WIN32
   est->blksize = 0;
   est->blocks = 0;
#else
   est->blksize = st.st_blksize;
   est->blocks = st.st_blocks;
#endif
   est->atime = st.st_atime;
   est->mtime = st.st_mtime;
   est->ctime = st.st_ctime;
#ifdef _STAT_VER_LINUX
# if (defined __USE_MISC && defined st_mtime)
   est->atimensec = st.st_atim.tv_nsec;
   est->mtimensec = st.st_mtim.tv_nsec;
   est->ctimensec = st.st_ctim.tv_nsec;
# else
   est->atimensec = st.st_atimensec;
   est->mtimensec = st.st_mtimensec;
   est->ctimensec = st.st_ctimensec;
# endif
#else
   est->atimensec = 0;
   est->mtimensec = 0;
   est->ctimensec = 0;
#endif

   if (memcmp(est, &backend->self, sizeof (Eina_Stat)) != 0)
     {
        ecore_thread_main_loop_begin();
        _eio_monitor_send(backend->parent, backend->parent->path, EIO_MONITOR_SELF_DELETED);
        ecore_thread_main_loop_end();
     }

   it = eina_file_direct_ls(backend->parent->path);
   EINA_ITERATOR_FOREACH(it, info)
     {
        Eio_Monitor_Stat *cmp;
        Eio_Monitor_Stat buffer;

        if (!backend->initialised)
          cmp = calloc(1, sizeof (Eio_Monitor_Stat));
        else
          cmp = &buffer;

        if (eina_file_statat(eina_iterator_container_get(it), info, &cmp->buffer))
          {
             if (!backend->initialised) free(cmp);
             continue ;
          }

        if (!backend->initialised)
          {
             eina_hash_add(backend->children, info->path, cmp);
          }
        else
          {
             cmp = eina_hash_find(backend->children, info->path);
             if (!cmp)
               {
                  /* New file or new directory added */
                  ecore_thread_main_loop_begin();
                  _eio_monitor_send(backend->parent, info->path,
                                    info->type != EINA_FILE_DIR ? EIO_MONITOR_FILE_CREATED : EIO_MONITOR_DIRECTORY_CREATED);
                  ecore_thread_main_loop_end();

                  cmp = malloc(sizeof (Eio_Monitor_Stat));
                  memcpy(cmp, &buffer, sizeof (Eina_Stat));

                  eina_hash_add(backend->children, info->path, cmp);
               }
             else if (memcmp(cmp, &buffer, sizeof (Eina_Stat)) != 0)
               {
                  /* file has been modified */
                  ecore_thread_main_loop_begin();
                  _eio_monitor_send(backend->parent, info->path,
                                    info->type != EINA_FILE_DIR ? EIO_MONITOR_FILE_MODIFIED : EIO_MONITOR_DIRECTORY_MODIFIED);
                  ecore_thread_main_loop_end();

                  memcpy(cmp, &buffer, sizeof (Eina_Stat));
               }
          }

        cmp->version = backend->version;
        if (ecore_thread_check(thread)) goto out;
     }
 out:
   if (it) eina_iterator_free(it);

   if (backend->initialised && !ecore_thread_check(thread))
     {
        Eina_Hash_Tuple *tuple;

        it = eina_hash_iterator_tuple_new(backend->children);
        ecore_thread_main_loop_begin();

        EINA_ITERATOR_FOREACH(it, tuple)
          {
             Eio_Monitor_Stat *cmp = tuple->data;

             if (cmp->version != backend->version)
               {
                  _eio_monitor_send(backend->parent, tuple->key,
                                    eio_file_is_dir(&cmp->buffer) ? EIO_MONITOR_DIRECTORY_DELETED : EIO_MONITOR_FILE_DELETED);
                  eina_hash_del(backend->children, tuple->key, tuple->data);
               }
          }

        ecore_thread_main_loop_end();
        eina_iterator_free(it);
     }

   backend->version++;
   backend->initialised = EINA_TRUE;
}

static void
_eio_monitor_fallback_end_cb(void *data, Ecore_Thread *thread EINA_UNUSED)
{
   Eio_Monitor_Backend *backend = data;

   backend->work = NULL;
   if (backend->delete_me)
     {
        eina_hash_free(backend->children);
        free(backend);
        return;
     }
   /* indicates eio shutdown is in progress */
   if (!timer_hash) return;
   backend->timer = ecore_timer_add(fallback_interval, _eio_monitor_fallback_timer_cb, backend);
   eina_hash_set(timer_hash, &backend, backend->timer);
}

static void
_eio_monitor_fallback_cancel_cb(void *data, Ecore_Thread *thread EINA_UNUSED)
{
   Eio_Monitor_Backend *backend = data;

   backend->work = NULL;
   if (backend->delete_me)
     {
        eina_hash_free(backend->children);
        free(backend);
        return;
     }
   /* indicates eio shutdown is in progress */
   if (!timer_hash) return;
   backend->timer = ecore_timer_add(fallback_interval, _eio_monitor_fallback_timer_cb, backend);
   eina_hash_set(timer_hash, &backend, backend->timer);
}

static Eina_Bool
_eio_monitor_fallback_timer_cb(void *data)
{
   Eio_Monitor_Backend *backend = data;

   backend->timer = NULL;
   eina_hash_set(timer_hash, &backend, NULL);
   backend->work = ecore_thread_run(_eio_monitor_fallback_heavy_cb,
                                    _eio_monitor_fallback_end_cb,
                                    _eio_monitor_fallback_cancel_cb,
                                    backend);

   return EINA_FALSE;
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

#if !defined HAVE_SYS_INOTIFY_H && !defined HAVE_NOTIFY_WIN32 && !defined HAVE_NOTIFY_COCOA \
    && !defined HAVE_NOTIFY_KEVENT
void eio_monitor_backend_init(void)
{
}

void eio_monitor_backend_shutdown(void)
{
}

void eio_monitor_backend_add(Eio_Monitor *monitor)
{
  eio_monitor_fallback_add(monitor);
}

void eio_monitor_backend_del(Eio_Monitor *monitor)
{
  eio_monitor_fallback_del(monitor);
}
#endif

void
eio_monitor_fallback_init(void)
{
   timer_hash = eina_hash_pointer_new(NULL);
}

void
eio_monitor_fallback_shutdown(void)
{
   eina_hash_free(timer_hash);
   timer_hash = NULL;
}

void
eio_monitor_fallback_add(Eio_Monitor *monitor)
{
   Eio_Monitor_Backend *backend;

   monitor->backend = NULL;

   backend = calloc(1, sizeof (Eio_Monitor_Backend));
   if (!backend) return;

   backend->children = eina_hash_string_superfast_new(free);
   backend->parent = monitor;
   monitor->backend = backend;
   backend->work = ecore_thread_run(_eio_monitor_fallback_heavy_cb,
                                    _eio_monitor_fallback_end_cb,
                                    _eio_monitor_fallback_cancel_cb,
                                    backend);
}

void
eio_monitor_fallback_del(Eio_Monitor *monitor)
{
   Eio_Monitor_Backend *backend;

   backend = monitor->backend;
   monitor->backend = NULL;

   if (!backend) return;
   backend->delete_me = EINA_TRUE;


   if (backend->timer) ecore_timer_del(backend->timer);
   eina_hash_set(timer_hash, &backend, NULL);
   backend->timer = NULL;

   if (backend->work)
     {
        ecore_thread_cancel(backend->work);
        return;
     }

   backend->parent = NULL;
   eina_hash_free(backend->children);
   free(backend);
}

/**
 * @endcond
 */


/*============================================================================*
 *                                   API                                      *
 *============================================================================*/


EAPI void
eio_monitoring_interval_set(double interval)
{
   Eina_Iterator *it;
   Ecore_Timer *timer;

   EINA_SAFETY_ON_TRUE_RETURN(interval < 0.0);
   fallback_interval = interval;
   if (!timer_hash) return;
   it = eina_hash_iterator_data_new(timer_hash);
   EINA_ITERATOR_FOREACH(it, timer)
     ecore_timer_interval_set(timer, fallback_interval);
   eina_iterator_free(it);
}
