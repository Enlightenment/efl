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

#ifdef HAVE_ESCAPE
# include <Escape.h>
#endif

#include "Ecore.h"
#include "ecore_private.h"
#include "Ecore_Con.h"
#include "ecore_con_private.h"
#include "ecore_con_url_curl.h"

#define MY_CLASS EFL_NETWORK_URL_CLASS


typedef enum _Ecore_Con_Url_Mode
{
   ECORE_CON_URL_MODE_AUTO = 0,
   ECORE_CON_URL_MODE_GET = 1,
   ECORE_CON_URL_MODE_POST = 2,
   ECORE_CON_URL_MODE_HEAD = 3,
} Ecore_Con_Url_Mode;

struct _Ecore_Con_Url_Data
{
   void *curl_easy;
   struct curl_slist *headers;
   Eina_List *additional_headers;
   Eina_List *response_headers;
   const char *url;
   long proxy_type;
   int status;

   Ecore_Timer *timer;

   Ecore_Con_Url_Time time_condition;
   double timestamp;
   void *data;
   
   void *post_data;

   int received;
   int write_fd;

   unsigned int event_count;
   Eina_Bool dead : 1;
   Eina_Bool multi : 1;
};

typedef struct _Ecore_Con_Url_Data Ecore_Con_Url_Data;
typedef struct _Ecore_Con_Url_Data Efl_Network_Url_Data;

int ECORE_CON_EVENT_URL_DATA = 0;
int ECORE_CON_EVENT_URL_COMPLETE = 0;
int ECORE_CON_EVENT_URL_PROGRESS = 0;

static void      _ecore_con_url_event_url_complete(Ecore_Con_Url *url_con, CURLMsg *curlmsg);
static void      _ecore_con_url_multi_remove(Ecore_Con_Url *url_con);
static Eina_Bool _ecore_con_url_perform(Ecore_Con_Url *url_con);
static size_t    _ecore_con_url_header_cb(void *ptr, size_t size, size_t nitems, void *stream);
static size_t    _ecore_con_url_data_cb(void *buffer, size_t size, size_t nitems, void *userp);
static int       _ecore_con_url_progress_cb(void *clientp, double dltotal, double dlnow, double ultotal, double ulnow);
static size_t    _ecore_con_url_read_cb(void *ptr, size_t size, size_t nitems, void *stream);
static void      _ecore_con_event_url_free(Ecore_Con_Url *url_con, void *ev);
static Eina_Bool _ecore_con_url_timer(void *data);
static Eina_Bool _ecore_con_url_fd_handler(void *data, Ecore_Fd_Handler *fd_handler);
static Eina_Bool _ecore_con_url_timeout_cb(void *data);
static void      _ecore_con_url_status_get(Ecore_Con_Url *url_con);

static Eina_List *_url_con_list = NULL;
static Eina_List *_fd_hd_list = NULL;
static int _init_count = 0;
static Ecore_Timer *_curl_timer = NULL;
static Eina_Bool pipelining = EINA_FALSE;

static void
_ecore_con_post_init(void)
{
   if (!_curl_timer)
     {
        _curl_timer = ecore_timer_add(_c_timeout, _ecore_con_url_timer, NULL);
        ecore_timer_freeze(_curl_timer);
     }
}

/**
 * @addtogroup Ecore_Con_Url_Group Ecore URL Connection Functions
 *
 * @{
 */

EAPI int
ecore_con_url_init(void)
{
   if (++_init_count > 1) return _init_count;
   if (!ecore_init()) return --_init_count;
   ECORE_CON_EVENT_URL_DATA = ecore_event_type_new();
   ECORE_CON_EVENT_URL_COMPLETE = ecore_event_type_new();
   ECORE_CON_EVENT_URL_PROGRESS = ecore_event_type_new();
   return _init_count;
}

EAPI int
ecore_con_url_shutdown(void)
{
   Ecore_Con_Url *url_con;
   Ecore_Fd_Handler *fd_handler;
   if (_init_count == 0) return 0;
   --_init_count;
   if (_init_count) return _init_count;
   if (_curl_timer)
     {
        ecore_timer_del(_curl_timer);
        _curl_timer = NULL;
     }
   EINA_LIST_FREE(_url_con_list, url_con)
     ecore_con_url_free(url_con);
   EINA_LIST_FREE(_fd_hd_list, fd_handler)
     ecore_main_fd_handler_del(fd_handler);
   _c_shutdown();
   ecore_shutdown();
   return 0;
}

EAPI void
ecore_con_url_pipeline_set(Eina_Bool enable)
{
   if (!_c_init()) return;
   _ecore_con_post_init();
   if (enable == pipelining) return;
   _c->curl_multi_setopt(_c->_curlm, CURLMOPT_PIPELINING, !!enable);
   pipelining = enable;
}

EAPI Eina_Bool
ecore_con_url_pipeline_get(void)
{
   return pipelining;
}

extern Ecore_Con_Socks *_ecore_con_proxy_global;

static void
_efl_network_url_event_complete_cb(void *data EINA_UNUSED, const Efl_Event *event)
{
   Ecore_Con_Event_Url_Complete *e, *f = event->info;

   e = calloc(1, sizeof(Ecore_Con_Event_Url_Complete));
   if (!e)
     {
        efl_event_callback_stop(event->object);
        return;
     }

   e->status = f->status;
   e->url_con = f->url_con;
   ecore_event_add(ECORE_CON_EVENT_URL_COMPLETE, e,
                   (Ecore_End_Cb)_ecore_con_event_url_free, event->object);

   efl_event_callback_stop(event->object);
}

static void
_efl_network_url_event_data_cb(void *data EINA_UNUSED, const Efl_Event *event)
{
   Ecore_Con_Event_Url_Data *e;
   Efl_Network_Event_Url_Data *f = event->info;

   e = malloc(sizeof(Ecore_Con_Event_Url_Data) + sizeof(unsigned char) * f->size);

   if (!e) return;

   e->url_con = f->url_con;
   e->size = f->size;
   memcpy(e->data, f->data, f->size);
   ecore_event_add(ECORE_CON_EVENT_URL_DATA, e,
                   (Ecore_End_Cb)_ecore_con_event_url_free, event->object);
}

static void
_efl_network_url_event_progress_cb(void *data EINA_UNUSED, const Efl_Event *event)
{
   Ecore_Con_Event_Url_Progress *e, *f = event->info;

   e = malloc(sizeof(Ecore_Con_Event_Url_Progress));
   if (!e) return;

   e->url_con = f->url_con;
   e->down.total = f->down.total;
   e->down.now = f->down.now;
   e->up.total = f->up.total;
   e->up.now = f->up.now;
   ecore_event_add(ECORE_CON_EVENT_URL_PROGRESS, e,
                   (Ecore_End_Cb)_ecore_con_event_url_free, event->object);
}

