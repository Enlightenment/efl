/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */
#include "ecore_file_private.h"

static int init = 0;

#ifdef HAVE_CURL
#include <curl/curl.h>

typedef struct _Ecore_File_Download_Job Ecore_File_Download_Job;

struct _Ecore_File_Download_Job
{
   Ecore_Fd_Handler *fd_handler;
   CURL *curl;
   void (*completion_cb)(void *data, const char *file, int status);
   int (*progress_cb)(void *data, const char *file, long int dltotal, long int dlnow, long int ultotal, long int ulnow);
   void *data;
   FILE *file;
   char *dst;
};

Ecore_File_Download_Job *_ecore_file_download_curl(const char *url, const char *dst,
						   void (*completion_cb)(void *data, const char *file, int status),
						   int (*progress_cb)(void *data, const char *file, long int dltotal, long int dlnow, long int ultotal, long int ulnow),
						   void *data);
static int _ecore_file_download_curl_fd_handler(void *data, Ecore_Fd_Handler *fd_handler);

static CURLM *curlm;
static Ecore_List *_job_list;
static fd_set _current_fd_set;
#endif

int
ecore_file_download_init(void)
{
   if (++init != 1) return init;

#ifdef HAVE_CURL
   FD_ZERO(&_current_fd_set);
   _job_list = ecore_list_new();
   if (!_job_list) return --init;

   if (curl_global_init(CURL_GLOBAL_NOTHING)) return 0;

   curlm = curl_multi_init();
   if (!curlm)
     {
	ecore_list_destroy(_job_list);
	_job_list = NULL;
	return --init;
     }
#endif
   return init;
}

int
ecore_file_download_shutdown(void)
{
   if (--init != 0) return init;
#ifdef HAVE_CURL
   Ecore_File_Download_Job *job;

   if (!ecore_list_is_empty(_job_list))
     {
	ecore_list_goto_first(_job_list);
	while ((job = ecore_list_next(_job_list)))
	  {
	     ecore_main_fd_handler_del(job->fd_handler);
	     curl_multi_remove_handle(curlm, job->curl);
	     curl_easy_cleanup(job->curl);
	     fclose(job->file);
	     free(job->dst);
	     free(job);
	  }
     }
   ecore_list_destroy(_job_list);
   curl_multi_cleanup(curlm);
   curl_global_cleanup();
#endif
   return init;
}

EAPI int
ecore_file_download(const char *url, const char *dst,
		    void (*completion_cb)(void *data, const char *file, int status),
		    int (*progress_cb)(void *data, const char *file, long int dltotal, long int dlnow, long int ultotal, long int ulnow),
		    void *data)
{
   if (!ecore_file_is_dir(ecore_file_get_dir((char *)dst))) return 0;
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
#ifdef HAVE_CURL
   else if ((!strncmp(url, "http://", 7)) ||
	    (!strncmp(url, "ftp://", 6)))
     {
	/* download */
	Ecore_File_Download_Job *job;
	
	job = _ecore_file_download_curl(url, dst, completion_cb, progress_cb, data);
	if (job)
	  return 1;
	else
	  return 0;
     }
#endif
   else
     {
	return 0;
     }
}

EAPI int
ecore_file_download_protocol_available(const char *protocol)
{
   if (!strncmp(protocol, "file://", 7)) return 1;
#ifdef HAVE_CURL
   else if (!strncmp(protocol, "http://", 7)) return 1;
   else if (!strncmp(protocol, "ftp://", 6)) return 1;
#endif

   return 0;
}

#ifdef HAVE_CURL
/* this reports the downloads progress. if we return 0, then download 
 * continues, if we return anything else, then the download stops */
int _ecore_file_download_curl_progress_func(void *clientp, double dltotal, double dlnow, double ultotal, double ulnow)
{  
   Ecore_File_Download_Job *job;
   
   job = clientp;
   
   if(job->progress_cb)
     return job->progress_cb(job->data, job->dst, (long int)dltotal, (long int)dlnow, (long int)ultotal, (long int)ulnow);
   return 0;
}

