#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

# include "Ecore_Con.h"

#include "ecore_file_private.h"

#define ECORE_MAGIC_FILE_DOWNLOAD_JOB 0xf7427cb8
#define ECORE_FILE_DOWNLOAD_TIMEOUT 30

struct _Ecore_File_Download_Job
{
   ECORE_MAGIC;

   Ecore_Con_Url        *url_con;
   FILE                 *file;

   char                 *dst;

   Ecore_File_Download_Completion_Cb completion_cb;
   Ecore_File_Download_Progress_Cb progress_cb;
};

Ecore_File_Download_Job *_ecore_file_download_curl(const char *url, const char *dst,
                                                   Ecore_File_Download_Completion_Cb completion_cb,
                                                   Ecore_File_Download_Progress_Cb progress_cb,
                                                   void *data,
                                                   Eina_Hash *headers);

static Eina_Bool _ecore_file_download_url_complete_cb(void *data, int type, void *event);
static Eina_Bool _ecore_file_download_url_progress_cb(void *data, int type, void *event);

static Ecore_Event_Handler *_url_complete_handler = NULL;
static Ecore_Event_Handler *_url_progress_download = NULL;
static Eina_List           *_job_list;

static int download_init = 0;

int
ecore_file_download_init(void)
{
   download_init++;
   if (download_init > 1) return 1;
   if (!ecore_con_init()) return 0;
   if (!ecore_con_url_init())
     {
        ecore_con_shutdown();
        return 0;
     }
   _url_complete_handler = ecore_event_handler_add(ECORE_CON_EVENT_URL_COMPLETE, _ecore_file_download_url_complete_cb, NULL);
   _url_progress_download = ecore_event_handler_add(ECORE_CON_EVENT_URL_PROGRESS, _ecore_file_download_url_progress_cb, NULL);
   return 1;
}

void
ecore_file_download_shutdown(void)
{
   download_init--;
   if (download_init > 0) return;
   if (_url_complete_handler)
     ecore_event_handler_del(_url_complete_handler);
   if (_url_progress_download)
     ecore_event_handler_del(_url_progress_download);
   _url_complete_handler = NULL;
   _url_progress_download = NULL;
   ecore_file_download_abort_all();
   ecore_con_url_shutdown();
   ecore_con_shutdown();
}

static Eina_Bool
_ecore_file_download_headers_foreach_cb(const Eina_Hash *hash EINA_UNUSED, const void *key, void *data, void *fdata)
{
   Ecore_File_Download_Job *job = fdata;
   ecore_con_url_additional_header_add(job->url_con, key, data);

   return EINA_TRUE;
}

static Eina_Bool
_ecore_file_download(const char *url,
                     const char *dst,
                     Ecore_File_Download_Completion_Cb completion_cb,
                     Ecore_File_Download_Progress_Cb progress_cb,
                     void *data,
                     Ecore_File_Download_Job **job_ret,
                     Eina_Hash *headers)
{
   if (!url)
     {
        CRI("Download URL is null");
        return EINA_FALSE;
     }

   char *dir = ecore_file_dir_get(dst);

   if (!ecore_file_is_dir(dir))
     {
        ERR("%s is not a directory", dir);
        free(dir);
        return EINA_FALSE;
     }
   free(dir);
   if (ecore_file_exists(dst))
     {
        WRN("%s already exists", dst);
        return EINA_FALSE;
     }

   if (!strncmp(url, "file://", 7))
     {
        /* FIXME: Maybe fork? Might take a while to copy.
         * Check filesize? */
        /* Just copy it */

        url += 7;
        /* skip hostname */
        if ((url = strchr(url, '/')))
          return ecore_file_cp(url, dst);
        else
          return EINA_FALSE;
     }
   else if ((!strncmp(url, "http://", 7)) || (!strncmp(url, "https://", 8)) ||
            (!strncmp(url, "ftp://", 6)))
     {
        /* download */
        Ecore_File_Download_Job *job;

        job = _ecore_file_download_curl(url, dst, completion_cb, progress_cb, data, headers);
        if(job_ret) *job_ret = job;
        if(job)
          return EINA_TRUE;
        else
          {
             ERR("no job returned\n");
             return EINA_FALSE;
          }
     }
   else
     {
        return EINA_FALSE;
     }
}

EAPI Eina_Bool
ecore_file_download(const char *url,
                    const char *dst,
                    Ecore_File_Download_Completion_Cb completion_cb,
                    Ecore_File_Download_Progress_Cb progress_cb,
                    void *data,
                    Ecore_File_Download_Job **job_ret)
{
   return _ecore_file_download(url, dst, completion_cb, progress_cb, data, job_ret, NULL);
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
   return _ecore_file_download(url, dst, completion_cb, progress_cb, data, job_ret, headers);
}