EFL_CALLBACKS_ARRAY_DEFINE(efl_network_url_event_table_callbacks,
  { EFL_NETWORK_URL_EVENT_DATA, _efl_network_url_event_data_cb },
  { EFL_NETWORK_URL_EVENT_PROGRESS, _efl_network_url_event_progress_cb },
  { EFL_NETWORK_URL_EVENT_COMPLETE, _efl_network_url_event_complete_cb }
);

EAPI Ecore_Con_Url *
ecore_con_url_new(const char *url)
{
   Ecore_Con_Url *url_obj;
   url_obj = efl_add(EFL_NETWORK_URL_CLASS, NULL, efl_network_url_set(efl_self, url));

   efl_event_callback_array_add(url_obj, efl_network_url_event_table_callbacks(), NULL);

   return url_obj;
}

EOLIAN static Eo *
_efl_network_url_efl_object_constructor(Efl_Network_Url *url_obj, Efl_Network_Url_Data *url_con EINA_UNUSED)
{
   url_obj = efl_constructor(efl_super(url_obj, MY_CLASS));

   if (!_init_count || !_c_init())
     {
        ERR("Failed");
        return NULL;
     }
   _ecore_con_post_init();

   url_con->curl_easy = _c->curl_easy_init();
   if (!url_con->curl_easy)
     {
        ERR("Failed");
        return NULL;
     }

   efl_manual_free_set(url_obj, EINA_TRUE);

   return url_obj;
}

EOLIAN static Eo *
_efl_network_url_efl_object_finalize(Efl_Network_Url *url_obj, Efl_Network_Url_Data *url_con)
{
   CURLcode ret;

   url_con->write_fd = -1;

   if (!url_con->url)
     {
        return NULL;
     }

   // Read socks proxy
   url_con->proxy_type = -1;
   if (_ecore_con_proxy_global && _ecore_con_proxy_global->ip &&
       ((_ecore_con_proxy_global->version == 4) ||
        (_ecore_con_proxy_global->version == 5)))
     {
        char proxy[256];
        char host[256];

        if (_ecore_con_proxy_global->version == 5)
          {
             if (_ecore_con_proxy_global->lookup)
               snprintf(host, sizeof(host), "socks5h://%s",
                        _ecore_con_proxy_global->ip);
             else
               snprintf(host, sizeof(host), "socks5://%s",
                        _ecore_con_proxy_global->ip);
          }
        else if (_ecore_con_proxy_global->version == 4)
          {
             if (_ecore_con_proxy_global->lookup)
               snprintf(host, sizeof(host), "socks4a://%s",
                        _ecore_con_proxy_global->ip);
             else
               snprintf(host, sizeof(host), "socks4://%s",
                        _ecore_con_proxy_global->ip);
          }

        if (_ecore_con_proxy_global->port > 0 &&
            _ecore_con_proxy_global->port <= 65535)
          snprintf(proxy, sizeof(proxy), "%s:%d", host,
                   _ecore_con_proxy_global->port);
        else snprintf(proxy, sizeof(proxy), "%s", host);

        ecore_con_url_proxy_set(url_obj, proxy);
        ecore_con_url_proxy_username_set(url_obj,
                                         _ecore_con_proxy_global->username);
     }

   ret = _c->curl_easy_setopt(url_con->curl_easy, CURLOPT_ENCODING,
                              "gzip,deflate");
   if (ret != CURLE_OK)
     {
        ERR("Could not set CURLOPT_ENCODING to \"gzip,deflate\": %s",
            _c->curl_easy_strerror(ret));
        ecore_con_url_free(url_obj);
        return NULL;
     }

   _c->curl_easy_setopt(url_con->curl_easy, CURLOPT_WRITEFUNCTION,
                        _ecore_con_url_data_cb);
   _c->curl_easy_setopt(url_con->curl_easy, CURLOPT_WRITEDATA, url_obj);

   _c->curl_easy_setopt(url_con->curl_easy, CURLOPT_PROGRESSFUNCTION,
                        _ecore_con_url_progress_cb);
   _c->curl_easy_setopt(url_con->curl_easy, CURLOPT_PROGRESSDATA, url_obj);
   _c->curl_easy_setopt(url_con->curl_easy, CURLOPT_NOPROGRESS, EINA_FALSE);

   _c->curl_easy_setopt(url_con->curl_easy, CURLOPT_HEADERFUNCTION,
                        _ecore_con_url_header_cb);
   _c->curl_easy_setopt(url_con->curl_easy, CURLOPT_HEADERDATA, url_obj);
   /*
    * FIXME: Check that these timeouts are sensible defaults
    * FIXME: Provide a means to change these timeouts
    */
   _c->curl_easy_setopt(url_con->curl_easy, CURLOPT_CONNECTTIMEOUT, 30);
   _c->curl_easy_setopt(url_con->curl_easy, CURLOPT_FOLLOWLOCATION, 1);
   return efl_finalize(efl_super(url_obj, MY_CLASS));
}

EAPI Ecore_Con_Url *
ecore_con_url_custom_new(const char *url,
                         const char *custom_request)
{
   Ecore_Con_Url *url_obj;
   CURLcode ret;

   if (!_init_count) return NULL;
   if (!_c_init()) return NULL;
   _ecore_con_post_init();
   if (!url) return NULL;
   if (!custom_request) return NULL;
   url_obj = ecore_con_url_new(url);
   if (!url_obj) return NULL;
   Ecore_Con_Url_Data *url_con = efl_data_scope_get(url_obj, MY_CLASS);

   ret = _c->curl_easy_setopt(url_con->curl_easy, CURLOPT_CUSTOMREQUEST,
                              custom_request);
   if (ret != CURLE_OK)
     {
        ERR("Could not set a custom request string: %s",
            _c->curl_easy_strerror(ret));
        ecore_con_url_free(url_obj);
        return NULL;
     }
   return url_obj;
}

EAPI void
ecore_con_url_free(Ecore_Con_Url *url_obj)
{
   if (!efl_isa(url_obj, EFL_NETWORK_URL_CLASS))
      return;

   efl_event_callback_array_del(url_obj, efl_network_url_event_table_callbacks(), NULL);

   efl_del(url_obj);
}

static void
_ecore_con_url_free_internal(Ecore_Con_Url *url_obj)
{
   Efl_Network_Url_Data *url_con = efl_data_scope_get(url_obj, MY_CLASS);
   char *s;

   if (_c) _c->curl_slist_free_all(url_con->headers);
   EINA_LIST_FREE(url_con->additional_headers, s)
     free(s);
   EINA_LIST_FREE(url_con->response_headers, s)
     free(s);
   eina_stringshare_del(url_con->url);
   if (url_con->post_data) free(url_con->post_data);
}

