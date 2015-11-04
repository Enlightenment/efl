#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "ecordova_filewriter_private.h"
#include "ecordova_entry_private.h"

#include <unistd.h>

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

#define MY_CLASS ECORDOVA_FILEWRITER_CLASS
#define MY_CLASS_NAME "Ecordova_FileWriter"

static void _write_cb(void *, Ecore_Thread *);
static void _truncate_cb(void *, Ecore_Thread *);
static Eina_Bool _stream_init(Ecordova_FileWriter_Data *);
static Eina_Bool _offset_set(Ecordova_FileWriter_Data *);
static void _write_end_cb(void *, Ecore_Thread *);
static void _write_abort_cb(void *, Ecore_Thread *);
static void _write_progress_cb(void *, Ecore_Thread *, void *);
static void _progress_notify(size_t, size_t, Eo *, Ecore_Thread *);

static Eo_Base *
_ecordova_filewriter_eo_base_constructor(Eo *obj,
                                         Ecordova_FileWriter_Data *pd)
{
   DBG("(%p)", obj);

   pd->obj = obj;
   pd->error = 0;
   pd->error = ECORDOVA_FILEWRITER_STATE_INIT;
   pd->url = NULL;
   pd->offset = 0;
   pd->length = 0;
   pd->stream = NULL;
   pd->data = NULL;
   pd->data_size = 0;
   pd->truncate_size = 0;

   eo_do_super(obj, MY_CLASS, obj = eo_constructor());
   eo_do(obj, ecordova_entry_file_is_set(EINA_TRUE));
   return obj;
}

static void
_ecordova_filewriter_file_set(Eo *obj,
                              Ecordova_FileWriter_Data *pd,
                              Ecordova_File *file)
{
   DBG("(%p)", obj);
   EINA_SAFETY_ON_NULL_RETURN(file);

   eo_do(file,
         pd->url = strdup(ecordova_file_url_get()),
         pd->offset = ecordova_file_start_get(),
         pd->length = ecordova_file_end_get() - pd->offset);
}

static void
_ecordova_filewriter_eo_base_destructor(Eo *obj,
                                        Ecordova_FileWriter_Data *pd)
{
   DBG("(%p)", obj);

   if (pd->thread) ERR("%s", "Destructing without aborting first");

   free(pd->url);
   if (pd->stream)
     fclose(pd->stream);
   if (pd->data)
     free(pd->data);

   eo_do_super(obj, MY_CLASS, eo_destructor());
}

static Ecordova_FileError
_ecordova_filewriter_abort(Eo *obj, Ecordova_FileWriter_Data *pd)
{
   DBG("(%p)", obj);

   if (ECORDOVA_FILEWRITER_STATE_DONE == pd->state ||
       ECORDOVA_FILEWRITER_STATE_INIT == pd->state)
     return ECORDOVA_FILEERROR_INVALID_STATE_ERR;
   pd->state = ECORDOVA_FILEWRITER_STATE_DONE;
   pd->error = ECORDOVA_FILEERROR_ABORT_ERR;

   if (pd->thread)
     ecore_thread_cancel(pd->thread);

   return 0;
}

static Ecordova_FileError
_ecordova_filewriter_write(Eo *obj,
                           Ecordova_FileWriter_Data *pd,
                           const char *data,
                           long size)
{
   DBG("(%p)", obj);

   if (ECORDOVA_FILEWRITER_STATE_WRITING == pd->state)
     return ECORDOVA_FILEERROR_INVALID_STATE_ERR;
   pd->state = ECORDOVA_FILEWRITER_STATE_WRITING;
   pd->error = 0;

   Ecordova_ProgressEvent writestart = {.type = "writestart", .target = obj};
   eo_do(obj,
         eo_event_callback_call(ECORDOVA_FILEWRITER_EVENT_ON_WRITE_START,
                                &writestart));

   pd->data_size = size ? size : (long)strlen(data);
   EINA_SAFETY_ON_FALSE_RETURN_VAL(pd->data_size > 0, ECORDOVA_FILEERROR_SYNTAX_ERR);

   pd->data = pd->data ? realloc(pd->data, pd->data_size) : malloc(pd->data_size);
   EINA_SAFETY_ON_NULL_RETURN_VAL(pd->data, ECORDOVA_FILEERROR_SYNTAX_ERR);

   memcpy(pd->data, data, pd->data_size);

   pd->thread = ecore_thread_feedback_run(_write_cb,
                                          _write_progress_cb,
                                          _write_end_cb,
                                          _write_abort_cb,
                                          pd,
                                          EINA_FALSE);
   return 0;
}

