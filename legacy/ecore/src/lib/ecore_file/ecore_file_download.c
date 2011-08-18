#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdio.h>
#include <string.h>

#ifdef BUILD_ECORE_CON
# include "Ecore_Con.h"
#endif

#include "ecore_file_private.h"

#ifdef BUILD_ECORE_CON

#define ECORE_MAGIC_FILE_DOWNLOAD_JOB 0xf7427cb8

struct _Ecore_File_Download_Job
{
   ECORE_MAGIC;

   Ecore_Con_Url        *url_con;
   FILE                 *file;

   char                 *dst;

   Ecore_File_Download_Completion_Cb completion_cb;
   Ecore_File_Download_Progress_Cb progress_cb;
};

#ifdef HAVE_CURL
Ecore_File_Download_Job *_ecore_file_download_curl(const char *url, const char *dst,
                                                   Ecore_File_Download_Completion_Cb completion_cb,
                                                   Ecore_File_Download_Progress_Cb progress_cb,
                                                   void *data,
                                                   Eina_Hash *headers);

static Eina_Bool _ecore_file_download_url_complete_cb(void *data, int type, void *event);
static Eina_Bool _ecore_file_download_url_progress_cb(void *data, int type, void *event);
#endif

static Ecore_Event_Handler *_url_complete_handler = NULL;
static Ecore_Event_Handler *_url_progress_download = NULL;
static Eina_List           *_job_list;

#endif /* BUILD_ECORE_CON */

int
ecore_file_download_init(void)
{
#ifdef BUILD_ECORE_CON
  if (!ecore_con_url_init())
    return 0;

# ifdef HAVE_CURL
  _url_complete_handler = ecore_event_handler_add(ECORE_CON_EVENT_URL_COMPLETE, _ecore_file_download_url_complete_cb, NULL);
  _url_progress_download = ecore_event_handler_add(ECORE_CON_EVENT_URL_PROGRESS, _ecore_file_download_url_progress_cb, NULL);
# endif

#endif /* BUILD_ECORE_CON */

   return 1;
}

void
ecore_file_download_shutdown(void)
{
#ifdef BUILD_ECORE_CON
  if (_url_complete_handler)
    ecore_event_handler_del(_url_complete_handler);
  if (_url_progress_download)
    ecore_event_handler_del(_url_progress_download);
  _url_complete_handler = NULL;
  _url_progress_download = NULL;
  ecore_file_download_abort_all();

  ecore_con_url_shutdown();
#endif /* BUILD_ECORE_CON */
}

#ifdef BUILD_ECORE_CON
# ifdef HAVE_CURL
static Eina_Bool
_ecore_file_download_headers_foreach_cb(const Eina_Hash *hash __UNUSED__, const void *key, void *data, void *fdata)
{
   Ecore_File_Download_Job *job = fdata;
   ecore_con_url_additional_header_add(job->url_con, key, data);

   return EINA_TRUE;
}
# endif
#endif

static Eina_Bool
_ecore_file_download(const char *url,
                     const char *dst,
                     Ecore_File_Download_Completion_Cb completion_cb,
                     Ecore_File_Download_Progress_Cb progress_cb,
                     void *data,
                     Ecore_File_Download_Job **job_ret,
                     Eina_Hash *headers)
{
#ifdef BUILD_ECORE_CON
   char *dir = ecore_file_dir_get(dst);

   if (!ecore_file_is_dir(dir))
     {
        EINA_LOG_ERR("%s is not a directory", dir);
        free(dir);
        return EINA_FALSE;
     }
   free(dir);
   if (ecore_file_exists(dst))
     {
        EINA_LOG_ERR("%s already exists", dst);
        return EINA_FALSE;
     }

   if (!strncmp(url, "file://", 7))
     {
        /* FIXME: Maybe fork? Might take a while to copy.
         * Check filesize? */
        /* Just copy it */

        url += 7;
        /* skip hostname */
        url = strchr(url, '/');
        return ecore_file_cp(url, dst);
     }
# ifdef HAVE_CURL
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
             EINA_LOG_ERR("no job returned\n");
             return EINA_FALSE;
          }
        return job ? EINA_TRUE : EINA_FALSE;
     }
# else
   else if ((!strncmp(url, "http://", 7)) || (!strncmp(url, "https://", 8)) ||
            (!strncmp(url, "ftp://", 6)))
     {
        (void)completion_cb;
        (void)progress_cb;
        (void)data;
        (void)job_ret;
        (void)headers;
        return EINA_FALSE;
     }
# endif
   else
     {
        return EINA_FALSE;
     }
#else
   (void)url;
   (void)dst;
   (void)completion_cb;
   (void)progress_cb;
   (void)data;
   (void)job_ret;
   (void)headers;
   return EINA_FALSE;
#endif /* BUILD_ECORE_CON */
}

/**
 * @addtogroup Ecore_File_Group Ecore_File - Files and directories convenience functions
 *
 * @{
 */

