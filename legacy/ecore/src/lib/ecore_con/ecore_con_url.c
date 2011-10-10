/*
 * For info on how to use libcurl, see:
 * http://curl.haxx.se/libcurl/c/libcurl-tutorial.html
 */

/*
 * FIXME: Support more CURL features...
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#ifdef HAVE_WS2TCPIP_H
# include <ws2tcpip.h>
#endif

#include "Ecore.h"
#include "ecore_private.h"
#include "Ecore_Con.h"
#include "ecore_con_private.h"

int ECORE_CON_EVENT_URL_DATA = 0;
int ECORE_CON_EVENT_URL_COMPLETE = 0;
int ECORE_CON_EVENT_URL_PROGRESS = 0;

#ifdef HAVE_CURL
static Eina_Bool _ecore_con_url_fd_handler(void             *data,
                                           Ecore_Fd_Handler *fd_handler);
static Eina_Bool _ecore_con_url_perform(Ecore_Con_Url *url_con);
static size_t    _ecore_con_url_header_cb(void  *ptr,
                                          size_t size,
                                          size_t nitems,
                                          void  *stream);
static size_t _ecore_con_url_data_cb(void  *buffer,
                                     size_t size,
                                     size_t nitems,
                                     void  *userp);
static int _ecore_con_url_progress_cb(void  *clientp,
                                      double dltotal,
                                      double dlnow,
                                      double ultotal,
                                      double ulnow);
static size_t _ecore_con_url_read_cb(void  *ptr,
                                     size_t size,
                                     size_t nitems,
                                     void  *stream);
static void      _ecore_con_event_url_free(void *data __UNUSED__,
                                           void      *ev);
static int       _ecore_con_url_process_completed_jobs(
  Ecore_Con_Url *url_con_to_match);
static Eina_Bool _ecore_con_url_idler_handler(void *data);

static Ecore_Idler *_fd_idler_handler = NULL;
static Eina_List *_url_con_list = NULL;
static CURLM *_curlm = NULL;
static fd_set _current_fd_set;
static int _init_count = 0;
static Ecore_Timer *_curl_timeout = NULL;
static Eina_Bool pipelining = EINA_FALSE;

typedef struct _Ecore_Con_Url_Event Ecore_Con_Url_Event;
struct _Ecore_Con_Url_Event
{
   int   type;
   void *ev;
};

static Eina_Bool
_url_complete_idler_cb(void *data)
{
   Ecore_Con_Url_Event *lev;

   lev = data;
   ecore_event_add(lev->type, lev->ev, _ecore_con_event_url_free, NULL);
   free(lev);

   return ECORE_CALLBACK_CANCEL;
}

static void
_url_complete_push_event(int   type,
                         void *ev)
{
   Ecore_Con_Url_Event *lev;

   lev = malloc(sizeof(Ecore_Con_Url_Event));
   lev->type = type;
   lev->ev = ev;

   ecore_idler_add(_url_complete_idler_cb, lev);
}

#endif

/**
 * @addtogroup Ecore_Con_Url_Group Ecore URL Connection Functions
 *
 * @{
 */

EAPI int
ecore_con_url_init(void)
{
#ifdef HAVE_CURL
   _init_count++;

   if (_init_count > 1)
     return _init_count;

   if (!ECORE_CON_EVENT_URL_DATA)
     {
        ECORE_CON_EVENT_URL_DATA = ecore_event_type_new();
        ECORE_CON_EVENT_URL_COMPLETE = ecore_event_type_new();
        ECORE_CON_EVENT_URL_PROGRESS = ecore_event_type_new();
     }

   if (!_curlm)
     {
        long ms;

        FD_ZERO(&_current_fd_set);
        if (curl_global_init(CURL_GLOBAL_ALL))
          {
             while (_url_con_list)
               ecore_con_url_free(eina_list_data_get(_url_con_list));
             return 0;
          }

        _curlm = curl_multi_init();
        if (!_curlm)
          {
             while (_url_con_list)
               ecore_con_url_free(eina_list_data_get(_url_con_list));

             _init_count--;
             return 0;
          }

        curl_multi_timeout(_curlm, &ms);
        if (ms <= 0)
          ms = 1000;

        _curl_timeout =
          ecore_timer_add((double)ms / 1000, _ecore_con_url_idler_handler,
                          (void *)0xACE);
        ecore_timer_freeze(_curl_timeout);
     }

   return 1;
#else
   return 0;
#endif
}

EAPI int
ecore_con_url_shutdown(void)
{
#ifdef HAVE_CURL
   if (!_init_count)
     return 0;

   _init_count--;

   if (_init_count != 0)
     return _init_count;

   if (_fd_idler_handler)
     ecore_idler_del(_fd_idler_handler);

   _fd_idler_handler = NULL;

   if (_curl_timeout)
     ecore_timer_del(_curl_timeout);

   _curl_timeout = NULL;

   while (_url_con_list)
     ecore_con_url_free(eina_list_data_get(_url_con_list));

   if (_curlm)
     {
        curl_multi_cleanup(_curlm);
        _curlm = NULL;
     }

   curl_global_cleanup();
#endif
   return 1;
}

EAPI void
ecore_con_url_pipeline_set(Eina_Bool enable)
{
#ifdef HAVE_CURL
  if (enable)
    curl_multi_setopt(_curlm, CURLMOPT_PIPELINING, 1);
  else
    curl_multi_setopt(_curlm, CURLMOPT_PIPELINING, 0);
  pipelining = enable;
#else
  return;
  (void)enable;
#endif
}

