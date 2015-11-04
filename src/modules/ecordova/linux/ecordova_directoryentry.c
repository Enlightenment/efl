#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <Eo.h>
#include <Ecore_File.h>
#include <Ecordova_Eo.h>

#include "ecordova_directoryentry_private.h"
#include "ecordova_entry_private.h"

#include <stdio.h>

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

#define MY_CLASS ECORDOVA_DIRECTORYENTRY_CLASS
#define MY_CLASS_NAME "Ecordova_DirectoryEntry"

static void _remove_notify(Eo *, void *);
static Eina_Bool _is_absolute(const char *);
static void _directory_get(Eo *, void *);
static void _file_get(Eo *, void *);
static void _eio_directory_get_cb(void *, Eio_File *, const Eina_Stat *);
static void _eio_file_get_cb(void *, Eio_File *, const Eina_Stat *);
static void _eio_create_directory_cb(void *, Eio_File *, int);
static void _eio_fail_if_path_exists_cb(void *, Eio_File *, const Eina_Stat *);
static void _eio_mkdir_cb(void *, Eio_File *);
static void _set_data_path_name_native(Eio_Operation_Data *, const char *);
static void _eio_create_file_cb(void *, Eio_File *, int);

static Eo_Base *
_ecordova_directoryentry_eo_base_constructor(Eo *obj,
                                             Ecordova_DirectoryEntry_Data *pd)
{
   DBG("(%p)", obj);

   pd->obj = obj;

   eo_do_super(obj, MY_CLASS, obj = eo_constructor());
   ecordova_entry_file_is_set(EINA_FALSE);
   return obj;
}

static void
_ecordova_directoryentry_eo_base_destructor(Eo *obj EINA_UNUSED,
                                            Ecordova_DirectoryEntry_Data *pd EINA_UNUSED)
{
   DBG("(%p)", obj);

   eo_do_super(obj, MY_CLASS, eo_destructor());
}

static Ecordova_DirectoryReader *
_ecordova_directoryentry_reader_create(Eo *obj,
                                       Ecordova_DirectoryEntry_Data *pd EINA_UNUSED)
{
   Ecordova_Entry_Data *super = eo_data_scope_get(obj, ECORDOVA_ENTRY_CLASS);
   EINA_SAFETY_ON_NULL_RETURN_VAL(super, NULL);

   return eo_add(ECORDOVA_DIRECTORYREADER_CLASS, NULL,
     ecordova_directoryreader_native_set(super->native));
}

static void
_ecordova_directoryentry_directory_get(Eo *obj,
                                       Ecordova_DirectoryEntry_Data *pd EINA_UNUSED,
                                       const char *path,
                                       Ecordova_FileFlags flags)
{
   DBG("(%p)", obj);
   EINA_SAFETY_ON_NULL_RETURN(path);

   Ecordova_Entry_Data *super = eo_data_scope_get(obj, ECORDOVA_ENTRY_CLASS);
   EINA_SAFETY_ON_NULL_RETURN(super);

   Eio_Operation_Data *data = _data_new(super, _directory_get, _error_notify);
   _set_data_path_name_native(data, path);

   Eio_Stat_Cb eio_stat_cb = _eio_directory_get_cb;
   Eio_Error_Cb eio_error_cb = _eio_error_cb;

   if (ECORDOVA_FILEFLAGS_CREATE & flags)
     {
        eio_error_cb = _eio_create_directory_cb;
        if (ECORDOVA_FILEFLAGS_EXCLUSIVE & flags)
          eio_stat_cb = _eio_fail_if_path_exists_cb;
     }

   super->pending = eina_list_append(super->pending, data);

   data->file = eio_file_direct_stat(data->native,
                                     eio_stat_cb,
                                     eio_error_cb,
                                     data);
   EINA_SAFETY_ON_NULL_RETURN(data->file);
}

static void
_eio_directory_get_cb(void *user_data,
                      Eio_File *handler EINA_UNUSED,
                      const Eina_Stat *stat EINA_UNUSED)
{
   Eio_Operation_Data *data = user_data;
   DBG("(%p)", data->pd->obj);
   Ecordova_DirectoryEntry *directory = eo_add(ECORDOVA_DIRECTORYENTRY_CLASS, NULL,
                                               ecordova_entry_name_set(data->name),
                                               ecordova_entry_path_set(data->path));

   data->success_cb(data->pd->obj, directory);
   _data_free(data);
}

static void
_eio_file_get_cb(void *user_data,
                 Eio_File *handler EINA_UNUSED,
                 const Eina_Stat *stat EINA_UNUSED)
{
   Eio_Operation_Data *data = user_data;
   Ecordova_FileEntry *file = eo_add(ECORDOVA_FILEENTRY_CLASS, NULL,
                                     ecordova_entry_name_set(data->name),
                                     ecordova_entry_path_set(data->path));

   data->success_cb(data->pd->obj, file);
   _data_free(data);
}

static void
_eio_fail_if_path_exists_cb(void *user_data,
                            Eio_File *handler EINA_UNUSED,
                            const Eina_Stat *stat EINA_UNUSED)
{
   Eio_Operation_Data *data = user_data;
   DBG("(%p)", data->pd->obj);
   Ecordova_FileError file_error = ECORDOVA_FILEERROR_PATH_EXISTS_ERR;
   eo_do(data->pd->obj,
         eo_event_callback_call(ECORDOVA_ENTRY_EVENT_ERROR, &file_error));
   _data_free(data);
}

static void
_eio_create_directory_cb(void *user_data,
                         Eio_File *handler EINA_UNUSED,
                         int error)
{
   Eio_Operation_Data *data = user_data;
   DBG("(%p)", data->pd->obj);
   if (ENOENT != error)
     {
        _error_notify(data->pd->obj, error);
        _data_free(data);
        return;
     }

   data->file = eio_file_mkdir(data->native,
                               0777,
                               _eio_mkdir_cb,
                               _eio_error_cb,
                               data);
}

