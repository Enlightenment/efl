#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "ecordova_filetransfer_private.h"
#include "ecordova_directoryentry_private.h"

#include <Ecore_Con.h>

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

#define MY_CLASS ECORDOVA_FILETRANSFER_CLASS
#define MY_CLASS_NAME "Ecordova_FileTransfer"

struct _Ecordova_FileTransfer_Job
{
   char                                *source;
   char                                *target;
   Ecore_File_Download_Job             *download;
   Ecore_Thread                        *upload;
   int                                  upload_error;
   char                                *upload_buffer;
   long int                             upload_length;
   Ecore_Con_Url                       *upload_con_url;
   Ecore_Event_Handler                 *progress_event_hanbler;
   Ecore_Event_Handler                 *complete_event_hanbler;
   Ecordova_FileTransfer_UploadOptions upload_options;
};

static int _download_progress_cb(void *, const char *, long int, long int, long int, long int);
static void _download_completion_cb(void *, const char *, int);
static void _clear(Ecordova_FileTransfer_Data *);
static void _abort_error_notify(Ecordova_FileTransfer_Data *);
static void _status_error_notify(Ecordova_FileTransfer_Data *, int);
static void _file_error_notify(Ecordova_FileTransfer_Data *, int);
static void _connection_error_notify(Ecordova_FileTransfer_Data *);
static void _already_in_progress_error_notify(Ecordova_FileTransfer_Data *, const char *, const char *);
static void _upload_cb(void *, Ecore_Thread *);
static void _upload_progress_notify(size_t, size_t, Eo *, Ecore_Thread *);
static void _upload_progress_cb(void *, Ecore_Thread *, void *);
static void _upload_end_cb(void *, Ecore_Thread *);
static void _upload_abort_cb(void *, Ecore_Thread *);
static void _progress_notify(Ecordova_FileTransfer_Data *, Ecordova_ProgressEvent *);
static Ecordova_FileTransfer_Job *_job_new(const char *, const char *);
static void _job_free(Ecordova_FileTransfer_Job **);
static Eina_Bool _url_progress_cb(void *, int, void *);
static Eina_Bool _url_complete_cb(void *, int, void *);

static Eo_Base *
_ecordova_filetransfer_eo_base_constructor(Eo *obj,
                                           Ecordova_FileTransfer_Data *pd)
{
   DBG("(%p)", obj);

   pd->obj = obj;
   pd->job = NULL;

   eo_do_super(obj, MY_CLASS, obj = eo_constructor());
   eo_do(obj, ecordova_entry_file_is_set(EINA_TRUE));
   return obj;
}

static void
_ecordova_filetransfer_eo_base_destructor(Eo *obj,
                                          Ecordova_FileTransfer_Data *pd)
{
   DBG("(%p)", obj);

   _clear(pd);

   eo_do_super(obj, MY_CLASS, eo_destructor());
}

static void
_ecordova_filetransfer_upload(Eo *obj EINA_UNUSED,
                              Ecordova_FileTransfer_Data *pd EINA_UNUSED,
                              const char *file_url EINA_UNUSED,
                              const char *server,
                              Ecordova_FileTransfer_UploadOptions *options EINA_UNUSED,
                              Eina_Bool trust_all_hosts EINA_UNUSED)
{
   DBG("(%p)", obj);
   EINA_SAFETY_ON_NULL_RETURN(file_url);
   EINA_SAFETY_ON_NULL_RETURN(server);

   if (pd->job)
     {
        _already_in_progress_error_notify(pd, file_url, server);
        return;
     }

   bool is_http = strncmp(server, "http://", 7) == 0;
   bool is_https = strncmp(server, "https://", 8) == 0;

   if (!is_http && !is_https)
     {
        ERR("%s", "Invalid server address");
        Ecordova_FileTransfer_Error error = {
          .code = ECORDOVA_FILETRANSFER_ERRORCODE_INVALID_URL_ERR,
          .source = file_url,
          .target = server,
          .http_status = 0,
          .body = NULL,
          .exception = "Invalid server address"
        };
        eo_do(pd->obj,
              eo_event_callback_call(ECORDOVA_FILETRANSFER_EVENT_ERROR, &error));
        return;
     }

   pd->job = _job_new(file_url, server);
   pd->job->upload = ecore_thread_feedback_run(_upload_cb,
                                               _upload_progress_cb,
                                               _upload_end_cb,
                                               _upload_abort_cb,
                                               pd,
                                               EINA_FALSE);
}

