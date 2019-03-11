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
#include <fcntl.h>
#include <libgen.h>

#include "Ecore.h"
#include "ecore_private.h"
#include "Ecore_Con.h"
#include "ecore_con_private.h"
#include "ecore_con_url_curl.h"
#include "Emile.h"

int ECORE_CON_EVENT_URL_DATA = 0;
int ECORE_CON_EVENT_URL_COMPLETE = 0;
int ECORE_CON_EVENT_URL_PROGRESS = 0;

static int _init_count = 0;
static Eina_Bool pipelining = EINA_FALSE;

static Eina_List *_url_con_url_list = NULL;

/**
 * @addtogroup Ecore_Con_Url_Group Ecore URL Connection Functions
 *
 * @{
 */

EAPI int
ecore_con_url_init(void)
{
   if (++_init_count > 1) return _init_count;
   if (!ecore_init()) goto ecore_init_failed;
   if (!ecore_con_init()) goto ecore_con_init_failed;
   if (!emile_init()) goto emile_init_failed;
   if (!emile_cipher_init()) goto emile_cipher_init_failed;
   _c_init();
   ECORE_CON_EVENT_URL_DATA = ecore_event_type_new();
   ECORE_CON_EVENT_URL_COMPLETE = ecore_event_type_new();
   ECORE_CON_EVENT_URL_PROGRESS = ecore_event_type_new();
   return _init_count;

 emile_cipher_init_failed:
   emile_shutdown();
 emile_init_failed:
   ecore_con_shutdown();
 ecore_con_init_failed:
   ecore_shutdown();
 ecore_init_failed:
   return --_init_count;
}

EAPI int
ecore_con_url_shutdown(void)
{
   Ecore_Con_Url *url_con_url;
   if (_init_count == 0) return 0;
   --_init_count;
   if (_init_count) return _init_count;
   EINA_LIST_FREE(_url_con_url_list, url_con_url)
     ecore_con_url_free(url_con_url);

   ecore_event_type_flush(ECORE_CON_EVENT_URL_DATA,
                          ECORE_CON_EVENT_URL_COMPLETE,
                          ECORE_CON_EVENT_URL_PROGRESS);

   _c_shutdown();
   emile_shutdown(); /* no emile_cipher_shutdown(), handled here */
   ecore_con_shutdown();
   ecore_shutdown();
   return 0;
}

EAPI void
ecore_con_url_pipeline_set(Eina_Bool enable)
{
   if (!_c_init()) return;
   if (enable == pipelining) return;
   _c->curl_multi_setopt(_c->_curlm, CURLMOPT_PIPELINING, !!enable);
   pipelining = enable;
}

EAPI Eina_Bool
ecore_con_url_pipeline_get(void)
{
   return pipelining;
}


/* The rest of this file exists solely to provide ABI compatibility */

struct _Ecore_Con_Url
{
   ECORE_MAGIC;
   Eo *dialer;
   Eo *send_copier;
   Eo *input;
   Ecore_Timer *timer;
   struct {
      Ecore_Animator *animator;
      struct {
         uint64_t total;
         uint64_t now;
      } download, upload;
   } progress;
   Eina_Stringshare *url;
   Eina_Stringshare *custom_request;
   void *data;
   struct {
      Eina_List *files; /* of Eina_Stringshare - read locations */
      Eina_List *cmds; /* of static-const strings - COOKIELIST commands */
      Eina_Stringshare *jar; /* write location */
      Eina_Bool ignore_old_session;
   } cookies;
   struct {
      Eina_Stringshare *url;
      Eina_Stringshare *username;
      Eina_Stringshare *password;
   } proxy;
   struct {
      Ecore_Con_Url_Time condition;
      double stamp;
   } time;
   struct {
      Eina_Stringshare *username;
      Eina_Stringshare *password;
      Efl_Net_Http_Authentication_Method method;
      Eina_Bool restricted;
   } httpauth;
   Eina_Stringshare *ca_path;
   Eina_List *request_headers;
   Eina_List *response_headers;
   unsigned event_count;
   int received_bytes;
   int status;
   int write_fd;
   Efl_Net_Http_Version http_version;
   Eina_Bool ssl_verify_peer;
   Eina_Bool verbose;
   Eina_Bool ftp_use_epsv;
   Eina_Bool delete_me;
};

#define ECORE_CON_URL_CHECK_RETURN(u, ...) \
  do \
    { \
       if (!ECORE_MAGIC_CHECK(u, ECORE_MAGIC_CON_URL)) \
         { \
            ECORE_MAGIC_FAIL(u, ECORE_MAGIC_CON_URL, __FUNCTION__); \
            return __VA_ARGS__; \
         } \
       EINA_SAFETY_ON_TRUE_RETURN_VAL(u->delete_me, __VA_ARGS__); \
    } \
  while (0)


static void
_ecore_con_url_dialer_close(Ecore_Con_Url *url_con)
{
   if (url_con->send_copier)
     {
        efl_del(url_con->send_copier);
        url_con->send_copier = NULL;
     }

   if (url_con->input)
     {
        efl_del(url_con->input);
        url_con->input = NULL;
     }

   if (url_con->timer)
     {
        ecore_timer_del(url_con->timer);
        url_con->timer = NULL;
     }

   if (url_con->progress.animator)
     {
        ecore_animator_del(url_con->progress.animator);
        url_con->progress.animator = NULL;
     }

   if (!url_con->dialer) return;

   if (!efl_io_closer_closed_get(url_con->dialer))
     efl_io_closer_close(url_con->dialer);
   efl_del(url_con->dialer);
   url_con->dialer = NULL;
}

static void
_ecore_con_url_response_headers_free(Ecore_Con_Url *url_con)
{
   char *str;
   EINA_LIST_FREE(url_con->response_headers, str)
     free(str);
}

static void
_ecore_con_url_request_headers_free(Ecore_Con_Url *url_con)
{
   Efl_Net_Http_Header *header;
   EINA_LIST_FREE(url_con->response_headers, header)
     free(header); /* key and value are inline */
}