static void
_eio_create_file_cb(void *user_data,
                    Eio_File *handler EINA_UNUSED,
                    int error)
{
   Eio_Operation_Data *data = user_data;
   DBG("(%p)", data->pd->obj);
   if (ENOENT != error)
     {
        _error_notify(data->pd->obj, error);
        _data_free(data);
        return;
     }

   // TODO: Create the file in a background thread
   FILE *fd = fopen(data->native, "ab+");
   if (!fd)
     {
        _error_notify(data->pd->obj, errno);
        _data_free(data);
        return;
     }
   fclose(fd);

   Ecordova_FileEntry *file = eo_add(ECORDOVA_FILEENTRY_CLASS, NULL,
                                     ecordova_entry_name_set(data->name),
                                     ecordova_entry_path_set(data->path));
   data->success_cb(data->pd->obj, file);
   _data_free(data);
}

static void
_eio_mkdir_cb(void *user_data, Eio_File *handler EINA_UNUSED)
{
   Eio_Operation_Data *data = user_data;
   DBG("(%p)", data->pd->obj);
   Ecordova_DirectoryEntry *directory = eo_add(ECORDOVA_DIRECTORYENTRY_CLASS, NULL,
                                               ecordova_entry_name_set(data->name),
                                               ecordova_entry_path_set(data->path));
   data->success_cb(data->pd->obj, directory);
   _data_free(data);
}

static void
_directory_get(Eo *obj, void *data)
{
   Ecordova_DirectoryEntry *directory = data;
   eo_do(obj,
     eo_event_callback_call(ECORDOVA_DIRECTORYENTRY_EVENT_DIRECTORY_GET, directory));
   eo_unref(directory);
}

static void
_file_get(Eo *obj, void *data)
{
   Ecordova_FileEntry *file = data;
   eo_do(obj, eo_event_callback_call(ECORDOVA_DIRECTORYENTRY_EVENT_FILE_GET, file));
   eo_unref(file);
}

static void
_ecordova_directoryentry_recursively_remove(Eo *obj,
                                            Ecordova_DirectoryEntry_Data *pd EINA_UNUSED)
{
   DBG("(%p)", obj);

   Ecordova_Entry_Data *super = eo_data_scope_get(obj, ECORDOVA_ENTRY_CLASS);
   EINA_SAFETY_ON_NULL_RETURN(super);

   _entry_remove(super, _remove_notify, _error_notify, EINA_TRUE);
}

static void
_ecordova_directoryentry_file_get(Eo *obj,
                                  Ecordova_DirectoryEntry_Data *pd EINA_UNUSED,
                                  const char *path,
                                  Ecordova_FileFlags flags)
{
   EINA_SAFETY_ON_NULL_RETURN(path);
   DBG("(%p) path=%s", obj, path);

   Ecordova_Entry_Data *super = eo_data_scope_get(obj, ECORDOVA_ENTRY_CLASS);
   EINA_SAFETY_ON_NULL_RETURN(super);

   Eio_Operation_Data *data = _data_new(super, _file_get, _error_notify);
   _set_data_path_name_native(data, path);

   Eio_Stat_Cb eio_stat_cb = _eio_file_get_cb;
   Eio_Error_Cb eio_error_cb = _eio_error_cb;

   if (ECORDOVA_FILEFLAGS_CREATE & flags)
     {
        eio_error_cb = _eio_create_file_cb;
        if (ECORDOVA_FILEFLAGS_EXCLUSIVE & flags)
          eio_stat_cb = _eio_fail_if_path_exists_cb;
     }

   super->pending = eina_list_append(super->pending, data);

   data->file = eio_file_direct_stat(data->native,
                                     eio_stat_cb,
                                     eio_error_cb,
                                     data);
}

static void
_remove_notify(Eo *obj, void *data EINA_UNUSED)
{
   eo_do(obj,
     eo_event_callback_call(ECORDOVA_DIRECTORYENTRY_EVENT_REMOVE_SUCCESS, NULL));
}

static Eina_Bool
_is_absolute(const char *path)
{
   // TODO: not multiplatform
   return path[0] == '/';
}

static void
_set_data_path_name_native(Eio_Operation_Data *data, const char *path)
{
   EINA_SAFETY_ON_NULL_RETURN(data);
   EINA_SAFETY_ON_NULL_RETURN(path);
   split_path(data->pd->native, path, &data->path, &data->name, &data->native);
}

void
split_path(const char *working_dir,
           const char *path,
           char **dir,
           char **name,
           char **url)
{
   EINA_SAFETY_ON_NULL_RETURN(path);
   EINA_SAFETY_ON_NULL_RETURN(dir);
   EINA_SAFETY_ON_NULL_RETURN(name);
   EINA_SAFETY_ON_NULL_RETURN(url);

   if (!working_dir || _is_absolute(path))
     *url = eina_file_path_sanitize(path);
   else
     {
        size_t len = strlen(working_dir) + 1 + strlen(path) + 1;
        char buffer[len];
        snprintf(buffer, len, "%s/%s", working_dir, path); // TODO: path separator ?
        *url = eina_file_path_sanitize(buffer);
     }

   const char *nameonly = ecore_file_file_get(*url);
   EINA_SAFETY_ON_NULL_RETURN(nameonly);
   *name = strdup(nameonly);

   *dir = ecore_file_dir_get(*url);
}

#include "undefs.h"

#define ecordova_directoryentry_class_get ecordova_directoryentry_impl_class_get

#include "ecordova_directoryentry.eo.c"