static void
_ecordova_filetransfer_download(Eo *obj,
                                Ecordova_FileTransfer_Data *pd,
                                const char *source,
                                const char *target,
                                Eina_Bool trust_all_hosts EINA_UNUSED,
                                Eina_Hash *options)
{
   DBG("(%p)", obj);
   EINA_SAFETY_ON_NULL_RETURN(source);
   EINA_SAFETY_ON_NULL_RETURN(target);

   if (pd->job)
     {
        _already_in_progress_error_notify(pd, source, target);
        return;
     }

   Ecordova_FileTransfer_Job *job = _job_new(source, target);

   Eina_Bool ret;
   if (options)
     ret = ecore_file_download_full(source,
                                    target,
                                    _download_completion_cb,
                                    _download_progress_cb,
                                    pd,
                                    &job->download,
                                    options);
   else
      ret = ecore_file_download(source,
                                target,
                                _download_completion_cb,
                                _download_progress_cb,
                                pd,
                                &job->download);

   if (!ret)
     {
        _job_free(&job);
        ERR("%s", "An error occurred downloading the file");
        Ecordova_FileTransfer_Error error = {
          .code = ECORDOVA_FILETRANSFER_ERRORCODE_ABORT_ERR,
          .source = source,
          .target = target,
          .http_status = 0,
          .body = NULL,
          .exception = "An error occurred downloading the file"
        };
        eo_do(obj, eo_event_callback_call(ECORDOVA_FILETRANSFER_EVENT_ERROR, &error));
        return;
     }

   pd->job = job;
}

static void
_ecordova_filetransfer_abort(Eo *obj, Ecordova_FileTransfer_Data *pd)
{
   DBG("(%p)", obj);

   if (!pd->job) return;

   if (pd->job->download)
     ecore_file_download_abort(pd->job->download);
   else
     ecore_thread_cancel(pd->job->upload);
}

static int
_download_progress_cb(void *data,
                      const char *file EINA_UNUSED,
                      long int dltotal,
                      long int dlnow,
                      long int ultotal EINA_UNUSED,
                      long int ulnow EINA_UNUSED)
{
   Ecordova_FileTransfer_Data *pd = data;
   DBG("(%p)", pd->obj);
   Ecordova_ProgressEvent event = {
      .type = "download",
      .cancelable = EINA_TRUE,
      .length_computable = EINA_TRUE,
      .loaded = dlnow,
      .total = dltotal,
      .target = pd->obj
   };
   _progress_notify(pd, &event);
   return ECORE_FILE_PROGRESS_CONTINUE;
}

static void
_download_completion_cb(void *data, const char *file, int status)
{
   Ecordova_FileTransfer_Data *pd = data;
   DBG("(%p)", pd->obj);
   if (1 == status)
     _abort_error_notify(pd);
   else if (200 != status)
     _status_error_notify(pd, status);
   else
     {
        char *path, *name, *native;
        split_path(NULL, file, &path, &name, &native);

        Ecordova_FileEntry *file_entry = eo_add(ECORDOVA_FILEENTRY_CLASS, NULL,
                                                ecordova_entry_name_set(name),
                                                ecordova_entry_path_set(path));
        eo_do(pd->obj,
              eo_event_callback_call(ECORDOVA_FILETRANSFER_EVENT_DOWNLOAD_SUCCESS, file_entry));
        eo_unref(file_entry);

        free(path);
        free(name);
        free(native);
     }

   _clear(pd);
}