EAPI Eina_Bool
ecore_con_url_pipeline_get(void)
{
#ifdef HAVE_CURL
  return pipelining;
#endif
  return EINA_FALSE;
}

EAPI Ecore_Con_Url *
ecore_con_url_new(const char *url)
{
#ifdef HAVE_CURL
   Ecore_Con_Url *url_con;
   CURLcode ret;

   if (!_init_count)
     return NULL;

   url_con = calloc(1, sizeof(Ecore_Con_Url));
   if (!url_con)
     return NULL;

   url_con->fd = -1;
   url_con->write_fd = -1;

   url_con->curl_easy = curl_easy_init();
   if (!url_con->curl_easy)
     {
        free(url_con);
        return NULL;
     }

   ECORE_MAGIC_SET(url_con, ECORE_MAGIC_CON_URL);

   if (!ecore_con_url_url_set(url_con, url))
     {
        ecore_con_url_free(url_con);
        return NULL;
     }

   ret = curl_easy_setopt(url_con->curl_easy, CURLOPT_ENCODING, "gzip,deflate");
   if (ret != CURLE_OK)
     {
        ERR("Could not set CURLOPT_ENCODING to \"gzip,deflate\": %s",
            curl_easy_strerror(ret));
        ecore_con_url_free(url_con);
        return NULL;
     }

   curl_easy_setopt(url_con->curl_easy, CURLOPT_WRITEFUNCTION,
                    _ecore_con_url_data_cb);
   curl_easy_setopt(url_con->curl_easy, CURLOPT_WRITEDATA, url_con);

   curl_easy_setopt(url_con->curl_easy, CURLOPT_PROGRESSFUNCTION,
                    _ecore_con_url_progress_cb);
   curl_easy_setopt(url_con->curl_easy, CURLOPT_PROGRESSDATA, url_con);
   curl_easy_setopt(url_con->curl_easy, CURLOPT_NOPROGRESS, EINA_FALSE);

   curl_easy_setopt(url_con->curl_easy, CURLOPT_HEADERFUNCTION,
                    _ecore_con_url_header_cb);
   curl_easy_setopt(url_con->curl_easy, CURLOPT_HEADERDATA, url_con);

   /*
    * FIXME: Check that these timeouts are sensible defaults
    * FIXME: Provide a means to change these timeouts
    */
   curl_easy_setopt(url_con->curl_easy, CURLOPT_CONNECTTIMEOUT, 30);
   curl_easy_setopt(url_con->curl_easy, CURLOPT_FOLLOWLOCATION, 1);

   return url_con;
#else
   return NULL;
   url = NULL;
#endif
}

EAPI Ecore_Con_Url *
ecore_con_url_custom_new(const char *url,
                         const char *custom_request)
{
#ifdef HAVE_CURL
   Ecore_Con_Url *url_con;
   CURLcode ret;

   if (!url)
     return NULL;

   if (!custom_request)
     return NULL;

   url_con = ecore_con_url_new(url);

   if (!url_con)
     return NULL;

   ret = curl_easy_setopt(url_con->curl_easy, CURLOPT_CUSTOMREQUEST, custom_request);
   if (ret != CURLE_OK)
     {
        ERR("Could not set a custom request string: %s",
            curl_easy_strerror(ret));
        ecore_con_url_free(url_con);
        return NULL;
     }

   return url_con;
#else
   return NULL;
   url = NULL;
   custom_request = NULL;
#endif
}

EAPI void
ecore_con_url_free(Ecore_Con_Url *url_con)
{
#ifdef HAVE_CURL
   char *s;
   CURLMcode ret;

   if (!url_con)
     return;

   if (!ECORE_MAGIC_CHECK(url_con, ECORE_MAGIC_CON_URL))
     {
        ECORE_MAGIC_FAIL(url_con, ECORE_MAGIC_CON_URL, "ecore_con_url_free");
        return;
     }

   ECORE_MAGIC_SET(url_con, ECORE_MAGIC_NONE);
   if(url_con->fd != -1)
     {
        FD_CLR(url_con->fd, &_current_fd_set);
        if (url_con->fd_handler)
          ecore_main_fd_handler_del(url_con->fd_handler);

        url_con->fd = -1;
        url_con->fd_handler = NULL;
     }

   if (url_con->curl_easy)
     {
        // FIXME: For an unknown reason, progress continue to arrive after destruction
        // this prevent any further call to the callback.
        curl_easy_setopt(url_con->curl_easy, CURLOPT_PROGRESSFUNCTION, NULL);

        if (url_con->active)
          {
             url_con->active = EINA_FALSE;

             ret = curl_multi_remove_handle(_curlm, url_con->curl_easy);
             if (ret != CURLM_OK)
                ERR("curl_multi_remove_handle failed: %s",
                    curl_multi_strerror(ret));
          }

        curl_easy_cleanup(url_con->curl_easy);
     }

   _url_con_list = eina_list_remove(_url_con_list, url_con);
   curl_slist_free_all(url_con->headers);
   EINA_LIST_FREE(url_con->additional_headers, s)
     free(s);
   EINA_LIST_FREE(url_con->response_headers, s)
     free(s);
   eina_stringshare_del(url_con->url);
   free(url_con);
#else
   return;
   (void)url_con;
#endif
}

