#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>

# include "Ecore_Con.h"

#include "ecore_file_private.h"

#define ECORE_MAGIC_FILE_DOWNLOAD_JOB 0xf7427cb8
#define ECORE_FILE_DOWNLOAD_TIMEOUT 30

struct _Ecore_File_Download_Job
{
   ECORE_MAGIC;

   Eo                   *input;
   Eo                   *output;
   Eo                   *copier;

   Ecore_File_Download_Completion_Cb completion_cb;
   Ecore_File_Download_Progress_Cb progress_cb;
   const void *data;
};

static Eina_List           *_job_list;
static int download_init = 0;

int
ecore_file_download_init(void)
{
   download_init++;
   if (download_init > 1) return 1;
   if (!ecore_con_init())
     {
        download_init--;
        return 0;
     }
   if (!ecore_con_url_init())
     {
        ecore_con_shutdown();
        download_init--;
        return 0;
     }
   return download_init;
}

void
ecore_file_download_shutdown(void)
{
   download_init--;
   if (download_init > 0) return;
   ecore_file_download_abort_all();
   ecore_con_url_shutdown();
   ecore_con_shutdown();
}

static void
_ecore_file_download_copier_done(void *data, const Efl_Event *event EINA_UNUSED)
{
   Ecore_File_Download_Job *job = data;
   Efl_Net_Http_Status status = efl_net_dialer_http_response_status_get(job->input);
   const char *file;

   file = efl_file_get(job->output);

   DBG("Finished downloading %s (status=%d) -> %s",
       efl_net_dialer_address_dial_get(job->input),
       status,
       file);

   if (job->completion_cb)
     {
        Ecore_File_Download_Completion_Cb cb = job->completion_cb;
        job->completion_cb = NULL;
        ECORE_MAGIC_SET(job, ECORE_MAGIC_NONE);
        cb((void *)job->data, file, status);
     }

   efl_del(job->copier);
}

static void
_ecore_file_download_copier_error(void *data, const Efl_Event *event)
{
   Ecore_File_Download_Job *job = data;
   Efl_Net_Http_Status status = efl_net_dialer_http_response_status_get(job->input);
   Eina_Error *perr = event->info;
   const char *file;

   file = efl_file_get(job->output);

   WRN("Failed downloading %s (status=%d) -> %s: %s",
       efl_net_dialer_address_dial_get(job->input),
       efl_net_dialer_http_response_status_get(job->input),
       file,
       eina_error_msg_get(*perr));

   if (job->completion_cb)
     {
        Ecore_File_Download_Completion_Cb cb = job->completion_cb;
        job->completion_cb = NULL;

        if ((status < 500) || ((int)status > 599))
          {
             /* not an HTTP error, likely copier or output, use 500 */
             status = 500;
          }

        cb((void *)job->data, file, status);
     }

   efl_del(job->copier);
}

static void
_ecore_file_download_copier_progress(void *data, const Efl_Event *event EINA_UNUSED)
{
   Ecore_File_Download_Job *job = data;
   Ecore_File_Progress_Return ret;
   uint64_t dn, dt, un, ut;
   const char *file;

   if (!job->progress_cb) return;

   file = efl_file_get(job->output);
   efl_net_dialer_http_progress_download_get(job->input, &dn, &dt);
   efl_net_dialer_http_progress_upload_get(job->input, &un, &ut);
   ret = job->progress_cb((void *)job->data, file, dt, dn, ut, un);

   if (ret == ECORE_FILE_PROGRESS_CONTINUE) return;

   ecore_file_download_abort(job);
}

static void
_ecore_file_download_copier_del(void *data, const Efl_Event *event EINA_UNUSED)
{
   Ecore_File_Download_Job *job = data;

   ECORE_MAGIC_SET(job, ECORE_MAGIC_NONE);

   efl_del(job->input);
   job->input = NULL;

   efl_del(job->output);
   job->output = NULL;

   job->completion_cb = NULL;
   job->progress_cb = NULL;
   job->data = NULL;

   _job_list = eina_list_remove(_job_list, job);
   free(job);
}

EFL_CALLBACKS_ARRAY_DEFINE(ecore_file_download_copier_cbs,
                           { EFL_IO_COPIER_EVENT_DONE, _ecore_file_download_copier_done },
                           { EFL_IO_COPIER_EVENT_ERROR, _ecore_file_download_copier_error },
                           { EFL_IO_COPIER_EVENT_PROGRESS, _ecore_file_download_copier_progress },
                           { EFL_EVENT_DEL, _ecore_file_download_copier_del });

static Eina_Bool
_ecore_file_download_headers_foreach_cb(const Eina_Hash *hash EINA_UNUSED, const void *key, void *data, void *fdata)
{
   Ecore_File_Download_Job *job = fdata;

   efl_net_dialer_http_request_header_add(job->input, key, data);

   return EINA_TRUE;
}

