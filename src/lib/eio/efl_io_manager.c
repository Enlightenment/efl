/* EIO - EFL data type library
 * Copyright (C) 2016 Enlightenment Developers:
 *           Felipe Magno de Almeida <felipe@expertisesolutions.com.br>
 *           Lauro Moura <lauromoura@expertisesolutions.com.br>
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
# include <config.h>
#endif

#include <Eo.h>
#include <Ecore.h>
#include "Eio.h"

#include "eio_private.h"

typedef struct _Efl_Io_Manager_Data Efl_Io_Manager_Data;

struct _Efl_Io_Manager_Data
{
   Eo *object;
   Eina_List *operations;
};

typedef Eio_File* (*Efl_Io_Manager_Direct_Ls_Func)(const char *path, Eio_Filter_Direct_Cb, Eio_Main_Direct_Cb, Eio_Done_Cb, Eio_Error_Cb, const void *data);

typedef struct _Job_Closure Job_Closure;
struct _Job_Closure
{
   Eo *object;
   Efl_Io_Manager_Data *pdata;
   Eina_Promise_Owner *promise;
   Eio_File *file;
   Eina_Bool delete_me;
   void *delayed_arg;
   Efl_Io_Manager_Direct_Ls_Func direct_func;  // Used when dispatching direct ls funcs.
};

/* Helper functions */

static void
_no_future(void *data, const Eo_Event *ev EINA_UNUSED)
{
   Eio_File *h = data;

   eio_file_cancel(h);
}

static void
_forced_shutdown(void *data, const Eo_Event *ev EINA_UNUSED)
{
   Eio_File *h = data;

   eio_file_cancel(h);
   // FIXME: handle memory lock here !
   // Acceptable strategy will be to unlock all thread as
   // if we were freeing some memory
   // FIXME: Handle long to finish thread
   ecore_thread_wait(h->thread, 1.0);
}

static void
_progress(void *data EINA_UNUSED, const Eo_Event *ev)
{
   eo_key_data_set(ev->object, "_eio.progress", (void*) EINA_TRUE);
}

EO_CALLBACKS_ARRAY_DEFINE(promise_progress_handling,
                          { EFL_PROMISE_EVENT_FUTURE_PROGRESS_SET, _progress },
                          { EFL_PROMISE_EVENT_FUTURE_NONE, _no_future },
                          { EO_EVENT_DEL, _forced_shutdown });

EO_CALLBACKS_ARRAY_DEFINE(promise_handling,
                          { EFL_PROMISE_EVENT_FUTURE_NONE, _no_future },
                          { EO_EVENT_DEL, _forced_shutdown });

static void
_file_done_data_cb(void *data, Eio_File *handler, const char *attr_data, unsigned int size)
{
   Efl_Promise *p = data;
   Eina_Binbuf *buf;

   eo_event_callback_array_del(p, promise_handling(), handler);

   buf = eina_binbuf_new();
   eina_binbuf_append_length(buf, (const unsigned char*) attr_data, size);

   efl_promise_value_set(p, buf, EINA_FREE_CB(eina_binbuf_free));

   eo_del(p);
}

static void
_file_error_cb(void *data, Eio_File *handler, int error)
{
   Efl_Promise *p = data;

   eo_event_callback_array_del(p, promise_handling(), handler);

   efl_promise_failed(p, error);

   eo_del(p);
}

static void
_file_done_cb(void *data, Eio_File *handler)
{
   Efl_Promise *p = data;
   uint64_t *v = calloc(1, sizeof (uint64_t));

   eo_event_callback_array_del(p, promise_handling(), handler);

   if (!v)
     {
        efl_promise_failed(p, EINA_ERROR_OUT_OF_MEMORY);
        goto end;
     }

   *v = handler->length;
   efl_promise_value_set(p, v, free);

 end:
   eo_del(p);
}