EAPI const char *
ecore_con_url_url_get(Ecore_Con_Url *url_con)
{
#ifdef HAVE_CURL
   if (!ECORE_MAGIC_CHECK(url_con, ECORE_MAGIC_CON_URL))
     {
        ECORE_MAGIC_FAIL(url_con, ECORE_MAGIC_CON_URL, __func__);
        return NULL;
     }
   return url_con->url;
#else
   return NULL;
   (void)url_con;
#endif
}

EAPI Eina_Bool
ecore_con_url_url_set(Ecore_Con_Url *url_con,
                      const char    *url)
{
#ifdef HAVE_CURL
   if (!ECORE_MAGIC_CHECK(url_con, ECORE_MAGIC_CON_URL))
     {
        ECORE_MAGIC_FAIL(url_con, ECORE_MAGIC_CON_URL, "ecore_con_url_url_set");
        return EINA_FALSE;
     }

   if (url_con->active)
     return EINA_FALSE;

   eina_stringshare_replace(&url_con->url, url);

   if (url_con->url)
     curl_easy_setopt(url_con->curl_easy, CURLOPT_URL,
                      url_con->url);
   else
     curl_easy_setopt(url_con->curl_easy, CURLOPT_URL, "");

   return EINA_TRUE;
#else
   return EINA_FALSE;
   (void)url;
   (void)url_con;
#endif
}

EAPI void
ecore_con_url_data_set(Ecore_Con_Url *url_con,
                       void          *data)
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

EAPI void
ecore_con_url_additional_header_add(Ecore_Con_Url *url_con,
                                    const char    *key,
                                    const char    *value)
{
#ifdef HAVE_CURL
   char *tmp;

   if (!ECORE_MAGIC_CHECK(url_con, ECORE_MAGIC_CON_URL))
     {
        ECORE_MAGIC_FAIL(url_con, ECORE_MAGIC_CON_URL,
                         "ecore_con_url_additional_header_add");
        return;
     }

   tmp = malloc(strlen(key) + strlen(value) + 3);
   if (!tmp)
     return;

   sprintf(tmp, "%s: %s", key, value);
   url_con->additional_headers = eina_list_append(url_con->additional_headers,
                                                  tmp);
#else
   return;
   url_con = NULL;
   key = NULL;
   value = NULL;
#endif
}

EAPI void
ecore_con_url_additional_headers_clear(Ecore_Con_Url *url_con)
{
#ifdef HAVE_CURL
   char *s;

   if (!ECORE_MAGIC_CHECK(url_con, ECORE_MAGIC_CON_URL))
     {
        ECORE_MAGIC_FAIL(url_con, ECORE_MAGIC_CON_URL,
                         "ecore_con_url_additional_headers_clear");
        return;
     }

   EINA_LIST_FREE(url_con->additional_headers, s)
     free(s);
#else
   return;
   url_con = NULL;
#endif
}

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

EAPI void
ecore_con_url_time(Ecore_Con_Url     *url_con,
                   Ecore_Con_Url_Time condition,
                   double             timestamp)
{
#ifdef HAVE_CURL
   if (!ECORE_MAGIC_CHECK(url_con, ECORE_MAGIC_CON_URL))
     {
        ECORE_MAGIC_FAIL(url_con, ECORE_MAGIC_CON_URL, "ecore_con_url_time");
        return;
     }

   url_con->time_condition = condition;
   url_con->timestamp = timestamp;
#else
   return;
   (void)url_con;
   (void)condition;
   (void)timestamp;
#endif
}

EAPI void
ecore_con_url_fd_set(Ecore_Con_Url *url_con,
                     int            fd)
{
#ifdef HAVE_CURL
   if (!ECORE_MAGIC_CHECK(url_con, ECORE_MAGIC_CON_URL))
     {
        ECORE_MAGIC_FAIL(url_con, ECORE_MAGIC_CON_URL, "ecore_con_url_set");
        return;
     }

   url_con->write_fd = fd;
#else
   return;
   (void)url_con;
   (void)fd;
#endif
}

EAPI int
ecore_con_url_received_bytes_get(Ecore_Con_Url *url_con)
{
#ifdef HAVE_CURL
   if (!ECORE_MAGIC_CHECK(url_con, ECORE_MAGIC_CON_URL))
     {
        ECORE_MAGIC_FAIL(url_con, ECORE_MAGIC_CON_URL,
                         "ecore_con_url_received_bytes_get");
        return -1;
     }

   return url_con->received;
#else
   return 0;
   (void)url_con;
#endif
}

EAPI const Eina_List *
ecore_con_url_response_headers_get(Ecore_Con_Url *url_con)
{
#ifdef HAVE_CURL
   return url_con->response_headers;
#else
   return NULL;
   (void)url_con;
#endif
}

EAPI Eina_Bool
ecore_con_url_httpauth_set(Ecore_Con_Url *url_con,
                           const char    *username,
                           const char    *password,
                           Eina_Bool      safe)
{
#ifdef HAVE_CURL
   CURLcode ret;

   if (!ECORE_MAGIC_CHECK(url_con, ECORE_MAGIC_CON_URL))
     {
        ECORE_MAGIC_FAIL(url_con, ECORE_MAGIC_CON_URL,
                         "ecore_con_url_httpauth_set");
        return EINA_FALSE;
     }

# if LIBCURL_VERSION_NUM >= 0x071301
   if ((username) && (password))
     {
        if (safe)
          curl_easy_setopt(url_con->curl_easy, CURLOPT_HTTPAUTH,
                           CURLAUTH_ANYSAFE);
        else
          curl_easy_setopt(url_con->curl_easy, CURLOPT_HTTPAUTH, CURLAUTH_ANY);

        ret = curl_easy_setopt(url_con->curl_easy, CURLOPT_USERNAME, username);
        if (ret != CURLE_OK)
          {
             ERR("Could not set username for HTTP authentication: %s",
                 curl_easy_strerror(ret));
             return EINA_FALSE;
          }

        ret = curl_easy_setopt(url_con->curl_easy, CURLOPT_PASSWORD, password);
        if (ret != CURLE_OK)
          {
             ERR("Could not set password for HTTP authentication: %s",
                 curl_easy_strerror(ret));
             return EINA_FALSE;
          }

        return EINA_TRUE;
     }
# endif
#else
   return EINA_FALSE;
   (void)url_con;
   (void)username;
   (void)password;
   (void)safe;
#endif

   return EINA_FALSE;
}