static void
_ecore_con_url_free_internal(Ecore_Con_Url *url_con)
{
   const char *s;

   url_con->delete_me = EINA_TRUE;
   if (url_con->event_count > 0) return;

   _ecore_con_url_dialer_close(url_con);

   eina_stringshare_replace(&url_con->url, NULL);
   eina_stringshare_replace(&url_con->custom_request, NULL);

   url_con->data = NULL;

   EINA_LIST_FREE(url_con->cookies.files, s)
     eina_stringshare_del(s);
   eina_list_free(url_con->cookies.cmds); /* data is not to be freed! */
   eina_stringshare_replace(&url_con->cookies.jar, NULL);

   eina_stringshare_replace(&url_con->proxy.url, NULL);
   eina_stringshare_replace(&url_con->proxy.username, NULL);
   eina_stringshare_replace(&url_con->proxy.password, NULL);

   eina_stringshare_replace(&url_con->httpauth.username, NULL);
   eina_stringshare_replace(&url_con->httpauth.password, NULL);

   eina_stringshare_replace(&url_con->ca_path, NULL);

   _ecore_con_url_request_headers_free(url_con);
   _ecore_con_url_response_headers_free(url_con);

   ECORE_MAGIC_SET(url_con, ECORE_MAGIC_NONE);
   free(url_con);
}

static void
_ecore_con_event_url_progress_free(void *data EINA_UNUSED, void *event)
{
   Ecore_Con_Event_Url_Progress *ev = event;
   Ecore_Con_Url *url_con = ev->url_con;

   EINA_SAFETY_ON_TRUE_GOTO(url_con->event_count == 0, end);

   url_con->event_count--;
   if ((url_con->event_count == 0) && (url_con->delete_me))
     _ecore_con_url_free_internal(url_con);

 end:
   free(ev);
}

static void
_ecore_con_event_url_progress_add(Ecore_Con_Url *url_con)
{
   Ecore_Con_Event_Url_Progress *ev;

   if (url_con->delete_me) return;

   ev = malloc(sizeof(*ev));
   EINA_SAFETY_ON_NULL_RETURN(ev);

   ev->url_con = url_con;
   ev->down.total = url_con->progress.download.total;
   ev->down.now = url_con->progress.download.now;
   ev->up.total = url_con->progress.upload.total;
   ev->up.now = url_con->progress.upload.now;
   url_con->event_count++;
   ecore_event_add(ECORE_CON_EVENT_URL_PROGRESS, ev, _ecore_con_event_url_progress_free, NULL);
}

static void
_ecore_con_event_url_complete_free(void *data EINA_UNUSED, void *event)
{
   Ecore_Con_Event_Url_Complete *ev = event;
   Ecore_Con_Url *url_con = ev->url_con;

   EINA_SAFETY_ON_TRUE_GOTO(url_con->event_count == 0, end);

   url_con->event_count--;
   if ((url_con->event_count == 0) && (url_con->delete_me))
     _ecore_con_url_free_internal(url_con);

 end:
   free(ev);
}

static void
_ecore_con_event_url_complete_add(Ecore_Con_Url *url_con, int status)
{
   Ecore_Con_Event_Url_Complete *ev;

   if (url_con->delete_me) return;

   if (url_con->progress.animator)
     {
        ecore_animator_del(url_con->progress.animator);
        url_con->progress.animator = NULL;
        _ecore_con_event_url_progress_add(url_con);
     }

   if (url_con->status)
     {
        DBG("URL '%s' was already complete with status=%d, new=%d", url_con->url, url_con->status, status);
        goto end;
     }

   url_con->status = status;

   ev = malloc(sizeof(Ecore_Con_Event_Url_Complete));
   EINA_SAFETY_ON_NULL_GOTO(ev, end);

   ev->url_con = url_con;
   ev->status = status;
   url_con->event_count++;
   ecore_event_add(ECORE_CON_EVENT_URL_COMPLETE, ev, _ecore_con_event_url_complete_free, NULL);

 end:
   _ecore_con_url_dialer_close(url_con);
}

static void
_ecore_con_url_dialer_error(void *data, const Efl_Event *event)
{
   Ecore_Con_Url *url_con = data;
   Eina_Error *perr = event->info;
   int status;

   status = efl_net_dialer_http_response_status_get(url_con->dialer);
   if ((status < 500) || (status > 599))
     {
        DBG("HTTP error %d reset to 1", status);
        status = 1; /* not a real HTTP error */
     }

   WRN("HTTP dialer error url='%s': %s",
       efl_net_dialer_address_dial_get(url_con->dialer),
       eina_error_msg_get(*perr));

   _ecore_con_event_url_complete_add(url_con, status);
}

static void
_ecore_con_event_url_data_free(void *data EINA_UNUSED, void *event)
{
   Ecore_Con_Event_Url_Data *ev = event;
   Ecore_Con_Url *url_con = ev->url_con;

   EINA_SAFETY_ON_TRUE_GOTO(url_con->event_count == 0, end);

   url_con->event_count--;
   if ((url_con->event_count == 0) && (url_con->delete_me))
     _ecore_con_url_free_internal(url_con);

 end:
   free(ev);
}

static void
_ecore_con_url_dialer_can_read_changed(void *data, const Efl_Event *event EINA_UNUSED)
{
   Ecore_Con_Url *url_con = data;
   Eina_Bool can_read;
   Ecore_Con_Event_Url_Data *ev;
   Eina_Rw_Slice slice;
   Eina_Error err;

   if (url_con->delete_me) return;

   can_read = efl_io_reader_can_read_get(url_con->dialer);
   if (!can_read) return;

   ev = malloc(sizeof(Ecore_Con_Event_Url_Data) + EFL_NET_DIALER_HTTP_BUFFER_RECEIVE_SIZE);
   EINA_SAFETY_ON_NULL_RETURN(ev);

   slice.mem = ev->data;
   slice.len = EFL_NET_DIALER_HTTP_BUFFER_RECEIVE_SIZE;

   err = efl_io_reader_read(url_con->dialer, &slice);
   if (err)
     {
        free(ev);
        if (err == EAGAIN) return;
        WRN("Error reading data from HTTP url='%s': %s",
            efl_net_dialer_address_dial_get(url_con->dialer),
            eina_error_msg_get(err));
        return;
     }

   ev->size = slice.len;
   ev->url_con = url_con;
   url_con->received_bytes += ev->size;

   if (url_con->write_fd == -1)
     {
        url_con->event_count++;
        ecore_event_add(ECORE_CON_EVENT_URL_DATA, ev, _ecore_con_event_url_data_free, NULL);
        return;
     }

   while (slice.len > 0)
     {
        ssize_t r = write(url_con->write_fd, slice.bytes, slice.len);
        if (r == -1)
          {
             ERR("Could not write to fd=%d: %s", url_con->write_fd, eina_error_msg_get(errno));
             break;
          }
        slice.bytes += r;
        slice.len -= r;
     }
   free(ev);
}