EOLIAN static void
_efl_network_url_efl_object_destructor(Efl_Network_Url *url_obj, Efl_Network_Url_Data *url_con)
{
   efl_destructor(efl_super(url_obj, MY_CLASS));

   if (!_c) return;
   if (url_con->curl_easy)
     {
        // FIXME : How can we delete curl_easy's fds ??
        // (Curl do not give this info.)
        // This cause "Failed to delete epoll fd xx!" error messages
        _c->curl_easy_setopt(url_con->curl_easy, CURLOPT_PROGRESSFUNCTION,
                             NULL);
        _c->curl_easy_setopt(url_con->curl_easy, CURLOPT_NOPROGRESS,
                             EINA_TRUE);

        if (url_con->multi)
          {
             _ecore_con_url_multi_remove(url_obj);
             _url_con_list = eina_list_remove(_url_con_list, url_obj);
          }

        _c->curl_easy_cleanup(url_con->curl_easy);
     }
   if (url_con->timer) ecore_timer_del(url_con->timer);

   url_con->curl_easy = NULL;
   url_con->timer = NULL;
   url_con->dead = EINA_TRUE;
   if (url_con->event_count) return;

   efl_manual_free_set(url_obj, EINA_FALSE);
   _ecore_con_url_free_internal(url_obj);
}

EOLIAN static const char *
_efl_network_url_url_get(Efl_Network_Url *url_obj EINA_UNUSED, Efl_Network_Url_Data *url_con)
{
   return url_con->url;
}

EAPI int
ecore_con_url_status_code_get(Ecore_Con_Url *url_obj)
{
   Ecore_Con_Url_Data *url_con = efl_data_scope_get(url_obj, MY_CLASS);
   if (!efl_isa(url_obj, EFL_NETWORK_URL_CLASS))
      return 0;
   if (url_con->status) return url_con->status;
   _ecore_con_url_status_get(url_obj);
   return url_con->status;
}

EOLIAN static Eina_Bool
_efl_network_url_url_set(Efl_Network_Url *url_obj EINA_UNUSED, Efl_Network_Url_Data *url_con, const char *url)
{
   if (!_c) return EINA_FALSE;
   if (url_con->dead) return EINA_FALSE;
   eina_stringshare_replace(&url_con->url, url);
   if (url_con->url)
     _c->curl_easy_setopt(url_con->curl_easy, CURLOPT_URL,
                          url_con->url);
   else
     _c->curl_easy_setopt(url_con->curl_easy, CURLOPT_URL, "");
   return EINA_TRUE;
}

EAPI void
ecore_con_url_data_set(Ecore_Con_Url *url_obj, void *data)
{
   Ecore_Con_Url_Data *url_con = efl_data_scope_get(url_obj, MY_CLASS);
   if (!efl_isa(url_obj, EFL_NETWORK_URL_CLASS))
      return;
   url_con->data = data;
}

EAPI void
ecore_con_url_additional_header_add(Ecore_Con_Url *url_obj, const char *key, const char *value)
{
   Ecore_Con_Url_Data *url_con = efl_data_scope_get(url_obj, MY_CLASS);
   if (!efl_isa(url_obj, EFL_NETWORK_URL_CLASS))
      return;
   char *tmp;

   if (url_con->dead) return;
   tmp = malloc(strlen(key) + strlen(value) + 3);
   if (!tmp) return;
   sprintf(tmp, "%s: %s", key, value);
   url_con->additional_headers = eina_list_append(url_con->additional_headers,
                                                  tmp);
}

EAPI void
ecore_con_url_additional_headers_clear(Ecore_Con_Url *url_obj)
{
   Ecore_Con_Url_Data *url_con = efl_data_scope_get(url_obj, MY_CLASS);
   if (!efl_isa(url_obj, EFL_NETWORK_URL_CLASS))
      return;
   char *s;

   EINA_LIST_FREE(url_con->additional_headers, s)
     free(s);
}

EAPI void *
ecore_con_url_data_get(Ecore_Con_Url *url_obj)
{
   Ecore_Con_Url_Data *url_con = efl_data_scope_get(url_obj, MY_CLASS);
   if (!efl_isa(url_obj, EFL_NETWORK_URL_CLASS))
      return NULL;
   return url_con->data;
}

EAPI void
ecore_con_url_time(Ecore_Con_Url *url_obj, Ecore_Con_Url_Time condition, double timestamp)
{
   Ecore_Con_Url_Data *url_con = efl_data_scope_get(url_obj, MY_CLASS);
   if (!efl_isa(url_obj, EFL_NETWORK_URL_CLASS))
      return;
   if (url_con->dead) return;
   url_con->time_condition = condition;
   url_con->timestamp = timestamp;
}

EAPI void
ecore_con_url_fd_set(Ecore_Con_Url *url_obj, int fd)
{
   Ecore_Con_Url_Data *url_con = efl_data_scope_get(url_obj, MY_CLASS);
   if (!efl_isa(url_obj, EFL_NETWORK_URL_CLASS))
      return;
   if (url_con->dead) return;
   url_con->write_fd = fd;
}

EAPI int
ecore_con_url_received_bytes_get(Ecore_Con_Url *url_obj)
{
   Ecore_Con_Url_Data *url_con = efl_data_scope_get(url_obj, MY_CLASS);
   if (!efl_isa(url_obj, EFL_NETWORK_URL_CLASS))
      return -1;
   return url_con->received;
}

EAPI const Eina_List *
ecore_con_url_response_headers_get(Ecore_Con_Url *url_obj)
{
   Ecore_Con_Url_Data *url_con = efl_data_scope_get(url_obj, MY_CLASS);
   if (!efl_isa(url_obj, EFL_NETWORK_URL_CLASS))
      return NULL;
   return url_con->response_headers;
}

