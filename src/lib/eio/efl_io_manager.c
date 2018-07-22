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
   Eio_File *file;
   Eina_Bool delete_me;
   void *delayed_arg;
   Efl_Io_Manager_Direct_Ls_Func direct_func;  // Used when dispatching direct ls funcs.
};

/* Helper functions */
static void
_future_file_done_cb(void *data, Eio_File *handler)
{
   Eina_Promise *p = data;

   eina_promise_resolve(p, eina_value_uint64_init(handler->length));
}

static void
_future_file_error_cb(void *data,
                      Eio_File *handler EINA_UNUSED,
                      int error)
{
   Eina_Promise *p = data;

   // error == 0 -> promise was cancelled, no need to reject it anymore
   if (error != 0) eina_promise_reject(p, error);
}

/* Basic listing callbacks */
static void
_future_string_cb(void *data EINA_UNUSED, Eio_File *handler, Eina_Array *gather)
{
   EflIoPath paths = ecore_thread_local_data_find(handler->thread, ".paths");
   void *paths_data = ecore_thread_local_data_find(handler->thread, ".paths_data");
   Eina_Stringshare *s;

   if (!paths) goto end;

   paths(paths_data, gather);

   // Cleanup strings, accessor and array
 end:
   while ((s = eina_array_pop(gather)))
     eina_stringshare_del(s);
   eina_array_free(gather);
}

/* Direct listing callbacks */
static void
_future_file_info_cb(void *data EINA_UNUSED, Eio_File *handler, Eina_Array *gather)
{
   EflIoDirectInfo info = ecore_thread_local_data_find(handler->thread, ".info");
   void *info_data = ecore_thread_local_data_find(handler->thread, ".info_data");
   Eio_File_Direct_Info *d;

   if (!info) goto end;
   if (ecore_thread_check(handler->thread)) goto end;

   info(info_data, gather);

 end:
   while ((d = eina_array_pop(gather)))
     eio_direct_info_free(d);
   eina_array_free(gather);
}

/* Method implementations */
static Eina_Future *
_efl_io_manager_direct_ls(const Eo *obj,
                          Efl_Io_Manager_Data *pd EINA_UNUSED,
                          const char *path,
                          Eina_Bool recursive,
                          void *info_data, EflIoDirectInfo info, Eina_Free_Cb info_free_cb)
{
   Eina_Promise *p;
   Eina_Future *future;
   Eio_File *h;

   p = efl_loop_promise_new(obj, _efl_io_manager_future_cancel, NULL);
   if (!p) return NULL;
   future = eina_future_new(p);

   if (!recursive)
     {
        h = _eio_file_direct_ls(path,
                                _future_file_info_cb,
                                _future_file_done_cb,
                                _future_file_error_cb,
                                p);
     }
   else
     {
        h = _eio_dir_direct_ls(path,
                               _future_file_info_cb,
                               _future_file_done_cb,
                               _future_file_error_cb,
                               p);
     }
   if (!h) goto end;

   ecore_thread_local_data_add(h->thread, ".info", info, NULL, EINA_TRUE);
   ecore_thread_local_data_add(h->thread, ".info_data", info_data, info_free_cb, EINA_TRUE);
   eina_promise_data_set(p, h);

   return efl_future_Eina_FutureXXX_then(obj, future);

 end:
   return future;
}

static Eina_Future *
_efl_io_manager_stat_ls(const Eo *obj,
                        Efl_Io_Manager_Data *pd EINA_UNUSED,
                        const char *path,
                        Eina_Bool recursive,
                        void *info_data, EflIoDirectInfo info, Eina_Free_Cb info_free_cb)
{
   Eina_Promise *p;
   Eina_Future *future;
   Eio_File *h;

   p = efl_loop_promise_new(obj, _efl_io_manager_future_cancel, NULL);
   if (!p) return NULL;
   future = eina_future_new(p);

   if (!recursive)
     {
        h = _eio_file_stat_ls(path,
                              _future_file_info_cb,
                              _future_file_done_cb,
                              _future_file_error_cb,
                              p);
     }
   else
     {
        h = _eio_dir_stat_ls(path,
                             _future_file_info_cb,
                             _future_file_done_cb,
                             _future_file_error_cb,
                             p);
     }
   if (!h) goto end;

   ecore_thread_local_data_add(h->thread, ".info", info, NULL, EINA_TRUE);
   ecore_thread_local_data_add(h->thread, ".info_data", info_data, info_free_cb, EINA_TRUE);
   eina_promise_data_set(p, h);

   return efl_future_Eina_FutureXXX_then(obj, future);

 end:
   return future;
}

