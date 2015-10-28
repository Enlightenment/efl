#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <Eo.h>
#include <Ecordova.h>

#include "ecordova_entry_private.h"
#include "ecordova_directoryentry_private.h"

#ifdef EAPI
# undef EAPI
#endif

#ifdef _WIN32
# ifdef DLL_EXPORT
#  define EAPI __declspec(dllexport)
# else
#  define EAPI
# endif /* ! DLL_EXPORT */
#else
# ifdef __GNUC__
#  if __GNUC__ >= 4
#   define EAPI __attribute__ ((visibility("default")))
#  else
#   define EAPI
#  endif
# else
#  define EAPI
# endif
#endif /* ! _WIN32 */

/* logging support */
extern int _ecordova_log_dom;

#define CRI(...) EINA_LOG_DOM_CRIT(_ecordova_log_dom, __VA_ARGS__)
#define ERR(...) EINA_LOG_DOM_ERR(_ecordova_log_dom, __VA_ARGS__)
#define WRN(...) EINA_LOG_DOM_WARN(_ecordova_log_dom, __VA_ARGS__)
#define INF(...) EINA_LOG_DOM_INFO(_ecordova_log_dom, __VA_ARGS__)
#define DBG(...) EINA_LOG_DOM_DBG(_ecordova_log_dom, __VA_ARGS__)

#define MY_CLASS ECORDOVA_ENTRY_CLASS
#define MY_CLASS_NAME "Ecordova_Entry"

static void _eio_stat_cb(void *, Eio_File *, const Eina_Stat *);
static void _eio_moved_cb(void *, Eio_File *);
static void _eio_copied_cb(void *, Eio_File *);
static void _metadata_notify(Eo *, void *);
static void _move_notify(Eo *, void *);
static void _copy_notify(Eo *, void *);
static void _remove_notify(Eo *, void *);
static Eina_Bool _eio_remove_non_recursively_filter_cb(void *, Eio_File *, const Eina_File_Direct_Info *);
static void _parent_get_cb(void *, Eio_File *, const Eina_Stat *);
static void _parent_get_notify(Eo *, void *);

static Eo_Base *
_ecordova_entry_eo_base_constructor(Eo *obj, Ecordova_Entry_Data *pd)
{
   DBG("(%p)", obj);

   pd->obj = obj;
   pd->name = NULL;
   pd->path = NULL;
   pd->file_system = NULL;
   pd->native = NULL;
   pd->pending = NULL;

   return eo_do_super_ret(obj, MY_CLASS, obj, eo_constructor());
}

static void
_ecordova_entry_constructor(Eo *obj,
                            Ecordova_Entry_Data *pd,
                            Eina_Bool file_is,
                            Eina_Bool directory_is,
                            const char *name,
                            const char *path,
                            Ecordova_FileSystem *file_system,
                            const char *url)
{
   EINA_SAFETY_ON_NULL_RETURN(name);
   EINA_SAFETY_ON_NULL_RETURN(path);
   //EINA_SAFETY_ON_NULL_RETURN(file_system);
   EINA_SAFETY_ON_NULL_RETURN(url);
   DBG("(%p) name=%s, path=%s, url=%s", obj, name, path, url);

   pd->is_file = file_is;
   pd->is_directory = directory_is;
   pd->name = strdup(name);
   pd->path = strdup(path);
   pd->file_system = eo_ref(file_system);
   pd->native = strdup(url);
}

static void
_ecordova_entry_eo_base_destructor(Eo *obj, Ecordova_Entry_Data *pd)
{
   DBG("(%p)", obj);

   free(pd->name);
   free(pd->path);
   eo_unref(pd->file_system);
   free(pd->native);

   Eio_Operation_Data *data;
   EINA_LIST_FREE(pd->pending, data)
     {
        eio_file_cancel(data->file);
        _data_free(data);
     }

   eo_do_super(obj, MY_CLASS, eo_destructor());
}

static void
_ecordova_entry_metadata_get(Eo *obj EINA_UNUSED, Ecordova_Entry_Data *pd)
{
   DBG("(%p)", obj);
   _metadata_get(pd, _metadata_notify, _error_notify);
}