EAPI Eina_Bool
ecore_con_url_httpauth_set(Ecore_Con_Url *url_obj, const char *username, const char *password, Eina_Bool safe)
{
   Ecore_Con_Url_Data *url_con = efl_data_scope_get(url_obj, MY_CLASS);
   if (!efl_isa(url_obj, EFL_NETWORK_URL_CLASS))
      return EINA_FALSE;
   CURLcode ret;
   curl_version_info_data *vers = NULL;

   if (!_c) return EINA_FALSE;
   if (url_con->dead) return EINA_FALSE;
   vers = _c->curl_version_info(CURLVERSION_NOW);
   if (vers->version_num >= 0x071301)
     {
        if ((username) && (password))
          {
             if (safe)
               _c->curl_easy_setopt(url_con->curl_easy, CURLOPT_HTTPAUTH,
                                    CURLAUTH_ANYSAFE);
             else
               _c->curl_easy_setopt(url_con->curl_easy, CURLOPT_HTTPAUTH,
                                    CURLAUTH_ANY);

             ret = _c->curl_easy_setopt(url_con->curl_easy, CURLOPT_USERNAME,
                                        username);
             if (ret != CURLE_OK)
               {
                  ERR("Could not set username for HTTP authentication: %s",
                      _c->curl_easy_strerror(ret));
                  return EINA_FALSE;
               }

             ret = _c->curl_easy_setopt(url_con->curl_easy, CURLOPT_PASSWORD,
                                        password);
             if (ret != CURLE_OK)
               {
                  ERR("Could not set password for HTTP authentication: %s",
                      _c->curl_easy_strerror(ret));
                  return EINA_FALSE;
               }
             return EINA_TRUE;
          }
     }
   return EINA_FALSE;
}


static Eina_Bool
_ecore_con_url_send(Ecore_Con_Url *url_obj, Ecore_Con_Url_Mode mode,
                    const void *data, long length, const char *content_type)
{
   Ecore_Con_Url_Data *url_con = efl_data_scope_get(url_obj, MY_CLASS);
   if (!efl_isa(url_obj, EFL_NETWORK_URL_CLASS))
      return EINA_FALSE;
   Eina_List *l;
   const char *s;
   char tmp[512];

   if (!_c) return EINA_FALSE;

   if (!url_con->url) return EINA_FALSE;
   if (url_con->dead) return EINA_FALSE;

   /* Free response headers from previous send() calls */
   EINA_LIST_FREE(url_con->response_headers, s)
     free((char *)s);
   url_con->response_headers = NULL;
   url_con->status = 0;

   _c->curl_slist_free_all(url_con->headers);
   url_con->headers = NULL;
   if ((mode == ECORE_CON_URL_MODE_POST) || (mode == ECORE_CON_URL_MODE_AUTO))
     {
        if (url_con->post_data) free(url_con->post_data);
        url_con->post_data = NULL;
        if ((data) && (length > 0))
          {
             url_con->post_data = malloc(length);
             if (url_con->post_data)
               {
                  memcpy(url_con->post_data, data, length);
                  if ((content_type) && (strlen(content_type) < 450))
                    {
                       snprintf(tmp, sizeof(tmp), "Content-Type: %s",
                                content_type);
                       url_con->headers =
                         _c->curl_slist_append(url_con->headers, tmp);
                    }
                  _c->curl_easy_setopt(url_con->curl_easy,
                                       CURLOPT_POSTFIELDS, url_con->post_data);
                  _c->curl_easy_setopt(url_con->curl_easy,
                                       CURLOPT_POSTFIELDSIZE, length);
               }
             else
               return EINA_FALSE;
          }
        else
          _c->curl_easy_setopt(url_con->curl_easy,
                               CURLOPT_POSTFIELDSIZE, 0);
        if (mode == ECORE_CON_URL_MODE_POST)
          _c->curl_easy_setopt(url_con->curl_easy, CURLOPT_POST, 1);
     }
   else if (mode == ECORE_CON_URL_MODE_HEAD)
     _c->curl_easy_setopt(url_con->curl_easy, CURLOPT_NOBODY, 1L);

   switch (url_con->time_condition)
     {
      case ECORE_CON_URL_TIME_NONE:
        _c->curl_easy_setopt(url_con->curl_easy, CURLOPT_TIMECONDITION,
                             CURL_TIMECOND_NONE);
        break;

      case ECORE_CON_URL_TIME_IFMODSINCE:
        _c->curl_easy_setopt(url_con->curl_easy, CURLOPT_TIMECONDITION,
                             CURL_TIMECOND_IFMODSINCE);
        _c->curl_easy_setopt(url_con->curl_easy, CURLOPT_TIMEVALUE,
                             (long)url_con->timestamp);
        break;

      case ECORE_CON_URL_TIME_IFUNMODSINCE:
        _c->curl_easy_setopt(url_con->curl_easy, CURLOPT_TIMECONDITION,
                             CURL_TIMECOND_IFUNMODSINCE);
        _c->curl_easy_setopt(url_con->curl_easy, CURLOPT_TIMEVALUE,
                             (long)url_con->timestamp);
        break;
     }
   /* Additional headers */
   EINA_LIST_FOREACH(url_con->additional_headers, l, s)
     url_con->headers = _c->curl_slist_append(url_con->headers, s);

   _c->curl_easy_setopt(url_con->curl_easy, CURLOPT_HTTPHEADER,
                        url_con->headers);
   url_con->received = 0;
   return _ecore_con_url_perform(url_obj);
}

EAPI Eina_Bool
ecore_con_url_get(Ecore_Con_Url *url_con)
{
   return _ecore_con_url_send(url_con, ECORE_CON_URL_MODE_GET, NULL, 0, NULL);
}

EAPI Eina_Bool
ecore_con_url_head(Ecore_Con_Url *url_con)
{
   return _ecore_con_url_send(url_con, ECORE_CON_URL_MODE_HEAD, NULL, 0, NULL);
}

EAPI Eina_Bool
ecore_con_url_post(Ecore_Con_Url *url_con, const void *data, long length, const char *content_type)
{
   return _ecore_con_url_send(url_con, ECORE_CON_URL_MODE_POST, data, length, content_type);
}

EAPI Eina_Bool
ecore_con_url_ftp_upload(Ecore_Con_Url *url_obj, const char *filename, const char *user, const char *pass, const char *upload_dir)
{
   Ecore_Con_Url_Data *url_con = efl_data_scope_get(url_obj, MY_CLASS);
   if (!efl_isa(url_obj, MY_CLASS))
      return EINA_FALSE;
   char url[4096];
   char userpwd[4096];
   FILE *fd;
   struct stat file_info;
   CURLcode ret;

   if (!_c) return EINA_FALSE;

   if (url_con->dead) return EINA_FALSE;
   if (!url_con->url) return EINA_FALSE;
   if ((!filename) || (!filename[0])) return EINA_FALSE;

   if (stat(filename, &file_info)) return EINA_FALSE;

   snprintf(userpwd, sizeof(userpwd), "%s:%s", user, pass);
   ret = _c->curl_easy_setopt(url_con->curl_easy, CURLOPT_USERPWD, userpwd);
   if (ret != CURLE_OK)
     {
        ERR("Could not set username and password for FTP upload: %s",
            _c->curl_easy_strerror(ret));
        return EINA_FALSE;
     }

   char tmp[PATH_MAX];
   snprintf(tmp, PATH_MAX, "%s", filename);

   if (upload_dir)
     snprintf(url, sizeof(url), "%s/%s/%s", url_con->url,
              upload_dir, basename(tmp));
   else
     snprintf(url, sizeof(url), "%s/%s", url_con->url,
              basename(tmp));

   if (!ecore_con_url_url_set(url_obj, url))
     return EINA_FALSE;
   _c->curl_easy_setopt(url_con->curl_easy, CURLOPT_INFILESIZE_LARGE,
                        (off_t)file_info.st_size);
   _c->curl_easy_setopt(url_con->curl_easy, CURLOPT_UPLOAD, 1);
   _c->curl_easy_setopt(url_con->curl_easy, CURLOPT_READFUNCTION,
                        _ecore_con_url_read_cb);
   fd = fopen(filename, "rb");
   if (!fd)
     {
        ERR("Could not open \"%s\" for FTP upload", filename);
        return EINA_FALSE;
     }
   _c->curl_easy_setopt(url_con->curl_easy, CURLOPT_READDATA, fd);
   return _ecore_con_url_perform(url_obj);
}