/* Basic listing callbacks */
static void
_cleanup_string_progress(void *data)
{
   Eina_Array *existing = data;
   Eina_Stringshare *s;
   Eina_Array_Iterator it;
   unsigned int i;

   EINA_ARRAY_ITER_NEXT(existing, i, s, it)
     eina_stringshare_del(s);
   eina_array_free(existing);
}

static void
_file_string_cb(void *data, Eio_File *handler, Eina_Array *gather)
{
   Efl_Promise *p = data;
   Eina_Array *existing = eo_key_data_get(p, "_eio.stored");
   void **tmp;

   // If a future is set, but without progress, we should assume
   // that we should discard all future progress. [[FIXME]]
   if (existing)
     {
        tmp = realloc(existing->data, sizeof (void*) * (existing->count + gather->count));
        if (!tmp)
          {
             eina_array_free(gather);
             eina_array_free(existing);
             eo_key_data_set(p, "_eio.stored", NULL);
             handler->error = EINA_ERROR_OUT_OF_MEMORY;
             eio_file_cancel(handler);
             return ;
          }

        existing->data = tmp;
        memcpy(existing->data + existing->count, gather->data, gather->count * sizeof (void*));
        existing->count += gather->count;
        existing->total = existing->count;
        eina_array_free(gather);
     }
   else
     {
        existing = gather;
     }

   if (!eo_key_data_get(p, "_eio.progress"))
     {
        eo_key_data_set(p, "_eio.stored", existing);
        return ;
     }

   efl_promise_progress_set(p, existing);
   eo_key_data_set(p, "_eio.stored", NULL);
   _cleanup_string_progress(existing);
}

/* Direct listing callbacks */
#if 0
static void
_file_direct_ls_main_cb(void *data, Eio_File *handler EINA_UNUSED, const Eina_File_Direct_Info *info)
{
   Job_Closure *operation = data;
   EINA_SAFETY_ON_NULL_RETURN(operation);
   EINA_SAFETY_ON_NULL_RETURN(operation->promise);

   eina_promise_owner_progress(operation->promise, (void*)info);
}

static void
_ls_direct_notify_start(void* data, Eina_Promise_Owner *promise EINA_UNUSED)
{
   Job_Closure *operation_data = (Job_Closure*)data;
   char* path = operation_data->delayed_arg;

   Eio_File *handle = operation_data->direct_func(path,
         _file_direct_ls_filter_cb,
         _file_direct_ls_main_cb,
         _file_done_cb,
         _file_error_cb,
         operation_data);
   operation_data->file = handle;
}

static void
_free_notify_start_data(void *data)
{
   Job_Closure *operation_data = (Job_Closure*)data;
   if (!operation_data->delayed_arg)
     return;
   free(operation_data->delayed_arg);
   operation_data->delayed_arg = NULL;
}

static void
_xattr_notify_start(void *data, Eina_Promise_Owner *promise EINA_UNUSED)
{
   Job_Closure *operation_data = (Job_Closure*)data;
   char* path = operation_data->delayed_arg;

   Eio_File *handle = eio_file_xattr(path,
         _file_ls_filter_xattr_cb,
         _file_ls_main_cb,
         _file_done_cb,
         _file_error_cb,
         operation_data);
   operation_data->file = handle;
}

static void
_job_direct_ls_helper(Efl_Io_Manager_Direct_Ls_Func ls_func,
      Eo* obj,
      Efl_Io_Manager_Data *pd,
      const char *path,
      Eina_Promise_Owner *promise)
{
   Job_Closure *operation_data = _job_closure_create(obj, pd, promise);

   if (!operation_data)
     {
        EINA_LOG_CRIT("Failed to create eio job operation data.");
        eina_promise_owner_error_set(promise, eina_error_get());
        eina_error_set(0);
        return;
     }

   operation_data->delayed_arg = (char*)calloc(sizeof(char), strlen(path) + 1);
   strcpy(operation_data->delayed_arg, path);

   operation_data->direct_func = ls_func;

   eina_promise_owner_progress_notify(promise,
         _ls_direct_notify_start,
         operation_data,
         _free_notify_start_data);
}
#endif