/**
 * @brief Download the given url to the given destination.
 *
 * @param  url The complete url to download.
 * @param  dst The local file to save the downloaded to.
 * @param  completion_cb A callback called on download complete.
 * @param  progress_cb A callback called during the download operation.
 * @param  data User data passed to both callbacks.
 * @param  job_ret Job used to abort the download.
 * @return EINA_TRUE if the download start or EINA_FALSE on failure
 *
 * This function starts the download of the URL @p url and saves it to
 * @p dst. @p url must provide the protocol, including 'http://',
 * 'ftp://' or 'file://'. Ecore_File must be compiled with CURL to
 * download using http and ftp protocols. If @p dst is ill-formed, or
 * if it already exists, the function returns EINA_FALSE. When the
 * download is complete, the callback @p completion_cb is called and
 * @p data is passed to it. The @p status parameter of @p completion_cb
 * will be filled with the status of the download (200, 404,...). The
 * @p progress_cb is called during the download operation, each time a
 * packet is received or when CURL wants. It can be used to display the
 * percentage of the downloaded file. Return 0 from this callback, if provided,
 * to continue the operation or anything else to abort the download. The only
 * operations that can be aborted are those with protocol 'http' or 'ftp'. In
 * that case @p job_ret can be filled. It can be used with
 * ecore_file_download_abort() or ecore_file_download_abort_all() to
 * respectively abort one or all download operations. This function returns
 * EINA_TRUE if the download starts, EINA_FALSE otherwise.
 */
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

/**
 * @brief Download the given url to the given destination with additional headers.
 *
 * @param  url The complete url to download.
 * @param  dst The local file to save the downloaded to.
 * @param  completion_cb A callback called on download complete.
 * @param  progress_cb A callback called during the download operation.
 * @param  data User data passed to both callbacks.
 * @param  job_ret Job used to abort the download.
 * @param  headers pointer of header lists.
 * @return EINA_TRUE if the download start or EINA_FALSE on failure
 */
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

/**
 * @brief Check if the given protocol is available.
 *
 * @param  protocol The protocol to check.
 * @return EINA_TRUE if protocol is handled, EINA_FALSE otherwise.
 *
 * This function returns EINA_TRUE if @p protocol is supported,
 * EINA_FALSE otherwise. @p protocol can be 'http://', 'ftp://' or
 * 'file://'. Ecore_FILE must be compiled with CURL to handle http and
 * ftp protocols.
 */
EAPI Eina_Bool
ecore_file_download_protocol_available(const char *protocol)
{
#ifdef BUILD_ECORE_CON
   if (!strncmp(protocol, "file://", 7)) return EINA_TRUE;
# ifdef HAVE_CURL
   else if (!strncmp(protocol, "http://", 7)) return EINA_TRUE;
   else if (!strncmp(protocol, "ftp://", 6)) return EINA_TRUE;
# endif
#else
   (void)protocol;
#endif /* BUILD_ECORE_CON */

   return EINA_FALSE;
}

#ifdef BUILD_ECORE_CON

# ifdef HAVE_CURL
static int
_ecore_file_download_url_compare_job(const void *data1, const void *data2)
{
   const Ecore_File_Download_Job *job = data1;
   const Ecore_Con_Url           *url = data2;

   if (job->url_con == url) return 0;
   return -1;
}

static Eina_Bool
_ecore_file_download_url_complete_cb(void *data __UNUSED__, int type __UNUSED__, void *event)
{
   Ecore_Con_Event_Url_Complete *ev = event;
   Ecore_File_Download_Job      *job;

   job = eina_list_search_unsorted(_job_list, _ecore_file_download_url_compare_job, ev->url_con);
   if (!ECORE_MAGIC_CHECK(job, ECORE_MAGIC_FILE_DOWNLOAD_JOB)) return ECORE_CALLBACK_PASS_ON;

   if (job->completion_cb)
     job->completion_cb(ecore_con_url_data_get(job->url_con), job->dst, ev->status);

   _job_list = eina_list_remove(_job_list, job);
   fclose(job->file);
   free(job->dst);
   ecore_con_url_free(job->url_con);
   free(job);

   return ECORE_CALLBACK_DONE;
}

static Eina_Bool
_ecore_file_download_url_progress_cb(void *data __UNUSED__, int type __UNUSED__, void *event)
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

   ecore_con_url_get(job->url_con);

   return job;
}
# endif
#endif

/**
 * @brief Abort the given download job and call the completion_cb
 * callbck with a status of 1 (error).
 *
 * @param job The download job to abort.
 *
 * This function aborts a download operation started by
 * ecore_file_download(). @p job is the #Ecore_File_Download_Job
 * structure filled by ecore_file_download(). If it is @c NULL, this
 * function does nothing. To abort all the currently downloading
 * operations, call ecore_file_download_abort_all().
 */
EAPI void
ecore_file_download_abort(Ecore_File_Download_Job *job)
{
   if (!job)
     return;

#ifdef BUILD_ECORE_CON
   if (job->completion_cb)
     job->completion_cb(ecore_con_url_data_get(job->url_con), job->dst, 1);
# ifdef HAVE_CURL
   ecore_con_url_free(job->url_con);
# endif
   _job_list = eina_list_remove(_job_list, job);
   fclose(job->file);
   free(job->dst);
   free(job);
#endif /* BUILD_ECORE_CON */
}

/**
 * @brief Abort all downloads.
 *
 * This function aborts all the downloads that have been started by
 * ecore_file_download(). It loops over the started downloads and call
 * ecore_file_download_abort() for each of them. To abort only one
 * specific download operation, call ecore_file_download_abort().
 */
EAPI void
ecore_file_download_abort_all(void)
{
#ifdef BUILD_ECORE_CON
   Ecore_File_Download_Job *job;

   EINA_LIST_FREE(_job_list, job)
             ecore_file_download_abort(job);
#endif /* BUILD_ECORE_CON */
}

/**
 * @}
 */
