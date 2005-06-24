/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */

/*
 * For info on how to use libcurl, see:
 * http://curl.haxx.se/libcurl/c/libcurl-tutorial.html
 */

/*
 * Brief usage:
 * 1. Create an Ecore_Con_Url object
 * 2. Register to receive the ECORE_CON_EVENT_URL_COMPLETE event
 *    (and optionally the ECORE_CON_EVENT_URL_DATA event to receive
 *    the response, e.g. for HTTP/FTP downloads)
 * 3. Set the URL with ecore_con_url_url_set(...);
 * 4. Perform the operation with ecore_con_url_send(...);
 *
 * Note that it is good to reuse Ecore_Con_Url objects wherever possible, but
 * bear in mind that each one can only perform one operation at a time.
 * You need to wait for the ECORE_CON_EVENT_URL_COMPLETE event before re-using
 * or destroying the object.
 * 
 * Example Usage 1 (HTTP GET):
 *   ecore_con_url_url_set(url_con, "http://www.google.com");
 *   ecore_con_url_send(url, NULL, 0, NULL);
 *
 * Example usage 2 (HTTP POST):
 *   ecore_con_url_url_set(url_con, "http://www.example.com/post_handler.cgi");
 *   ecore_con_url_send(url, data, data_length, "multipart/form-data");
 *
 * Example Usage 3 (FTP download):
 *   ecore_con_url_url_set(url_con, "ftp://ftp.example.com/pub/myfile");
 *   ecore_con_url_send(url, NULL, 0, NULL);
 *
 * FIXME: Support more CURL features: Authentication, FTP upload, Progress callbacks and more...
 */
#include "Ecore.h"
#include "config.h"
#include "ecore_private.h"
#include "ecore_con_private.h"
#include "Ecore_Con.h"

#ifdef HAVE_CURL
static int _ecore_con_url_fd_handler(void *data, Ecore_Fd_Handler *fd_handler);
static int _ecore_con_url_perform(Ecore_Con_Url *url_con);
size_t _ecore_con_url_data_cb(void *buffer, size_t size, size_t nmemb, void *userp);
static void _ecore_con_event_url_complete_free(void *data __UNUSED__, void *ev);
static void _ecore_con_event_url_data_free(void *data __UNUSED__, void *ev);
static int _ecore_con_url_process_completed_jobs(Ecore_Con_Url *url_con_to_match);

int ECORE_CON_EVENT_URL_DATA = 0;
int ECORE_CON_EVENT_URL_COMPLETE = 0;

static CURLM *curlm = NULL;
static Ecore_List *_url_con_list = NULL;
static fd_set _current_fd_set;
static int init_count = 0;
#endif

int
ecore_con_url_init(void)
{
#ifdef HAVE_CURL
   if (!ECORE_CON_EVENT_URL_DATA)
     {
	ECORE_CON_EVENT_URL_DATA = ecore_event_type_new();
	ECORE_CON_EVENT_URL_COMPLETE = ecore_event_type_new();
     }

   if (!_url_con_list)
     {
	_url_con_list = ecore_list_new();
	if (!_url_con_list) return 0;
     }

   if (!curlm)
     {
	FD_ZERO(&_current_fd_set);
	if (curl_global_init(CURL_GLOBAL_NOTHING))
	  {
	     ecore_list_destroy(_url_con_list);
	     _url_con_list = NULL;
	     return 0;
	  }

	curlm = curl_multi_init();
	if (!curlm)
	  {
	     ecore_list_destroy(_url_con_list);
	     _url_con_list = NULL;
	     return 0;
	  }
     }
   init_count++;
   return 1;
#else
   return 0;
#endif
}

int
ecore_con_url_shutdown(void)
{
#ifdef HAVE_CURL

   if (!init_count)
     return 0;

   init_count--;
   if (_url_con_list)
     {
	if (!ecore_list_is_empty(_url_con_list))
	  {
	     Ecore_Con_Url *url_con;
	     while ((url_con = ecore_list_remove_first(_url_con_list)))
	       {
		  ecore_con_url_destroy(url_con);
	       }
	  }
	ecore_list_destroy(_url_con_list);
	_url_con_list = NULL;
     }

   if (curlm)
     {
	curl_multi_cleanup(curlm);
	curlm = NULL;
     }

   curl_global_cleanup();
#endif
   return 1;
}