/* Method implementations */
static Efl_Future *
_efl_io_manager_direct_ls(Eo *obj,
                          Efl_Io_Manager_Data *pd,
                          const char *path,
                          Eina_Bool recursive)
{
#if 0
   Eina_Promise_Owner* promise = eina_promise_add();
   _job_direct_ls_helper(&eio_file_direct_ls, obj, pd, path, promise);
   return eina_promise_owner_promise_get(promise);
#endif
}

static Efl_Future *
_efl_io_manager_stat_ls(Eo *obj,
                        Efl_Io_Manager_Data *pd,
                        const char *path,
                        Eina_Bool recursive)
{
#if 0
   Eina_Promise_Owner* promise = eina_promise_add();
   _job_direct_ls_helper(&eio_file_stat_ls, obj, pd, path, promise);
   return eina_promise_owner_promise_get(promise);
#endif
}

#if 0
static Eina_Promise*
_efl_io_manager_dir_stat_ls(Eo *obj,
      Efl_Io_Manager_Data *pd,
      const char *path)
{
   Eina_Promise_Owner* promise = eina_promise_add();
   _job_direct_ls_helper(&eio_dir_stat_ls, obj, pd, path, promise);
   return eina_promise_owner_promise_get(promise);
}

static Eina_Promise*
_efl_io_manager_dir_direct_ls(Eo *obj,
                       Efl_Io_Manager_Data *pd,
                       const char *path)
{
   Eina_Promise_Owner* promise = eina_promise_add();
   // Had to add the cast as dir_direct differs in the filter callback constness of one of
   // its arguments.
   _job_direct_ls_helper((Efl_Io_Manager_Direct_Ls_Func)&eio_dir_direct_ls, obj, pd, path, promise);
   return eina_promise_owner_promise_get(promise);
}
#endif

static Efl_Future *
_efl_io_manager_ls(Eo *obj,
                   Efl_Io_Manager_Data *pd,
                   const char *path)
{
   Efl_Promise *p;
   Eio_File *h;

   p = eo_add(EFL_PROMISE_CLASS, obj);
   if (!p) return NULL;

   h = _eio_file_ls(path,
                    _file_string_cb,
                    _file_done_cb,
                    _file_error_cb,
                    p);
   if (!h) goto end;

   eo_event_callback_array_add(p, promise_progress_handling(), h);
   return efl_promise_future_get(p);

 end:
   eo_del(p);
   return NULL;
}

/* Stat function */
static void
_file_stat_done_cb(void *data, Eio_File *handle EINA_UNUSED, const Eina_Stat *stat)
{
   Efl_Promise *p = data;
   Eina_Stat *c;

   c = calloc(1, sizeof (Eina_Stat));
   if (!c)
     {
        efl_promise_failed(p, EINA_ERROR_OUT_OF_MEMORY);
        goto end;
     }

   memcpy(c, stat, sizeof (Eina_Stat));
   efl_promise_value_set(p, c, free);

 end:
   eo_del(p);
}

static Efl_Future *
_efl_io_manager_stat(Eo *obj,
                     Efl_Io_Manager_Data *pd,
                     const char *path)
{
   Efl_Promise *p;
   Eio_File *h;

   p = eo_add(EFL_PROMISE_CLASS, obj);
   if (!p) return NULL;

   h = eio_file_direct_stat(path,
                            _file_stat_done_cb,
                            _file_error_cb,
                            p);
   if (!h) goto end;

   eo_event_callback_array_add(p, promise_handling(), h);
   return efl_promise_future_get(p);

 end:
   eo_del(p);
   return NULL;
}

/* eXtended attribute manipulation */