#define MODE_AUTO 0
#define MODE_GET  1
#define MODE_POST 2

static Eina_Bool
_ecore_con_url_send(Ecore_Con_Url *url_con,
                    int            mode,
                    const void    *data,
                    long           length,
                    const char    *content_type)
{
#ifdef HAVE_CURL
   Eina_List *l;
   const char *s;
   char tmp[256];

   if (!ECORE_MAGIC_CHECK(url_con, ECORE_MAGIC_CON_URL))
     {
        ECORE_MAGIC_FAIL(url_con, ECORE_MAGIC_CON_URL, "ecore_con_url_send");
        return EINA_FALSE;
     }

   if (url_con->active)
     return EINA_FALSE;

   if (!url_con->url)
     return EINA_FALSE;

   /* Free response headers from previous send() calls */
   EINA_LIST_FREE(url_con->response_headers, s)
     free((char *)s);
   url_con->response_headers = NULL;

   curl_slist_free_all(url_con->headers);
   url_con->headers = NULL;

   if ((mode == MODE_POST) || (mode == MODE_AUTO))
     {
        if (data)
          {
             if ((content_type) && (strlen(content_type) < 200))
               {
                  snprintf(tmp, sizeof(tmp), "Content-Type: %s", content_type);
                  url_con->headers = curl_slist_append(url_con->headers, tmp);
               }

             curl_easy_setopt(url_con->curl_easy, CURLOPT_POSTFIELDS, data);
             curl_easy_setopt(url_con->curl_easy, CURLOPT_POSTFIELDSIZE, length);
          }
     }

   switch (url_con->time_condition)
     {
      case ECORE_CON_URL_TIME_NONE:
        curl_easy_setopt(url_con->curl_easy, CURLOPT_TIMECONDITION,
                         CURL_TIMECOND_NONE);
        break;

      case ECORE_CON_URL_TIME_IFMODSINCE:
        curl_easy_setopt(url_con->curl_easy, CURLOPT_TIMECONDITION,
                         CURL_TIMECOND_IFMODSINCE);
        curl_easy_setopt(url_con->curl_easy, CURLOPT_TIMEVALUE,
                         (long)url_con->timestamp);
        break;

      case ECORE_CON_URL_TIME_IFUNMODSINCE:
        curl_easy_setopt(url_con->curl_easy, CURLOPT_TIMECONDITION,
                         CURL_TIMECOND_IFUNMODSINCE);
        curl_easy_setopt(url_con->curl_easy, CURLOPT_TIMEVALUE,
                         (long)url_con->timestamp);
        break;
     }

   /* Additional headers */
   EINA_LIST_FOREACH(url_con->additional_headers, l, s)
     url_con->headers = curl_slist_append(url_con->headers, s);

   curl_easy_setopt(url_con->curl_easy, CURLOPT_HTTPHEADER, url_con->headers);

   url_con->received = 0;

   return _ecore_con_url_perform(url_con);
#else
   return EINA_FALSE;
   (void)url_con;
   (void)mode;
   (void)data;
   (void)length;
   (void)content_type;
#endif
}

EINA_DEPRECATED EAPI Eina_Bool
ecore_con_url_send(Ecore_Con_Url *url_con,
                   const void    *data,
                   long           length,
                   const char    *content_type)
{
   return _ecore_con_url_send(url_con, MODE_AUTO, data, length, content_type);
}

EAPI Eina_Bool
ecore_con_url_get(Ecore_Con_Url *url_con)
{
   return _ecore_con_url_send(url_con, MODE_GET, NULL, 0, NULL);
}

EAPI Eina_Bool
ecore_con_url_post(Ecore_Con_Url *url_con,
                   const void    *data,
                   long           length,
                   const char    *content_type)
{
   return _ecore_con_url_send(url_con, MODE_POST, data, length, content_type);
}