static void
_ecore_con_url_dialer_eos(void *data, const Efl_Event *event EINA_UNUSED)
{
   Ecore_Con_Url *url_con = data;

   DBG("HTTP EOS url='%s'", efl_net_dialer_address_dial_get(url_con->dialer));

   if (url_con->send_copier && (!efl_io_copier_done_get(url_con->send_copier)))
     {
        DBG("done receiving, waiting for send copier...");
        return;
     }

   _ecore_con_event_url_complete_add(url_con, efl_net_dialer_http_response_status_get(url_con->dialer));
}

static void
_ecore_con_url_dialer_headers_done(void *data, const Efl_Event *event EINA_UNUSED)
{
   Ecore_Con_Url *url_con = data;
   Eina_Iterator *it;
   Efl_Net_Http_Header *header;
   size_t len;
   int status = efl_net_dialer_http_response_status_get(url_con->dialer);
   char *str;

   DBG("HTTP headers done, status=%d url='%s'",
       status,
       efl_net_dialer_address_dial_get(url_con->dialer));

   _ecore_con_url_response_headers_free(url_con);

   it = efl_net_dialer_http_response_headers_all_get(url_con->dialer);
   EINA_SAFETY_ON_NULL_RETURN(it);
   EINA_ITERATOR_FOREACH(it, header)
     {
        if (header->key)
          {
             len = strlen(header->key) + strlen(header->value) + strlen(": \r\n") + 1;
             str = malloc(len);
             EINA_SAFETY_ON_NULL_GOTO(str, end);
             snprintf(str, len, "%s: %s\r\n", header->key, header->value);
             url_con->response_headers = eina_list_append(url_con->response_headers, str);
          }
        else
          {
             if (url_con->response_headers)
               {
                  str = malloc(strlen("\r\n") + 1);
                  EINA_SAFETY_ON_NULL_GOTO(str, end);
                  memcpy(str, "\r\n", strlen("\r\n") + 1);
                  url_con->response_headers = eina_list_append(url_con->response_headers, str);
               }

             len = strlen(header->value) + strlen("\r\n") + 1;
             str = malloc(len);
             EINA_SAFETY_ON_NULL_GOTO(str, end);
             snprintf(str, len, "%s\r\n", header->value);
             url_con->response_headers = eina_list_append(url_con->response_headers, str);
          }
     }

   str = malloc(strlen("\r\n") + 1);
   EINA_SAFETY_ON_NULL_GOTO(str, end);
   memcpy(str, "\r\n", strlen("\r\n") + 1);
   url_con->response_headers = eina_list_append(url_con->response_headers, str);

 end:
   eina_iterator_free(it);
}

EFL_CALLBACKS_ARRAY_DEFINE(ecore_con_url_dialer_cbs,
                           { EFL_IO_READER_EVENT_CAN_READ_CHANGED, _ecore_con_url_dialer_can_read_changed },
                           { EFL_IO_READER_EVENT_EOS, _ecore_con_url_dialer_eos },
                           { EFL_NET_DIALER_EVENT_ERROR, _ecore_con_url_dialer_error },
                           { EFL_NET_DIALER_HTTP_EVENT_HEADERS_DONE, _ecore_con_url_dialer_headers_done });

static Eina_Bool
_ecore_con_url_progress_animator_cb(void *data)
{
   Ecore_Con_Url *url_con = data;
   uint64_t dn, dt, un, ut;

   efl_net_dialer_http_progress_download_get(url_con->dialer, &dn, &dt);
   efl_net_dialer_http_progress_upload_get(url_con->dialer, &un, &ut);

   if ((dn == url_con->progress.download.now) &&
       (dt == url_con->progress.download.total) &&
       (un == url_con->progress.upload.now) &&
       (ut == url_con->progress.upload.total))
     return EINA_TRUE;

   url_con->progress.download.now = dn;
   url_con->progress.download.total = dt;
   url_con->progress.upload.now = un;
   url_con->progress.upload.total = ut;

   _ecore_con_event_url_progress_add(url_con);

   return EINA_TRUE;
}

/*
 * creates a new efl_net_dialer_proxy_set() URL based on legacy parameters:
 *  - proxy url (that could contain user + pass encoded, optional protocol)
 *    - no protocol = http://
 *  - username
 *  - password
 *
 * May return NULL (= use envvar http_proxy)
 */
static char *
_ecore_con_url_proxy_url_new(const Ecore_Con_Url *url_con)
{
   Eina_Slice protocol, user = {}, pass = {}, address;
   char buf[4096];
   const char *p;

   if (!url_con->proxy.url) return NULL; /* use from envvar */

   p = strstr(url_con->proxy.url, "://");
   if (!p)
     {
        protocol = (Eina_Slice)EINA_SLICE_STR_LITERAL("http");
        address = (Eina_Slice)EINA_SLICE_STR(url_con->proxy.url);
     }
   else
     {
        const char *s;

        protocol.mem = url_con->proxy.url;
        protocol.len = p - url_con->proxy.url;
        if (protocol.len == 0)
          protocol = (Eina_Slice)EINA_SLICE_STR_LITERAL("http");

        p += strlen("://");
        s = strchr(p, '@');
        if (!s)
          {
             address = (Eina_Slice)EINA_SLICE_STR(p);
          }
        else
          {
             address = (Eina_Slice)EINA_SLICE_STR(s + 1);

             user.mem = p;
             user.len = s - p;

             s = eina_slice_strchr(user, ':');
             if (s)
               {
                  pass.mem = s + 1;
                  pass.len = user.len - (user.bytes - pass.bytes);
                  user.len = s - (const char *)user.bytes;
               }
          }
     }

   if (url_con->proxy.username)
     user = eina_stringshare_slice_get(url_con->proxy.username);

   if (url_con->proxy.password)
     pass = eina_stringshare_slice_get(url_con->proxy.password);

   if (user.len && pass.len)
     {
        snprintf(buf, sizeof(buf),
                 EINA_SLICE_STR_FMT "://"
                 EINA_SLICE_STR_FMT ":"
                 EINA_SLICE_STR_FMT "@"
                 EINA_SLICE_STR_FMT,
                 EINA_SLICE_STR_PRINT(protocol),
                 EINA_SLICE_STR_PRINT(user),
                 EINA_SLICE_STR_PRINT(pass),
                 EINA_SLICE_STR_PRINT(address));
     }
   else if (user.len)
     {
        snprintf(buf, sizeof(buf),
                 EINA_SLICE_STR_FMT "://"
                 EINA_SLICE_STR_FMT "@"
                 EINA_SLICE_STR_FMT,
                 EINA_SLICE_STR_PRINT(protocol),
                 EINA_SLICE_STR_PRINT(user),
                 EINA_SLICE_STR_PRINT(address));
     }
   else
     {
        snprintf(buf, sizeof(buf),
                 EINA_SLICE_STR_FMT "://" EINA_SLICE_STR_FMT,
                 EINA_SLICE_STR_PRINT(protocol),
                 EINA_SLICE_STR_PRINT(address));
     }

   return strdup(buf);
}