EAPI void
ecore_con_url_cookies_init(Ecore_Con_Url *url_obj)
{
   Ecore_Con_Url_Data *url_con = efl_data_scope_get(url_obj, MY_CLASS);
   if (!efl_isa(url_obj, EFL_NETWORK_URL_CLASS))
      return;
   if (!_c) return;
   if (url_con->dead) return;
   _c->curl_easy_setopt(url_con->curl_easy, CURLOPT_COOKIEFILE, "");
}

EAPI void
ecore_con_url_cookies_ignore_old_session_set(Ecore_Con_Url *url_obj, Eina_Bool ignore)
{
   Ecore_Con_Url_Data *url_con = efl_data_scope_get(url_obj, MY_CLASS);
   if (!efl_isa(url_obj, EFL_NETWORK_URL_CLASS))
      return;
   if (!_c) return;
   if (url_con->dead) return;
   _c->curl_easy_setopt(url_con->curl_easy, CURLOPT_COOKIESESSION, ignore);
}

EAPI void
ecore_con_url_cookies_clear(Ecore_Con_Url *url_obj)
{
   Ecore_Con_Url_Data *url_con = efl_data_scope_get(url_obj, MY_CLASS);
   if (!efl_isa(url_obj, EFL_NETWORK_URL_CLASS))
      return;
   if (!_c) return;
   if (url_con->dead) return;
   _c->curl_easy_setopt(url_con->curl_easy, CURLOPT_COOKIELIST, "ALL");
}

EAPI void
ecore_con_url_cookies_session_clear(Ecore_Con_Url *url_obj)
{
   Ecore_Con_Url_Data *url_con = efl_data_scope_get(url_obj, MY_CLASS);
   if (!efl_isa(url_obj, EFL_NETWORK_URL_CLASS))
      return;
   if (!_c) return;
   if (url_con->dead) return;
   _c->curl_easy_setopt(url_con->curl_easy, CURLOPT_COOKIELIST, "SESS");
}

EAPI void
ecore_con_url_cookies_file_add(Ecore_Con_Url *url_obj, const char *const file_name)
{
   Ecore_Con_Url_Data *url_con = efl_data_scope_get(url_obj, MY_CLASS);
   if (!efl_isa(url_obj, EFL_NETWORK_URL_CLASS))
      return;
   if (!_c) return;
   if (url_con->dead) return;
   _c->curl_easy_setopt(url_con->curl_easy, CURLOPT_COOKIEFILE, file_name);
}

EAPI Eina_Bool
ecore_con_url_cookies_jar_file_set(Ecore_Con_Url *url_obj, const char *const cookiejar_file)
{
   Ecore_Con_Url_Data *url_con = efl_data_scope_get(url_obj, MY_CLASS);
   if (!efl_isa(url_obj, EFL_NETWORK_URL_CLASS))
      return EINA_FALSE;
   CURLcode ret;

   if (!_c) return EINA_FALSE;
   if (url_con->dead) return EINA_FALSE;
   ret = _c->curl_easy_setopt(url_con->curl_easy, CURLOPT_COOKIEJAR,
                              cookiejar_file);
   if (ret != CURLE_OK)
     {
        ERR("Setting the cookie-jar name failed: %s",
            _c->curl_easy_strerror(ret));
        return EINA_FALSE;
     }
   return EINA_TRUE;
}

EAPI void
ecore_con_url_cookies_jar_write(Ecore_Con_Url *url_obj)
{
   Ecore_Con_Url_Data *url_con = efl_data_scope_get(url_obj, MY_CLASS);
   if (!efl_isa(url_obj, EFL_NETWORK_URL_CLASS))
      return;
   if (!_c) return;

   if (url_con->dead) return;
   _c->curl_easy_setopt(url_con->curl_easy, CURLOPT_COOKIELIST, "FLUSH");
}

EAPI void
ecore_con_url_verbose_set(Ecore_Con_Url *url_obj, Eina_Bool verbose)
{
   Ecore_Con_Url_Data *url_con = efl_data_scope_get(url_obj, MY_CLASS);
   if (!efl_isa(url_obj, EFL_NETWORK_URL_CLASS))
      return;
   if (!_c) return;
   if (!url_con->url) return;
   if (url_con->dead) return;
   _c->curl_easy_setopt(url_con->curl_easy, CURLOPT_VERBOSE, (int)verbose);
}