static Ecordova_FileError
_ecordova_filewriter_seek(Eo *obj, Ecordova_FileWriter_Data *pd, long offset)
{
   DBG("(%p)", obj);
   if (ECORDOVA_FILEWRITER_STATE_WRITING == pd->state)
     return ECORDOVA_FILEERROR_INVALID_STATE_ERR;

   if (offset < 0)
     pd->offset = MAX(offset + pd->length, 0);
   else if (offset > pd->length)
     pd->offset = pd->length;
   else
     pd->offset = offset;

   return 0;
}

static Ecordova_FileError
_ecordova_filewriter_truncate(Eo *obj, Ecordova_FileWriter_Data *pd, long size)
{
   DBG("(%p)", obj);

   if (ECORDOVA_FILEWRITER_STATE_WRITING == pd->state)
     return ECORDOVA_FILEERROR_INVALID_STATE_ERR;
   pd->state = ECORDOVA_FILEWRITER_STATE_WRITING;
   pd->error = 0;
   pd->truncate_size = size;

   pd->thread = ecore_thread_run(_truncate_cb,
                                 _write_end_cb,
                                 _write_abort_cb,
                                 pd);
   return 0;
}

static Ecordova_FileError
_ecordova_filewriter_error_get(Eo *obj, Ecordova_FileWriter_Data *pd)
{
   DBG("(%p)", obj);
   return pd->error;
}

static long
_ecordova_filewriter_position_get(Eo *obj, Ecordova_FileWriter_Data *pd)
{
   DBG("(%p)", obj);
   return pd->offset;
}

static Ecordova_FileWriter_State
_ecordova_filewriter_state_get(Eo *obj, Ecordova_FileWriter_Data *pd)
{
   DBG("(%p)", obj);
   return pd->state;
}

static void
_write_cb(void *data, Ecore_Thread *thread)
{
   Ecordova_FileWriter_Data *pd = data;
   DBG("(%p)", pd->obj);

   if (ecore_thread_check(thread))
     return;

   if (!pd->stream && !_stream_init(pd))
     return;

   if (!_offset_set(pd))
     return;

   _progress_notify(0, pd->data_size, pd->obj, thread);
   if (ecore_thread_check(thread))
     return;

   size_t total = pd->data_size;
   char *buffer = pd->data;
   do
     {
        size_t written = fwrite(buffer, sizeof(char), total, pd->stream);
        if (!written)
          {
             pd->error = _translate_errno(errno);
             return;
          }

        buffer += written;
        total -= written;
        pd->offset += written;

        _progress_notify(pd->data_size - total, pd->data_size, pd->obj, thread);
        if (ecore_thread_check(thread))
          return;
     }
   while (total > 0);

   pd->length = MAX(pd->offset, pd->length);
}

static void
_truncate_cb(void *data, Ecore_Thread *thread EINA_UNUSED)
{
   Ecordova_FileWriter_Data *pd = data;
   DBG("(%p)", pd->obj);

   if (ecore_thread_check(thread))
     return;

   if (!pd->stream && !_stream_init(pd))
     return;

   if (!_offset_set(pd))
     return;

   if (ftruncate(fileno(pd->stream), pd->truncate_size) != 0)
     {
        pd->error = _translate_errno(errno);
        return;
     }

   pd->length = pd->truncate_size;
   pd->offset = MIN(pd->offset, pd->truncate_size);
}

