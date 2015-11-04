#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "ecordova_filereader_private.h"
#include "ecordova_entry_private.h"

#include <Eio.h>

#ifdef EAPI
#undef EAPI
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

#define MY_CLASS ECORDOVA_FILEREADER_CLASS
#define MY_CLASS_NAME "Ecordova_FileReader"

static void _read_cb(void *, Ecore_Thread *);
static void _progress_notify(size_t, size_t, Eo *, Ecore_Thread *);
static void _read_progress_cb(void *, Ecore_Thread *, void *);
static void _read_end_cb(void *, Ecore_Thread *);
static void _read_abort_cb(void *, Ecore_Thread *);

static Eo_Base *
_ecordova_filereader_eo_base_constructor(Eo *obj, Ecordova_FileReader_Data *pd)
{
   DBG("(%p)", obj);

   pd->obj = obj;
   pd->error = 0;
   pd->state = ECORDOVA_FILEREADER_STATE_EMPTY;
   pd->result = NULL;
   pd->result_length = 0;
   pd->thread = NULL;
   pd->url = NULL;
   pd->offset = 0;
   pd->length = 0;

   eo_do_super(obj, MY_CLASS, obj = eo_constructor());
   ecordova_entry_file_is_set(EINA_TRUE);
   return obj;
}

static void
_ecordova_filereader_eo_base_destructor(Eo *obj, Ecordova_FileReader_Data *pd)
{
   DBG("(%p)", obj);

   if (pd->thread) ERR("%s", "Destructing without aborting first");

   free(pd->result);
   free(pd->url);

   eo_do_super(obj, MY_CLASS, eo_destructor());
}

static void
_ecordova_filereader_abort(Eo *obj, Ecordova_FileReader_Data *pd)
{
   DBG("(%p)", obj);

   if (ECORDOVA_FILEREADER_STATE_DONE == pd->state ||
       ECORDOVA_FILEREADER_STATE_EMPTY == pd->state)
     return;
   pd->error = ECORDOVA_FILEERROR_ABORT_ERR;
   pd->state = ECORDOVA_FILEREADER_STATE_DONE;

   if (pd->thread)
     ecore_thread_cancel(pd->thread);
}

static Ecordova_FileError
_ecordova_filereader_read(Eo *obj,
                          Ecordova_FileReader_Data *pd,
                          Ecordova_File *file)
{
   DBG("(%p)", obj);
   EINA_SAFETY_ON_NULL_RETURN_VAL(file, ECORDOVA_FILEERROR_SYNTAX_ERR);

   if (ECORDOVA_FILEREADER_STATE_LOADING == pd->state)
     return ECORDOVA_FILEERROR_INVALID_STATE_ERR;
   pd->state = ECORDOVA_FILEREADER_STATE_LOADING;
   pd->error = 0;

   const char *url;
   eo_do(file,
         url = ecordova_file_url_get(),
         pd->offset = ecordova_file_start_get(),
         pd->length = ecordova_file_end_get() - pd->offset);
   EINA_SAFETY_ON_NULL_RETURN_VAL(url, ECORDOVA_FILEERROR_SYNTAX_ERR);
   pd->url = strdup(url);

   Ecordova_ProgressEvent loadstart = {.type = "loadstart", .target = obj};
   eo_do(obj, eo_event_callback_call(ECORDOVA_FILEREADER_EVENT_ON_LOAD_START, &loadstart));

   pd->thread = ecore_thread_feedback_run(_read_cb,
                                          _read_progress_cb,
                                          _read_end_cb,
                                          _read_abort_cb,
                                          pd,
                                          EINA_FALSE);
   return 0;
}

static Ecordova_FileError
_ecordova_filereader_error_get(Eo *obj, Ecordova_FileReader_Data *pd)
{
   DBG("(%p)", obj);
   return pd->error;
}

static const char *
_ecordova_filereader_result_get(Eo *obj, Ecordova_FileReader_Data *pd)
{
   DBG("(%p)", obj);
   return pd->result;
}

static size_t
_ecordova_filereader_length_get(Eo *obj, Ecordova_FileReader_Data *pd)
{
   DBG("(%p)", obj);
   return pd->result_length;
}

static Ecordova_FileReader_State
_ecordova_filereader_state_get(Eo *obj, Ecordova_FileReader_Data *pd)
{
   DBG("(%p)", obj);
   return pd->state;
}