static void
_ecore_con_url_copier_done(void *data, const Efl_Event *event)
{
   Ecore_Con_Url *url_con = data;
   int status = efl_net_dialer_http_response_status_get(url_con->dialer);

   DBG("copier %s %p for url='%s' is done", efl_name_get(event->object), event->object, efl_net_dialer_address_dial_get(url_con->dialer));

   if (!efl_io_reader_eos_get(url_con->dialer))
     {
        DBG("done sending, waiting for dialer EOS...");
        return;
     }

   _ecore_con_event_url_complete_add(url_con, status);
}

static void
_ecore_con_url_copier_error(void *data, const Efl_Event *event)
{
   Ecore_Con_Url *url_con = data;
   Eina_Error *perr = event->info;
   int status;

   status = efl_net_dialer_http_response_status_get(url_con->dialer);
   if ((status < 500) || (status > 599))
     {
        DBG("HTTP error %d reset to 1", status);
        status = 1; /* not a real HTTP error */
     }

   WRN("HTTP copier %s %p error url='%s': %s",
       efl_name_get(event->object), event->object,
       efl_net_dialer_address_dial_get(url_con->dialer),
       eina_error_msg_get(*perr));

   _ecore_con_event_url_complete_add(url_con, status);
}

EFL_CALLBACKS_ARRAY_DEFINE(_ecore_con_url_copier_cbs,
                           { EFL_IO_COPIER_EVENT_ERROR, _ecore_con_url_copier_error },
                           { EFL_IO_COPIER_EVENT_DONE, _ecore_con_url_copier_done });

/*
 * Ecore_Con_Url is documented as 'reusable', while Efl.Net.Dialers
 * are one-shot and must be recreated on every usage.
 *
 * Then _ecore_con_url_request_prepare() will close (cancel) any
 * previous dialer and create a new one with all parameters set.
 */
static Eina_Bool
_ecore_con_url_request_prepare(Ecore_Con_Url *url_con, const char *method)
{
   const Efl_Net_Http_Header *header;
   const Eina_List *n;
   const char *s;
   char *proxy_url = NULL;
   CURL *curl_easy;

   _ecore_con_url_dialer_close(url_con);

   url_con->status = 0;
   url_con->received_bytes = 0;
   url_con->progress.download.now = 0;
   url_con->progress.download.total = 0;
   url_con->progress.upload.now = 0;
   url_con->progress.upload.total = 0;
   _ecore_con_url_response_headers_free(url_con);

   proxy_url = _ecore_con_url_proxy_url_new(url_con);
   if (proxy_url)
     DBG("proxy_url='%s'", proxy_url);

   url_con->dialer = efl_add(EFL_NET_DIALER_HTTP_CLASS, efl_main_loop_get(),
                             efl_net_dialer_http_method_set(efl_added, url_con->custom_request ? url_con->custom_request : method),
                             efl_net_dialer_http_primary_mode_set(efl_added, (strcmp(method, "PUT") == 0) ? EFL_NET_DIALER_HTTP_PRIMARY_MODE_UPLOAD : EFL_NET_DIALER_HTTP_PRIMARY_MODE_DOWNLOAD),
                             efl_net_dialer_proxy_set(efl_added, proxy_url),
                             efl_net_dialer_http_authentication_set(efl_added, url_con->httpauth.username, url_con->httpauth.password, url_con->httpauth.method, url_con->httpauth.restricted),
                             efl_net_dialer_http_version_set(efl_added, url_con->http_version),
                             efl_net_dialer_http_allow_redirects_set(efl_added, EINA_TRUE),
                             efl_net_dialer_http_ssl_verify_set(efl_added, url_con->ssl_verify_peer, url_con->ssl_verify_peer),
                             efl_net_dialer_http_ssl_certificate_authority_set(efl_added, url_con->ca_path),
                             efl_event_callback_array_add(efl_added, ecore_con_url_dialer_cbs(), url_con));
   EINA_SAFETY_ON_NULL_GOTO(url_con->dialer, error);

   curl_easy = efl_net_dialer_http_curl_get(url_con->dialer);
   EINA_SAFETY_ON_NULL_GOTO(curl_easy, error_curl_easy);

   if (url_con->verbose)
     {
        _c->curl_easy_setopt(curl_easy, CURLOPT_DEBUGFUNCTION, NULL);
        _c->curl_easy_setopt(curl_easy, CURLOPT_DEBUGDATA, NULL);
        _c->curl_easy_setopt(curl_easy, CURLOPT_VERBOSE, 1L);
        DBG("HTTP Dialer %p is set to legacy debug function (CURL's default, no eina_log)", url_con->dialer);
     }

   _c->curl_easy_setopt(curl_easy, CURLOPT_FTP_USE_EPSV, (long)url_con->ftp_use_epsv);

   /* previously always set encoding to gzip,deflate */
   efl_net_dialer_http_request_header_add(url_con->dialer, "Accept-Encoding", "gzip,deflate");

   if (url_con->time.condition != ECORE_CON_URL_TIME_NONE)
     {
        char *ts = efl_net_dialer_http_date_serialize(url_con->time.stamp);
        if (ts)
          {
             efl_net_dialer_http_request_header_add(url_con->dialer,
                                                    url_con->time.condition == ECORE_CON_URL_TIME_IFMODSINCE ? "If-Modified-Since" : "If-Unmodified-Since",
                                                    ts);
             free(ts);
          }
     }

   EINA_LIST_FOREACH(url_con->request_headers, n, header)
     efl_net_dialer_http_request_header_add(url_con->dialer, header->key, header->value);

   _c->curl_easy_setopt(curl_easy, CURLOPT_COOKIESESSION, (long)url_con->cookies.ignore_old_session);

   EINA_LIST_FOREACH(url_con->cookies.files, n, s)
     _c->curl_easy_setopt(curl_easy, CURLOPT_COOKIEFILE, s);

   if (url_con->cookies.jar)
     _c->curl_easy_setopt(curl_easy, CURLOPT_COOKIEJAR, url_con->cookies.jar);

   EINA_LIST_FREE(url_con->cookies.cmds, s) /* free: only to execute once! */
     _c->curl_easy_setopt(curl_easy, CURLOPT_COOKIELIST, s);

   // Users should hook to their window animator if they want to show in real-time,
   // or have a slower timer... but the old API requested a period event, so add it
   // based on global animator timeout
   url_con->progress.animator = ecore_animator_add(_ecore_con_url_progress_animator_cb, url_con);

   DBG("prepared %p %s (%s), proxy=%s, primary_mode=%d",
       url_con->dialer,
       method,
       efl_net_dialer_http_method_get(url_con->dialer),
       efl_net_dialer_proxy_get(url_con->dialer),
       efl_net_dialer_http_primary_mode_get(url_con->dialer));

   free(proxy_url);
   return EINA_TRUE;

 error_curl_easy:
   _ecore_con_url_dialer_close(url_con);
 error:
   free(proxy_url);
   return EINA_FALSE;
}

