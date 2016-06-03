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
#include "Eio.h"
#include "eio_job_private.h"

typedef Eio_File* (*Eio_Job_Direct_Ls_Func)(const char *path, Eio_Filter_Direct_Cb, Eio_Main_Direct_Cb, Eio_Done_Cb, Eio_Error_Cb, const void *data);

typedef struct _Job_Closure Job_Closure;
struct _Job_Closure
{
   Eo *object;
   Eio_Job_Data *pdata;
   Eina_Promise_Owner *promise;
   Eio_File *file;
   Eina_Bool delete_me;
   void *delayed_arg;
   Eio_Job_Direct_Ls_Func direct_func;  // Used when dispatching direct ls funcs.
};

/* Helper functions */

static Job_Closure *
_job_closure_create(Eo *obj, Eio_Job_Data *pdata, Eina_Promise_Owner *owner)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(obj, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(pdata, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(owner, NULL);

   Job_Closure *closure = malloc(sizeof(Job_Closure));

   if (!closure)
     {
        EINA_LOG_CRIT("Failed to allocate memory.");
        return 0;
     }

   closure->object = eo_ref(obj);
   closure->pdata = pdata;
   closure->promise = owner;
   closure->file = NULL; // Will be set once the Eio operation is under way
   closure->delete_me = EINA_FALSE;
   closure->delayed_arg = NULL;
   closure->direct_func = NULL;

   pdata->operations = eina_list_prepend(pdata->operations, closure);

   return closure;
}

static void
_job_closure_del(Job_Closure *closure)
{
   EINA_SAFETY_ON_NULL_RETURN(closure);
   Eio_Job_Data *pdata = closure->pdata;
   if (pdata)
     pdata->operations = eina_list_remove(pdata->operations, closure);

   eo_unref(closure->object);

   if (closure->delayed_arg)
     free(closure->delayed_arg);

   free(closure);
}

static void
_file_error_cb(void *data, Eio_File *handler EINA_UNUSED, int error)
{
   Job_Closure *operation = data;

   EINA_SAFETY_ON_NULL_RETURN(operation);
   EINA_SAFETY_ON_NULL_RETURN(operation->promise);

   eina_promise_owner_error_set(operation->promise, error);

   _job_closure_del(operation);
}

/* Basic listing callbacks */

static Eina_Bool
_file_ls_filter_cb_helper(const Eo_Event_Description *event, void *data, const char *file)
{
   Job_Closure *operation = data;

   EINA_SAFETY_ON_NULL_RETURN_VAL(operation, EINA_FALSE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(operation->pdata, EINA_FALSE);

   Eio_Filter_Name_Data* event_info = malloc(sizeof(Eio_Filter_Name_Data));

   EINA_SAFETY_ON_NULL_RETURN_VAL(event_info, EINA_FALSE);

   event_info->file = file;
   event_info->filter = EINA_FALSE;

   eo_event_callback_call(operation->pdata->object, event, event_info);

   Eina_Bool filter = event_info->filter;

   free(event_info);

   return filter;
}

static Eina_Bool
_file_ls_filter_xattr_cb(void *data, Eio_File *handler EINA_UNUSED, const char *file)
{
   return _file_ls_filter_cb_helper(EIO_JOB_EVENT_XATTR, data, file);
}

static Eina_Bool
_file_ls_filter_named_cb(void *data, Eio_File *handler EINA_UNUSED, const char *file)
{
   return _file_ls_filter_cb_helper(EIO_JOB_EVENT_FILTER_NAME, data, file);
}

static void
_file_ls_main_cb(void *data, Eio_File *handler EINA_UNUSED, const char *file)
{
   Job_Closure *operation = data;
   EINA_SAFETY_ON_NULL_RETURN(operation);
   EINA_SAFETY_ON_NULL_RETURN(operation->promise);

   eina_promise_owner_progress(operation->promise, (void*)file);
}

static void
_file_done_cb(void *data, Eio_File *handler EINA_UNUSED)
{
   Job_Closure *operation = data;

   EINA_SAFETY_ON_NULL_RETURN(operation);
   EINA_SAFETY_ON_NULL_RETURN(operation->promise);

   // Placeholder value. We just want the callback to be called.
   eina_promise_owner_value_set(operation->promise, NULL, NULL);

   _job_closure_del(operation);
}

static void
_free_xattr_data(Eio_Xattr_Data *value)
{
    EINA_SAFETY_ON_NULL_RETURN(value);
    if (value->data)
      free((void*)value->data);
}

static void
_file_done_data_cb(void *data, Eio_File *handler EINA_UNUSED, const char *attr_data, unsigned int size)
{
   Job_Closure *operation = data;
   Eio_Xattr_Data *ret_data = NULL;

   EINA_SAFETY_ON_NULL_RETURN(operation);
   EINA_SAFETY_ON_NULL_RETURN(operation->promise);

   ret_data = eina_promise_owner_buffer_get(operation->promise);

   ret_data->data = calloc(sizeof(char), size + 1);
   strcpy((char*)ret_data->data, attr_data);
   ret_data->size = size;

   eina_promise_owner_value_set(operation->promise, NULL, (Eina_Promise_Free_Cb)&_free_xattr_data);

   _job_closure_del(operation);
}

/* Direct listing callbacks */

static Eina_Bool
_file_direct_ls_filter_cb(void *data, Eio_File *handle EINA_UNUSED, const Eina_File_Direct_Info *info)
{
   Job_Closure *operation = data;

   EINA_SAFETY_ON_NULL_RETURN_VAL(operation, EINA_FALSE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(operation->pdata, EINA_FALSE);

   Eio_Filter_Direct_Data event_info;

   event_info.info = info;
   event_info.filter = EINA_FALSE;

   eo_event_callback_call(operation->pdata->object, EIO_JOB_EVENT_FILTER_DIRECT, &event_info);

   Eina_Bool filter = event_info.filter;

   return filter;
}

static void
_file_direct_ls_main_cb(void *data, Eio_File *handler EINA_UNUSED, const Eina_File_Direct_Info *info)
{
   Job_Closure *operation = data;
   EINA_SAFETY_ON_NULL_RETURN(operation);
   EINA_SAFETY_ON_NULL_RETURN(operation->promise);

   eina_promise_owner_progress(operation->promise, (void*)info);
}

static void
_ls_direct_notify_start(void* data, Eina_Promise_Owner *promise)
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

   promise->progress_notify = NULL;
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
_ls_notify_start(void *data, Eina_Promise_Owner* promise)
{
   Job_Closure *operation_data = (Job_Closure*)data;
   char* path = operation_data->delayed_arg;

   Eio_File *handle = eio_file_ls(path,
         _file_ls_filter_named_cb,
         _file_ls_main_cb,
         _file_done_cb,
         _file_error_cb,
         operation_data);
   operation_data->file = handle;

   promise->progress_notify = NULL; // Don't ever think about calling me again...
}

static void
_xattr_notify_start(void *data, Eina_Promise_Owner *promise)
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

   promise->progress_notify = NULL; // Don't ever think about calling me again...
}

static void
_job_direct_ls_helper(Eio_Job_Direct_Ls_Func ls_func,
      Eo* obj,
      Eio_Job_Data *pd,
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

/* Method implementations */

static Eina_Promise*
_eio_job_file_direct_ls(Eo *obj,
      Eio_Job_Data *pd,
      const char *path)
{
   Eina_Promise_Owner* promise = eina_promise_value_add(sizeof(int));
   _job_direct_ls_helper(&eio_file_direct_ls, obj, pd, path, promise);
   return eina_promise_owner_promise_get(promise);
}

static Eina_Promise*
_eio_job_file_stat_ls(Eo *obj,
      Eio_Job_Data *pd,
      const char *path)
{
   Eina_Promise_Owner* promise = eina_promise_value_add(sizeof(int));
   _job_direct_ls_helper(&eio_file_stat_ls, obj, pd, path, promise);
   return eina_promise_owner_promise_get(promise);
}

static Eina_Promise*
_eio_job_dir_stat_ls(Eo *obj,
      Eio_Job_Data *pd,
      const char *path)
{
   Eina_Promise_Owner* promise = eina_promise_value_add(sizeof(int));
   _job_direct_ls_helper(&eio_dir_stat_ls, obj, pd, path, promise);
   return eina_promise_owner_promise_get(promise);
}

static Eina_Promise*
_eio_job_dir_direct_ls(Eo *obj,
                       Eio_Job_Data *pd,
                       const char *path)
{
   Eina_Promise_Owner* promise = eina_promise_value_add(sizeof(int));
   // Had to add the cast as dir_direct differs in the filter callback constness of one of
   // its arguments.
   _job_direct_ls_helper((Eio_Job_Direct_Ls_Func)&eio_dir_direct_ls, obj, pd, path, promise);
   return eina_promise_owner_promise_get(promise);
}

static Eina_Promise*
_eio_job_file_ls(Eo *obj,
      Eio_Job_Data *pd,
      const char *path)
{
   Eina_Promise_Owner* promise = eina_promise_value_add(sizeof(int));
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
      _ls_notify_start,
      operation_data,
      _free_notify_start_data);
   return p;
}

/* Stat function */

static void
_file_stat_done_cb(void *data, Eio_File *handle EINA_UNUSED, const Eina_Stat *stat)
{
   Job_Closure *operation = data;

   EINA_SAFETY_ON_NULL_RETURN(operation);
   EINA_SAFETY_ON_NULL_RETURN(operation->promise);

   // Placeholder value. We just want the callback to be called.
   eina_promise_owner_value_set(operation->promise, stat, NULL);

   _job_closure_del(operation);
}

static Eina_Promise*
_eio_job_file_direct_stat(Eo *obj,
                          Eio_Job_Data *pd,
                          const char *path)
{
   Eina_Promise_Owner* promise = eina_promise_value_add(sizeof(Eina_Stat));
   Job_Closure *operation_data = _job_closure_create(obj, pd, promise);

   Eina_Promise* p = eina_promise_owner_promise_get(promise);
   if (!operation_data)
     {
        EINA_LOG_CRIT("Failed to create eio job operation data.");
        eina_promise_owner_error_set(promise, eina_error_get());
        eina_error_set(0);
        return p;
     }

   Eio_File *handle = eio_file_direct_stat(path,
         _file_stat_done_cb,
         _file_error_cb,
         operation_data);
   operation_data->file = handle;
   return p;
}

/* eXtended attribute manipulation */

static Eina_Promise*
_eio_job_file_xattr_list_get(Eo *obj,
                    Eio_Job_Data *pd,
                    const char *path)
{
   Eina_Promise_Owner *promise = eina_promise_value_add(sizeof(int));
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
}

static Eina_Promise*
_eio_job_file_xattr_set(Eo *obj,
                        Eio_Job_Data *pd,
                        const char *path,
                        const char *attribute,
                        const char *xattr_data,
                        unsigned int xattr_size,
                        Eina_Xattr_Flags flags)
{
   Eina_Promise_Owner* promise = eina_promise_value_add(sizeof(int));
   Job_Closure *operation_data = _job_closure_create(obj, pd, promise);

   Eina_Promise* p = eina_promise_owner_promise_get(promise);

   if (!operation_data)
     {
        EINA_LOG_CRIT("Failed to create eio job operation data.");
        eina_promise_owner_error_set(promise, eina_error_get());
        eina_error_set(0);
        return p;
     }

   Eio_File *handle = eio_file_xattr_set(path,
         attribute,
         xattr_data,
         xattr_size,
         flags,
         _file_done_cb,
         _file_error_cb,
         operation_data);
   operation_data->file = handle;
   return p;
}

static Eina_Promise*
_eio_job_file_xattr_get(Eo *obj,
                        Eio_Job_Data *pd,
                        const char *path,
                        const char *attribute)
{
   Eina_Promise_Owner* promise = eina_promise_value_add(sizeof(Eio_Xattr_Data));
   Job_Closure *operation_data = _job_closure_create(obj, pd, promise);

   Eina_Promise* p = eina_promise_owner_promise_get(promise);
   if (!operation_data)
     {
        EINA_LOG_CRIT("Failed to create eio job operation data.");
        eina_promise_owner_error_set(promise, eina_error_get());
        eina_error_set(0);
        return p;
     }

   Eio_File *handle = eio_file_xattr_get(path,
         attribute,
         _file_done_data_cb,
         _file_error_cb,
         operation_data);
   operation_data->file = handle;
   return p;
}

static void
_file_open_open_cb(void *data, Eio_File *handler EINA_UNUSED, Eina_File *file)
{
   Job_Closure *operation = data;
   EINA_SAFETY_ON_NULL_RETURN(operation);
   EINA_SAFETY_ON_NULL_RETURN(operation->promise);
   eina_promise_owner_value_set(operation->promise, file, (Eina_Promise_Free_Cb)&eina_file_close);

   _job_closure_del(operation);
}

static Eina_Promise*
_eio_job_file_open(Eo *obj,
                   Eio_Job_Data *pd,
                   const char *path,
                   Eina_Bool shared)
{
   Eina_Promise_Owner* promise = eina_promise_add();
   Job_Closure *operation_data = _job_closure_create(obj, pd, promise);

   Eina_Promise* p = eina_promise_owner_promise_get(promise);
   if (!operation_data)
     {
        EINA_LOG_CRIT("Failed to create eio job operation data.");
        eina_promise_owner_error_set(promise, eina_error_get());
        eina_error_set(0);
        return p;
     }

   Eio_File *handle = eio_file_open(path, shared, _file_open_open_cb, _file_error_cb, operation_data);
   operation_data->file = handle;
   return p;
}


static void
_file_close_done_cb(void *data, Eio_File *handler EINA_UNUSED)
{
   EINA_SAFETY_ON_NULL_RETURN(data);
   Job_Closure *operation = data;
   eina_promise_owner_value_set(operation->promise, NULL, NULL);

   _job_closure_del(operation);
}

static void
_eio_job_file_close(Eo *obj,
                         Eio_Job_Data *pd,
                         Eina_File *file,
                         Eina_Promise_Owner *promise)
{
   Job_Closure *operation_data = _job_closure_create(obj, pd, promise);

   if (!operation_data)
     {
        EINA_LOG_CRIT("Failed to create eio job operation data.");
        return;
     }

   Eio_File *handle = eio_file_close(file, _file_close_done_cb, _file_error_cb, operation_data);
   operation_data->file = handle;
}

static Eo_Base*
_eio_job_eo_base_constructor(Eo *obj, Eio_Job_Data *pd EINA_UNUSED)
{
   obj = eo_constructor(eo_super(obj, EIO_JOB_CLASS));

   pd->object = obj;
   pd->operations = NULL;

   return obj;
}

static void
_eio_job_eo_base_destructor(Eo *obj, Eio_Job_Data *pd EINA_UNUSED)
{
   eo_destructor(eo_super(obj, EIO_JOB_CLASS));
}

#include "eio_job.eo.c"