static Efl_Future *
_efl_io_manager_xattr_ls(Eo *obj,
                         Efl_Io_Manager_Data *pd,
                         const char *path)
{
#if 0
   Eina_Promise_Owner *promise = eina_promise_add();
   Job_Closure *operation_data = _job_closure_create(obj, pd, promise);

   Eina_Promise* p = eina_promise_owner_promise_get(promise);
   if (!operation_data)
     {
        EINA_LOG_CRIT("Failed to create eio job operation data.");
        eina_promise_owner_error_set(promise, eina_error_get());
        eina_error_set(0);
        return p;
     }

   operation_data->delayed_arg = (char*)calloc(sizeof(char), strlen(path) + 1);
   strcpy(operation_data->delayed_arg, path);

   eina_promise_owner_progress_notify(promise,
      _xattr_notify_start,
      operation_data,
      _free_notify_start_data);
   return p;
#endif
}

static Efl_Future *
_efl_io_manager_xattr_set(Eo *obj,
                          Efl_Io_Manager_Data *pd,
                          const char *path,
                          const char *attribute,
                          Eina_Binbuf *data,
                          Eina_Xattr_Flags flags)
{
   Efl_Promise *p;
   Eio_File *h;

   p = eo_add(EFL_PROMISE_CLASS, obj);
   if (!p) return NULL;

   h = eio_file_xattr_set(path, attribute,
                          (const char *) eina_binbuf_string_get(data),
                          eina_binbuf_length_get(data),
                          flags,
                          _file_done_cb,
                          _file_error_cb,
                          p);
   if (!h) goto end;

   eo_event_callback_array_add(p, promise_handling(), h);
   return efl_promise_future_get(p);

 end:
   eo_del(p);
   return NULL;
}

static Efl_Future *
_efl_io_manager_xattr_get(Eo *obj,
                          Efl_Io_Manager_Data *pd,
                          const char *path,
                          const char *attribute)
{
   Efl_Promise *p;
   Eio_File *h;

   p = eo_add(EFL_PROMISE_CLASS, obj);
   if (!p) return NULL;

   h = eio_file_xattr_get(path, attribute,
                          _file_done_data_cb,
                          _file_error_cb,
                          p);
   if (!h) goto end;

   eo_event_callback_array_add(p, promise_handling(), h);
   return efl_promise_future_get(p);

 end:
   eo_del(p);
   return NULL;
}

static void
_file_open_cb(void *data, Eio_File *handler, Eina_File *file)
{
   Efl_Promise *p = data;

   eo_event_callback_array_del(p, promise_handling(), handler);

   efl_promise_value_set(p, eina_file_dup(file), EINA_FREE_CB(eina_file_close));

   eo_del(p);
}

static Efl_Future *
_efl_io_manager_open(Eo *obj,
                     Efl_Io_Manager_Data *pd,
                     const char *path,
                     Eina_Bool shared)
{
   Efl_Promise *p;
   Eio_File *h;

   p = eo_add(EFL_PROMISE_CLASS, obj);
   if (!p) return NULL;

   h = eio_file_open(path, shared,
                     _file_open_cb,
                     _file_error_cb,
                     p);

   if (!h) goto end;

   eo_event_callback_array_add(p, promise_handling(), h);
   return efl_promise_future_get(p);

 end:
   eo_del(p);
   return NULL;
}

static Efl_Future *
_efl_io_manager_close(Eo *obj,
                      Efl_Io_Manager_Data *pd,
                      Eina_File *file)
{
   Efl_Promise *p;
   Eio_File *h;

   p = eo_add(EFL_PROMISE_CLASS, obj);
   if (!p) return NULL;

   h = eio_file_close(file,
                      _file_done_cb,
                      _file_error_cb,
                      p);
   if (!h) goto end;

   eo_event_callback_array_add(p, promise_handling(), h);
   return efl_promise_future_get(p);

 end:
   eo_del(p);
   return NULL;
}

#include "efl_io_manager.eo.c"