static void
_clear(Ecordova_FileTransfer_Data *pd)
{
   DBG("(%p)", pd->obj);
   _job_free(&pd->job);
}

static void
_upload_cb(void *data, Ecore_Thread *thread)
{
   Ecordova_FileTransfer_Data *pd = data;
   DBG("(%p)", pd->obj);

   if (ecore_thread_check(thread))
     {
        pd->job->upload_error = 1;
        return;
     }

   FILE *stream = fopen(pd->job->source, "rb");
   if (!stream)
     {
        pd->job->upload_error = errno;
        return;
     }

   int error = fseek(stream, 0L, SEEK_END);
   if (error)
     {
        pd->job->upload_error = errno;
        goto on_error;
     }

   pd->job->upload_length = ftell(stream);
   if (pd->job->upload_length < 0)
     {
        pd->job->upload_error = errno;
        goto on_error;
     }

   error = fseek(stream, 0L, SEEK_SET);
     {
        pd->job->upload_error = errno;
        goto on_error;
     }

   _upload_progress_notify(0, pd->job->upload_length, pd->obj, thread);

   pd->job->upload_buffer = malloc(pd->job->upload_length);
   if (!pd->job->upload_buffer)
     {
        pd->job->upload_error = errno;
        goto on_error;
     }

   long int total_read = 0;
   while (total_read < pd->job->upload_length)
     {
        size_t read = fread(&pd->job->upload_buffer[total_read],
                            sizeof(char),
                            pd->job->upload_length - total_read,
                            stream);
        total_read += read;

        _upload_progress_notify(0, pd->job->upload_length, pd->obj, thread);
        if (ecore_thread_check(thread))
          {
             pd->job->upload_error = 1;
             break;
          }
     }

on_error:
   fclose(stream);
}

static void
_upload_progress_notify(size_t uploaded,
                        size_t total,
                        Eo *obj,
                        Ecore_Thread *thread)
{
   DBG("(%p)", obj);

   Ecordova_ProgressEvent *progress = malloc(sizeof(Ecordova_ProgressEvent));
   *progress = (Ecordova_ProgressEvent)
     {
        .type = "upload",
        .length_computable = EINA_TRUE,
        .loaded = uploaded,
        .total = total,
        .target = obj
     };
   if (!ecore_thread_feedback(thread, progress))
     free(progress);
}

static void
_upload_progress_cb(void *data,
                    Ecore_Thread *thread EINA_UNUSED,
                    void *msg_data)
{
   Ecordova_FileTransfer_Data *pd = data;
   DBG("(%p)", pd->obj);
   Ecordova_ProgressEvent *event = msg_data;
   _progress_notify(pd, event);
   free(event);
}

static void
_upload_end_cb(void *data, Ecore_Thread *thread EINA_UNUSED)
{
   Ecordova_FileTransfer_Data *pd = data;
   DBG("(%p)", pd->obj);

   if (1 == pd->job->upload_error)
     _abort_error_notify(pd);
   else if (pd->job->upload_error)
     _file_error_notify(pd, pd->job->upload_error);
   else
     {
        pd->job->upload_con_url = ecore_con_url_custom_new(pd->job->target,
                                                           pd->job->upload_options.http_method);
        if (!pd->job->upload_con_url)
          {
             _connection_error_notify(pd);
             goto on_error;
          }

        pd->job->complete_event_hanbler = ecore_event_handler_add(ECORE_CON_EVENT_URL_COMPLETE, _url_complete_cb, pd);
        pd->job->progress_event_hanbler = ecore_event_handler_add(ECORE_CON_EVENT_URL_PROGRESS, _url_progress_cb, pd);
        if (!pd->job->complete_event_hanbler || !pd->job->progress_event_hanbler)
          {
             _connection_error_notify(pd);
             goto on_error;
          }

        if (pd->job->upload_options.headers)
          {
             Eina_Iterator *it = eina_hash_iterator_tuple_new(pd->job->upload_options.headers);
             Eina_Hash_Tuple *tuple;
             EINA_ITERATOR_FOREACH(it, tuple)
               ecore_con_url_additional_header_add(pd->job->upload_con_url, tuple->key, tuple->data);
             eina_iterator_free(it);
          }

        Eina_Bool ret = ecore_con_url_post(pd->job->upload_con_url,
                                           pd->job->upload_buffer,
                                           pd->job->upload_length,
                                           pd->job->upload_options.mime_type);
        if (!ret)
          {
             _connection_error_notify(pd);
             goto on_error;
          }

        return;
     }

on_error:
   _clear(pd);
}