static void
_ecordova_entry_metadata_set(Eo *obj EINA_UNUSED,
                             Ecordova_Entry_Data *pd EINA_UNUSED,
                             Ecordova_Metadata *metadata EINA_UNUSED)
{
   ERR("Not implemented.");
}

static void
_ecordova_entry_move(Eo *obj EINA_UNUSED,
                     Ecordova_Entry_Data *pd,
                     Ecordova_DirectoryEntry *parent,
                     const char *new_name)
{
   DBG("(%p)", obj);
   EINA_SAFETY_ON_NULL_RETURN(parent);
   if (!new_name) new_name = pd->name;

   Eio_Operation_Data *data = _data_new(pd, _move_notify, _error_notify);

   Ecordova_Entry_Data *dest_dir = eo_data_scope_get(parent, ECORDOVA_ENTRY_CLASS);

   data->name = strdup(new_name);
   data->path = strdup(dest_dir->native);

   // TODO: file_system?

   size_t len = strlen(data->path) + 1 + strlen(data->name) + 1;
   data->native = malloc(len);
   EINA_SAFETY_ON_NULL_GOTO(data->native, on_error);
   snprintf(data->native, len, "%s/%s", data->path, new_name);

   if (pd->is_file)
     {
        DBG("Moving file from %s to %s", pd->native, data->native);
        data->file = eio_file_move(pd->native,
                                   data->native,
                                   _eio_progress_cb,
                                   _eio_moved_cb,
                                   _eio_error_cb,
                                   data);
     }
   else
     {
        DBG("Moving directory from %s to %s", pd->native, data->native);
        data->file = eio_dir_move(pd->native,
                                  data->native,
                                  _eio_filter_cb,
                                  _eio_progress_cb,
                                  _eio_moved_cb,
                                  _eio_error_cb,
                                  data);
     }

   pd->pending = eina_list_append(pd->pending, data);
   return;

on_error:
   _data_free(data);
}

static void
_ecordova_entry_copy(Eo *obj EINA_UNUSED,
                     Ecordova_Entry_Data *pd,
                     Ecordova_DirectoryEntry *parent,
                     const char *new_name)
{
   DBG("(%p)", obj);
   EINA_SAFETY_ON_NULL_RETURN(parent);
   if (!new_name) new_name = pd->name;

   Eio_Operation_Data *data = _data_new(pd, _copy_notify, _error_notify);

   Ecordova_Entry_Data *dest_dir = eo_data_scope_get(parent, ECORDOVA_ENTRY_CLASS);

   data->name = strdup(new_name);
   data->path = strdup(dest_dir->native);

   // TODO: file_system?

   size_t len = strlen(data->path) + 1 + strlen(data->name) + 1;
   data->native = malloc(len);
   EINA_SAFETY_ON_NULL_GOTO(data->native, on_error);
   snprintf(data->native, len, "%s/%s", data->path, new_name);

   if (pd->is_file)
     data->file = eio_file_copy(pd->native,
                                data->native,
                                _eio_progress_cb,
                                _eio_copied_cb,
                                _eio_error_cb,
                                data);
   else
     data->file = eio_dir_copy(pd->native,
                               data->native,
                               _eio_filter_cb,
                               _eio_progress_cb,
                               _eio_copied_cb,
                               _eio_error_cb,
                               data);

   pd->pending = eina_list_append(pd->pending, data);
   return;

on_error:
   _data_free(data);
}

static void
_ecordova_entry_remove(Eo *obj EINA_UNUSED, Ecordova_Entry_Data *pd)
{
   DBG("(%p)", obj);
   _entry_remove(pd, _remove_notify, _error_notify, false);
}

void
_entry_remove(Ecordova_Entry_Data *pd,
              Ecordova_Entry_Success_Callback success_cb,
              Ecordova_Entry_Error_Callback error_cb,
              bool recursively)
{
   DBG("(%p)", pd->obj);

   Eio_Operation_Data *data = _data_new(pd, success_cb, error_cb);

   // TODO: file_system?

   if (pd->is_file)
     data->file = eio_file_unlink(pd->native,
                                  _eio_removed_cb,
                                  _eio_error_cb,
                                  data);
   else
     {
        Eio_Filter_Direct_Cb filter_cb =
          recursively ? _eio_filter_cb
                      : _eio_remove_non_recursively_filter_cb;

        data->file = eio_dir_unlink(pd->native,
                                    filter_cb,
                                    _eio_progress_cb,
                                    _eio_removed_cb,
                                    _eio_error_cb,
                                    data);
     }

   pd->pending = eina_list_append(pd->pending, data);
}