EAPI Eina_Bool
ecore_con_url_ftp_upload(Ecore_Con_Url *url_con,
                         const char    *filename,
                         const char    *user,
                         const char    *pass,
                         const char    *upload_dir)
{
#ifdef HAVE_CURL
   char url[4096];
   char userpwd[4096];
   FILE *fd;
   struct stat file_info;
   CURLcode ret;

   if (!ECORE_MAGIC_CHECK(url_con, ECORE_MAGIC_CON_URL))
     {
        ECORE_MAGIC_FAIL(url_con,
                         ECORE_MAGIC_CON_URL,
                         "ecore_con_url_ftp_upload");
        return EINA_FALSE;
     }

   if (url_con->active)
     return EINA_FALSE;

   if (!url_con->url)
     return EINA_FALSE;

   if (filename)
     {
        if (stat(filename, &file_info))
          return EINA_FALSE;

        snprintf(userpwd, sizeof(userpwd), "%s:%s", user, pass);
        ret = curl_easy_setopt(url_con->curl_easy, CURLOPT_USERPWD, userpwd);
        if (ret != CURLE_OK)
          {
             ERR("Could not set username and password for FTP upload: %s",
                 curl_easy_strerror(ret));
             return EINA_FALSE;
          }

        char tmp[PATH_MAX];
        snprintf(tmp, PATH_MAX, "%s", filename);

        if (upload_dir)
          snprintf(url, sizeof(url), "ftp://%s/%s/%s", url_con->url,
                   upload_dir, basename(tmp));
        else
          snprintf(url, sizeof(url), "ftp://%s/%s", url_con->url,
                   basename(tmp));

        if (!ecore_con_url_url_set(url_con, url))
          return EINA_FALSE;

        curl_easy_setopt(url_con->curl_easy, CURLOPT_INFILESIZE_LARGE,
                         (curl_off_t)file_info.st_size);
        curl_easy_setopt(url_con->curl_easy, CURLOPT_UPLOAD, 1);
        curl_easy_setopt(url_con->curl_easy, CURLOPT_READFUNCTION,
                         _ecore_con_url_read_cb);

        fd = fopen(filename, "rb");
        if (!fd)
          {
             ERR("Could not open \"%s\" for FTP upload", filename);
             return EINA_FALSE;
          }
        curl_easy_setopt(url_con->curl_easy, CURLOPT_READDATA, fd);

        return _ecore_con_url_perform(url_con);
     }
#else
   return EINA_FALSE;
   (void)url_con;
   (void)filename;
   (void)user;
   (void)pass;
   (void)upload_dir;
#endif

   return EINA_FALSE;
}

EAPI void
ecore_con_url_cookies_init(Ecore_Con_Url *url_con)
{
#ifdef HAVE_CURL
   if (!url_con)
     return;

   if (!ECORE_MAGIC_CHECK(url_con, ECORE_MAGIC_CON_URL))
     {
        ECORE_MAGIC_FAIL(url_con, ECORE_MAGIC_CON_URL,
                         "ecore_con_url_cookies_init");
        return;
     }

   curl_easy_setopt(url_con->curl_easy, CURLOPT_COOKIEFILE, "");
#else
   return;
   (void)url_con;
#endif
}

EAPI void
ecore_con_url_cookies_ignore_old_session_set(Ecore_Con_Url *url_con, Eina_Bool ignore)
{
#ifdef HAVE_CURL
   if (!url_con)
     return;

   if (!ECORE_MAGIC_CHECK(url_con, ECORE_MAGIC_CON_URL))
     {
        ECORE_MAGIC_FAIL(url_con, ECORE_MAGIC_CON_URL,
                         "ecore_con_url_cookies_ignore_old_session_set");
        return;
     }

   curl_easy_setopt(url_con->curl_easy, CURLOPT_COOKIESESSION, ignore);
#else
   return;
   (void)url_con;
   (void)ignore;
#endif
}

EAPI void
ecore_con_url_cookies_clear(Ecore_Con_Url *url_con)
{
#ifdef HAVE_CURL
   if (!url_con)
     return;

   if (!ECORE_MAGIC_CHECK(url_con, ECORE_MAGIC_CON_URL))
     {
        ECORE_MAGIC_FAIL(url_con, ECORE_MAGIC_CON_URL,
                         "ecore_con_url_cookies_clear");
        return;
     }

   curl_easy_setopt(url_con->curl_easy, CURLOPT_COOKIELIST, "ALL");
#else
   return;
   (void)url_con;
#endif
}

EAPI void
ecore_con_url_cookies_session_clear(Ecore_Con_Url *url_con)
{
#ifdef HAVE_CURL
   if (!url_con)
     return;

   if (!ECORE_MAGIC_CHECK(url_con, ECORE_MAGIC_CON_URL))
     {
        ECORE_MAGIC_FAIL(url_con, ECORE_MAGIC_CON_URL,
                         "ecore_con_url_cookies_session_clear");
        return;
     }

   curl_easy_setopt(url_con->curl_easy, CURLOPT_COOKIELIST, "SESS");
#else
   return;
   (void)url_con;
#endif
}

EAPI void
ecore_con_url_cookies_file_add(Ecore_Con_Url *url_con, const char * const file_name)
{
#ifdef HAVE_CURL
   if (!url_con)
     return;

   if (!ECORE_MAGIC_CHECK(url_con, ECORE_MAGIC_CON_URL))
     {
        ECORE_MAGIC_FAIL(url_con, ECORE_MAGIC_CON_URL,
                         "ecore_con_url_cookies_file_add");
        return;
     }

   curl_easy_setopt(url_con->curl_easy, CURLOPT_COOKIEFILE, file_name);
#else
   return;
   (void)url_con;
   (void)file_name;
#endif
}

EAPI Eina_Bool
ecore_con_url_cookies_jar_file_set(Ecore_Con_Url *url_con, const char * const cookiejar_file)
{
#ifdef HAVE_CURL
   CURLcode ret;

   if (!url_con)
     return EINA_FALSE;

   if (!ECORE_MAGIC_CHECK(url_con, ECORE_MAGIC_CON_URL))
     {
        ECORE_MAGIC_FAIL(url_con, ECORE_MAGIC_CON_URL,
                         "ecore_con_url_cookies_jar_file_set");
        return EINA_FALSE;
     }

   ret = curl_easy_setopt(url_con->curl_easy, CURLOPT_COOKIEJAR,
                          cookiejar_file);
   if (ret != CURLE_OK)
     {
        ERR("Setting the cookie-jar name failed: %s",
            curl_easy_strerror(ret));
        return EINA_FALSE;
     }

   return EINA_TRUE;
#else
   return EINA_FALSE;
   (void)url_con;
   (void)cookiejar_file;
#endif
}