Ecore_File_Download_Job *
_ecore_file_download_curl(const char *url, const char *dst,
			  void (*completion_cb)(void *data, const char *file,
						int status),
			  int (*progress_cb)(void *data, const char *file, 
					     long int dltotal, long int dlnow,
					     long int ultotal, 
					     long int ulnow),
			  void *data)
{
   CURLMsg *curlmsg;
   fd_set read_set, write_set, exc_set;
   int fd_max;
   int fd;
   int flags;
   int n_remaining, still_running;
   Ecore_File_Download_Job *job;

   job = calloc(1, sizeof(Ecore_File_Download_Job));
   if (!job) return NULL;

   job->file = fopen(dst, "wb");
   if (!job->file)
     {
	free(job);
	return NULL;
     }
   job->curl = curl_easy_init();
   if (!job->curl)
     {
	fclose(job->file);
	free(job);
	return NULL;
     }
   
   curl_easy_setopt(job->curl, CURLOPT_URL, url);
   curl_easy_setopt(job->curl, CURLOPT_WRITEDATA, job->file);
   
   if(progress_cb)
     {
	curl_easy_setopt(job->curl, CURLOPT_NOPROGRESS, FALSE);   
	curl_easy_setopt(job->curl, CURLOPT_PROGRESSDATA, job);   
	curl_easy_setopt(job->curl, CURLOPT_PROGRESSFUNCTION, _ecore_file_download_curl_progress_func);
     }
   
   job->data = data;
   job->completion_cb = completion_cb;
   job->progress_cb = progress_cb;
   job->dst = strdup(dst);
   ecore_list_append(_job_list, job);

   curl_multi_add_handle(curlm, job->curl);
   while (curl_multi_perform(curlm, &still_running) == CURLM_CALL_MULTI_PERFORM);

   /* check for completed jobs */
   while ((curlmsg = curl_multi_info_read(curlm, &n_remaining)) != NULL)
     {
	Ecore_File_Download_Job *current;

	if (curlmsg->msg != CURLMSG_DONE) continue;

	/* find the job which is done */
	ecore_list_goto_first(_job_list);
	while ((current = ecore_list_current(_job_list)))
	  {
	     if (curlmsg->easy_handle == current->curl)
	       {
		  /* We have a match -- delete the job */
		  if (current == job)
		    job = NULL;
		  if (current->fd_handler)
		    {
		       FD_CLR(ecore_main_fd_handler_fd_get(current->fd_handler),
			      &_current_fd_set);
		       ecore_main_fd_handler_del(current->fd_handler);
		    }
		  ecore_list_remove(_job_list);
		  curl_multi_remove_handle(curlm, current->curl);
		  curl_easy_cleanup(current->curl);
		  fclose(current->file);
		  if (current->completion_cb)
		    current->completion_cb(current->data, current->dst,
					   curlmsg->data.result);
		  free(current->dst);
		  free(current);
		  break;
	       }
	     ecore_list_next(_job_list);
	  }
     }

   if (job)
     {
	FD_ZERO(&read_set);
	FD_ZERO(&write_set);
	FD_ZERO(&exc_set);

	/* Stupid curl, why can't I get the fd to the current added job? */
	curl_multi_fdset(curlm, &read_set, &write_set, &exc_set, &fd_max);
	for (fd = 0; fd <= fd_max; fd++)
	  {
	     if (!FD_ISSET(fd, &_current_fd_set))
	       {
		  flags = 0;
		  if (FD_ISSET(fd, &read_set)) flags |= ECORE_FD_READ;
		  if (FD_ISSET(fd, &write_set)) flags |= ECORE_FD_WRITE;
		  if (FD_ISSET(fd, &exc_set)) flags |= ECORE_FD_ERROR;
		  if (flags)
		    {
		       FD_SET(fd, &_current_fd_set);
		       job->fd_handler = ecore_main_fd_handler_add(fd, flags,
								   _ecore_file_download_curl_fd_handler,
								   NULL, NULL, NULL);
		    }
	       }
	  }
	if (!job->fd_handler)
	  {
	     curl_easy_cleanup(job->curl);
	     fclose(job->file);
	     free(job);
	     job = NULL;
	  }
     }

   return job;
}

static int
_ecore_file_download_curl_fd_handler(void *data, Ecore_Fd_Handler *fd_handler)
{
   Ecore_File_Download_Job *job;
   CURLMsg *curlmsg;
   int n_remaining, still_running;

   /* FIXME: Can this run for a long time? Maybe limit how long it can run */
   while (curl_multi_perform(curlm, &still_running) == CURLM_CALL_MULTI_PERFORM);

   /* Loop jobs and check if any are done */
   while ((curlmsg = curl_multi_info_read(curlm, &n_remaining)) != NULL)
     {
	if (curlmsg->msg != CURLMSG_DONE) continue;

	/* find the job which is done */
	ecore_list_goto_first(_job_list);
	while ((job = ecore_list_current(_job_list)))
	  {
	     if (curlmsg->easy_handle == job->curl)
	       {
		  /* We have a match -- delete the job */
		  FD_CLR(ecore_main_fd_handler_fd_get(job->fd_handler),
			&_current_fd_set);
		  ecore_list_remove(_job_list);
		  ecore_main_fd_handler_del(job->fd_handler);
		  curl_multi_remove_handle(curlm, job->curl);
		  curl_easy_cleanup(job->curl);
		  fclose(job->file);
		  if (job->completion_cb)
		    job->completion_cb(job->data, job->dst, !curlmsg->data.result);
		  free(job->dst);
		  free(job);
		  break;
	       }
	     ecore_list_next(_job_list);
	  }
     }
   return 1;
}
#endif