static void
_ecordova_entry_parent_get(Eo *obj, Ecordova_Entry_Data *pd)
{
   DBG("(%p)", obj);

   Eio_Operation_Data *data = _data_new(pd, _parent_get_notify, _error_notify);

   data->file = eio_file_direct_stat(pd->native,
                                     _parent_get_cb,
                                     _eio_error_cb,
                                     data);

   pd->pending = eina_list_append(pd->pending, data);
 }

static void
_parent_get_cb(void *user_data,
               Eio_File *handler EINA_UNUSED,
               const Eina_Stat *stat EINA_UNUSED)
{
   Eio_Operation_Data *data = user_data;
   data->success_cb(data->pd->obj, data->pd);
   _data_free(data);
}

static void
_parent_get_notify(Eo *obj, void *data)
{
   Ecordova_Entry_Data *pd = data;

   char *name, *path, *url;
   split_path(NULL, pd->path, &path, &name, &url);

   Ecordova_DirectoryEntry *parent =
     eo_add(ECORDOVA_DIRECTORYENTRY_CLASS,
            NULL,
            ecordova_directoryentry_constructor(name,
                                                path,
                                                NULL, // TODO: filesystem ?
                                                url));
   free(name);
   free(path);
   free(url);

   eo_do(obj, eo_event_callback_call(ECORDOVA_ENTRY_EVENT_PARENT_GET, parent));
   eo_unref(parent);
}

static Eina_Bool
_ecordova_entry_file_is_get(Eo *obj EINA_UNUSED, Ecordova_Entry_Data *pd)
{
   return pd->is_file;
}

static Eina_Bool
_ecordova_entry_directory_is_get(Eo *obj EINA_UNUSED, Ecordova_Entry_Data *pd)
{
   return pd->is_directory;
}

static const char *
_ecordova_entry_name_get(Eo *obj EINA_UNUSED, Ecordova_Entry_Data *pd)
{
   return pd->name;
}

static const char *
_ecordova_entry_path_get(Eo *obj EINA_UNUSED, Ecordova_Entry_Data *pd)
{
   return pd->path;
}

void
_metadata_get(Ecordova_Entry_Data *pd,
              Ecordova_Entry_Success_Callback success_cb,
              Ecordova_Entry_Error_Callback error_cb)
{
   DBG("(%p)", pd->obj);

   Eio_Operation_Data *data = _data_new(pd, success_cb, error_cb);

   data->file = eio_file_direct_stat(pd->native,
                                     _eio_stat_cb,
                                     _eio_error_cb,
                                     data);

   pd->pending = eina_list_append(pd->pending, data);
}

static void
_eio_stat_cb(void *user_data,
             Eio_File *handler EINA_UNUSED,
             const Eina_Stat *stat)
{
   Eio_Operation_Data *data = user_data;
   Ecordova_Metadata metadata = {
     .modification_date = (time_t)stat->mtime,
     .size = stat->size
   };

   data->success_cb(data->pd->obj, &metadata);
   _data_free(data);
}

static void
_metadata_notify(Eo *obj, void *data)
{
   eo_do(obj, eo_event_callback_call(ECORDOVA_ENTRY_EVENT_METADATA_GET, data));
}

void
_eio_error_cb(void *user_data,
              Eio_File *handler EINA_UNUSED,
              int error)
{
   Eio_Operation_Data *data = user_data;
   DBG("(%p)", data->pd->obj);
   data->error_cb(data->pd->obj, error);
   _data_free(data);
}

void
_eio_progress_cb(void *data EINA_UNUSED,
                 Eio_File *handler EINA_UNUSED,
                 const Eio_Progress *info EINA_UNUSED)
{
}