EAPI Eina_Bool
ecore_file_download_full(const char *url,
                         const char *dst,
                         Ecore_File_Download_Completion_Cb completion_cb,
                         Ecore_File_Download_Progress_Cb progress_cb,
                         void *data,
                         Ecore_File_Download_Job **job_ret,
                         Eina_Hash *headers)
{
   Ecore_File_Download_Job *job;
   Eina_Error err;
   Eo *loop;
   char *dir;

   if (job_ret) *job_ret = NULL;
   if (!url)
     {
        CRI("Download URL is null");
        return EINA_FALSE;
     }

   if (!strstr(url, "://"))
     {
        ERR("'%s' is not an URL, missing protocol://", url);
        return EINA_FALSE;
     }

   dir = ecore_file_dir_get(dst);
   if (!ecore_file_is_dir(dir))
     {
        ERR("%s is not a directory", dir);
        free(dir);
        return EINA_FALSE;
     }
   free(dir);
   if (ecore_file_exists(dst))
     {
        ERR("%s already exists", dst);
        return EINA_FALSE;
     }

   loop = efl_main_loop_get();
   EINA_SAFETY_ON_NULL_RETURN_VAL(loop, EINA_FALSE);

   job = calloc(1, sizeof(Ecore_File_Download_Job));
   EINA_SAFETY_ON_NULL_RETURN_VAL(job, EINA_FALSE);
   ECORE_MAGIC_SET(job, ECORE_MAGIC_FILE_DOWNLOAD_JOB);

   job->input = efl_add(EFL_NET_DIALER_HTTP_CLASS, loop,
                        efl_net_dialer_http_allow_redirects_set(efl_added, EINA_TRUE));
   EINA_SAFETY_ON_NULL_GOTO(job->input, error_input);

   job->output = efl_add(EFL_IO_FILE_CLASS, loop,
                         efl_file_set(efl_added, dst),
                         efl_io_file_flags_set(efl_added, O_WRONLY | O_CREAT),
                         efl_io_closer_close_on_exec_set(efl_added, EINA_TRUE),
                         efl_io_closer_close_on_invalidate_set(efl_added, EINA_TRUE),
                         efl_io_file_mode_set(efl_added, 0644));
   EINA_SAFETY_ON_NULL_GOTO(job->output, error_output);

   job->copier = efl_add(EFL_IO_COPIER_CLASS, loop,
                         efl_io_copier_source_set(efl_added, job->input),
                         efl_io_copier_destination_set(efl_added, job->output),
                         efl_io_closer_close_on_invalidate_set(efl_added, EINA_TRUE),
                         efl_event_callback_array_add(efl_added, ecore_file_download_copier_cbs(), job));
   EINA_SAFETY_ON_NULL_GOTO(job->copier, error_copier);

   _job_list = eina_list_append(_job_list, job);

   if (headers)
     eina_hash_foreach(headers, _ecore_file_download_headers_foreach_cb, job);

   job->completion_cb = completion_cb;
   job->progress_cb = progress_cb;
   job->data = data;

   err = efl_net_dialer_dial(job->input, url);
   if (err)
     {
        ERR("Could not download %s: %s", url, eina_error_msg_get(err));
        goto error_dial;
     }

   if (job_ret) *job_ret = job;
   return EINA_TRUE;

 error_dial:
   efl_del(job->copier);
   return EINA_FALSE; /* copier's "del" event will delete everything else */

 error_copier:
   efl_del(job->output);
 error_output:
   efl_del(job->input);
 error_input:
   ECORE_MAGIC_SET(job, ECORE_MAGIC_NONE);
   free(job);
   return EINA_FALSE;
}

EAPI Eina_Bool
ecore_file_download(const char *url,
                    const char *dst,
                    Ecore_File_Download_Completion_Cb completion_cb,
                    Ecore_File_Download_Progress_Cb progress_cb,
                    void *data,
                    Ecore_File_Download_Job **job_ret)
{
   return ecore_file_download_full(url, dst, completion_cb, progress_cb, data, job_ret, NULL);
}

EAPI Eina_Bool
ecore_file_download_protocol_available(const char *protocol)
{
   if (!strncmp(protocol, "file://", 7)) return EINA_TRUE;
   else if (!strncmp(protocol, "http://", 7)) return EINA_TRUE;
   else if (!strncmp(protocol, "https://", 8)) return EINA_TRUE;
   else if (!strncmp(protocol, "ftp://", 6)) return EINA_TRUE;

   return EINA_FALSE;
}

EAPI void
ecore_file_download_abort(Ecore_File_Download_Job *job)
{
   const char *file;

   if (!job)
     return;
   if (!ECORE_MAGIC_CHECK(job, ECORE_MAGIC_FILE_DOWNLOAD_JOB))
     {
        ECORE_MAGIC_FAIL(job, ECORE_MAGIC_FILE_DOWNLOAD_JOB, __FUNCTION__);
        return;
     }

   file = efl_file_get(job->output);
   DBG("Aborting download %s -> %s",
       efl_net_dialer_address_dial_get(job->input),
       file);

   /* abort should have status = 1 */
   if (job->completion_cb)
     {
        Ecore_File_Download_Completion_Cb cb = job->completion_cb;
        job->completion_cb = NULL;
        ECORE_MAGIC_SET(job, ECORE_MAGIC_NONE);
        cb((void *)job->data, file, 1);
     }

   /* efl_io_closer_close()
    *  -> _ecore_file_download_copier_done()
    *       -> efl_del()
    *           -> _ecore_file_download_copier_del()
    */
   efl_io_closer_close(job->copier);
}

EAPI void
ecore_file_download_abort_all(void)
{
   Ecore_File_Download_Job *job;

   EINA_LIST_FREE(_job_list, job)
             ecore_file_download_abort(job);
}