static Eina_Bool
_stream_init(Ecordova_FileWriter_Data *pd)
{
   pd->stream = fopen(pd->url, "rb+");
   if (!pd->stream)
     {
        pd->error = _translate_errno(errno);
        return EINA_FALSE;
     }

  return EINA_TRUE;
}

static Eina_Bool
_offset_set(Ecordova_FileWriter_Data *pd)
{
  int error = fseek(pd->stream, pd->offset, SEEK_SET);
  if (error)
    {
       pd->error = _translate_errno(errno);
       return EINA_FALSE;
    }

  return EINA_TRUE;
}

static void
_progress_notify(size_t written, size_t total, Eo *obj, Ecore_Thread *thread)
{
   Ecordova_ProgressEvent *progress = malloc(sizeof(Ecordova_ProgressEvent));
   *progress = (Ecordova_ProgressEvent)
     {
        .type = "progress",
        .length_computable = EINA_TRUE,
        .loaded = written,
        .total = total,
        .target = obj
     };
   if (!ecore_thread_feedback(thread, progress))
     free(progress);
}

static void
_write_progress_cb(void *data,
                   Ecore_Thread *thread EINA_UNUSED,
                   void *msg_data)
{
   Ecordova_FileWriter_Data *pd = data;
   DBG("(%p)", pd->obj);
   Ecordova_ProgressEvent *event = msg_data;
   eo_do(pd->obj,
         eo_event_callback_call(ECORDOVA_FILEWRITER_EVENT_ON_PROGRESS, &event));
   free(event);
}

static void
_write_end_cb(void *data, Ecore_Thread *thread EINA_UNUSED)
{
   Ecordova_FileWriter_Data *pd = data;
   DBG("(%p)", pd->obj);

   // If DONE (cancelled), then don't do anything
   if (ECORDOVA_FILEWRITER_STATE_DONE == pd->state)
     return;
   pd->thread = NULL;
   pd->state = ECORDOVA_FILEWRITER_STATE_DONE;

   if (pd->error)
     {
        Ecordova_ProgressEvent error = {.type = "error", .target = pd->obj};
        eo_do(pd->obj,
              eo_event_callback_call(ECORDOVA_FILEWRITER_EVENT_ON_ERROR,
                                     &error));
     }
   else
     {
        Ecordova_ProgressEvent write = {.type = "write", .target = pd->obj};
        eo_do(pd->obj,
              eo_event_callback_call(ECORDOVA_FILEWRITER_EVENT_ON_WRITE,
                                     &write));
     }

   Ecordova_ProgressEvent writeend = {.type = "writeend", .target = pd->obj};
   eo_do(pd->obj,
         eo_event_callback_call(ECORDOVA_FILEWRITER_EVENT_ON_WRITE_END,
                                &writeend));
}

static void
_write_abort_cb(void *data, Ecore_Thread *thread EINA_UNUSED)
{
   Ecordova_FileWriter_Data *pd = data;
   DBG("(%p)", pd->obj);

   pd->thread = NULL;

   Ecordova_ProgressEvent on_abort = {.type = "abort", .target = pd->obj};
   eo_do(pd->obj,
         eo_event_callback_call(ECORDOVA_FILEWRITER_EVENT_ON_ABORT,
                                &on_abort));

   Ecordova_ProgressEvent writeend = {.type = "writeend", .target = pd->obj};
   eo_do(pd->obj,
         eo_event_callback_call(ECORDOVA_FILEWRITER_EVENT_ON_WRITE_END,
                                &writeend));
}

#undef EOAPI
#define EOAPI EAPI

#include "undefs.h"

#define ecordova_filewriter_class_get ecordova_filewriter_impl_class_get

#include "ecordova_filewriter.eo.c"