EAPI void
ecore_con_url_cookies_jar_write(Ecore_Con_Url *url_con)
{
#ifdef HAVE_CURL
   if (!url_con)
     return;

   if (!ECORE_MAGIC_CHECK(url_con, ECORE_MAGIC_CON_URL))
     {
        ECORE_MAGIC_FAIL(url_con, ECORE_MAGIC_CON_URL,
                         "ecore_con_url_cookies_jar_write");
        return;
     }

   curl_easy_setopt(url_con->curl_easy, CURLOPT_COOKIELIST, "FLUSH");
#else
   return;
   (void)url_con;
#endif
}

EAPI void
ecore_con_url_verbose_set(Ecore_Con_Url *url_con,
                          Eina_Bool      verbose)
{
#ifdef HAVE_CURL
   if (!ECORE_MAGIC_CHECK(url_con, ECORE_MAGIC_CON_URL))
     {
        ECORE_MAGIC_FAIL(url_con, ECORE_MAGIC_CON_URL,
                         "ecore_con_url_verbose_set");
        return;
     }

   if (url_con->active)
     return;

   if (!url_con->url)
     return;

   curl_easy_setopt(url_con->curl_easy, CURLOPT_VERBOSE, (int)verbose);
#else
   return;
   (void)url_con;
   (void)verbose;
#endif
}

EAPI void
ecore_con_url_ftp_use_epsv_set(Ecore_Con_Url *url_con,
                               Eina_Bool      use_epsv)
{
#ifdef HAVE_CURL
   if (!ECORE_MAGIC_CHECK(url_con, ECORE_MAGIC_CON_URL))
     {
        ECORE_MAGIC_FAIL(url_con, ECORE_MAGIC_CON_URL,
                         "ecore_con_url_ftp_use_epsv_set");
        return;
     }

   if (url_con->active)
     return;

   if (!url_con->url)
     return;

   curl_easy_setopt(url_con->curl_easy, CURLOPT_FTP_USE_EPSV, (int)use_epsv);
#else
   return;
   (void)url_con;
   (void)use_epsv;
#endif
}

/**
 * Toggle libcurl's verify peer's certificate option.
 *
 * If @p verify is @c EINA_TRUE, libcurl will verify
 * the authenticity of the peer's certificate, otherwise
 * it will not. Default behavior of libcurl is to check
 * peer's certificate.
 *
 * @param url_con Ecore_Con_Url instance which will be acted upon.
 * @param verify Whether or not libcurl will check peer's certificate.
 * @since 1.1.0
 */
EAPI void
ecore_con_url_ssl_verify_peer_set(Ecore_Con_Url *url_con,
				  Eina_Bool      verify)
{
#ifdef HAVE_CURL
   if (!ECORE_MAGIC_CHECK(url_con, ECORE_MAGIC_CON_URL))
     {
        ECORE_MAGIC_FAIL(url_con, ECORE_MAGIC_CON_URL,
                         "ecore_con_url_ssl_verify_peer_set");
        return;
     }

   if (url_con->active)
     return;

   if (!url_con->url)
     return;

   curl_easy_setopt(url_con->curl_easy, CURLOPT_SSL_VERIFYPEER, (int)verify);
#else
   return;
   (void)url_con;
   (void)verify;
#endif
}

/**
 * Set a custom CA to trust for SSL/TLS connections.
 *
 * Specify the path of a file (in PEM format) containing one or more
 * CA certificate(s) to use for the validation of the server certificate.
 *
 * This function can also disable CA validation if @p ca_path is @c NULL.
 * However, the server certificate still needs to be valid for the connection
 * to succeed (i.e., the certificate must concern the server the
 * connection is made to).
 *
 * @param url_con Connection object that will use the custom CA.
 * @param ca_path Path to a CA certificate(s) file or @c NULL to disable
 *                CA validation.
 *
 * @return  @c 0 on success. When cURL is used, non-zero return values
 *          are equal to cURL error codes.
 */
EAPI int
ecore_con_url_ssl_ca_set(Ecore_Con_Url *url_con, const char *ca_path)
{
   int res = -1;

#ifdef HAVE_CURL
   if (!ECORE_MAGIC_CHECK(url_con, ECORE_MAGIC_CON_URL))
     {
       ECORE_MAGIC_FAIL(url_con, ECORE_MAGIC_CON_URL, "ecore_con_url_ssl_ca_set");
	      return -1;
     }

   if (url_con->active) return -1;
   if (!url_con->url) return -1;
   if (ca_path == NULL)
     res = curl_easy_setopt(url_con->curl_easy, CURLOPT_SSL_VERIFYPEER, 0);
   else
     {
       res = curl_easy_setopt(url_con->curl_easy, CURLOPT_SSL_VERIFYPEER, 1);
       if (!res)
         res = curl_easy_setopt(url_con->curl_easy, CURLOPT_CAINFO, ca_path);
     }
#else
   return -1;
   (void)url_con;
   (void)ca_path;
#endif

   return res;
}


/**
 * @}
 */