static Eina_Future *
_efl_io_manager_ls(const Eo *obj,
                   Efl_Io_Manager_Data *pd EINA_UNUSED,
                   const char *path,
                   void *paths_data, EflIoPath paths, Eina_Free_Cb paths_free_cb)
{
   Eina_Promise *p;
   Eina_Future *future;
   Eio_File *h;

   p = efl_loop_promise_new(obj, _efl_io_manager_future_cancel, NULL);
   if (!p) return NULL;
   future = eina_future_new(p);

   h = _eio_file_ls(path,
                    _future_string_cb,
                    _future_file_done_cb,
                    _future_file_error_cb,
                    p);
   if (!h) goto end;

   ecore_thread_local_data_add(h->thread, ".paths", paths, NULL, EINA_TRUE);
   ecore_thread_local_data_add(h->thread, ".paths_data", paths_data, paths_free_cb, EINA_TRUE);
   eina_promise_data_set(p, h);

   return efl_future_Eina_FutureXXX_then(obj, future);

 end:
   return future;
}

/* Stat function */
static void
_file_stat_done_cb(void *data, Eio_File *handle EINA_UNUSED, const Eina_Stat *st)
{
   const Eina_Value_Struct value = { _eina_stat_desc(), (void*) st };
   Eina_Promise *p = data;
   Eina_Value r = EINA_VALUE_EMPTY;

   if (!eina_value_setup(&r, EINA_VALUE_TYPE_STRUCT))
     goto on_error;
   if (!eina_value_pset(&r, &value))
     goto on_error;

   eina_promise_resolve(p, r);

   return ;

 on_error:
   eina_value_flush(&r);
   eina_promise_reject(p, eina_error_get());
}

static Eina_Future *
_efl_io_manager_stat(const Eo *obj,
                     Efl_Io_Manager_Data *pd EINA_UNUSED,
                     const char *path)
{
   Eina_Promise *p;
   Eina_Future *future;
   Eio_File *h;

   p = efl_loop_promise_new(obj, _efl_io_manager_future_cancel, NULL);
   if (!p) return NULL;
   future = eina_future_new(p);

   h = eio_file_direct_stat(path,
                            _file_stat_done_cb,
                            _future_file_error_cb,
                            p);
   if (!h) goto end;
   eina_promise_data_set(p, h);

   return efl_future_Eina_FutureXXX_then(obj, future);

 end:
   return future;
}

/* eXtended attribute manipulation */

static Eina_Future *
_efl_io_manager_xattr_ls(const Eo *obj,
                         Efl_Io_Manager_Data *pd EINA_UNUSED,
                         const char *path,
                         void *paths_data, EflIoPath paths, Eina_Free_Cb paths_free_cb)
{
   Eina_Promise *p;
   Eina_Future *future;
   Eio_File *h;

   p = efl_loop_promise_new(obj, _efl_io_manager_future_cancel, NULL);
   if (!p) return NULL;
   future = eina_future_new(p);

   h = _eio_file_xattr(path,
                       _future_string_cb,
                       _future_file_done_cb,
                       _future_file_error_cb,
                       p);
   if (!h) goto end;

   // There is no race condition here as all the callback are called in the main loop after this
   ecore_thread_local_data_add(h->thread, ".paths", paths, NULL, EINA_TRUE);
   ecore_thread_local_data_add(h->thread, ".paths_data", paths_data, paths_free_cb, EINA_TRUE);
   eina_promise_data_set(p, h);

   return efl_future_Eina_FutureXXX_then(obj, future);

 end:
   return efl_future_Eina_FutureXXX_then(obj, future);;
}

