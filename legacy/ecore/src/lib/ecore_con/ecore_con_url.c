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
 *   ecore_con_url_send(url_con, NULL, 0, NULL);
 *
 * Example usage 2 (HTTP POST):
 *   ecore_con_url_url_set(url_con, "http://www.example.com/post_handler.cgi");
 *   ecore_con_url_send(url_con, data, data_length, "multipart/form-data");
 *
 * Example Usage 3 (FTP download):
 *   ecore_con_url_url_set(url_con, "ftp://ftp.example.com/pub/myfile");
 *   ecore_con_url_send(url_con, NULL, 0, NULL);
 *
 * Example Usage 4 (FTP upload as ftp://ftp.example.com/file):
 *   ecore_con_url_url_set(url_con, "ftp://ftp.example.com");
 *   ecore_con_url_ftp_upload(url_con, "/tmp/file", "user", "pass", NULL);
 *
 * Example Usage 5 (FTP upload as ftp://ftp.example.com/dir/file):
 *   ecore_con_url_url_set(url_con, "ftp://ftp.example.com");
 *   ecore_con_url_ftp_upload(url_con, "/tmp/file", "user", "pass","dir");
 *
 * FIXME: Support more CURL features: Authentication, Progress callbacks and more...
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "Ecore.h"
#include "ecore_private.h"
#include "Ecore_Con.h"
#include "ecore_con_private.h"

/**
 * @defgroup Ecore_Con_Url_Group Ecore URL Connection Functions
 *
 * Utility functions that set up, use and shut down the Ecore URL 
 * Connection library.
 * FIXME: write detailed description
 */

int ECORE_CON_EVENT_URL_DATA = 0;
int ECORE_CON_EVENT_URL_COMPLETE = 0;
int ECORE_CON_EVENT_URL_PROGRESS = 0;

#ifdef HAVE_CURL
static int _ecore_con_url_fd_handler(void *data, Ecore_Fd_Handler *fd_handler);
static int _ecore_con_url_perform(Ecore_Con_Url *url_con);
static size_t _ecore_con_url_data_cb(void *buffer, size_t size, size_t nitems, void *userp);
static int _ecore_con_url_progress_cb(void *clientp, double dltotal, double dlnow, double ultotal, double ulnow);
static size_t _ecore_con_url_read_cb(void *ptr, size_t size, size_t nitems, void *stream);
static void _ecore_con_event_url_free(void *data __UNUSED__, void *ev);
static int _ecore_con_url_process_completed_jobs(Ecore_Con_Url *url_con_to_match);

static Ecore_Idler	*_fd_idler_handler = NULL;
static Eina_List	*_url_con_list = NULL;
static CURLM		*curlm = NULL;
static fd_set		 _current_fd_set;
static int		 init_count = 0;

struct _Ecore_Con_Url_Event
{
  int    type;
  void  *ev;
};
typedef struct _Ecore_Con_Url_Event Ecore_Con_Url_Event;

static int
_url_complete_idler_cb(void *data)
{
   Ecore_Con_Url_Event *lev;

   lev = data;

   ecore_event_add(lev->type, lev->ev, _ecore_con_event_url_free, NULL);
   free(lev);

   return 0;
}

static void
_url_complete_push_event(int type, void *ev)
{
   Ecore_Con_Url_Event *lev;

   lev = malloc(sizeof(Ecore_Con_Url_Event));
   lev->type = type;
   lev->ev = ev;

   ecore_idler_add(_url_complete_idler_cb, lev);
}

#endif

/**
 * Initialises the Ecore_Con_Url library.
 * @return Number of times the library has been initialised without being
 *          shut down.
 * @ingroup Ecore_Con_Url_Group
 */
EAPI int
ecore_con_url_init(void)
{
#ifdef HAVE_CURL
   if (!ECORE_CON_EVENT_URL_DATA)
     {
	ECORE_CON_EVENT_URL_DATA = ecore_event_type_new();
	ECORE_CON_EVENT_URL_COMPLETE = ecore_event_type_new();
	ECORE_CON_EVENT_URL_PROGRESS = ecore_event_type_new();
     }

   if (!curlm)
     {
	FD_ZERO(&_current_fd_set);
	if (curl_global_init(CURL_GLOBAL_NOTHING))
	  {
	     while (_url_con_list)
	       ecore_con_url_destroy(eina_list_data_get(_url_con_list));
	     return 0;
	  }

	curlm = curl_multi_init();
	if (!curlm)
	  {
	     while (_url_con_list)
	       ecore_con_url_destroy(eina_list_data_get(_url_con_list));
	     return 0;
	  }
     }
   init_count++;
   return 1;
#else
   return 0;
#endif
}

/**
 * Shuts down the Ecore_Con_Url library.
 * @return  Number of calls that still uses Ecore_Con_Url
 * @ingroup Ecore_Con_Url_Group
 */
EAPI int
ecore_con_url_shutdown(void)
{
#ifdef HAVE_CURL
   if (!init_count)
     return 0;

   init_count--;
   while (_url_con_list)
     ecore_con_url_destroy(eina_list_data_get(_url_con_list));

   if (curlm)
     {
	curl_multi_cleanup(curlm);
	curlm = NULL;
     }

   curl_global_cleanup();
#endif
   return 1;
}

/**
 * Creates and initializes a new Ecore_Con_Url.
 * @return  NULL on error, a new Ecore_Con_Url on success.
 * @ingroup Ecore_Con_Url_Group
 */
EAPI Ecore_Con_Url *
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

   ECORE_MAGIC_SET(url_con, ECORE_MAGIC_CON_URL);

   ecore_con_url_url_set(url_con, url);

   curl_easy_setopt(url_con->curl_easy, CURLOPT_WRITEFUNCTION, _ecore_con_url_data_cb);
   curl_easy_setopt(url_con->curl_easy, CURLOPT_WRITEDATA, url_con);

   curl_easy_setopt(url_con->curl_easy, CURLOPT_PROGRESSFUNCTION, _ecore_con_url_progress_cb);
   curl_easy_setopt(url_con->curl_easy, CURLOPT_PROGRESSDATA, url_con);
   curl_easy_setopt(url_con->curl_easy, CURLOPT_NOPROGRESS, FALSE);

   /*
    * FIXME: Check that these timeouts are sensible defaults
    * FIXME: Provide a means to change these timeouts
    */
   curl_easy_setopt(url_con->curl_easy, CURLOPT_CONNECTTIMEOUT, 30);
   curl_easy_setopt(url_con->curl_easy, CURLOPT_TIMEOUT, 300);
   curl_easy_setopt(url_con->curl_easy, CURLOPT_FOLLOWLOCATION, 1);

   curl_easy_setopt(url_con->curl_easy, CURLOPT_ENCODING, "gzip,deflate");

   url_con->fd = -1;
   url_con->write_fd = -1;

   return url_con;
#else
   return NULL;
   url = NULL;
#endif
}

/**
 * Frees the Ecore_Con_Url.
 * @return  FIXME: To be documented. 
 * @ingroup Ecore_Con_Url_Group
 */
EAPI void
ecore_con_url_destroy(Ecore_Con_Url *url_con)
{
#ifdef HAVE_CURL
   if (!url_con) return;
   if (!ECORE_MAGIC_CHECK(url_con, ECORE_MAGIC_CON_URL))
     {
	ECORE_MAGIC_FAIL(url_con, ECORE_MAGIC_CON_URL, "ecore_con_url_destroy");
	return;
     }

   ECORE_MAGIC_SET(url_con, ECORE_MAGIC_NONE);
   if (url_con->fd_handler)
     {
	ecore_main_fd_handler_del(url_con->fd_handler);
	url_con->fd = -1;
     }
   if (url_con->curl_easy)
     {
	if (url_con->active)
	  {
	     url_con->active = 0;

	     curl_multi_remove_handle(curlm, url_con->curl_easy);
	  }
	curl_easy_cleanup(url_con->curl_easy);
     }
   _url_con_list = eina_list_remove(_url_con_list, url_con);
   curl_slist_free_all(url_con->headers);
   free(url_con->url);
   free(url_con);
#else
   return;
   url_con = NULL;
#endif
}

/**
 * FIXME: To be documented.
 * @return  FIXME: To be documented.
 * @ingroup Ecore_Con_Url_Group
 */
EAPI int
ecore_con_url_url_set(Ecore_Con_Url *url_con, const char *url)
{
#ifdef HAVE_CURL
   if (!ECORE_MAGIC_CHECK(url_con, ECORE_MAGIC_CON_URL))
     {
	ECORE_MAGIC_FAIL(url_con, ECORE_MAGIC_CON_URL, "ecore_con_url_url_set");
	return 0;
     }

   if (url_con->active) return 0;

   free(url_con->url);
   url_con->url = NULL;
   if (url)
     url_con->url = strdup(url);
   curl_easy_setopt(url_con->curl_easy, CURLOPT_URL, url_con->url);
   return 1;
#else
   return 0;
   url_con = NULL;
   url = NULL;
#endif
}

/**
 * FIXME: To be documented.
 * @return  FIXME: To be documented.
 * @ingroup Ecore_Con_Url_Group
 */
EAPI void
ecore_con_url_data_set(Ecore_Con_Url *url_con, void *data)
{
#ifdef HAVE_CURL
   if (!ECORE_MAGIC_CHECK(url_con, ECORE_MAGIC_CON_URL))
     {
	ECORE_MAGIC_FAIL(url_con, ECORE_MAGIC_CON_URL, "ecore_con_url_data_set");
	return;
     }

   url_con->data = data;
#else
   return;
   url_con = NULL;
   data = NULL;
#endif
}

/**
 * FIXME: To be documented.
 * @return  FIXME: To be documented.
 * @ingroup Ecore_Con_Url_Group
 */
EAPI void *
ecore_con_url_data_get(Ecore_Con_Url *url_con)
{
#ifdef HAVE_CURL
   if (!ECORE_MAGIC_CHECK(url_con, ECORE_MAGIC_CON_URL))
     {
	ECORE_MAGIC_FAIL(url_con, ECORE_MAGIC_CON_URL, "ecore_con_url_data_get");
	return NULL;
     }

   return url_con->data;
#else
   return NULL;
   url_con = NULL;
#endif
}

/**
 * FIXME: To be documented.
 * @return  FIXME: To be documented.
 * @ingroup Ecore_Con_Url_Group
 */
EAPI void
ecore_con_url_time(Ecore_Con_Url *url_con, Ecore_Con_Url_Time condition, time_t tm)
{
#ifdef HAVE_CURL
   if (!ECORE_MAGIC_CHECK(url_con, ECORE_MAGIC_CON_URL))
     {
	ECORE_MAGIC_FAIL(url_con, ECORE_MAGIC_CON_URL, "ecore_con_url_time");
	return;
     }

   url_con->condition = condition;
   url_con->time = tm;
#else
   return;
   url_con = NULL;
   condition = 0;
   tm = 0;
#endif
}

/**
 * FIXME: To be documented.
 * @return  FIXME: To be documented.
 * @ingroup Ecore_Con_Url_Group
 */
EAPI void
ecore_con_url_fd_set(Ecore_Con_Url *url_con, int fd)
{
#ifdef HAVE_CURL
   if (!ECORE_MAGIC_CHECK(url_con, ECORE_MAGIC_CON_URL))
     {
	ECORE_MAGIC_FAIL(url_con, ECORE_MAGIC_CON_URL, "ecore_con_url_set");
	return ;
     }
   url_con->write_fd = fd;
#endif   
}

/**
 * FIXME: To be documented.
 * @return  FIXME: To be documented.
 * @ingroup Ecore_Con_Url_Group
 */
EAPI int
ecore_con_url_received_bytes_get(Ecore_Con_Url *url_con)
{
#ifdef HAVE_CURL
   if (!ECORE_MAGIC_CHECK(url_con, ECORE_MAGIC_CON_URL))
     {
	ECORE_MAGIC_FAIL(url_con, ECORE_MAGIC_CON_URL, "ecore_con_url_received_bytes_get");
	return -1;
     }

   return url_con->received;
#endif   
   return 0;
}

/**
 * FIXME: To be documented.
 * @return  FIXME: To be documented.
 * @ingroup Ecore_Con_Url_Group
 */
EAPI int
ecore_con_url_send(Ecore_Con_Url *url_con, const void *data, size_t length, const char *content_type)
{
#ifdef HAVE_CURL
   char tmp[256];

   if (!ECORE_MAGIC_CHECK(url_con, ECORE_MAGIC_CON_URL))
     {
	ECORE_MAGIC_FAIL(url_con, ECORE_MAGIC_CON_URL, "ecore_con_url_send");
	return 0;
     }

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

   switch (url_con->condition)
     {
      case ECORE_CON_URL_TIME_NONE:
	 curl_easy_setopt(url_con->curl_easy, CURLOPT_TIMECONDITION, CURL_TIMECOND_NONE);
	 break;
      case ECORE_CON_URL_TIME_IFMODSINCE:
	 curl_easy_setopt(url_con->curl_easy, CURLOPT_TIMECONDITION, CURL_TIMECOND_IFMODSINCE);
	 curl_easy_setopt(url_con->curl_easy, CURLOPT_TIMEVALUE, url_con->time);
	 break;
      case ECORE_CON_URL_TIME_IFUNMODSINCE:
	 curl_easy_setopt(url_con->curl_easy, CURLOPT_TIMECONDITION, CURL_TIMECOND_IFUNMODSINCE);
	 curl_easy_setopt(url_con->curl_easy, CURLOPT_TIMEVALUE, url_con->time);
	 break;
      case ECORE_CON_URL_TIME_LASTMOD:
	 curl_easy_setopt(url_con->curl_easy, CURLOPT_TIMECONDITION, CURL_TIMECOND_LASTMOD);
	 curl_easy_setopt(url_con->curl_easy, CURLOPT_TIMEVALUE, url_con->time);
	 break;
     }

   curl_easy_setopt(url_con->curl_easy, CURLOPT_HTTPHEADER, url_con->headers);

   url_con->received = 0;

   return _ecore_con_url_perform(url_con);
#else
   return 0;
   url_con = NULL;
   data = NULL;
   length = 0;
   content_type = NULL;
#endif
}

/**
 * Makes a FTP upload
 * @return  FIXME: To be more documented.
 * @ingroup Ecore_Con_Url_Group
 */
EAPI int 
ecore_con_url_ftp_upload(Ecore_Con_Url *url_con, const char *filename, const char *user, const char *pass, const char *upload_dir)
{
#ifdef HAVE_CURL
   char url[4096];
   char userpwd[4096];
   FILE *fd;
   struct stat file_info;
	
   if (!ECORE_MAGIC_CHECK(url_con, ECORE_MAGIC_CON_URL))
     {
	ECORE_MAGIC_FAIL(url_con, ECORE_MAGIC_CON_URL, "ecore_con_url_ftp_upload");
	return 0;
     }
     
   if (url_con->active) return 0;
   if (!url_con->url) return 0;
   if (filename)
     {
	if (stat(filename, &file_info)) return 0;
	fd = fopen(filename, "rb");
	if (upload_dir)
	   snprintf(url, sizeof(url), "ftp://%s/%s/%s", url_con->url, upload_dir, basename(filename));
	else
	   snprintf(url, sizeof(url), "ftp://%s/%s", url_con->url, basename(filename));
	snprintf(userpwd, sizeof(userpwd), "%s:%s", user, pass);
	curl_easy_setopt(url_con->curl_easy, CURLOPT_INFILESIZE_LARGE, (curl_off_t)file_info.st_size);
	curl_easy_setopt(url_con->curl_easy, CURLOPT_USERPWD, userpwd);
	curl_easy_setopt(url_con->curl_easy, CURLOPT_UPLOAD, 1);
	curl_easy_setopt(url_con->curl_easy, CURLOPT_READFUNCTION, _ecore_con_url_read_cb);
	curl_easy_setopt(url_con->curl_easy, CURLOPT_READDATA, fd);
	ecore_con_url_url_set(url_con, url);

	return _ecore_con_url_perform(url_con);
     }
   else
	return 0;
#else
   return 0;
   url_con = NULL;
   filename = NULL;
   user = NULL;
   pass = NULL;
   upload_dir = NULL;
#endif   
}

/**
 * Enable or disable libcurl verbose output, useful for debug
 * @return  FIXME: To be more documented.
 * @ingroup Ecore_Con_Url_Group
 */
EAPI void
ecore_con_url_verbose_set(Ecore_Con_Url *url_con, int verbose)
{
#ifdef HAVE_CURL
   if (!ECORE_MAGIC_CHECK(url_con, ECORE_MAGIC_CON_URL))
     {
	ECORE_MAGIC_FAIL(url_con, ECORE_MAGIC_CON_URL, "ecore_con_url_verbose_set");
	return;
     }
     
   if (url_con->active) return;
   if (!url_con->url) return;
   if (verbose == TRUE) 
	curl_easy_setopt(url_con->curl_easy, CURLOPT_VERBOSE, 1);
   else 
	curl_easy_setopt(url_con->curl_easy, CURLOPT_VERBOSE, 0);
#endif
}

/**
 * Enable or disable EPSV extension
 * @return  FIXME: To be more documented.
 * @ingroup Ecore_Con_Url_Group
 */
EAPI void
ecore_con_url_ftp_use_epsv_set(Ecore_Con_Url *url_con, int use_epsv)
{
#ifdef HAVE_CURL
   if (!ECORE_MAGIC_CHECK(url_con, ECORE_MAGIC_CON_URL))
     {
	ECORE_MAGIC_FAIL(url_con, ECORE_MAGIC_CON_URL, "ecore_con_url_ftp_use_epsv_set");
	return;
     }
     
   if (url_con->active) return;
   if (!url_con->url) return;
   if (use_epsv == TRUE) 
	curl_easy_setopt(url_con->curl_easy, CURLOPT_FTP_USE_EPSV, 1);
   else 
	curl_easy_setopt(url_con->curl_easy, CURLOPT_FTP_USE_EPSV, 0);
#endif
}

#ifdef HAVE_CURL
static int
_ecore_con_url_suspend_fd_handler(void)
{
   Eina_List		*l;
   Ecore_Con_Url	*url_con;
   int			 deleted = 0;

   if (!_url_con_list)
     return 0;

   EINA_LIST_FOREACH(_url_con_list, l, url_con)
     {
	if (url_con->active && url_con->fd_handler)
	  {
	     ecore_main_fd_handler_del(url_con->fd_handler);
	     url_con->fd_handler = NULL;
	     deleted++;
	  }
     }

   return deleted;
}

static int
_ecore_con_url_restart_fd_handler(void)
{
   Eina_List		*l;
   Ecore_Con_Url	*url_con;
   int			 activated = 0;

   if (!_url_con_list)
     return 0;

   EINA_LIST_FOREACH(_url_con_list, l, url_con)
     {
	if (url_con->fd_handler == NULL && url_con->fd != -1)
	  {
	     url_con->fd_handler = ecore_main_fd_handler_add(url_con->fd,
							     url_con->flags,
							     _ecore_con_url_fd_handler,
							     NULL, NULL, NULL);
	     activated++;
	  }
     }

   return activated;
}

static size_t
_ecore_con_url_data_cb(void *buffer, size_t size, size_t nitems, void *userp)
{
   Ecore_Con_Url *url_con;
   Ecore_Con_Event_Url_Data *e;
   size_t real_size = size * nitems;

   url_con = (Ecore_Con_Url *)userp;

   if (!url_con) return -1;
   if (!ECORE_MAGIC_CHECK(url_con, ECORE_MAGIC_CON_URL))
     {
	ECORE_MAGIC_FAIL(url_con, ECORE_MAGIC_CON_URL, "ecore_con_url_data_cb");
	return -1;
     }

   url_con->received += real_size;

   if (url_con->write_fd < 0)
     {
	e = malloc(sizeof(Ecore_Con_Event_Url_Data) + sizeof(unsigned char) * (real_size - 1));
	if (e)
	  {
	     e->url_con = url_con;
	     e->size = real_size;
	     memcpy(e->data, buffer, real_size);
	     ecore_event_add(ECORE_CON_EVENT_URL_DATA, e,
			     _ecore_con_event_url_free, NULL);
	  }
     }
   else
     {
	ssize_t	count = 0;
	size_t	total_size = real_size;
	size_t	offset = 0;

	while (total_size > 0)
	  {
	     count = write(url_con->write_fd, (char*) buffer + offset, total_size);
	     if (count < 0)
	       {
		  if (errno != EAGAIN && errno != EINTR)
		    return -1;
	       }
	     else
	       {
		  total_size -= count;
		  offset += count;
	       }
	  }
     }

   return real_size;
}

#define ECORE_CON_URL_TRANSMISSION(Transmit, Event, Url_con, Total, Now) \
{ \
   Ecore_Con_Event_Url_Progress *e; \
   if ((Total != 0) || (Now != 0)) \
     { \
	e = calloc(1, sizeof(Ecore_Con_Event_Url_Progress)); \
	if (e) \
	  { \
	     e->url_con = url_con; \
	     e->total = Total; \
	     e->now = Now; \
	     ecore_event_add(Event, e, _ecore_con_event_url_free, NULL); \
	  } \
     } \
}

static int
_ecore_con_url_progress_cb(void *clientp, double dltotal, double dlnow, double ultotal, double ulnow)
{
   Ecore_Con_Event_Url_Progress	*e;
   Ecore_Con_Url		*url_con;

   url_con = clientp;

   e = malloc(sizeof(Ecore_Con_Event_Url_Progress));
   if (e)
     {
	e->url_con = url_con;
	e->down.total = dltotal;
	e->down.now = dlnow;
	e->up.total = ultotal;
	e->up.now = ulnow;
	ecore_event_add(ECORE_CON_EVENT_URL_PROGRESS, e, _ecore_con_event_url_free, NULL);
     }

   return 0;
}

static size_t 
_ecore_con_url_read_cb(void *ptr, size_t size, size_t nitems, void *stream)
{
   size_t retcode = fread(ptr, size, nitems, stream);
   if (ferror((FILE*)stream)) {
	fclose(stream);
	return CURL_READFUNC_ABORT;
   } else if ((retcode == 0) || (retcode < nitems)) {
	fclose((FILE*)stream);
	return 0;
   }
   fprintf(stderr, "*** We read %d bytes from file\n", retcode);
   return retcode;
}

static int
_ecore_con_url_perform(Ecore_Con_Url *url_con)
{
   fd_set read_set, write_set, exc_set;
   int fd_max;
   int fd;
   int flags;
   int still_running;
   int completed_immediately = 0;

   _url_con_list = eina_list_append(_url_con_list, url_con);

   url_con->active = 1;
   curl_multi_add_handle(curlm, url_con->curl_easy);
   /* This one can't be stopped, or the download never start. */
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
		       url_con->fd = fd;
		       url_con->flags = flags;
		       url_con->fd_handler = ecore_main_fd_handler_add(fd, flags,
								       _ecore_con_url_fd_handler,
								       NULL, NULL, NULL);
		       break;
		    }
	       }
	  }
	if (!url_con->fd_handler)
	  {
	     /* Failed to set up an fd_handler */
	     curl_multi_remove_handle(curlm, url_con->curl_easy);
	     url_con->active = 0;
	     url_con->fd = -1;
	     return 0;
	  }
     }

   return 1;
}

