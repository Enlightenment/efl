#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <Eo.h>
#include <Ecordova_Eo.h>

#include "ecordova_directoryreader_private.h"
#include "ecordova_entry_private.h"

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

#define MY_CLASS ECORDOVA_DIRECTORYREADER_CLASS
#define MY_CLASS_NAME "Ecordova_DirectoryReader"

typedef struct
{
  Ecordova_DirectoryReader_Data   *pd;
  Eio_File                        *file;
  Eina_List                       *entries;
} DirectoryReader_Data;

static Eina_Bool _filter_cb(void *, Eio_File *, const Eina_File_Direct_Info *);
static void _main_cb(void *, Eio_File *, const Eina_File_Direct_Info *);
static void _done_cb(void *, Eio_File *);
static void _error_cb(void *, Eio_File *, int);
static void _directory_data_free(DirectoryReader_Data *);

static Eo_Base *
_ecordova_directoryreader_eo_base_constructor(Eo *obj,
                                              Ecordova_DirectoryReader_Data *pd)
{
   DBG("(%p)", obj);
   fprintf(stderr, "%s:%s:%d\n", __func__, __FILE__, __LINE__); fflush(stderr);

   pd->obj = obj;
   pd->native = NULL;
   pd->pending = NULL;

   return eo_do_super_ret(obj, MY_CLASS, obj, eo_constructor());
}

static void
_ecordova_directoryreader_constructor(Eo *obj,
                                      Ecordova_DirectoryReader_Data *pd,
                                      const char *native)
{
   DBG("(%p) url=%s", obj, native);
   fprintf(stderr, "%s:%s:%d\n", __func__, __FILE__, __LINE__); fflush(stderr);
   pd->native = strdup(native);
}

static void
_ecordova_directoryreader_eo_base_destructor(Eo *obj,
                                             Ecordova_DirectoryReader_Data *pd)
{
   DBG("(%p)", obj);
   fprintf(stderr, "%s:%s:%d\n", __func__, __FILE__, __LINE__); fflush(stderr);

   free(pd->native);

   DirectoryReader_Data *data;
   EINA_LIST_FREE(pd->pending, data)
     {
        eio_file_cancel(data->file);
        _directory_data_free(data);
     }

   eo_do_super(obj, MY_CLASS, eo_destructor());
}

static void
_ecordova_directoryreader_entries_read(Eo *obj,
                                       Ecordova_DirectoryReader_Data *pd)
{
   DBG("(%p)", obj);
   fprintf(stderr, "%s:%s:%d\n", __func__, __FILE__, __LINE__); fflush(stderr);

   DirectoryReader_Data *data = calloc(1, sizeof(DirectoryReader_Data));
   data->pd = pd;
   data->file = eio_file_stat_ls(pd->native,
                                 _filter_cb,
                                 _main_cb,
                                 _done_cb,
                                 _error_cb,
                                 data);
   fprintf(stderr, "%s:%s:%d\n", __func__, __FILE__, __LINE__); fflush(stderr);

   pd->pending = eina_list_append(pd->pending, data);
   fprintf(stderr, "%s:%s:%d\n", __func__, __FILE__, __LINE__); fflush(stderr);
}

static Eina_Bool
_filter_cb(void *data EINA_UNUSED,
           Eio_File *handler EINA_UNUSED,
           const Eina_File_Direct_Info *info EINA_UNUSED)
{
   fprintf(stderr, "%s:%s:%d\n", __func__, __FILE__, __LINE__); fflush(stderr);
   return EINA_TRUE;
}

static void
_main_cb(void *user_data,
         Eio_File *handler EINA_UNUSED,
         const Eina_File_Direct_Info *info)
{
   fprintf(stderr, "%s:%s:%d\n", __func__, __FILE__, __LINE__); fflush(stderr);
   DirectoryReader_Data *data = user_data;
   Ecordova_Entry *entry = NULL;

   size_t len = info->path_length - info->name_length - 1;
   char path[len + 1];
   strncpy(path, info->path, len);
   path[len] = '\0';

   const char *name = &info->path[info->name_start];

   switch (info->type)
     {
      case EINA_FILE_DIR:
        {
           entry = eo_add(ECORDOVA_DIRECTORYENTRY_CLASS, NULL,
             ecordova_directoryentry_constructor(name, path, NULL, info->path)); // TODO: filesystem?
           break;
        }
      case EINA_FILE_REG:
        {
           entry = eo_add(ECORDOVA_FILEENTRY_CLASS, NULL,
             ecordova_fileentry_constructor(name, path, NULL, info->path)); // TODO: filesystem?
           break;
        }
      // TODO: case EINA_FILE_LNK ?
      default: break;
     }

  if (entry)
    data->entries = eina_list_append(data->entries, entry);
}

static void
_done_cb(void *user_data, Eio_File *handler EINA_UNUSED)
{
   fprintf(stderr, "%s:%s:%d\n", __func__, __FILE__, __LINE__); fflush(stderr);
   DirectoryReader_Data *data = user_data;
   data->pd->pending = eina_list_remove(data->pd->pending, data);

   eo_do(data->pd->obj,
         eo_event_callback_call(ECORDOVA_DIRECTORYREADER_EVENT_SUCCESS,
                                data->entries));
   _directory_data_free(data);
}

static void
_error_cb(void *user_data, Eio_File *handler EINA_UNUSED, int error)
{
   fprintf(stderr, "%s:%s:%d\n", __func__, __FILE__, __LINE__); fflush(stderr);
   DirectoryReader_Data *data = user_data;
   data->pd->pending = eina_list_remove(data->pd->pending, data);

   Ecordova_FileError file_error = _translate_errno(error);
   eo_do(data->pd->obj,
         eo_event_callback_call(ECORDOVA_DIRECTORYREADER_EVENT_ERROR,
                                &file_error));
   _directory_data_free(data);
}

static void
_directory_data_free(DirectoryReader_Data *data)
{
   fprintf(stderr, "%s:%s:%d\n", __func__, __FILE__, __LINE__); fflush(stderr);
   Ecordova_Entry *entry;
   EINA_LIST_FREE(data->entries, entry)
     eo_unref(entry);
   free(data);
}

#include "undefs.h"

#define ecordova_directoryreader_class_get ecordova_directoryreader_impl_class_get

#include "ecordova_directoryreader.eo.c"