static void
_future_file_done_data_cb(void *data, Eio_File *handler EINA_UNUSED, const char *attr_data, unsigned int size)
{
   Eina_Promise *p = data;
   Eina_Value_Blob blob = { EINA_VALUE_BLOB_OPERATIONS_MALLOC, NULL, size };
   Eina_Value v = EINA_VALUE_EMPTY;
   char *tmp;

   tmp = malloc(size);
   memcpy(tmp, attr_data, size);
   blob.memory = tmp;

   eina_value_setup(&v, EINA_VALUE_TYPE_BLOB);
   eina_value_set(&v, &blob);
   eina_promise_resolve(p, v);
}

static Eina_Future *
_efl_io_manager_xattr_set(Eo *obj,
                          Efl_Io_Manager_Data *pd EINA_UNUSED,
                          const char *path,
                          const char *attribute,
                          Eina_Binbuf *data,
                          Eina_Xattr_Flags flags)
{
   Eina_Promise *p;
   Eina_Future *future;
   Eio_File *h;

   p = efl_loop_promise_new(obj, _efl_io_manager_future_cancel, NULL);
   if (!p) return NULL;
   future = eina_future_new(p);

   h = eio_file_xattr_set(path, attribute,
                          (const char *) eina_binbuf_string_get(data),
                          eina_binbuf_length_get(data),
                          flags,
                          _future_file_done_cb,
                          _future_file_error_cb,
                          p);
   if (!h) goto end;
   eina_promise_data_set(p, h);

   return efl_future_Eina_FutureXXX_then(obj, future);

 end:
   return future;
}

static Eina_Future *
_efl_io_manager_xattr_get(const Eo *obj,
                          Efl_Io_Manager_Data *pd EINA_UNUSED,
                          const char *path,
                          const char *attribute)
{
   Eina_Promise *p;
   Eina_Future *future;
   Eio_File *h;

   p = efl_loop_promise_new(obj, _efl_io_manager_future_cancel, NULL);
   if (!p) return NULL;
   future = eina_future_new(p);

   h = eio_file_xattr_get(path, attribute,
                          _future_file_done_data_cb,
                          _future_file_error_cb,
                          p);
   if (!h) goto end;
   eina_promise_data_set(p, h);

   /* XXX const */
   return efl_future_Eina_FutureXXX_then((Eo *)obj, future);

 end:
   return future;
}

static void
_future_file_open_cb(void *data, Eio_File *handler EINA_UNUSED, Eina_File *file)
{
   Eina_Promise *p = data;
   Eina_Value v = EINA_VALUE_EMPTY;

   eina_value_setup(&v, EINA_VALUE_TYPE_FILE);
   eina_value_set(&v, file);
   eina_promise_resolve(p, v);
}

static Eina_Future *
_efl_io_manager_open(const Eo *obj,
                     Efl_Io_Manager_Data *pd EINA_UNUSED,
                     const char *path,
                     Eina_Bool shared)
{
   Eina_Promise *p;
   Eina_Future *future;
   Eio_File *h;

   p = efl_loop_promise_new(obj, _efl_io_manager_future_cancel, NULL);
   if (!p) return NULL;
   future = eina_future_new(p);

   h = eio_file_open(path, shared,
                     _future_file_open_cb,
                     _future_file_error_cb,
                     p);
   if (!h) goto end;
   eina_promise_data_set(p, h);

   return efl_future_Eina_FutureXXX_then(obj, future);

 end:
   return future;
}

static Eina_Future *
_efl_io_manager_close(const Eo *obj,
                      Efl_Io_Manager_Data *pd EINA_UNUSED,
                      Eina_File *file)
{
   Eina_Promise *p;
   Eina_Future *future;
   Eio_File *h;

   p = efl_loop_promise_new(obj, _efl_io_manager_future_cancel, NULL);
   if (!p) return NULL;
   future = eina_future_new(p);

   h = eio_file_close(file,
                      _future_file_done_cb,
                      _future_file_error_cb,
                      p);
   if (!h) goto end;
   eina_promise_data_set(p, h);

   return efl_future_Eina_FutureXXX_then(obj, future);

 end:
   return future;
}

#include "efl_io_manager.eo.c"