static void
_upload_abort_cb(void *data, Ecore_Thread *thread EINA_UNUSED)
{
   Ecordova_FileTransfer_Data *pd = data;
   DBG("(%p)", pd->obj);

   _abort_error_notify(pd);
   _clear(pd);
}

static void
_progress_notify(Ecordova_FileTransfer_Data *pd,
                 Ecordova_ProgressEvent *event)
{
   eo_do(pd->obj,
         eo_event_callback_call(ECORDOVA_FILETRANSFER_EVENT_ON_PROGRESS, event));
}

static Ecordova_FileTransfer_Job *
_job_new(const char *source, const char *target)
{
   Ecordova_FileTransfer_Job *job = calloc(1, sizeof(Ecordova_FileTransfer_Job));
   job->source = strdup(source);
   job->target = strdup(target);
   job->upload_options = (Ecordova_FileTransfer_UploadOptions){
     .file_key = "file",
     .file_name = "image.jpg",
     .http_method = "POST",
     .mime_type = "image/jpeg",
     .chunked_mode = EINA_TRUE
   };
   return job;
}

static void
_job_free(Ecordova_FileTransfer_Job **job)
{
   if (!*job) return;

   if ((*job)->upload_con_url)
     {
        if ((*job)->progress_event_hanbler)
          ecore_event_handler_del((*job)->progress_event_hanbler);
        if ((*job)->complete_event_hanbler)
          ecore_event_handler_del((*job)->complete_event_hanbler);
        ecore_con_url_free((*job)->upload_con_url);
     }
   free((*job)->upload_buffer);
   free((*job)->source);
   free((*job)->target);
   free(*job);
   *job = NULL;
}

static Eina_Bool
_url_progress_cb(void *data EINA_UNUSED, int type EINA_UNUSED, void *event_info)
{
   Ecore_Con_Event_Url_Progress *url_progress = event_info;
   Ecordova_FileTransfer_Data *pd = data;

   Ecordova_ProgressEvent event = {
      .type = "upload",
      .cancelable = EINA_FALSE,
      .length_computable = EINA_TRUE,
      .loaded = url_progress->up.now,
      .total = url_progress->up.total,
      .target = pd->obj
   };
   _progress_notify(pd, &event);

   return EINA_TRUE;
}

static Eina_Bool
_url_complete_cb(void *data, int type EINA_UNUSED, void *event_info)
{
   Ecore_Con_Event_Url_Complete *url_complete = event_info;
   Ecordova_FileTransfer_Data *pd = data;

   if (200 != url_complete->status)
     _status_error_notify(pd, url_complete->status);
   else
     {
        Eina_Hash *headers_hash = NULL;
        const Eina_List *headers = ecore_con_url_response_headers_get(url_complete->url_con);
        if (eina_list_count(headers))
          {
             headers_hash = eina_hash_string_superfast_new(free);
             const char *header_line;
             const Eina_List *it;
             EINA_LIST_FOREACH(headers, it, header_line)
               {
                  const char *separator = strchr(header_line, ':');
                  if (!separator) continue;

                  size_t key_len = separator - header_line + 1;
                  char key[key_len];
                  strncpy(key, header_line, key_len);
                  key[key_len - 1] = '\0';

                  if (*(++separator) == ' ')
                    ++separator;
                  char *value = strdup(separator);
                  eina_hash_add(headers_hash, key, value);
               }
          }

        Ecordova_FileTransfer_UploadResult result = {
          .bytes_sent = pd->job->upload_length,
          .response_code = url_complete->status,
          .headers = headers_hash,
          .response = NULL // TODO: Get the HTTP response
        };
        eo_do(pd->obj,
              eo_event_callback_call(ECORDOVA_FILETRANSFER_EVENT_UPLOAD_SUCCESS,
                                     &result));
        if (headers_hash)
          eina_hash_free(headers_hash);
     }

   _clear(pd);
   return EINA_TRUE;
}