EAPI void
ecore_con_url_ftp_use_epsv_set(Ecore_Con_Url *url_obj, Eina_Bool use_epsv)
{
   Ecore_Con_Url_Data *url_con = efl_data_scope_get(url_obj, MY_CLASS);
   if (!efl_isa(url_obj, EFL_NETWORK_URL_CLASS))
      return;
   if (!_c) return;
   if (!url_con->url) return;
   if (url_con->dead) return;
   _c->curl_easy_setopt(url_con->curl_easy, CURLOPT_FTP_USE_EPSV,
                        (int)use_epsv);
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
ecore_con_url_ssl_verify_peer_set(Ecore_Con_Url *url_obj, Eina_Bool verify)
{
   Ecore_Con_Url_Data *url_con = efl_data_scope_get(url_obj, MY_CLASS);
   if (!efl_isa(url_obj, EFL_NETWORK_URL_CLASS))
      return;
   if (!_c) return;
   if (!url_con->url) return;
   if (url_con->dead) return;
   _c->curl_easy_setopt(url_con->curl_easy, CURLOPT_SSL_VERIFYPEER,
                        (int)verify);
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
ecore_con_url_ssl_ca_set(Ecore_Con_Url *url_obj, const char *ca_path)
{
   Ecore_Con_Url_Data *url_con = efl_data_scope_get(url_obj, MY_CLASS);
   if (!efl_isa(url_obj, EFL_NETWORK_URL_CLASS))
      return -1;
   int res = -1;

   if (!_c) return -1;
   if (!url_con->url) return -1;
   if (url_con->dead) return -1;
   if (ca_path == NULL)
     res = _c->curl_easy_setopt(url_con->curl_easy, CURLOPT_SSL_VERIFYPEER, 0);
   else
     {
        res = _c->curl_easy_setopt(url_con->curl_easy,
                                   CURLOPT_SSL_VERIFYPEER, 1);
        if (!res)
          res = _c->curl_easy_setopt(url_con->curl_easy,
                                     CURLOPT_CAINFO, ca_path);
     }
   return res;
}

EAPI Eina_Bool
ecore_con_url_http_version_set(Ecore_Con_Url *url_obj, Ecore_Con_Url_Http_Version version)
{
   Ecore_Con_Url_Data *url_con = efl_data_scope_get(url_obj, MY_CLASS);
   if (!efl_isa(url_obj, EFL_NETWORK_URL_CLASS))
      return EINA_FALSE;
   int res = -1;

   if (!_c) return EINA_FALSE;
   if (url_con->dead) return EINA_FALSE;
   switch (version)
     {
      case ECORE_CON_URL_HTTP_VERSION_1_0:
        res = _c->curl_easy_setopt(url_con->curl_easy,
                                   CURLOPT_HTTP_VERSION,
                                   CURL_HTTP_VERSION_1_0);
        break;

      case ECORE_CON_URL_HTTP_VERSION_1_1:
        res = _c->curl_easy_setopt(url_con->curl_easy,
                                   CURLOPT_HTTP_VERSION,
                                   CURL_HTTP_VERSION_1_1);
        break;

      default:
        break;
     }
   if (res != CURLE_OK)
     {
        ERR("curl http version setting failed: %s", _c->curl_easy_strerror(res));
        return EINA_FALSE;
     }
   return EINA_TRUE;
}

EAPI Eina_Bool
ecore_con_url_proxy_set(Ecore_Con_Url *url_obj, const char *proxy)
{
   Ecore_Con_Url_Data *url_con = efl_data_scope_get(url_obj, MY_CLASS);
   if (!efl_isa(url_obj, EFL_NETWORK_URL_CLASS))
      return EINA_FALSE;
   int res = -1;
   curl_version_info_data *vers = NULL;

   if (!_c) return EINA_FALSE;
   if (!url_con->url) return EINA_FALSE;
   if (url_con->dead) return EINA_FALSE;
   if (!proxy)
     res = _c->curl_easy_setopt(url_con->curl_easy,
                                CURLOPT_PROXY, "");
   else
     {
        // before curl version 7.21.7, socks protocol:// prefix is not supported
        // (e.g. socks4://, socks4a://, socks5:// or socks5h://, etc.)
        vers = _c->curl_version_info(CURLVERSION_NOW);
        if (vers->version_num < 0x71507)
          {
             url_con->proxy_type = CURLPROXY_HTTP;
             if (strstr(proxy, "socks4a"))
               url_con->proxy_type = CURLPROXY_SOCKS4A;
             else if (strstr(proxy, "socks4"))
               url_con->proxy_type = CURLPROXY_SOCKS4;
             else if (strstr(proxy, "socks5h"))
               url_con->proxy_type = CURLPROXY_SOCKS5_HOSTNAME;
             else if (strstr(proxy, "socks5"))
               url_con->proxy_type = CURLPROXY_SOCKS5;
             res = _c->curl_easy_setopt(url_con->curl_easy, CURLOPT_PROXYTYPE,
                                        url_con->proxy_type);
             if (res != CURLE_OK)
               {
                  ERR("curl proxy type setting failed: %s",
                      _c->curl_easy_strerror(res));
                  url_con->proxy_type = -1;
                  return EINA_FALSE;
               }
          }
        res = _c->curl_easy_setopt(url_con->curl_easy, CURLOPT_PROXY, proxy);
     }
   if (res != CURLE_OK)
     {
        ERR("curl proxy setting failed: %s", _c->curl_easy_strerror(res));
        url_con->proxy_type = -1;
        return EINA_FALSE;
     }
   return EINA_TRUE;
}

EAPI void
ecore_con_url_timeout_set(Ecore_Con_Url *url_obj, double timeout)
{
   Ecore_Con_Url_Data *url_con = efl_data_scope_get(url_obj, MY_CLASS);
   if (!efl_isa(url_obj, EFL_NETWORK_URL_CLASS))
      return;
   if (url_con->dead) return;
   if (!url_con->url || timeout < 0) return;
   if (url_con->timer) ecore_timer_del(url_con->timer);
   url_con->timer = ecore_timer_add(timeout, _ecore_con_url_timeout_cb,
                                    url_obj);
}

EAPI Eina_Bool
ecore_con_url_proxy_username_set(Ecore_Con_Url *url_obj, const char *username)
{
   Ecore_Con_Url_Data *url_con = efl_data_scope_get(url_obj, MY_CLASS);
   if (!efl_isa(url_obj, EFL_NETWORK_URL_CLASS))
      return EINA_FALSE;
   int res = -1;

   if (!_c) return EINA_FALSE;
   if (url_con->dead) return EINA_FALSE;
   if (!url_con->url) return EINA_FALSE;
   if ((!username) || (!username[0])) return EINA_FALSE;
   if ((url_con->proxy_type == CURLPROXY_SOCKS4) ||
       (url_con->proxy_type == CURLPROXY_SOCKS4A))
     {
        ERR("Proxy type should be socks5 and above");
        return EINA_FALSE;
     }

   res = _c->curl_easy_setopt(url_con->curl_easy, CURLOPT_USERNAME, username);
   if (res != CURLE_OK)
     {
        ERR("curl_easy_setopt() failed: %s", _c->curl_easy_strerror(res));
        return EINA_FALSE;
     }
   return EINA_TRUE;
}

EAPI Eina_Bool
ecore_con_url_proxy_password_set(Ecore_Con_Url *url_obj, const char *password)
{
   Ecore_Con_Url_Data *url_con = efl_data_scope_get(url_obj, MY_CLASS);
   if (!efl_isa(url_obj, EFL_NETWORK_URL_CLASS))
      return EINA_FALSE;
   int res = -1;

   if (!_c) return EINA_FALSE;
   if (!url_con->url) return EINA_FALSE;
   if (url_con->dead) return EINA_FALSE;
   if (!password) return EINA_FALSE;
   if (url_con->proxy_type == CURLPROXY_SOCKS4 || url_con->proxy_type == CURLPROXY_SOCKS4A)
     {
        ERR("Proxy type should be socks5 and above");
        return EINA_FALSE;
     }
   res = _c->curl_easy_setopt(url_con->curl_easy, CURLOPT_PASSWORD, password);
   if (res != CURLE_OK)
     {
        ERR("curl_easy_setopt() failed: %s", _c->curl_easy_strerror(res));
        return EINA_FALSE;
     }
   return EINA_TRUE;
}

/**
 * @}
 */

static void
_ecore_con_url_status_get(Ecore_Con_Url *url_obj)
{
   Ecore_Con_Url_Data *url_con = efl_data_scope_get(url_obj, MY_CLASS);
   long status = 0;

   if (!_c) return;
   if (!url_con->curl_easy) return;
   if (!_c->curl_easy_getinfo(url_con->curl_easy, CURLINFO_RESPONSE_CODE,
                              &status))
     url_con->status = status;
   else
     url_con->status = 0;
}

static void
_ecore_con_url_event_url_complete(Ecore_Con_Url *url_obj, CURLMsg *curlmsg)
{
   Efl_Network_Url_Data *url_con = efl_data_scope_get(url_obj, MY_CLASS);
   Efl_Network_Event_Url_Complete e;
   int status = url_con->status;

   if (!_c) return;

   if (!curlmsg)
     ERR("Event completed without CURL message handle. Shouldn't happen");
   else if ((curlmsg->msg == CURLMSG_DONE) &&
            (curlmsg->data.result == CURLE_OPERATION_TIMEDOUT) &&
            (!curlmsg->easy_handle))
     {
        /* easy_handle is set to NULL on timeout messages */
        status = 408; /* Request Timeout */
     }
   else if (curlmsg->data.result == CURLE_OK)
     {
        if (!status)
          {
             _ecore_con_url_status_get(url_obj);
             status = url_con->status;
          }
     }
   else
     {
        ERR("Curl message have errors: %d (%s)",
            curlmsg->data.result, _c->curl_easy_strerror(curlmsg->data.result));
     }
   e.status = status;
   e.url_con = url_obj;
   url_con->event_count++;
   efl_event_callback_legacy_call(url_obj, EFL_NETWORK_URL_EVENT_COMPLETE, &e);
}

static void
_ecore_con_url_multi_remove(Ecore_Con_Url *url_obj)
{
   Ecore_Con_Url_Data *url_con = efl_data_scope_get(url_obj, MY_CLASS);
   CURLMcode ret;

   if (!_c) return;
   ret = _c->curl_multi_remove_handle(_c->_curlm, url_con->curl_easy);
   url_con->multi = EINA_FALSE;
   if (ret != CURLM_OK) ERR("curl_multi_remove_handle failed: %s", _c->curl_multi_strerror(ret));
}

static Eina_Bool
_ecore_con_url_timeout_cb(void *data)
{
   Ecore_Con_Url *url_obj = data;
   Ecore_Con_Url_Data *url_con = efl_data_scope_get(url_obj, MY_CLASS);
   CURLMsg timeout_msg;

   if (!_c) return ECORE_CALLBACK_CANCEL;
   if (!url_obj) return ECORE_CALLBACK_CANCEL;
   if (!url_con->curl_easy) return ECORE_CALLBACK_CANCEL;

   _ecore_con_url_multi_remove(url_obj);
   _url_con_list = eina_list_remove(_url_con_list, url_obj);

   _c->curl_slist_free_all(url_con->headers);
   url_con->headers = NULL;

   url_con->timer = NULL;

   timeout_msg.msg = CURLMSG_DONE;
   timeout_msg.easy_handle = NULL;
   timeout_msg.data.result = CURLE_OPERATION_TIMEDOUT;

   _ecore_con_url_event_url_complete(url_obj, &timeout_msg);
   return ECORE_CALLBACK_CANCEL;
}

static size_t
_ecore_con_url_data_cb(void *buffer, size_t size, size_t nitems, void *userp)
{
   Efl_Network_Url *url_obj = (Efl_Network_Url *)userp;
   Efl_Network_Event_Url_Data e;
   size_t real_size = size * nitems;

   Efl_Network_Url_Data *url_con = efl_data_scope_get(url_obj, MY_CLASS);
   if (!efl_isa(url_obj, EFL_NETWORK_URL_CLASS))
      return -1;

   url_con->received += real_size;
   INF("reading from %s", url_con->url);
   if (url_con->write_fd < 0)
     {
        e.url_con = url_obj;
        e.size = real_size;
        e.data = buffer;
        url_con->event_count++;
        efl_event_callback_legacy_call(url_obj, EFL_NETWORK_URL_EVENT_DATA, &e);
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
                  if ((errno != EAGAIN) && (errno != EINTR)) return -1;
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

static size_t
_ecore_con_url_header_cb(void *ptr, size_t size, size_t nitems, void *stream)
{
   size_t real_size = size * nitems;
   Ecore_Con_Url *url_obj = stream;
   Ecore_Con_Url_Data *url_con = efl_data_scope_get(url_obj, MY_CLASS);
   if (!efl_isa(url_obj, EFL_NETWORK_URL_CLASS))
      return 0;

   char *header = malloc(sizeof(char) * (real_size + 1));
   if (!header) return real_size;
   memcpy(header, ptr, real_size);
   header[real_size] = '\0';
   url_con->response_headers = eina_list_append(url_con->response_headers,
                                                header);
   return real_size;
}

static int
_ecore_con_url_progress_cb(void *clientp, double dltotal, double dlnow, double ultotal, double ulnow)
{
   Efl_Network_Event_Url_Progress e;
   Efl_Network_Url *url_obj = clientp;

   Efl_Network_Url_Data *url_con = efl_data_scope_get(url_obj, MY_CLASS);

   e.url_con = url_obj;
   e.down.total = dltotal;
   e.down.now = dlnow;
   e.up.total = ultotal;
   e.up.now = ulnow;
   url_con->event_count++;
   efl_event_callback_legacy_call(url_obj, EFL_NETWORK_URL_EVENT_PROGRESS, &e);

   return 0;
}

static size_t
_ecore_con_url_read_cb(void *ptr, size_t size, size_t nitems, void *stream)
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
   INF("*** We read %zu bytes from file", retcode);
   return retcode;
}

static void
_ecore_con_url_info_read(void)
{
   CURLMsg *curlmsg;
   int n_remaining;

   if (!_c) return;
   while ((curlmsg = _c->curl_multi_info_read(_c->_curlm, &n_remaining)))
     {
        Eina_List *l, *ll;
        Ecore_Con_Url *url_obj = NULL;

        DBG("Curl message: %d", curlmsg->msg);
        if (curlmsg->msg == CURLMSG_DONE)
          {
             EINA_LIST_FOREACH_SAFE(_url_con_list, l, ll, url_obj)
               {
                  Ecore_Con_Url_Data *url_con = efl_data_scope_get(url_obj, MY_CLASS);
                  if (curlmsg->easy_handle == url_con->curl_easy)
                    _ecore_con_url_event_url_complete(url_obj, curlmsg);
               }
          }
     }
}

static void
_ecore_con_url_curl_clear(void)
{
   Ecore_Fd_Handler *fdh;
   Ecore_Con_Url *url_con;

   EINA_LIST_FREE(_fd_hd_list, fdh)
     ecore_main_fd_handler_del(fdh);
   EINA_LIST_FREE(_url_con_list, url_con)
     _ecore_con_url_multi_remove(url_con);
}

static Eina_Bool
_ecore_con_url_do_multi_timeout(long *retms)
{
   long ms = 0;
   int ret;

   ret = _c->curl_multi_timeout(_c->_curlm, &ms);
   *retms = ms;
   if (!ret)
     {
        if (!ms)
          {
             _ecore_con_url_timer(NULL);
             DBG("multiperform is still running: timeout: %ld", ms);
          }
        return EINA_TRUE;
     }
   else
     {
         ERR("curl_multi_perform() failed: %s",
              _c->curl_multi_strerror(ret));
         _ecore_con_url_curl_clear();
         ecore_timer_freeze(_curl_timer);

         return EINA_FALSE;
     }
}

static Eina_Bool
_ecore_con_url_fd_handler(void *data EINA_UNUSED, Ecore_Fd_Handler *fd_handler EINA_UNUSED)
{
   Ecore_Fd_Handler *fdh;
   long ms = 0;

   if (!_c) return ECORE_CALLBACK_CANCEL;
   EINA_LIST_FREE(_fd_hd_list, fdh)
     ecore_main_fd_handler_del(fdh);
   if (!_ecore_con_url_do_multi_timeout(&ms)) return EINA_FALSE;
   if ((ms >= CURL_MIN_TIMEOUT) || (ms <= 0)) ms = CURL_MIN_TIMEOUT;
   ecore_timer_interval_set(_curl_timer, (double)ms / 1000.0);
   ecore_timer_reset(_curl_timer);
   _ecore_con_url_timer(NULL);
   return ECORE_CALLBACK_CANCEL;
}

static void
_ecore_con_url_fdset(void)
{
   CURLMcode ret;
   fd_set read_set, write_set, exc_set;
   int fd, fd_max;

   if (!_c) return;

   FD_ZERO(&read_set);
   FD_ZERO(&write_set);
   FD_ZERO(&exc_set);

   ret = _c->curl_multi_fdset(_c->_curlm, &read_set,
                              &write_set, &exc_set, &fd_max);
   if (ret != CURLM_OK)
     {
        ERR("curl_multi_fdset failed: %s", _c->curl_multi_strerror(ret));
        return;
     }

   for (fd = 0; fd <= fd_max; fd++)
     {
        int flags = 0;
        if (FD_ISSET(fd, &read_set)) flags |= ECORE_FD_READ;
        if (FD_ISSET(fd, &write_set)) flags |= ECORE_FD_WRITE;
        if (FD_ISSET(fd, &exc_set)) flags |= ECORE_FD_ERROR;
        if (flags)
          {
             // FIXME: Who is owner (easy_handle) of this fd??
             // (Curl do not give this info.)
             // This cause "Failed to delete epoll fd xx!" error messages
             Ecore_Fd_Handler *fd_handler;
             fd_handler = ecore_main_fd_handler_add(fd, flags,
                                                    _ecore_con_url_fd_handler,
                                                    NULL, NULL, NULL);
             if (fd_handler)
               _fd_hd_list = eina_list_append(_fd_hd_list, fd_handler);
          }
     }
}

static Eina_Bool
_ecore_con_url_timer(void *data EINA_UNUSED)
{
   Ecore_Fd_Handler *fdh;
   int still_running;
   CURLMcode ret;

   EINA_LIST_FREE(_fd_hd_list, fdh)
     ecore_main_fd_handler_del(fdh);
   _ecore_con_url_info_read();
   if (!_c) return ECORE_CALLBACK_RENEW;
   ret = _c->curl_multi_perform(_c->_curlm, &still_running);
   if (ret == CURLM_CALL_MULTI_PERFORM)
     {
        DBG("curl_multi_perform() again immediately");
        ecore_timer_interval_set(_curl_timer, 0.000001);
        return ECORE_CALLBACK_RENEW;
     }
   else if (ret != CURLM_OK)
     {
        ERR("curl_multi_perform() failed: %s", _c->curl_multi_strerror(ret));
        _ecore_con_url_curl_clear();
        ecore_timer_freeze(_curl_timer);
     }
   if (still_running)
     {
        long ms = 0;

        _ecore_con_url_fdset();
        if (!_ecore_con_url_do_multi_timeout(&ms)) return EINA_FALSE;
        DBG("multiperform is still running: %d, timeout: %ld",
            still_running, ms);
        if ((ms >= CURL_MIN_TIMEOUT) || (ms < 0)) ms = CURL_MIN_TIMEOUT;
        ecore_timer_interval_set(_curl_timer, (double)ms / 1000.0);
     }
   else
     {
        DBG("multiperform ended");
        _ecore_con_url_info_read();
        _ecore_con_url_curl_clear();
        ecore_timer_freeze(_curl_timer);
     }
   return ECORE_CALLBACK_RENEW;
}

static Eina_Bool
_ecore_con_url_perform(Ecore_Con_Url *url_obj)
{
   Ecore_Con_Url_Data *url_con = efl_data_scope_get(url_obj, MY_CLASS);
   CURLMcode ret;

   if (!_c) return EINA_FALSE;
   ret = _c->curl_multi_add_handle(_c->_curlm, url_con->curl_easy);
   if (ret != CURLM_OK)
     {
        ERR("curl_multi_add_handle() failed: %s",
            _c->curl_multi_strerror(ret));
        return EINA_FALSE;
     }
   url_con->multi = EINA_TRUE;
   _url_con_list = eina_list_append(_url_con_list, url_obj);
   ecore_timer_thaw(_curl_timer);
   return EINA_TRUE;
}

static void
_ecore_con_event_url_free(Ecore_Con_Url *url_obj, void *ev)
{
   Ecore_Con_Url_Data *url_con = efl_data_scope_get(url_obj, MY_CLASS);

   free(ev);
   url_con->event_count--;
   if (url_con->dead && (!url_con->event_count))
     {
        _ecore_con_url_free_internal(url_obj);
        efl_manual_free(url_obj);
     }
}

#include "efl_network_url.eo.c"
#include "ecore_con_legacy.c"