EAPI Ecore_Con_Url *
ecore_con_url_new(const char *url)
{
   Ecore_Con_Url *url_con;

   EINA_SAFETY_ON_NULL_RETURN_VAL(url, NULL);

   url_con = calloc(1, sizeof(Ecore_Con_Url));
   EINA_SAFETY_ON_NULL_RETURN_VAL(url_con, NULL);

   url_con->url = eina_stringshare_add(url);
   url_con->http_version = EFL_NET_HTTP_VERSION_V1_1;
   url_con->write_fd = -1;

   EINA_MAGIC_SET(url_con, ECORE_MAGIC_CON_URL);
   _url_con_url_list = eina_list_append(_url_con_url_list, url_con);

   return url_con;
}

EAPI Ecore_Con_Url *
ecore_con_url_custom_new(const char *url,
                         const char *custom_request)
{
   Ecore_Con_Url *url_con;

   EINA_SAFETY_ON_NULL_RETURN_VAL(url, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(custom_request, NULL);

   url_con = ecore_con_url_new(url);
   EINA_SAFETY_ON_NULL_RETURN_VAL(url_con, NULL);

   url_con->custom_request = eina_stringshare_add(custom_request);

   return url_con;
}

EAPI void
ecore_con_url_free(Ecore_Con_Url *url_con)
{
   ECORE_CON_URL_CHECK_RETURN(url_con);
   /* remove from list as early as possible, we don't want to call
    * ecore_con_url_free() again on pending handles in ecore_con_url_shutdown()
    */
   _url_con_url_list = eina_list_remove(_url_con_url_list, url_con);
   _ecore_con_url_free_internal(url_con);
}

EAPI void *
ecore_con_url_data_get(Ecore_Con_Url *url_con)
{
   ECORE_CON_URL_CHECK_RETURN(url_con, NULL);
   return url_con->data;
}

EAPI void
ecore_con_url_data_set(Ecore_Con_Url *url_con,
                       void *data)
{
   ECORE_CON_URL_CHECK_RETURN(url_con);
   url_con->data = data;
}

EAPI Eina_Bool
ecore_con_url_url_set(Ecore_Con_Url *url_con,
                      const char *url)
{
   ECORE_CON_URL_CHECK_RETURN(url_con, EINA_FALSE);
   eina_stringshare_replace(&url_con->url, url ? url : "");
   return EINA_TRUE;
}

EAPI const char *
ecore_con_url_url_get(Ecore_Con_Url *url_con)
{
   ECORE_CON_URL_CHECK_RETURN(url_con, NULL);
   return url_con->url;
}

/* LEGACY: HTTP requests */
EAPI Eina_Bool
ecore_con_url_get(Ecore_Con_Url *url_con)
{
   Eina_Error err;

   ECORE_CON_URL_CHECK_RETURN(url_con, EINA_FALSE);

   if (!_ecore_con_url_request_prepare(url_con, "GET"))
     return EINA_FALSE;

   err = efl_net_dialer_dial(url_con->dialer, url_con->url);
   if (err)
     {
        WRN("failed to HTTP GET '%s': %s", url_con->url, eina_error_msg_get(err));
        _ecore_con_url_dialer_close(url_con);
        return EINA_FALSE;
     }

   return EINA_TRUE;
}

EAPI Eina_Bool
ecore_con_url_head(Ecore_Con_Url *url_con)
{
   Eina_Error err;

   ECORE_CON_URL_CHECK_RETURN(url_con, EINA_FALSE);

   if (!_ecore_con_url_request_prepare(url_con, "HEAD"))
     return EINA_FALSE;

   err = efl_net_dialer_dial(url_con->dialer, url_con->url);
   if (err)
     {
        WRN("failed to HTTP HEAD '%s': %s", url_con->url, eina_error_msg_get(err));
        _ecore_con_url_dialer_close(url_con);
        return EINA_FALSE;
     }

   return EINA_TRUE;
}

EAPI Eina_Bool
ecore_con_url_post(Ecore_Con_Url *url_con,
                   const void *data,
                   long length,
                   const char *content_type)
{
   Eo *buffer, *copier;
   Eina_Slice slice = { .mem = data, .len = length };
   Eina_Error err;

   ECORE_CON_URL_CHECK_RETURN(url_con, EINA_FALSE);

   if (!_ecore_con_url_request_prepare(url_con, "POST"))
     return EINA_FALSE;

   if (content_type)
     efl_net_dialer_http_request_header_add(url_con->dialer, "Content-Type", content_type);

   buffer = efl_add(EFL_IO_BUFFER_CLASS, efl_loop_get(url_con->dialer),
                    efl_name_set(efl_added, "post-buffer"),
                    efl_io_closer_close_on_invalidate_set(efl_added, EINA_TRUE),
                    efl_io_closer_close_on_exec_set(efl_added, EINA_TRUE));
   EINA_SAFETY_ON_NULL_GOTO(buffer, error_buffer);

   err = efl_io_writer_write(buffer, &slice, NULL);
   if (err)
     {
        WRN("could not populate buffer %p with %ld bytes: %s",
            buffer, length, eina_error_msg_get(err));
        goto error_copier;
     }

   copier = efl_add(EFL_IO_COPIER_CLASS, efl_loop_get(url_con->dialer),
                    efl_name_set(efl_added, "send-copier"),
                    efl_io_copier_source_set(efl_added, buffer),
                    efl_io_copier_destination_set(efl_added, url_con->dialer),
                    efl_io_closer_close_on_invalidate_set(efl_added, EINA_FALSE),
                    efl_event_callback_array_add(efl_added, _ecore_con_url_copier_cbs(), url_con));
   EINA_SAFETY_ON_NULL_GOTO(copier, error_copier);

   err = efl_net_dialer_dial(url_con->dialer, url_con->url);
   if (err)
     {
        WRN("failed to post to '%s': %s", url_con->url, eina_error_msg_get(err));
        goto error_dialer;
     }

   url_con->input = buffer;
   url_con->send_copier = copier;
   DBG("posting to '%s' using an Efl.Io.Copier=%p", url_con->url, copier);

   return EINA_TRUE;

 error_dialer:
   efl_del(copier);
 error_copier:
   efl_del(buffer);
 error_buffer:
   _ecore_con_url_dialer_close(url_con);
   return EINA_FALSE;
}

/* LEGACY: headers */
EAPI void
ecore_con_url_additional_header_add(Ecore_Con_Url *url_con,
                                    const char *key,
                                    const char *value)
{
   Efl_Net_Http_Header *header;
   char *s;

   ECORE_CON_URL_CHECK_RETURN(url_con);
   EINA_SAFETY_ON_NULL_RETURN(key);
   EINA_SAFETY_ON_NULL_RETURN(value);

   header = malloc(sizeof(Efl_Net_Http_Header) +
                   strlen(key) + 1 +
                   strlen(value) + 1);
   EINA_SAFETY_ON_NULL_RETURN(header);

   header->key = s = (char *)header + sizeof(Efl_Net_Http_Header);
   memcpy(s, key, strlen(key) + 1);

   header->value = s = s + strlen(key) + 1;
   memcpy(s, value, strlen(value) + 1);

   url_con->request_headers = eina_list_append(url_con->request_headers,
                                               header);
}

EAPI void
ecore_con_url_additional_headers_clear(Ecore_Con_Url *url_con)
{
   ECORE_CON_URL_CHECK_RETURN(url_con);
   _ecore_con_url_request_headers_free(url_con);
}

EAPI void
ecore_con_url_time(Ecore_Con_Url *url_con,
                   Ecore_Con_Url_Time time_condition,
                   double timestamp)
{
   ECORE_CON_URL_CHECK_RETURN(url_con);
   url_con->time.condition = time_condition;
   url_con->time.stamp = timestamp;
}

/* LEGACY: cookies */
EAPI void
ecore_con_url_cookies_init(Ecore_Con_Url *url_con)
{
   CURL *curl_easy;

   ECORE_CON_URL_CHECK_RETURN(url_con);

   /* meaningful before and after dial, persist and apply */
   url_con->cookies.files = eina_list_append(url_con->cookies.files, eina_stringshare_add(""));

   if (!url_con->dialer) return;

   curl_easy = efl_net_dialer_http_curl_get(url_con->dialer);
   EINA_SAFETY_ON_NULL_RETURN(curl_easy);

   _c->curl_easy_setopt(curl_easy, CURLOPT_COOKIEFILE, "");
}

EAPI void
ecore_con_url_cookies_file_add(Ecore_Con_Url *url_con,
                               const char * const file_name)
{
   CURL *curl_easy;

   ECORE_CON_URL_CHECK_RETURN(url_con);
   EINA_SAFETY_ON_NULL_RETURN(file_name);

   /* meaningful before and after dial, persist and apply */
   url_con->cookies.files = eina_list_append(url_con->cookies.files, eina_stringshare_add(file_name));

   if (!url_con->dialer) return;

   curl_easy = efl_net_dialer_http_curl_get(url_con->dialer);
   EINA_SAFETY_ON_NULL_RETURN(curl_easy);

   _c->curl_easy_setopt(curl_easy, CURLOPT_COOKIEFILE, file_name);
}

EAPI void
ecore_con_url_cookies_clear(Ecore_Con_Url *url_con)
{
   static const char cookielist_cmd_all[] = "ALL";
   CURL *curl_easy;

   ECORE_CON_URL_CHECK_RETURN(url_con);

   /* only meaningful once, if not dialed, queue, otherwise execute */
   if (!url_con->dialer)
     {
        url_con->cookies.cmds = eina_list_append(url_con->cookies.cmds, cookielist_cmd_all);
        return;
     }

   curl_easy = efl_net_dialer_http_curl_get(url_con->dialer);
   EINA_SAFETY_ON_NULL_RETURN(curl_easy);

   _c->curl_easy_setopt(curl_easy, CURLOPT_COOKIELIST, cookielist_cmd_all);
}

EAPI void
ecore_con_url_cookies_session_clear(Ecore_Con_Url *url_con)
{
   static const char cookielist_cmd_sess[] = "SESS";
   CURL *curl_easy;

   ECORE_CON_URL_CHECK_RETURN(url_con);

   /* only meaningful once, if not dialed, queue, otherwise execute */
   if (!url_con->dialer)
     {
        url_con->cookies.cmds = eina_list_append(url_con->cookies.cmds, cookielist_cmd_sess);
        return;
     }

   curl_easy = efl_net_dialer_http_curl_get(url_con->dialer);
   EINA_SAFETY_ON_NULL_RETURN(curl_easy);

   _c->curl_easy_setopt(curl_easy, CURLOPT_COOKIELIST, cookielist_cmd_sess);
}

EAPI void
ecore_con_url_cookies_ignore_old_session_set(Ecore_Con_Url *url_con,
                                             Eina_Bool ignore)
{
   ECORE_CON_URL_CHECK_RETURN(url_con);
   url_con->cookies.ignore_old_session = ignore;
}

EAPI Eina_Bool
ecore_con_url_cookies_jar_file_set(Ecore_Con_Url *url_con,
                                   const char * const cookiejar_file)
{
   CURL *curl_easy;

   ECORE_CON_URL_CHECK_RETURN(url_con, EINA_FALSE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(cookiejar_file, EINA_FALSE);

   /* meaningful before and after dial, persist and apply */
   eina_stringshare_replace(&url_con->cookies.jar, cookiejar_file);

   if (!url_con->dialer) return EINA_TRUE;

   curl_easy = efl_net_dialer_http_curl_get(url_con->dialer);
   EINA_SAFETY_ON_NULL_RETURN_VAL(curl_easy, EINA_FALSE);

   _c->curl_easy_setopt(curl_easy, CURLOPT_COOKIEJAR, url_con->cookies.jar);
   return EINA_TRUE;
}

EAPI void
ecore_con_url_cookies_jar_write(Ecore_Con_Url *url_con)
{
   CURL *curl_easy;

   ECORE_CON_URL_CHECK_RETURN(url_con);

   /* only meaningful after dialed */
   if (!url_con->dialer) return;

   curl_easy = efl_net_dialer_http_curl_get(url_con->dialer);
   EINA_SAFETY_ON_NULL_RETURN(curl_easy);

   _c->curl_easy_setopt(curl_easy, CURLOPT_COOKIELIST, "FLUSH");
}

/* LEGACY: file upload/download */
EAPI void
ecore_con_url_fd_set(Ecore_Con_Url *url_con, int fd)
{
   ECORE_CON_URL_CHECK_RETURN(url_con);

   if (url_con->write_fd == fd) return;

   url_con->write_fd = fd;
   if (!url_con->dialer) return;
}

EAPI Eina_Bool
ecore_con_url_ftp_upload(Ecore_Con_Url *url_con,
                         const char *filename,
                         const char *user,
                         const char *pass,
                         const char *upload_dir)
{
   char tmp[4096];
   char *bname;
   Eo *file, *copier;
   Eina_Error err;

   ECORE_CON_URL_CHECK_RETURN(url_con, EINA_FALSE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(filename, EINA_FALSE);
   EINA_SAFETY_ON_TRUE_RETURN_VAL(filename[0] == '\0', EINA_FALSE);

   bname = strdup(filename);
   if (upload_dir)
     snprintf(tmp, sizeof(tmp), "%s/%s/%s", url_con->url, upload_dir, basename(bname));
   else
     snprintf(tmp, sizeof(tmp), "%s/%s", url_con->url, basename(bname));
   free(bname);

   if (!_ecore_con_url_request_prepare(url_con, "PUT"))
     return EINA_FALSE;

   efl_net_dialer_http_authentication_set(url_con->dialer, user, pass, EFL_NET_HTTP_AUTHENTICATION_METHOD_ANY, EINA_FALSE);

   file = efl_add(EFL_IO_FILE_CLASS, efl_loop_get(url_con->dialer),
                  efl_name_set(efl_added, "upload-file"),
                  efl_file_set(efl_added, filename),
                  efl_io_file_flags_set(efl_added, O_RDONLY),
                  efl_io_closer_close_on_invalidate_set(efl_added, EINA_TRUE),
                  efl_io_closer_close_on_exec_set(efl_added, EINA_TRUE));
   EINA_SAFETY_ON_NULL_GOTO(file, error_file);

   copier = efl_add(EFL_IO_COPIER_CLASS, efl_loop_get(url_con->dialer),
                    efl_name_set(efl_added, "send-copier"),
                    efl_io_copier_source_set(efl_added, file),
                    efl_io_copier_destination_set(efl_added, url_con->dialer),
                    efl_io_closer_close_on_invalidate_set(efl_added, EINA_FALSE),
                    efl_event_callback_array_add(efl_added, _ecore_con_url_copier_cbs(), url_con));
   EINA_SAFETY_ON_NULL_GOTO(copier, error_copier);

   err = efl_net_dialer_dial(url_con->dialer, tmp);
   if (err)
     {
        WRN("failed to upload file '%s' to '%s': %s", filename, tmp, eina_error_msg_get(err));
        goto error_dialer;
     }

   url_con->input = file;
   url_con->send_copier = copier;
   DBG("uploading file '%s' to '%s' using an Efl.Io.Copier=%p", filename, tmp, copier);

   return EINA_TRUE;

 error_dialer:
   efl_del(copier);
 error_copier:
   efl_del(file);
 error_file:
   _ecore_con_url_dialer_close(url_con);
   return EINA_FALSE;
}

EAPI void
ecore_con_url_ftp_use_epsv_set(Ecore_Con_Url *url_con,
                               Eina_Bool use_epsv)
{
   ECORE_CON_URL_CHECK_RETURN(url_con);
   url_con->ftp_use_epsv = use_epsv;
}

EAPI void
ecore_con_url_limit_upload_speed(Ecore_Con_Url *url_con, off_t max_speed)
{
   CURL *curl_easy;

   ECORE_CON_URL_CHECK_RETURN(url_con);
   EINA_SAFETY_ON_NULL_RETURN(_c);

   curl_easy = efl_net_dialer_http_curl_get(url_con->dialer);
   EINA_SAFETY_ON_NULL_RETURN(curl_easy);

   _c->curl_easy_setopt(curl_easy, CURLOPT_MAX_SEND_SPEED_LARGE, max_speed);
}

EAPI void
ecore_con_url_limit_download_speed(Ecore_Con_Url *url_con, off_t max_speed)
{
   CURL *curl_easy;

   ECORE_CON_URL_CHECK_RETURN(url_con);
   EINA_SAFETY_ON_NULL_RETURN(_c);

   curl_easy = efl_net_dialer_http_curl_get(url_con->dialer);
   EINA_SAFETY_ON_NULL_RETURN(curl_easy);

   _c->curl_easy_setopt(curl_easy, CURLOPT_MAX_RECV_SPEED_LARGE, max_speed);
}

/* LEGACY: proxy */
EAPI Eina_Bool
ecore_con_url_proxy_password_set(Ecore_Con_Url *url_con, const char *password)
{
   ECORE_CON_URL_CHECK_RETURN(url_con, EINA_FALSE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(password, EINA_FALSE);
   eina_stringshare_replace(&url_con->proxy.password, password);
   return EINA_TRUE;
}

EAPI Eina_Bool
ecore_con_url_proxy_username_set(Ecore_Con_Url *url_con, const char *username)
{
   ECORE_CON_URL_CHECK_RETURN(url_con, EINA_FALSE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(username, EINA_FALSE);
   eina_stringshare_replace(&url_con->proxy.username, username);
   return EINA_TRUE;
}

EAPI Eina_Bool
ecore_con_url_proxy_set(Ecore_Con_Url *url_con, const char *proxy_url)
{
   ECORE_CON_URL_CHECK_RETURN(url_con, EINA_FALSE);
   eina_stringshare_replace(&url_con->proxy.url, proxy_url);
   return EINA_TRUE;
}

/* LEGACY: response */
EAPI int
ecore_con_url_received_bytes_get(Ecore_Con_Url *url_con)
{
   ECORE_CON_URL_CHECK_RETURN(url_con, EINA_FALSE);
   return url_con->received_bytes;
}

EAPI int
ecore_con_url_status_code_get(Ecore_Con_Url *url_con)
{
   ECORE_CON_URL_CHECK_RETURN(url_con, 0);
   if (!url_con->dialer) return url_con->status;
   return efl_net_dialer_http_response_status_get(url_con->dialer);
}

EAPI const Eina_List *
ecore_con_url_response_headers_get(Ecore_Con_Url *url_con)
{
   ECORE_CON_URL_CHECK_RETURN(url_con, NULL);
   return url_con->response_headers;
}

/* LEGACY: SSL */
EAPI int
ecore_con_url_ssl_ca_set(Ecore_Con_Url *url_con,
                         const char *ca_path)
{
   ECORE_CON_URL_CHECK_RETURN(url_con, -1);
   eina_stringshare_replace(&url_con->ca_path, ca_path);
   url_con->ssl_verify_peer = !!ca_path;
   return 0;
}

EAPI void
ecore_con_url_ssl_verify_peer_set(Ecore_Con_Url *url_con,
                                  Eina_Bool verify)
{
   ECORE_CON_URL_CHECK_RETURN(url_con);
   url_con->ssl_verify_peer = !!verify;
}

/* LEGACY: misc */
EAPI Eina_Bool
ecore_con_url_httpauth_set(Ecore_Con_Url *url_con,
                           const char *username,
                           const char *password,
                           Eina_Bool safe)
{
   ECORE_CON_URL_CHECK_RETURN(url_con, EINA_FALSE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(username, EINA_FALSE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(password, EINA_FALSE);

   eina_stringshare_replace(&url_con->httpauth.username, username);
   eina_stringshare_replace(&url_con->httpauth.password, password);
   url_con->httpauth.method = safe ? EFL_NET_HTTP_AUTHENTICATION_METHOD_ANY_SAFE : EFL_NET_HTTP_AUTHENTICATION_METHOD_ANY;
   url_con->httpauth.restricted = safe;
   return EINA_TRUE;
}

EAPI Eina_Bool
ecore_con_url_http_version_set(Ecore_Con_Url *url_con, Ecore_Con_Url_Http_Version version)
{
   ECORE_CON_URL_CHECK_RETURN(url_con, EINA_FALSE);
   switch (version)
     {
      case ECORE_CON_URL_HTTP_VERSION_1_0:
         url_con->http_version = EFL_NET_HTTP_VERSION_V1_0;
         break;
      case ECORE_CON_URL_HTTP_VERSION_1_1:
         url_con->http_version = EFL_NET_HTTP_VERSION_V1_1;
         break;
      default:
         ERR("unknown HTTP version enum value %d", version);
         return EINA_FALSE;
     }
   return EINA_TRUE;
}

static Eina_Bool
_ecore_con_url_timeout_cb(void *data)
{
   Ecore_Con_Url *url_con = data;
   int status;

   url_con->timer = NULL;

   WRN("HTTP timeout url='%s'", efl_net_dialer_address_dial_get(url_con->dialer));

   status = efl_net_dialer_http_response_status_get(url_con->dialer);
   if ((status < 500) || (status > 599))
     {
        DBG("HTTP error %d reset to 1", status);
        status = 1; /* not a real HTTP error */
     }

   _ecore_con_event_url_complete_add(url_con, status);

   return EINA_FALSE;
}

EAPI void
ecore_con_url_timeout_set(Ecore_Con_Url *url_con, double timeout)
{
   ECORE_CON_URL_CHECK_RETURN(url_con);

   if (url_con->timer)
     {
        ecore_timer_del(url_con->timer);
        url_con->timer = NULL;
     }

   if (timeout <= 0.0) return;

   // NOTE: it is weird to start the timeout right away here, but it
   // was done like that and we're keeping it for compatibility
   url_con->timer = ecore_timer_add(timeout, _ecore_con_url_timeout_cb, url_con);
}

EAPI void
ecore_con_url_verbose_set(Ecore_Con_Url *url_con,
                          Eina_Bool verbose)
{
   CURL *curl_easy;

   ECORE_CON_URL_CHECK_RETURN(url_con);

   /* meaningful before and after dial, persist and apply */
   url_con->verbose = !!verbose;

   if (!url_con->dialer) return;

   curl_easy = efl_net_dialer_http_curl_get(url_con->dialer);
   EINA_SAFETY_ON_NULL_RETURN(curl_easy);

   if (url_con->verbose)
     {
        _c->curl_easy_setopt(curl_easy, CURLOPT_DEBUGFUNCTION, NULL);
        _c->curl_easy_setopt(curl_easy, CURLOPT_DEBUGDATA, NULL);
        DBG("HTTP Dialer %p is set to legacy debug function (CURL's default, no eina_log)", url_con->dialer);
     }
   _c->curl_easy_setopt(curl_easy, CURLOPT_VERBOSE, (long)url_con->verbose);
}