EAPI Eina_Bool
ecore_file_download_protocol_available(const char *protocol)
{
   if (!strncmp(protocol, "file://", 7)) return EINA_TRUE;
   else if (!strncmp(protocol, "http://", 7)) return EINA_TRUE;
   else if (!strncmp(protocol, "ftp://", 6)) return EINA_TRUE;

   return EINA_FALSE;
}

static int
_ecore_file_download_url_compare_job(const void *data1, const void *data2)
{
   const Ecore_File_Download_Job *job = data1;
   const Ecore_Con_Url           *url = data2;

   if (job->url_con == url) return 0;
   return -1;
}

static Eina_Bool
_ecore_file_download_url_complete_cb(void *data EINA_UNUSED, int type EINA_UNUSED, void *event)
{
   Ecore_Con_Event_Url_Complete *ev = event;
   Ecore_File_Download_Job      *job;

   job = eina_list_search_unsorted(_job_list, _ecore_file_download_url_compare_job, ev->url_con);
   if (!ECORE_MAGIC_CHECK(job, ECORE_MAGIC_FILE_DOWNLOAD_JOB)) return ECORE_CALLBACK_PASS_ON;

   fclose(job->file);
   if (job->completion_cb)
     job->completion_cb(ecore_con_url_data_get(job->url_con), job->dst, ev->status);

   _job_list = eina_list_remove(_job_list, job);
   free(job->dst);
   ecore_con_url_free(job->url_con);
   free(job);

   return ECORE_CALLBACK_DONE;
}

static Eina_Bool
_ecore_file_download_url_progress_cb(void *data EINA_UNUSED, int type EINA_UNUSED, void *event)
{
/* this reports the downloads progress. if we return 0, then download
 * continues, if we return anything else, then the download stops */
   Ecore_Con_Event_Url_Progress *ev = event;
   Ecore_File_Download_Job      *job;

   job = eina_list_search_unsorted(_job_list, _ecore_file_download_url_compare_job, ev->url_con);
   if (!ECORE_MAGIC_CHECK(job, ECORE_MAGIC_FILE_DOWNLOAD_JOB)) return ECORE_CALLBACK_PASS_ON;

   if (job->progress_cb)
     if (job->progress_cb(ecore_con_url_data_get(job->url_con), job->dst,
                          (long int) ev->down.total, (long int) ev->down.now,
                          (long int) ev->up.total, (long int) ev->up.now) != 0)
       {
          _job_list = eina_list_remove(_job_list, job);
          fclose(job->file);
          free(job->dst);
          free(job);

          return ECORE_CALLBACK_PASS_ON;
       }

   return ECORE_CALLBACK_DONE;
}

Ecore_File_Download_Job *
_ecore_file_download_curl(const char *url, const char *dst,
                          Ecore_File_Download_Completion_Cb completion_cb,
                          Ecore_File_Download_Progress_Cb progress_cb,
                          void *data,
                          Eina_Hash *headers)
{
   Ecore_File_Download_Job *job;

   job = calloc(1, sizeof(Ecore_File_Download_Job));
   if (!job) return NULL;

   ECORE_MAGIC_SET(job, ECORE_MAGIC_FILE_DOWNLOAD_JOB);

   job->file = fopen(dst, "wb");
   if (!job->file)
     {
        free(job);
        return NULL;
     }
   job->url_con = ecore_con_url_new(url);
   if (!job->url_con)
     {
        fclose(job->file);
        free(job);
        return NULL;
     }

   if (headers) eina_hash_foreach(headers, _ecore_file_download_headers_foreach_cb, job);
   ecore_con_url_fd_set(job->url_con, fileno(job->file));
   ecore_con_url_data_set(job->url_con, data);

   job->dst = strdup(dst);

   job->completion_cb = completion_cb;
   job->progress_cb = progress_cb;
   _job_list = eina_list_append(_job_list, job);

   if (!ecore_con_url_get(job->url_con))
     {
        ecore_con_url_free(job->url_con);
        _job_list = eina_list_remove(_job_list, job);
        fclose(job->file);
        ecore_file_remove(job->dst);
        free(job->dst);
        free(job);
        return NULL;
     }

   return job;
}

EAPI void
ecore_file_download_abort(Ecore_File_Download_Job *job)
{
   if (!job)
     return;

   if (job->completion_cb)
     job->completion_cb(ecore_con_url_data_get(job->url_con), job->dst, 1);
   ecore_con_url_free(job->url_con);
   _job_list = eina_list_remove(_job_list, job);
   fclose(job->file);
   free(job->dst);
   free(job);
}

EAPI void
ecore_file_download_abort_all(void)
{
   Ecore_File_Download_Job *job;

   EINA_LIST_FREE(_job_list, job)
             ecore_file_download_abort(job);
}