static void
_eio_moved_cb(void *user_data, Eio_File *handler EINA_UNUSED)
{
   Eio_Operation_Data *data = user_data;
   free(data->pd->path);
   free(data->pd->name);
   free(data->pd->native);
   data->pd->path = data->path;
   data->pd->name = data->name;
   data->pd->native = data->native;
   data->path = NULL;
   data->name = NULL;
   data->native = NULL;

   data->success_cb(data->pd->obj, data->pd->obj);
   _data_free(data);
}

static void
_move_notify(Eo *obj, void *data)
{
   eo_do(obj, eo_event_callback_call(ECORDOVA_ENTRY_EVENT_MOVE_SUCCESS, data));
}

static void
_eio_copied_cb(void *user_data, Eio_File *handler EINA_UNUSED)
{
   Eio_Operation_Data *data = user_data;
   Ecordova_Entry *entry = NULL;
   if (data->pd->is_file)
     entry = eo_add(ECORDOVA_FILEENTRY_CLASS,
                    NULL,
                    ecordova_fileentry_constructor(data->name,
                                                   data->path,
                                                   data->pd->file_system,
                                                   data->native));
   else
     entry = eo_add(ECORDOVA_DIRECTORYENTRY_CLASS,
                    NULL,
                    ecordova_directoryentry_constructor(data->name,
                                                        data->path,
                                                        data->pd->file_system,
                                                        data->native));

   data->success_cb(data->pd->obj, entry);

   eo_unref(entry);
   _data_free(data);
}

static void
_copy_notify(Eo *obj, void *data)
{
   eo_do(obj, eo_event_callback_call(ECORDOVA_ENTRY_EVENT_COPY_SUCCESS, data));
}

void
_eio_removed_cb(void *user_data, Eio_File *handler EINA_UNUSED)
{
   Eio_Operation_Data *data = user_data;
   data->success_cb(data->pd->obj, NULL);
   _data_free(data);
}

static void
_remove_notify(Eo *obj, void *data)
{
   eo_do(obj, eo_event_callback_call(ECORDOVA_ENTRY_EVENT_REMOVE_SUCCESS, data));
}

Eina_Bool
_eio_filter_cb(void *data EINA_UNUSED,
               Eio_File *handler EINA_UNUSED,
               const Eina_File_Direct_Info *info EINA_UNUSED)
{
   return EINA_TRUE;
}

static Eina_Bool
_eio_remove_non_recursively_filter_cb(void *user_data,
                                      Eio_File *handler EINA_UNUSED,
                                      const Eina_File_Direct_Info *info EINA_UNUSED)
{
   Eio_Operation_Data *data = user_data;
   DBG("filter_cb: %s", info->path);
   if (++data->count != 1)
     {
        eio_file_cancel(handler);
        return EINA_FALSE;
     }

   return EINA_TRUE;
}

Eio_Operation_Data *
_data_new(Ecordova_Entry_Data *pd,
          Ecordova_Entry_Success_Callback success_cb,
          Ecordova_Entry_Error_Callback error_cb)
{
   Eio_Operation_Data *data = calloc(1, sizeof(Eio_Operation_Data));
   data->pd = pd;
   data->success_cb = success_cb;
   data->error_cb = error_cb;
   return data;
}

void
_data_free(Eio_Operation_Data *data)
{
   EINA_SAFETY_ON_NULL_RETURN(data);

   data->pd->pending = eina_list_remove(data->pd->pending, data);
   free(data->path);
   free(data->name);
   free(data->native);
   free(data);
}

void
_error_notify(Eo *obj, int error)
{
   Ecordova_FileError file_error = _translate_errno(error);
   eo_do(obj, eo_event_callback_call(ECORDOVA_ENTRY_EVENT_ERROR, &file_error));
}

Ecordova_FileError
_translate_errno(int error)
{
   // TODO: translate other errors
   switch (error)
     {
      case EPERM:
      case EACCES:
        return ECORDOVA_FILEERROR_SECURITY_ERR;
      case ENOENT:
        return ECORDOVA_FILEERROR_NOT_FOUND_ERR;
     }

   return -1;
}

#include "undefs.h"

#define ecordova_entry_class_get ecordova_entry_impl_class_get

#include "ecordova_entry.eo.c"