Ecore_Con_Url *
ecore_con_url_new(const char *url)
{
#ifdef HAVE_CURL
   Ecore_Con_Url *url_con;

   if (!init_count) return NULL;

   url_con = calloc(1, sizeof(Ecore_Con_Url));
   if (!url_con) return NULL;

   url_con->curl_easy = curl_easy_init();
   if (!url_con->curl_easy)
     {
	free(url_con);
	return NULL;
     }
   
   ecore_con_url_url_set(url_con, url);

   curl_easy_setopt(url_con->curl_easy, CURLOPT_WRITEFUNCTION, _ecore_con_url_data_cb);
   curl_easy_setopt(url_con->curl_easy, CURLOPT_WRITEDATA, url_con);
   /*
    * FIXME: Check that these timeouts are sensible defaults
    * FIXME: Provide a means to change these timeouts
    */
   curl_easy_setopt(url_con->curl_easy, CURLOPT_CONNECTTIMEOUT, 30);
   curl_easy_setopt(url_con->curl_easy, CURLOPT_TIMEOUT, 300);

   return url_con;
#else
   return NULL;
#endif
}

void
ecore_con_url_destroy(Ecore_Con_Url *url_con)
{
#ifdef HAVE_CURL
   if (!url_con) return;

   if (url_con->fd_handler)
     ecore_main_fd_handler_del(url_con->fd_handler);
   if (url_con->curl_easy)
     {
	if (curlm)
	  curl_multi_remove_handle(curlm, url_con->curl_easy);
	curl_easy_cleanup(url_con->curl_easy);
     }
   curl_slist_free_all(url_con->headers);
   free(url_con->url);
   free(url_con);
#endif
}

int
ecore_con_url_url_set(Ecore_Con_Url *url_con, const char *url)
{
#ifdef HAVE_CURL
   if (url_con->active) return 0;

   free(url_con->url);
   url_con->url = NULL;
   if (url)
     url_con->url = strdup(url);
   curl_easy_setopt(url_con->curl_easy, CURLOPT_URL, url_con->url);

#endif
   return 1;
}

int
ecore_con_url_send(Ecore_Con_Url *url_con, void *data, size_t length, char *content_type)
{
#ifdef HAVE_CURL
   char tmp[256];

   if (url_con->active) return 0;
   if (!url_con->url) return 0;

   curl_slist_free_all(url_con->headers);
   url_con->headers = NULL;

   if (data)
     {
	curl_easy_setopt(url_con->curl_easy, CURLOPT_POSTFIELDS, data);
	curl_easy_setopt(url_con->curl_easy, CURLOPT_POSTFIELDSIZE, length);

	if (content_type && (strlen(content_type) < 200))
	  {
	     sprintf(tmp, "Content-type: %s", content_type);
	     url_con->headers = curl_slist_append(url_con->headers, tmp);
	  }
	sprintf(tmp, "Content-length: %d", length);
	url_con->headers = curl_slist_append(url_con->headers, tmp);
     }
   else
     {
	curl_easy_setopt(url_con->curl_easy, CURLOPT_POSTFIELDS, NULL);
     }

   curl_easy_setopt(url_con->curl_easy, CURLOPT_HTTPHEADER, url_con->headers);

   return _ecore_con_url_perform(url_con);
#else
   return 0;
#endif
}

#ifdef HAVE_CURL
size_t
_ecore_con_url_data_cb(void *buffer, size_t size, size_t nmemb, void *userp)
{
   Ecore_Con_Url *url_con;
   Ecore_Con_Event_Url_Data *e;
   size_t real_size = size * nmemb;

   url_con = (Ecore_Con_Url *)userp;
   e = calloc(1, sizeof(Ecore_Con_Event_Url_Data));
   if (e)
     {
	e->url_con = url_con;
	e->data = buffer;
	e->size = real_size;
	ecore_event_add(ECORE_CON_EVENT_URL_DATA, e,
			_ecore_con_event_url_data_free, NULL);
     }
   return real_size;
}