#ifdef HAVE_CURL
static int
_ecore_con_url_suspend_fd_handler(void)
{
   Eina_List *l;
   Ecore_Con_Url *url_con;
   int deleted = 0;

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
   Eina_List *l;
   Ecore_Con_Url *url_con;
   int activated = 0;

   if (!_url_con_list)
     return 0;

   EINA_LIST_FOREACH(_url_con_list, l, url_con)
     {
        if (!url_con->fd_handler && url_con->fd != -1)
          {
             url_con->fd_handler =
               ecore_main_fd_handler_add(url_con->fd, url_con->flags,
                                         _ecore_con_url_fd_handler,
                                         NULL, NULL, NULL);
             activated++;
          }
     }

   return activated;
}

static size_t
_ecore_con_url_data_cb(void  *buffer,
                       size_t size,
                       size_t nitems,
                       void  *userp)
{
   Ecore_Con_Url *url_con;
   Ecore_Con_Event_Url_Data *e;
   size_t real_size = size * nitems;

   url_con = (Ecore_Con_Url *)userp;

   if (!url_con)
     return -1;

   if (!ECORE_MAGIC_CHECK(url_con, ECORE_MAGIC_CON_URL))
     {
        ECORE_MAGIC_FAIL(url_con, ECORE_MAGIC_CON_URL, "ecore_con_url_data_cb");
        return -1;
     }

   url_con->received += real_size;

   if (url_con->write_fd < 0)
     {
        e =
          malloc(sizeof(Ecore_Con_Event_Url_Data) + sizeof(unsigned char) *
                 (real_size - 1));
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
        ssize_t count = 0;
        size_t total_size = real_size;
        size_t offset = 0;

        while (total_size > 0)
          {
             count = write(url_con->write_fd,
                           (char *)buffer + offset,
                           total_size);
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

#define ECORE_CON_URL_TRANSMISSION(Transmit, Event, Url_con, Total, Now)   \
  {                                                                        \
     Ecore_Con_Event_Url_Progress *e;                                      \
     if ((Total != 0) || (Now != 0))                                       \
       {                                                                   \
          e = calloc(1, sizeof(Ecore_Con_Event_Url_Progress));             \
          if (e)                                                           \
            {                                                              \
               e->url_con = url_con;                                       \
               e->total = Total;                                           \
               e->now = Now;                                               \
               ecore_event_add(Event, e, _ecore_con_event_url_free, NULL); \
            }                                                              \
       }                                                                   \
  }

static size_t
_ecore_con_url_header_cb(void  *ptr,
                         size_t size,
                         size_t nitems,
                         void  *stream)
{
   size_t real_size = size * nitems;
   Ecore_Con_Url *url_con = stream;

   char *header = malloc(sizeof(char) * (real_size + 1));
   if (!header)
     return real_size;

   memcpy(header, ptr, real_size);
   header[real_size] = '\0';

   url_con->response_headers = eina_list_append(url_con->response_headers,
                                                header);

   return real_size;
}

static int
_ecore_con_url_progress_cb(void  *clientp,
                           double dltotal,
                           double dlnow,
                           double ultotal,
                           double ulnow)
{
   Ecore_Con_Event_Url_Progress *e;
   Ecore_Con_Url *url_con;

   url_con = clientp;

   e = malloc(sizeof(Ecore_Con_Event_Url_Progress));
   if (e)
     {
        e->url_con = url_con;
        e->down.total = dltotal;
        e->down.now = dlnow;
        e->up.total = ultotal;
        e->up.now = ulnow;
        ecore_event_add(ECORE_CON_EVENT_URL_PROGRESS, e,
                        _ecore_con_event_url_free, NULL);
     }

   return 0;
}

static size_t
_ecore_con_url_read_cb(void  *ptr,
                       size_t size,
                       size_t nitems,
                       void  *stream)
{
   size_t retcode = fread(ptr, size, nitems, stream);

   if (ferror((FILE *)stream))
     {
        fclose(stream);
        return CURL_READFUNC_ABORT;
     }
   else if (retcode == 0)
     {
        fclose((FILE *)stream);
        return 0;
     }

#ifdef _WIN32
   INF("*** We read %Iu bytes from file", retcode);
#else
   INF("*** We read %zu bytes from file", retcode);
#endif
   return retcode;
}

static Eina_Bool
_ecore_con_url_perform(Ecore_Con_Url *url_con)
{
   fd_set read_set, write_set, exc_set;
   int fd_max, fd;
   int flags, still_running;
   int completed_immediately = 0;
//   double start;
   CURLMcode ret;

   _url_con_list = eina_list_append(_url_con_list, url_con);

   url_con->active = EINA_TRUE;
   curl_multi_add_handle(_curlm, url_con->curl_easy);
/*
   start = ecore_time_get();
   while (curl_multi_perform(_curlm, &still_running) == 
          CURLM_CALL_MULTI_PERFORM)
     {
        if ((ecore_time_get() - start) > (0.7 * ecore_animator_frametime_get()))
          {
             break;
          }
     }
 */
   curl_multi_perform(_curlm, &still_running);
   
   completed_immediately = _ecore_con_url_process_completed_jobs(url_con);

   if (!completed_immediately)
     {
        if (url_con->fd_handler)
          ecore_main_fd_handler_del(url_con->fd_handler);

        url_con->fd_handler = NULL;

        /* url_con still active -- set up an fd_handler */
        FD_ZERO(&read_set);
        FD_ZERO(&write_set);
        FD_ZERO(&exc_set);

        /* Stupid curl, why can't I get the fd to the current added job? */
        ret = curl_multi_fdset(_curlm, &read_set, &write_set, &exc_set,
                               &fd_max);
        if (ret != CURLM_OK)
          {
             ERR("curl_multi_fdset failed: %s", curl_multi_strerror(ret));
             return EINA_FALSE;
          }

        for (fd = 0; fd <= fd_max; fd++)
          {
             if (!FD_ISSET(fd, &_current_fd_set))
               {
                  flags = 0;
                  if (FD_ISSET(fd, &read_set))
                    flags |= ECORE_FD_READ;

                  if (FD_ISSET(fd, &write_set))
                    flags |= ECORE_FD_WRITE;

                  if (FD_ISSET(fd, &exc_set))
                    flags |= ECORE_FD_ERROR;

                  if (flags)
                    {
                       long ms = 0;

                       ret = curl_multi_timeout(_curlm, &ms);
                       if (ret != CURLM_OK)
                         ERR("curl_multi_timeout failed: %s",
                             curl_multi_strerror(ret));

                       if (ms == 0)
                         ms = 1000;

                       FD_SET(fd, &_current_fd_set);
                       url_con->fd = fd;
                       url_con->flags = flags;
                       url_con->fd_handler =
                         ecore_main_fd_handler_add(fd, flags,
                                                   _ecore_con_url_fd_handler,
                                                   NULL, NULL, NULL);
                       break;
                    }
               }
          }
        if (!url_con->fd_handler)
          {
             /* Failed to set up an fd_handler */
              ecore_timer_freeze(_curl_timeout);

              ret = curl_multi_remove_handle(_curlm, url_con->curl_easy);
              if (ret != CURLM_OK)
                ERR("curl_multi_remove_handle failed: %s",
                    curl_multi_strerror(ret));

              url_con->active = EINA_FALSE;
              url_con->fd = -1;
              return EINA_FALSE;
          }

        ecore_timer_thaw(_curl_timeout);
     }

   return EINA_TRUE;
}

static Eina_Bool
_ecore_con_url_idler_handler(void *data)
{
   double start;
   int done = 1, still_running;

   start = ecore_time_get();
   while (curl_multi_perform(_curlm, &still_running) == CURLM_CALL_MULTI_PERFORM)
     /* make this not more than a frametime to keep interactivity high */
     if ((ecore_time_get() - start) > (0.7 * ecore_animator_frametime_get()))
       {
          done = 0;
          break;
       }

   _ecore_con_url_process_completed_jobs(NULL);

   if (done)
     {
        _ecore_con_url_restart_fd_handler();
        _fd_idler_handler = NULL;

        if (!_url_con_list)
          ecore_timer_freeze(_curl_timeout);

        return data ==
               (void *)0xACE ? ECORE_CALLBACK_RENEW : ECORE_CALLBACK_CANCEL;
     }

   return ECORE_CALLBACK_RENEW;
}

static Eina_Bool
_ecore_con_url_fd_handler(void *data                   __UNUSED__,
                          Ecore_Fd_Handler *fd_handler __UNUSED__)
{
   _ecore_con_url_suspend_fd_handler();

   if (!_fd_idler_handler)
     _fd_idler_handler = ecore_idler_add(
         _ecore_con_url_idler_handler, NULL);

   return ECORE_CALLBACK_RENEW;
}

static int
_ecore_con_url_process_completed_jobs(Ecore_Con_Url *url_con_to_match)
{
   Eina_List *l;
   Ecore_Con_Url *url_con;
   Ecore_Con_Event_Url_Complete *e;
   CURLMsg *curlmsg;
   CURLMcode ret;
   int n_remaining;
   int job_matched = 0;

   /* Loop jobs and check if any are done */
   while ((curlmsg = curl_multi_info_read(_curlm, &n_remaining)))
     {
        if (curlmsg->msg != CURLMSG_DONE)
          continue;

        /* find the job which is done */
        EINA_LIST_FOREACH(_url_con_list, l, url_con)
          {
             if (curlmsg->easy_handle == url_con->curl_easy)
               {
                  if (url_con_to_match &&
                      (url_con == url_con_to_match))
                    job_matched = 1;

                  if(url_con->fd != -1)
                    {
                       FD_CLR(url_con->fd, &_current_fd_set);
                       if (url_con->fd_handler)
                         ecore_main_fd_handler_del(
                           url_con->fd_handler);

                       url_con->fd = -1;
                       url_con->fd_handler = NULL;
                    }

                  _url_con_list = eina_list_remove(_url_con_list, url_con);
                  url_con->active = EINA_FALSE;
                  e = calloc(1, sizeof(Ecore_Con_Event_Url_Complete));
                  if (e)
                    {
                       e->url_con = url_con;
                       e->status = 0;
                       if (curlmsg->data.result == CURLE_OK)
                         {
                            long status; /* curl API uses long, not int */

                            status = 0;
                            curl_easy_getinfo(curlmsg->easy_handle,
                                              CURLINFO_RESPONSE_CODE,
                                              &status);
                            e->status = status;
                         }

                       _url_complete_push_event(ECORE_CON_EVENT_URL_COMPLETE, e);
                    }

                  ret = curl_multi_remove_handle(_curlm, url_con->curl_easy);
                  if (ret != CURLM_OK)
                    ERR("curl_multi_remove_handle failed: %s",
                        curl_multi_strerror(ret));

                  break;
               }
          }
     }

   return job_matched;
}

static void
_ecore_con_event_url_free(void *data __UNUSED__,
                          void      *ev)
{
   free(ev);
}

#endif
