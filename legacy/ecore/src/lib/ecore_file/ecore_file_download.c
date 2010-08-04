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

#define ECORE_MAGIC_FILE_DOWNLOAD_JOB	0xf7427cb8

struct _Ecore_File_Download_Job
{
   ECORE_MAGIC;

   Ecore_Con_Url	*url_con;
   FILE			*file;

   char			*dst;

   void	(*completion_cb)(void *data, const char *file, int status);

   int	(*progress_cb)  (void *data, const char *file,
			 long int dltotal, long int dlnow,
			 long int ultotal, long int ulnow);
};

#ifdef HAVE_CURL
Ecore_File_Download_Job *_ecore_file_download_curl(const char *url, const char *dst,
						   void (*completion_cb)(void *data, const char *file, int status),
						   int (*progress_cb)(void *data, const char *file, long int dltotal, long int dlnow, long int ultotal, long int ulnow),
						   void *data);

static Eina_Bool _ecore_file_download_url_complete_cb(void *data, int type, void *event);
static Eina_Bool _ecore_file_download_url_progress_cb(void *data, int type, void *event);
#endif

static Ecore_Event_Handler	*_url_complete_handler = NULL;
static Ecore_Event_Handler	*_url_progress_download = NULL;
static Eina_List		*_job_list;

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
 * Download @p url to the given @p dst
 * @param  url The complete url to download
 * @param  dst The local file to save the downloaded to
 * @param  completion_cb A callback called on download complete
 * @param  progress_cb A callback called during the download operation
 * @param  data User data passed to both callbacks
 * @param  job_ret If the protocol in use is http or ftp, this parameter will be
 * filled with the job. Then you can use ecore_file_download_abort() to cancel it.
 * 
 * @return 1 if the download start or 0 on failure
 *
 * You must provide the full url, including 'http://', 'ftp://' or 'file://'.\n
 * If @p dst already exist it will not be overwritten and the function will fail.\n
 * Ecore must be compiled with CURL to download using http and ftp protocols.\n
 * The @p status param in the @p completion_cb() will be 0 if the download goes well or
 * 1 in case of failure.
 */
EAPI int
ecore_file_download(const char *url, const char *dst,
		    void (*completion_cb)(void *data, const char *file, int status),
		    int (*progress_cb)(void *data, const char *file, long int dltotal, long int dlnow, long int ultotal, long int ulnow),
		    void *data, Ecore_File_Download_Job **job_ret)
{
#ifdef BUILD_ECORE_CON
   char *dir = ecore_file_dir_get(dst);

   if (!ecore_file_is_dir(dir))
     {
	free(dir);
	return 0;
     }
   free(dir);
   if (ecore_file_exists(dst)) return 0;

   /* FIXME: Add handlers for http and ftp! */
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
   else if ((!strncmp(url, "http://", 7)) ||
	    (!strncmp(url, "ftp://", 6)))
     {
	/* download */
	Ecore_File_Download_Job *job;

	job = _ecore_file_download_curl(url, dst, completion_cb, progress_cb, data);
	if(job_ret) *job_ret = job;
	return job != NULL;
     }
# endif
   else
     {
	return 0;
     }
#else
   completion_cb = NULL;
   progress_cb = NULL;
   data = NULL;
   return 0;
#endif /* BUILD_ECORE_CON */
}

/**
 * Check if the given protocol is available
 * @param  protocol The protocol to check
 * @return 1 if protocol is handled or 0 if not
 *
 * @p protocol can be 'http://', 'ftp://' or 'file://'.\n
 * Ecore must be compiled with CURL to handle http and ftp protocols.
 */
EAPI int
ecore_file_download_protocol_available(const char *protocol)
{
#ifdef BUILD_ECORE_CON
   if (!strncmp(protocol, "file://", 7)) return 1;
# ifdef HAVE_CURL
   else if (!strncmp(protocol, "http://", 7)) return 1;
   else if (!strncmp(protocol, "ftp://", 6)) return 1;
# endif
#endif /* BUILD_ECORE_CON */

   return 0;
}

#ifdef BUILD_ECORE_CON

# ifdef HAVE_CURL
static int
_ecore_file_download_url_compare_job(const void *data1, const void *data2)
{
   const Ecore_File_Download_Job	*job = data1;
   const Ecore_Con_Url			*url = data2;

   if (job->url_con == url) return 0;
   return -1;
}

static Eina_Bool
_ecore_file_download_url_complete_cb(void *data __UNUSED__, int type __UNUSED__, void *event)
{
   Ecore_Con_Event_Url_Complete	*ev = event;
   Ecore_File_Download_Job	*job;

   job = eina_list_search_unsorted(_job_list, _ecore_file_download_url_compare_job, ev->url_con);
   if (!ECORE_MAGIC_CHECK(job, ECORE_MAGIC_FILE_DOWNLOAD_JOB)) return ECORE_CALLBACK_PASS_ON;

   if (job->completion_cb)
     job->completion_cb(ecore_con_url_data_get(job->url_con), job->dst, !ev->status);

   _job_list = eina_list_remove(_job_list, job);
   fclose(job->file);
   free(job->dst);
   free(job);

   return ECORE_CALLBACK_DONE;
}

static Eina_Bool
_ecore_file_download_url_progress_cb(void *data __UNUSED__, int type __UNUSED__, void *event)
{
/* this reports the downloads progress. if we return 0, then download
 * continues, if we return anything else, then the download stops */
   Ecore_Con_Event_Url_Progress	*ev = event;
   Ecore_File_Download_Job	*job;

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
			  void (*completion_cb)(void *data, const char *file,
						int status),
			  int (*progress_cb)(void *data, const char *file,
					     long int dltotal, long int dlnow,
					     long int ultotal, long int ulnow),
			  void *data)
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

   ecore_con_url_fd_set(job->url_con, fileno(job->file));
   ecore_con_url_data_set(job->url_con, data);

   job->dst = strdup(dst);

   job->completion_cb = completion_cb;
   job->progress_cb = progress_cb;
   _job_list = eina_list_append(_job_list, job);

   ecore_con_url_send(job->url_con, NULL, 0, NULL);

   return job;
}
# endif
#endif

/**
 * Abort the given download job and call the @p completion_cb function with a
 * @status of 1 (error)
 * @param  job The download job to abort
 */

EAPI void
ecore_file_download_abort(Ecore_File_Download_Job *job)
{
#ifdef BUILD_ECORE_CON
   if (job->completion_cb)
     job->completion_cb(ecore_con_url_data_get(job->url_con), job->dst, 1);
# ifdef HAVE_CURL
   ecore_con_url_destroy(job->url_con);
# endif
   _job_list = eina_list_remove(_job_list, job);
   fclose(job->file);
   free(job->dst);
   free(job);
#endif /* BUILD_ECORE_CON */
}