/*
 * FIXME: Use
 *   CURLOPT_PROGRESSFUNCTION and CURLOPT_PROGRESSDATA to
 *   get reports on progress.
 * And maybe other nifty functions...
 */
static int
_ecore_con_url_perform(Ecore_Con_Url *url_con)
{
   fd_set read_set, write_set, exc_set;
   int fd_max;
   int fd;
   int flags;
   int still_running;
   int completed_immediately = 0;

   ecore_list_append(_url_con_list, url_con);

   url_con->active = 1;
   curl_multi_add_handle(curlm, url_con->curl_easy);
   while (curl_multi_perform(curlm, &still_running) == CURLM_CALL_MULTI_PERFORM);

   completed_immediately =  _ecore_con_url_process_completed_jobs(url_con);

   if (!completed_immediately)
     {
	/* url_con still active -- set up an fd_handler */
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
		       url_con->fd_handler = ecore_main_fd_handler_add(fd, flags,
								       _ecore_con_url_fd_handler,
								       NULL, NULL, NULL);
		    }
	       }
	  }
	if (!url_con->fd_handler)
	  {
	     /* Failed to set up an fd_handler */
	     curl_multi_remove_handle(curlm, url_con->curl_easy);
	     url_con->active = 0;
	     return 0;
	  }
     }

   return 1;
}

static int
_ecore_con_url_fd_handler(void *data, Ecore_Fd_Handler *fd_handler)
{
   int still_running;

   /* FIXME: Can this run for a long time? Maybe limit how long it can run */
   while (curl_multi_perform(curlm, &still_running) == CURLM_CALL_MULTI_PERFORM);

   _ecore_con_url_process_completed_jobs(NULL);
   return 1;
}

static int
_ecore_con_url_process_completed_jobs(Ecore_Con_Url *url_con_to_match)
{
   Ecore_Con_Url *url_con;
   CURLMsg *curlmsg;
   int n_remaining;
   int job_matched = 0;

   /* Loop jobs and check if any are done */
   while ((curlmsg = curl_multi_info_read(curlm, &n_remaining)) != NULL)
     {
	if (curlmsg->msg != CURLMSG_DONE) continue;

	/* find the job which is done */
	ecore_list_goto_first(_url_con_list);
	while ((url_con = ecore_list_current(_url_con_list)))
	  {
	     if (curlmsg->easy_handle == url_con->curl_easy)
	       {
		  /* We have found the completed job in our job list */
		  if (url_con_to_match && (url_con == url_con_to_match)) {
		       job_matched = 1;
		  }
		  if (url_con->fd_handler)
		    {
		       FD_CLR(ecore_main_fd_handler_fd_get(url_con->fd_handler),
			      &_current_fd_set);
		       ecore_main_fd_handler_del(url_con->fd_handler);
		    }
		  ecore_list_remove(_url_con_list);
		  curl_multi_remove_handle(curlm, url_con->curl_easy);
		  url_con->active = 0;
		    {
		       Ecore_Con_Event_Url_Complete *e;
		       e = calloc(1, sizeof(Ecore_Con_Event_Url_Complete));
		       if (e)
			 {
			    e->url_con = url_con;
			    e->status = curlmsg->data.result;
			    ecore_event_add(ECORE_CON_EVENT_URL_COMPLETE, e,
					    _ecore_con_event_url_complete_free, NULL);
			 }
		    }
		  break;
	       }
	     ecore_list_next(_url_con_list);
	  }
     }
   return job_matched;
}
static void
_ecore_con_event_url_data_free(void *data __UNUSED__, void *ev)
{
   Ecore_Con_Event_Url_Data *e;

   e = ev;
   free(e);
}

static void
_ecore_con_event_url_complete_free(void *data __UNUSED__, void *ev)
{
   Ecore_Con_Event_Url_Complete *e;

   e = ev;
   free(e);
}
#endif