static int
_ecore_con_url_idler_handler(void *data __UNUSED__)
{
   double	start;
   int		done = 1;
   int		still_running;

   start = ecore_time_get();
   while (curl_multi_perform(curlm, &still_running) == CURLM_CALL_MULTI_PERFORM)
     /* make this not more than a frametime to keep interactivity high */
     if ((ecore_time_get() - start) > ecore_animator_frametime_get())
       {
	  done = 0;
	  break;
       }

   _ecore_con_url_process_completed_jobs(NULL);

   if (done)
     {
	_ecore_con_url_restart_fd_handler();
	_fd_idler_handler = NULL;
	return 0;
     }

   return 1;
}

static int
_ecore_con_url_fd_handler(void *data __UNUSED__, Ecore_Fd_Handler *fd_handler __UNUSED__)
{
   _ecore_con_url_suspend_fd_handler();

   if (_fd_idler_handler == NULL)
     _fd_idler_handler = ecore_idler_add(_ecore_con_url_idler_handler, NULL);

   return 1;
}

static int
_ecore_con_url_process_completed_jobs(Ecore_Con_Url *url_con_to_match)
{
   Eina_List *l;
   Ecore_Con_Url *url_con;
   Ecore_Con_Event_Url_Complete *e;
   CURLMsg *curlmsg;
   int n_remaining;
   int job_matched = 0;

   /* Loop jobs and check if any are done */
   while ((curlmsg = curl_multi_info_read(curlm, &n_remaining)) != NULL)
     {
	if (curlmsg->msg != CURLMSG_DONE) continue;

	/* find the job which is done */
	EINA_LIST_FOREACH(_url_con_list, l, url_con)
	  {
	     if (curlmsg->easy_handle == url_con->curl_easy)
	       {
		  if (url_con_to_match && (url_con == url_con_to_match))
		       job_matched = 1;
		  if(url_con->fd != -1)
		    {
		       FD_CLR(url_con->fd, &_current_fd_set);
		       if (url_con->fd_handler)
			 ecore_main_fd_handler_del(url_con->fd_handler);
		       url_con->fd = -1;
		       url_con->fd_handler = NULL;
		    }
		  _url_con_list = eina_list_remove(_url_con_list, url_con);
		  url_con->active = 0;
		  e = calloc(1, sizeof(Ecore_Con_Event_Url_Complete));
		  if (e)
		    {
		       e->url_con = url_con;
		       e->status = 0;
		       curl_easy_getinfo(curlmsg->easy_handle, CURLINFO_RESPONSE_CODE, &e->status);
		       _url_complete_push_event(ECORE_CON_EVENT_URL_COMPLETE, e);
		    }
		  curl_multi_remove_handle(curlm, url_con->curl_easy);
		  break;
	       }
	  }
     }

   return job_matched;
}

static void
_ecore_con_event_url_free(void *data __UNUSED__, void *ev)
{
   free(ev);
}

#endif