static void
_read_cb(void *data, Ecore_Thread *thread)
{
   Ecordova_FileReader_Data *pd = data;
   DBG("(%p)", pd->obj);

   if (ecore_thread_check(thread))
     return;

   FILE *stream = fopen(pd->url, "rb");
   if (!stream)
     {
        pd->error = _translate_errno(errno);
        return;
     }

   if (pd->offset > 0)
     {
       int error = fseek(stream, pd->offset, SEEK_SET);
       if (error)
         {
            pd->error = _translate_errno(errno);
            goto on_error;
         }
     }

   _progress_notify(0, pd->length, pd->obj, thread);
   if (ecore_thread_check(thread))
     goto on_error;

   pd->result = pd->result ? realloc(pd->result, pd->length * sizeof(char))
                           : malloc(pd->length * sizeof(char));
   if (NULL == pd->result)
     {
        pd->error = _translate_errno(errno);
        goto on_error;
     }

   pd->result_length = 0;
   size_t total = pd->length;
   char *buffer = pd->result;
   do
     {
        size_t read = fread(buffer, sizeof(char), total, stream);
        if (!read)
          {
             pd->error = _translate_errno(errno);
             goto on_error;
          }

        buffer += read;
        total -= read;
        pd->result_length += read;

        _progress_notify(pd->length - total, pd->length, pd->obj, thread);
        if (ecore_thread_check(thread))
          goto on_error;
     }
   while (total > 0);

on_error:
   fclose(stream);
}

static void
_progress_notify(size_t read, size_t total, Eo *obj, Ecore_Thread *thread)
{
   DBG("(%p)", obj);
   Ecordova_ProgressEvent *progress = malloc(sizeof(Ecordova_ProgressEvent));
   *progress = (Ecordova_ProgressEvent)
     {
        .type = "progress",
        .length_computable = EINA_TRUE,
        .loaded = read,
        .total = total,
        .target = obj
     };
   if (!ecore_thread_feedback(thread, progress))
     free(progress);
}

static void
_read_progress_cb(void *data,
                  Ecore_Thread *thread EINA_UNUSED,
                  void *msg_data)
{
   Ecordova_FileReader_Data *pd = data;
   DBG("(%p)", pd->obj);
   Ecordova_ProgressEvent *event = msg_data;
   eo_do(pd->obj,
         eo_event_callback_call(ECORDOVA_FILEREADER_EVENT_ON_PROGRESS, &event));
   free(event);
}

static void
_read_end_cb(void *data, Ecore_Thread *thread EINA_UNUSED)
{
   Ecordova_FileReader_Data *pd = data;
   DBG("(%p)", pd->obj);

   // If DONE (cancelled), then don't do anything
   if (ECORDOVA_FILEREADER_STATE_DONE == pd->state)
     return;
   pd->thread = NULL;
   pd->state = ECORDOVA_FILEREADER_STATE_DONE;

   if (pd->error)
     {
        Ecordova_ProgressEvent error = {.type = "error", .target = pd->obj};
        eo_do(pd->obj,
              eo_event_callback_call(ECORDOVA_FILEREADER_EVENT_ON_ERROR, &error));
     }
   else
     {
        Ecordova_ProgressEvent load = {.type = "load", .target = pd->obj};
        eo_do(pd->obj,
              eo_event_callback_call(ECORDOVA_FILEREADER_EVENT_ON_LOAD, &load));
     }

   Ecordova_ProgressEvent loadend = {.type = "loadend", .target = pd->obj};
   eo_do(pd->obj,
         eo_event_callback_call(ECORDOVA_FILEREADER_EVENT_ON_LOAD_END, &loadend));
}

static void
_read_abort_cb(void *data, Ecore_Thread *thread EINA_UNUSED)
{
   Ecordova_FileReader_Data *pd = data;
   DBG("(%p)", pd->obj);

   pd->thread = NULL;

   Ecordova_ProgressEvent on_abort = {.type = "abort", .target = pd->obj};
   eo_do(pd->obj, eo_event_callback_call(ECORDOVA_FILEREADER_EVENT_ON_ABORT, &on_abort));

   Ecordova_ProgressEvent loadend = {.type = "loadend", .target = pd->obj};
   eo_do(pd->obj, eo_event_callback_call(ECORDOVA_FILEREADER_EVENT_ON_LOAD_END, &loadend));
}

#undef EOAPI
#define EOAPI EAPI

#include "undefs.h"

#define ecordova_filereader_class_get ecordova_filereader_impl_class_get

#include "ecordova_filereader.eo.c"