static void
_abort_error_notify(Ecordova_FileTransfer_Data *pd)
{
   INF("%s", "Aborted");
   Ecordova_FileTransfer_Error error = {
     .code = ECORDOVA_FILETRANSFER_ERRORCODE_ABORT_ERR,
     .source = pd->job->source,
     .target = pd->job->target,
     .http_status = 0,
     .body = NULL,
     .exception = "Aborted"
   };
   eo_do(pd->obj,
         eo_event_callback_call(ECORDOVA_FILETRANSFER_EVENT_ERROR, &error));
}

static void
_status_error_notify(Ecordova_FileTransfer_Data *pd, int status)
{
   ERR("Error status: %d", status);
   Ecordova_FileTransfer_Error error = {
     // TODO: translate other errors checking first which protocol it is.
     .code = 404 == status ? ECORDOVA_FILETRANSFER_ERRORCODE_FILE_NOT_FOUND_ERR :
                             ECORDOVA_FILETRANSFER_ERRORCODE_ABORT_ERR,
     .source = pd->job->source,
     .target = pd->job->target,
     .http_status = status,
     .body = NULL,
     .exception = "Error"
   };
   eo_do(pd->obj,
         eo_event_callback_call(ECORDOVA_FILETRANSFER_EVENT_ERROR, &error));
}

static void
_file_error_notify(Ecordova_FileTransfer_Data *pd, int code)
{
   ERR("Error code: %d", code);
   Ecordova_FileTransfer_Error error = {
     .code = ECORDOVA_FILETRANSFER_ERRORCODE_FILE_NOT_FOUND_ERR,
     .source = pd->job->source,
     .target = pd->job->target,
     .http_status = 0,
     .body = NULL,
     .exception = "Internal error"
   };
   eo_do(pd->obj,
         eo_event_callback_call(ECORDOVA_FILETRANSFER_EVENT_ERROR, &error));
}

static void
_connection_error_notify(Ecordova_FileTransfer_Data *pd)
{
   ERR("%s", "Connection error");
   Ecordova_FileTransfer_Error error = {
     .code = ECORDOVA_FILETRANSFER_ERRORCODE_CONNECTION_ERR,
     .source = pd->job->source,
     .target = pd->job->target,
     .http_status = 0,
     .body = NULL,
     .exception = "Connection error"
   };
   eo_do(pd->obj,
         eo_event_callback_call(ECORDOVA_FILETRANSFER_EVENT_ERROR, &error));
}

static void
_already_in_progress_error_notify(Ecordova_FileTransfer_Data *pd,
                                  const char *source,
                                  const char *target)
{
   ERR("%s", "A job is already in progress");
   Ecordova_FileTransfer_Error error = {
     .code = ECORDOVA_FILETRANSFER_ERRORCODE_ABORT_ERR,
     .source = source,
     .target = target,
     .http_status = 0,
     .body = NULL,
     .exception = "A job is already in progress"
   };
   eo_do(pd->obj,
         eo_event_callback_call(ECORDOVA_FILETRANSFER_EVENT_ERROR, &error));
}

#undef EOAPI
#define EOAPI EAPI

#include "undefs.h"

#define ecordova_filetransfer_class_get ecordova_filetransfer_impl_class_get

#include "ecordova_filetransfer.eo.c"
