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

// all the types, defines, enums etc. from curl that we actuall USE.
// we have to add to this if we use more things from curl not already
// defined here. see culr headers to get them from
typedef enum
{
   CURLM_CALL_MULTI_PERFORM = -1,
   CURLM_OK = 0
} CURLMcode;
typedef enum
{
   CURLE_OK = 0,
   CURLE_OPERATION_TIMEDOUT = 28
} CURLcode;
#define CURLOPTTYPE_LONG          0
#define CURLOPTTYPE_OBJECTPOINT   10000
#define CURLOPTTYPE_FUNCTIONPOINT 20000
#define CURLOPTTYPE_OFF_T         30000
#define CINIT(na, t, nu) CURLOPT_ ## na = CURLOPTTYPE_ ## t + nu
typedef enum
{
   CINIT(FILE, OBJECTPOINT, 1),
   CINIT(URL, OBJECTPOINT, 2),
   CINIT(PROXY, OBJECTPOINT, 4),
   CINIT(USERPWD, OBJECTPOINT, 5),
   CINIT(INFILE, OBJECTPOINT, 9),
   CINIT(WRITEFUNCTION, FUNCTIONPOINT, 11),
   CINIT(READFUNCTION, FUNCTIONPOINT, 12),
   CINIT(POSTFIELDS, OBJECTPOINT, 15),
   CINIT(HTTPHEADER, OBJECTPOINT, 23),
   CINIT(WRITEHEADER, OBJECTPOINT, 29),
   CINIT(COOKIEFILE, OBJECTPOINT, 31),
   CINIT(TIMECONDITION, LONG, 33),
   CINIT(TIMEVALUE, LONG, 34),
   CINIT(CUSTOMREQUEST, OBJECTPOINT, 36),
   CINIT(VERBOSE, LONG, 41),
   CINIT(NOPROGRESS, LONG, 43),
   CINIT(UPLOAD, LONG, 46),
   CINIT(POST, LONG, 47),
   CINIT(FOLLOWLOCATION, LONG, 52),
   CINIT(PROGRESSFUNCTION, FUNCTIONPOINT, 56),
   CINIT(PROGRESSDATA, OBJECTPOINT, 57),
   CINIT(POSTFIELDSIZE, LONG, 60),
   CINIT(SSL_VERIFYPEER, LONG, 64),
   CINIT(CAINFO, OBJECTPOINT, 65),
   CINIT(CONNECTTIMEOUT, LONG, 78),
   CINIT(HEADERFUNCTION, FUNCTIONPOINT, 79),
   CINIT(COOKIEJAR, OBJECTPOINT, 82),
   CINIT(HTTP_VERSION, LONG, 84),
   CINIT(FTP_USE_EPSV, LONG, 85),
   CINIT(COOKIESESSION, LONG, 96),
   CINIT(PROXYTYPE, LONG, 101),
   CINIT(ACCEPT_ENCODING, OBJECTPOINT, 102),
   CINIT(HTTPAUTH, LONG, 107),
   CINIT(INFILESIZE_LARGE, OFF_T, 115),
   CINIT(COOKIELIST, OBJECTPOINT, 135),
   CINIT(USERNAME, OBJECTPOINT, 173),
   CINIT(PASSWORD, OBJECTPOINT, 174)
} CURLoption;
#define CURLINFO_STRING   0x100000
#define CURLINFO_LONG     0x200000
#define CURLINFO_DOUBLE   0x300000
#define CURLINFO_SLIST    0x400000
#define CURLINFO_MASK     0x0fffff
#define CURLINFO_TYPEMASK 0xf00000
typedef enum
{
   CURLINFO_RESPONSE_CODE = CURLINFO_LONG + 2,
} CURLINFO;
typedef enum
{
   CURLVERSION_FOURTH = 3
} CURLversion;
typedef enum
{
   CURLMSG_DONE = 1
} CURLMSG;
#undef CINIT
#define CINIT(name, type, num) CURLMOPT_ ## name = CURLOPTTYPE_ ## type + num
typedef enum
{
   CINIT(PIPELINING, LONG, 3)
} CURLMoption;
typedef enum
{
   CURL_TIMECOND_NONE = 0,
   CURL_TIMECOND_IFMODSINCE = 1,
   CURL_TIMECOND_IFUNMODSINCE = 2
} curl_TimeCond;
enum
{
   CURL_HTTP_VERSION_1_0 = 1,
   CURL_HTTP_VERSION_1_1 = 2,
};
typedef enum
{
   CURLPROXY_HTTP = 0,
   CURLPROXY_SOCKS4 = 4,
   CURLPROXY_SOCKS5 = 5,
   CURLPROXY_SOCKS4A = 6,
   CURLPROXY_SOCKS5_HOSTNAME = 7
} curl_proxytype;

#define CURL_GLOBAL_SSL     (1 << 0)
#define CURL_GLOBAL_WIN32   (1 << 1)
#define CURL_GLOBAL_ALL     (CURL_GLOBAL_SSL | CURL_GLOBAL_WIN32)
#define CURLOPT_ENCODING    CURLOPT_ACCEPT_ENCODING
#define CURLOPT_WRITEDATA   CURLOPT_FILE
#define CURLOPT_READDATA    CURLOPT_INFILE
#define CURLOPT_HEADERDATA  CURLOPT_WRITEHEADER
#define CURLVERSION_NOW     CURLVERSION_FOURTH
#define CURLAUTH_BASIC      (((unsigned long)1) << 0)
#define CURLAUTH_DIGEST_IE  (((unsigned long)1) << 4)
#define CURLAUTH_ANY        (~CURLAUTH_DIGEST_IE)
#define CURLAUTH_ANYSAFE    (~(CURLAUTH_BASIC | CURLAUTH_DIGEST_IE))
#define CURL_READFUNC_ABORT 0x10000000

typedef void CURLM;
typedef void CURL;
struct curl_slist
{
   char              *data;
   struct curl_slist *next;
};
typedef struct
{
   CURLversion        age;
   const char        *version;
   unsigned int       version_num;
   const char        *host;
   int                features;
   const char        *ssl_version;
   long               ssl_version_num;
   const char        *libz_version;
   const char *const *protocols;
   const char        *ares;
   int                ares_num;
   const char        *libidn;
   int                iconv_ver_num;
   const char        *libssh_version;
} curl_version_info_data;
typedef struct
{
   CURLMSG msg;
   CURL   *easy_handle;
   union
   {
      void    *whatever;
      CURLcode result;
   } data;
} CURLMsg;

typedef struct _Ecore_Con_Curl Ecore_Con_Curl;

struct _Ecore_Con_Curl
{
   Eina_Module            *mod;

   CURLM                  *_curlm;

   CURLcode                (*curl_global_init)(long flags);
   void                    (*curl_global_cleanup)(void);
   CURLM                  *(*curl_multi_init)(void);
   CURLMcode               (*curl_multi_timeout)(CURLM *multi_handle,
                                                 long *milliseconds);
   CURLMcode               (*curl_multi_cleanup)(CURLM *multi_handle);
   CURLMcode               (*curl_multi_remove_handle)(CURLM *multi_handle,
                                                       CURL *curl_handle);
   const char             *(*curl_multi_strerror)(CURLMcode);
   CURLMsg                *(*curl_multi_info_read)(CURLM * multi_handle,
                                                   int *msgs_in_queue);
   CURLMcode               (*curl_multi_fdset)(CURLM *multi_handle,
                                               fd_set *read_fd_set,
                                               fd_set *write_fd_set,
                                               fd_set *exc_fd_set,
                                               int *max_fd);
   CURLMcode               (*curl_multi_perform)(CURLM *multi_handle,
                                                 int *running_handles);
   CURLMcode               (*curl_multi_add_handle)(CURLM *multi_handle,
                                                    CURL *curl_handle);
   CURLMcode               (*curl_multi_setopt)(CURLM *multi_handle,
                                                CURLMoption option, ...);
   CURL                   *(*curl_easy_init)(void);
   CURLcode                (*curl_easy_setopt)(CURL *curl, CURLoption option, ...);
   const char             *(*curl_easy_strerror)(CURLcode);
   void                    (*curl_easy_cleanup)(CURL *curl);
   CURLcode                (*curl_easy_getinfo)(CURL *curl, CURLINFO info, ...);
   void                    (*curl_slist_free_all)(struct curl_slist *);
   struct curl_slist      *(*curl_slist_append)(struct curl_slist *list,
                                                const char *string);
   curl_version_info_data *(*curl_version_info)(CURLversion);
};

#define CURL_MIN_TIMEOUT 100

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

static Ecore_Con_Curl *_c = NULL;
static Eina_Bool _c_fail = EINA_FALSE;

static Eina_Bool
_c_init(void)
{
   long ms = 0;

   if (_c) return EINA_TRUE;
   if (_c_fail) return EINA_FALSE;
   _c = calloc(1, sizeof(Ecore_Con_Curl));
   if (!_c) goto error;

#define LOAD(x)                               \
  if (!_c->mod) {                             \
       if ((_c->mod = eina_module_new(x))) {  \
            if (!eina_module_load(_c->mod)) { \
                 eina_module_free(_c->mod);   \
                 _c->mod = NULL;              \
              }                               \
         }                                    \
    }
#if defined(_WIN32) || defined(__CYGWIN__)
   LOAD("libcurl.dll"); // try 1
   LOAD("curllib.dll"); // if fail try 2
#elif defined(__APPLE__) && defined(__MACH__)
   LOAD("libcurl.4.dylib"); // try 1
   LOAD("libcurl.so.4"); // if fail try 2
#else
   LOAD("libcurl.so.4"); // try only
#endif
   if (!_c->mod) goto error;

#define SYM(x) if (!(_c->x = eina_module_symbol_get(_c->mod, #x))) \
    goto error
   SYM(curl_global_init);
   SYM(curl_global_cleanup);
   SYM(curl_multi_init);
   SYM(curl_multi_timeout);
   SYM(curl_multi_cleanup);
   SYM(curl_multi_remove_handle);
   SYM(curl_multi_strerror);
   SYM(curl_multi_info_read);
   SYM(curl_multi_fdset);
   SYM(curl_multi_perform);
   SYM(curl_multi_add_handle);
   SYM(curl_multi_setopt);
   SYM(curl_easy_init);
   SYM(curl_easy_setopt);
   SYM(curl_easy_strerror);
   SYM(curl_easy_cleanup);
   SYM(curl_easy_getinfo);
   SYM(curl_slist_free_all);
   SYM(curl_slist_append);
   SYM(curl_version_info);

   // curl_global_init() is not thread safe!
   if (_c->curl_global_init(CURL_GLOBAL_ALL)) goto error;
   _c->_curlm = _c->curl_multi_init();
   if (!_c->_curlm)
     {
        _c->curl_global_cleanup();
        goto error;
     }
   _c->curl_multi_timeout(_c->_curlm, &ms);
   if ((ms >= CURL_MIN_TIMEOUT) || (ms <= 0)) ms = CURL_MIN_TIMEOUT;
   _curl_timer = ecore_timer_add((double)ms / 1000.0,
                                 _ecore_con_url_timer, NULL);
   ecore_timer_freeze(_curl_timer);
   return EINA_TRUE;
error:
   if (_c)
     {
        if (_c->mod) eina_module_free(_c->mod);
        free(_c);
        _c = NULL;
     }
   _c_fail = EINA_TRUE;
   return EINA_FALSE;
}

static void
_c_shutdown(void)
{
   if (!_c) return;
   if (_c->_curlm)
     {
        _c->curl_multi_cleanup(_c->_curlm);
     }
   _c->curl_global_cleanup();
   if (_c->mod) eina_module_free(_c->mod);
   free(_c);
   _c = NULL;
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

extern Ecore_Con_Socks *_ecore_con_proxy_global;

EAPI Ecore_Con_Url *
ecore_con_url_new(const char *url)
{
   Ecore_Con_Url *url_con;
   CURLcode ret;

   if (!_init_count) return NULL;
   if (!_c_init()) return NULL;

   url_con = calloc(1, sizeof(Ecore_Con_Url));
   if (!url_con) return NULL;

   url_con->write_fd = -1;

   url_con->curl_easy = _c->curl_easy_init();
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

        ecore_con_url_proxy_set(url_con, proxy);
        ecore_con_url_proxy_username_set(url_con,
                                         _ecore_con_proxy_global->username);
     }

   ret = _c->curl_easy_setopt(url_con->curl_easy, CURLOPT_ENCODING,
                              "gzip,deflate");
   if (ret != CURLE_OK)
     {
        ERR("Could not set CURLOPT_ENCODING to \"gzip,deflate\": %s",
            _c->curl_easy_strerror(ret));
        ecore_con_url_free(url_con);
        return NULL;
     }

   _c->curl_easy_setopt(url_con->curl_easy, CURLOPT_WRITEFUNCTION,
                        _ecore_con_url_data_cb);
   _c->curl_easy_setopt(url_con->curl_easy, CURLOPT_WRITEDATA, url_con);

   _c->curl_easy_setopt(url_con->curl_easy, CURLOPT_PROGRESSFUNCTION,
                        _ecore_con_url_progress_cb);
   _c->curl_easy_setopt(url_con->curl_easy, CURLOPT_PROGRESSDATA, url_con);
   _c->curl_easy_setopt(url_con->curl_easy, CURLOPT_NOPROGRESS, EINA_FALSE);

   _c->curl_easy_setopt(url_con->curl_easy, CURLOPT_HEADERFUNCTION,
                        _ecore_con_url_header_cb);
   _c->curl_easy_setopt(url_con->curl_easy, CURLOPT_HEADERDATA, url_con);
   /*
    * FIXME: Check that these timeouts are sensible defaults
    * FIXME: Provide a means to change these timeouts
    */
   _c->curl_easy_setopt(url_con->curl_easy, CURLOPT_CONNECTTIMEOUT, 30);
   _c->curl_easy_setopt(url_con->curl_easy, CURLOPT_FOLLOWLOCATION, 1);
   return url_con;
}

EAPI Ecore_Con_Url *
ecore_con_url_custom_new(const char *url,
                         const char *custom_request)
{
   Ecore_Con_Url *url_con;
   CURLcode ret;

   if (!_init_count) return NULL;
   if (!_c_init()) return NULL;
   if (!url) return NULL;
   if (!custom_request) return NULL;
   url_con = ecore_con_url_new(url);
   if (!url_con) return NULL;

   ret = _c->curl_easy_setopt(url_con->curl_easy, CURLOPT_CUSTOMREQUEST,
                              custom_request);
   if (ret != CURLE_OK)
     {
        ERR("Could not set a custom request string: %s",
            _c->curl_easy_strerror(ret));
        ecore_con_url_free(url_con);
        return NULL;
     }
   return url_con;
}

EAPI void
ecore_con_url_free(Ecore_Con_Url *url_con)
{
   char *s;

   if (!_c) return;
   if (!ECORE_MAGIC_CHECK(url_con, ECORE_MAGIC_CON_URL))
     {
        ECORE_MAGIC_FAIL(url_con, ECORE_MAGIC_CON_URL, __func__);
        return;
     }
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
             _ecore_con_url_multi_remove(url_con);
             _url_con_list = eina_list_remove(_url_con_list, url_con);
          }

        _c->curl_easy_cleanup(url_con->curl_easy);
     }
   if (url_con->timer) ecore_timer_del(url_con->timer);

   url_con->curl_easy = NULL;
   url_con->timer = NULL;
   url_con->dead = EINA_TRUE;
   if (url_con->event_count) return;
   ECORE_MAGIC_SET(url_con, ECORE_MAGIC_NONE);

   _c->curl_slist_free_all(url_con->headers);
   EINA_LIST_FREE(url_con->additional_headers, s)
     free(s);
   EINA_LIST_FREE(url_con->response_headers, s)
     free(s);
   eina_stringshare_del(url_con->url);
   if (url_con->post_data) free(url_con->post_data);
   free(url_con);
}

EAPI const char *
ecore_con_url_url_get(Ecore_Con_Url *url_con)
{
   if (!ECORE_MAGIC_CHECK(url_con, ECORE_MAGIC_CON_URL))
     {
        ECORE_MAGIC_FAIL(url_con, ECORE_MAGIC_CON_URL, __func__);
        return NULL;
     }
   return url_con->url;
}

EAPI int
ecore_con_url_status_code_get(Ecore_Con_Url *url_con)
{
   if (!ECORE_MAGIC_CHECK(url_con, ECORE_MAGIC_CON_URL))
     {
        ECORE_MAGIC_FAIL(url_con, ECORE_MAGIC_CON_URL, __func__);
        return 0;
     }
   if (url_con->status) return url_con->status;
   _ecore_con_url_status_get(url_con);
   return url_con->status;
}

EAPI Eina_Bool
ecore_con_url_url_set(Ecore_Con_Url *url_con, const char *url)
{
   if (!_c) return EINA_FALSE;
   if (!ECORE_MAGIC_CHECK(url_con, ECORE_MAGIC_CON_URL))
     {
        ECORE_MAGIC_FAIL(url_con, ECORE_MAGIC_CON_URL, __func__);
        return EINA_FALSE;
     }
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
ecore_con_url_data_set(Ecore_Con_Url *url_con, void *data)
{
   if (!ECORE_MAGIC_CHECK(url_con, ECORE_MAGIC_CON_URL))
     {
        ECORE_MAGIC_FAIL(url_con, ECORE_MAGIC_CON_URL, __func__);
        return;
     }
   url_con->data = data;
}

EAPI void
ecore_con_url_additional_header_add(Ecore_Con_Url *url_con, const char *key, const char *value)
{
   char *tmp;

   if (!ECORE_MAGIC_CHECK(url_con, ECORE_MAGIC_CON_URL))
     {
        ECORE_MAGIC_FAIL(url_con, ECORE_MAGIC_CON_URL, __func__);
        return;
     }

   if (url_con->dead) return;
   tmp = malloc(strlen(key) + strlen(value) + 3);
   if (!tmp) return;
   sprintf(tmp, "%s: %s", key, value);
   url_con->additional_headers = eina_list_append(url_con->additional_headers,
                                                  tmp);
}

EAPI void
ecore_con_url_additional_headers_clear(Ecore_Con_Url *url_con)
{
   char *s;

   if (!ECORE_MAGIC_CHECK(url_con, ECORE_MAGIC_CON_URL))
     {
        ECORE_MAGIC_FAIL(url_con, ECORE_MAGIC_CON_URL, __func__);
        return;
     }
   EINA_LIST_FREE(url_con->additional_headers, s)
     free(s);
}

EAPI void *
ecore_con_url_data_get(Ecore_Con_Url *url_con)
{
   if (!ECORE_MAGIC_CHECK(url_con, ECORE_MAGIC_CON_URL))
     {
        ECORE_MAGIC_FAIL(url_con, ECORE_MAGIC_CON_URL, __func__);
        return NULL;
     }
   return url_con->data;
}

EAPI void
ecore_con_url_time(Ecore_Con_Url *url_con, Ecore_Con_Url_Time condition, double timestamp)
{
   if (!ECORE_MAGIC_CHECK(url_con, ECORE_MAGIC_CON_URL))
     {
        ECORE_MAGIC_FAIL(url_con, ECORE_MAGIC_CON_URL, __func__);
        return;
     }
   if (url_con->dead) return;
   url_con->time_condition = condition;
   url_con->timestamp = timestamp;
}

EAPI void
ecore_con_url_fd_set(Ecore_Con_Url *url_con, int fd)
{
   if (!ECORE_MAGIC_CHECK(url_con, ECORE_MAGIC_CON_URL))
     {
        ECORE_MAGIC_FAIL(url_con, ECORE_MAGIC_CON_URL, __func__);
        return;
     }
   if (url_con->dead) return;
   url_con->write_fd = fd;
}

EAPI int
ecore_con_url_received_bytes_get(Ecore_Con_Url *url_con)
{
   if (!ECORE_MAGIC_CHECK(url_con, ECORE_MAGIC_CON_URL))
     {
        ECORE_MAGIC_FAIL(url_con, ECORE_MAGIC_CON_URL, __func__);
        return -1;
     }
   return url_con->received;
}

EAPI const Eina_List *
ecore_con_url_response_headers_get(Ecore_Con_Url *url_con)
{
   if (!ECORE_MAGIC_CHECK(url_con, ECORE_MAGIC_CON_URL))
     {
        ECORE_MAGIC_FAIL(url_con, ECORE_MAGIC_CON_URL, __func__);
        return NULL;
     }
   return url_con->response_headers;
}

EAPI Eina_Bool
ecore_con_url_httpauth_set(Ecore_Con_Url *url_con, const char *username, const char *password, Eina_Bool safe)
{
   CURLcode ret;
   curl_version_info_data *vers = NULL;

   if (!_c) return EINA_FALSE;
   if (!ECORE_MAGIC_CHECK(url_con, ECORE_MAGIC_CON_URL))
     {
        ECORE_MAGIC_FAIL(url_con, ECORE_MAGIC_CON_URL, __func__);
        return EINA_FALSE;
     }
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

#define MODE_AUTO 0
#define MODE_GET  1
#define MODE_POST 2

static Eina_Bool
_ecore_con_url_send(Ecore_Con_Url *url_con, int mode, const void *data, long length, const char *content_type)
{
   Eina_List *l;
   const char *s;
   char tmp[512];

   if (!_c) return EINA_FALSE;
   if (!ECORE_MAGIC_CHECK(url_con, ECORE_MAGIC_CON_URL))
     {
        ECORE_MAGIC_FAIL(url_con, ECORE_MAGIC_CON_URL, __func__);
        return EINA_FALSE;
     }

   if (!url_con->url) return EINA_FALSE;
   if (url_con->dead) return EINA_FALSE;

   /* Free response headers from previous send() calls */
   EINA_LIST_FREE(url_con->response_headers, s)
     free((char *)s);
   url_con->response_headers = NULL;
   url_con->status = 0;

   _c->curl_slist_free_all(url_con->headers);
   url_con->headers = NULL;
   if ((mode == MODE_POST) || (mode == MODE_AUTO))
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
        if (mode == MODE_POST)
          _c->curl_easy_setopt(url_con->curl_easy, CURLOPT_POST, 1);
     }

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
   return _ecore_con_url_perform(url_con);
}

EAPI Eina_Bool
ecore_con_url_get(Ecore_Con_Url *url_con)
{
   return _ecore_con_url_send(url_con, MODE_GET, NULL, 0, NULL);
}

EAPI Eina_Bool
ecore_con_url_post(Ecore_Con_Url *url_con, const void *data, long length, const char *content_type)
{
   return _ecore_con_url_send(url_con, MODE_POST, data, length, content_type);
}

EAPI Eina_Bool
ecore_con_url_ftp_upload(Ecore_Con_Url *url_con, const char *filename, const char *user, const char *pass, const char *upload_dir)
{
   char url[4096];
   char userpwd[4096];
   FILE *fd;
   struct stat file_info;
   CURLcode ret;

   if (!_c) return EINA_FALSE;
   if (!ECORE_MAGIC_CHECK(url_con, ECORE_MAGIC_CON_URL))
     {
        ECORE_MAGIC_FAIL(url_con, ECORE_MAGIC_CON_URL, __func__);
        return EINA_FALSE;
     }

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
     snprintf(url, sizeof(url), "ftp://%s/%s/%s", url_con->url,
              upload_dir, basename(tmp));
   else
     snprintf(url, sizeof(url), "ftp://%s/%s", url_con->url,
              basename(tmp));

   if (!ecore_con_url_url_set(url_con, url))
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
   return _ecore_con_url_perform(url_con);
}

EAPI void
ecore_con_url_cookies_init(Ecore_Con_Url *url_con)
{
   if (!_c) return;
   if (!ECORE_MAGIC_CHECK(url_con, ECORE_MAGIC_CON_URL))
     {
        ECORE_MAGIC_FAIL(url_con, ECORE_MAGIC_CON_URL, __func__);
        return;
     }
   if (url_con->dead) return;
   _c->curl_easy_setopt(url_con->curl_easy, CURLOPT_COOKIEFILE, "");
}

EAPI void
ecore_con_url_cookies_ignore_old_session_set(Ecore_Con_Url *url_con, Eina_Bool ignore)
{
   if (!_c) return;
   if (!ECORE_MAGIC_CHECK(url_con, ECORE_MAGIC_CON_URL))
     {
        ECORE_MAGIC_FAIL(url_con, ECORE_MAGIC_CON_URL, __func__);
        return;
     }
   if (url_con->dead) return;
   _c->curl_easy_setopt(url_con->curl_easy, CURLOPT_COOKIESESSION, ignore);
}

EAPI void
ecore_con_url_cookies_clear(Ecore_Con_Url *url_con)
{
   if (!_c) return;
   if (!ECORE_MAGIC_CHECK(url_con, ECORE_MAGIC_CON_URL))
     {
        ECORE_MAGIC_FAIL(url_con, ECORE_MAGIC_CON_URL, __func__);
        return;
     }
   if (url_con->dead) return;
   _c->curl_easy_setopt(url_con->curl_easy, CURLOPT_COOKIELIST, "ALL");
}

EAPI void
ecore_con_url_cookies_session_clear(Ecore_Con_Url *url_con)
{
   if (!_c) return;
   if (!ECORE_MAGIC_CHECK(url_con, ECORE_MAGIC_CON_URL))
     {
        ECORE_MAGIC_FAIL(url_con, ECORE_MAGIC_CON_URL, __func__);
        return;
     }
   if (url_con->dead) return;
   _c->curl_easy_setopt(url_con->curl_easy, CURLOPT_COOKIELIST, "SESS");
}

EAPI void
ecore_con_url_cookies_file_add(Ecore_Con_Url *url_con, const char *const file_name)
{
   if (!_c) return;
   if (!ECORE_MAGIC_CHECK(url_con, ECORE_MAGIC_CON_URL))
     {
        ECORE_MAGIC_FAIL(url_con, ECORE_MAGIC_CON_URL, __func__);
        return;
     }
   if (url_con->dead) return;
   _c->curl_easy_setopt(url_con->curl_easy, CURLOPT_COOKIEFILE, file_name);
}

EAPI Eina_Bool
ecore_con_url_cookies_jar_file_set(Ecore_Con_Url *url_con, const char *const cookiejar_file)
{
   CURLcode ret;

   if (!_c) return EINA_FALSE;
   if (!ECORE_MAGIC_CHECK(url_con, ECORE_MAGIC_CON_URL))
     {
        ECORE_MAGIC_FAIL(url_con, ECORE_MAGIC_CON_URL, __func__);
        return EINA_FALSE;
     }
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
ecore_con_url_cookies_jar_write(Ecore_Con_Url *url_con)
{
   if (!_c) return;
   if (!ECORE_MAGIC_CHECK(url_con, ECORE_MAGIC_CON_URL))
     {
        ECORE_MAGIC_FAIL(url_con, ECORE_MAGIC_CON_URL, __func__);
        return;
     }

   if (url_con->dead) return;
   _c->curl_easy_setopt(url_con->curl_easy, CURLOPT_COOKIELIST, "FLUSH");
}

EAPI void
ecore_con_url_verbose_set(Ecore_Con_Url *url_con, Eina_Bool verbose)
{
   if (!_c) return;
   if (!ECORE_MAGIC_CHECK(url_con, ECORE_MAGIC_CON_URL))
     {
        ECORE_MAGIC_FAIL(url_con, ECORE_MAGIC_CON_URL, __func__);
        return;
     }
   if (!url_con->url) return;
   if (url_con->dead) return;
   _c->curl_easy_setopt(url_con->curl_easy, CURLOPT_VERBOSE, (int)verbose);
}

EAPI void
ecore_con_url_ftp_use_epsv_set(Ecore_Con_Url *url_con, Eina_Bool use_epsv)
{
   if (!_c) return;
   if (!ECORE_MAGIC_CHECK(url_con, ECORE_MAGIC_CON_URL))
     {
        ECORE_MAGIC_FAIL(url_con, ECORE_MAGIC_CON_URL, __func__);
        return;
     }
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
ecore_con_url_ssl_verify_peer_set(Ecore_Con_Url *url_con, Eina_Bool verify)
{
   if (!_c) return;
   if (!ECORE_MAGIC_CHECK(url_con, ECORE_MAGIC_CON_URL))
     {
        ECORE_MAGIC_FAIL(url_con, ECORE_MAGIC_CON_URL, __func__);
        return;
     }
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
ecore_con_url_ssl_ca_set(Ecore_Con_Url *url_con, const char *ca_path)
{
   int res = -1;

   if (!_c) return -1;
   if (!ECORE_MAGIC_CHECK(url_con, ECORE_MAGIC_CON_URL))
     {
        ECORE_MAGIC_FAIL(url_con, ECORE_MAGIC_CON_URL, __func__);
        return -1;
     }
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
ecore_con_url_http_version_set(Ecore_Con_Url *url_con, Ecore_Con_Url_Http_Version version)
{
   int res = -1;

   if (!_c) return EINA_FALSE;
   if (!ECORE_MAGIC_CHECK(url_con, ECORE_MAGIC_CON_URL))
     {
        ECORE_MAGIC_FAIL(url_con, ECORE_MAGIC_CON_URL, __func__);
        return EINA_FALSE;
     }
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
ecore_con_url_proxy_set(Ecore_Con_Url *url_con, const char *proxy)
{
   int res = -1;
   curl_version_info_data *vers = NULL;

   if (!_c) return EINA_FALSE;
   if (!ECORE_MAGIC_CHECK(url_con, ECORE_MAGIC_CON_URL))
     {
        ECORE_MAGIC_FAIL(url_con, ECORE_MAGIC_CON_URL, __func__);
        return EINA_FALSE;
     }
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
ecore_con_url_timeout_set(Ecore_Con_Url *url_con, double timeout)
{
   if (!ECORE_MAGIC_CHECK(url_con, ECORE_MAGIC_CON_URL))
     {
        ECORE_MAGIC_FAIL(url_con, ECORE_MAGIC_CON_URL, __func__);
        return;
     }
   if (url_con->dead) return;
   if (!url_con->url || timeout < 0) return;
   if (url_con->timer) ecore_timer_del(url_con->timer);
   url_con->timer = ecore_timer_add(timeout, _ecore_con_url_timeout_cb,
                                    url_con);
}

EAPI Eina_Bool
ecore_con_url_proxy_username_set(Ecore_Con_Url *url_con, const char *username)
{
   int res = -1;

   if (!_c) return EINA_FALSE;
   if (!ECORE_MAGIC_CHECK(url_con, ECORE_MAGIC_CON_URL))
     {
        ECORE_MAGIC_FAIL(url_con, ECORE_MAGIC_CON_URL, __func__);
        return EINA_FALSE;
     }
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
ecore_con_url_proxy_password_set(Ecore_Con_Url *url_con, const char *password)
{
   int res = -1;

   if (!_c) return EINA_FALSE;
   if (!ECORE_MAGIC_CHECK(url_con, ECORE_MAGIC_CON_URL))
     {
        ECORE_MAGIC_FAIL(url_con, ECORE_MAGIC_CON_URL, __func__);
        return EINA_FALSE;
     }
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
_ecore_con_url_status_get(Ecore_Con_Url *url_con)
{
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
_ecore_con_url_event_url_complete(Ecore_Con_Url *url_con, CURLMsg *curlmsg)
{
   Ecore_Con_Event_Url_Complete *e;
   int status = url_con->status;

   if (!_c) return;
   e = calloc(1, sizeof(Ecore_Con_Event_Url_Complete));
   if (!e) return;

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
             _ecore_con_url_status_get(url_con);
             status = url_con->status;
          }
     }
   else
     {
        ERR("Curl message have errors: %d (%s)",
            curlmsg->data.result, _c->curl_easy_strerror(curlmsg->data.result));
     }
   e->status = status;
   e->url_con = url_con;
   url_con->event_count++;
   ecore_event_add(ECORE_CON_EVENT_URL_COMPLETE, e,
                   (Ecore_End_Cb)_ecore_con_event_url_free, url_con);
}

static void
_ecore_con_url_multi_remove(Ecore_Con_Url *url_con)
{
   CURLMcode ret;

   if (!_c) return;
   ret = _c->curl_multi_remove_handle(_c->_curlm, url_con->curl_easy);
   url_con->multi = EINA_FALSE;
   if (ret != CURLM_OK) ERR("curl_multi_remove_handle failed: %s", _c->curl_multi_strerror(ret));
}

static Eina_Bool
_ecore_con_url_timeout_cb(void *data)
{
   Ecore_Con_Url *url_con = data;
   CURLMsg timeout_msg;

   if (!_c) return ECORE_CALLBACK_CANCEL;
   if (!url_con) return ECORE_CALLBACK_CANCEL;
   if (!url_con->curl_easy) return ECORE_CALLBACK_CANCEL;

   _ecore_con_url_multi_remove(url_con);
   _url_con_list = eina_list_remove(_url_con_list, url_con);

   _c->curl_slist_free_all(url_con->headers);
   url_con->headers = NULL;

   url_con->timer = NULL;

   timeout_msg.msg = CURLMSG_DONE;
   timeout_msg.easy_handle = NULL;
   timeout_msg.data.result = CURLE_OPERATION_TIMEDOUT;

   _ecore_con_url_event_url_complete(url_con, &timeout_msg);
   return ECORE_CALLBACK_CANCEL;
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
        ECORE_MAGIC_FAIL(url_con, ECORE_MAGIC_CON_URL, __func__);
        return -1;
     }

   url_con->received += real_size;
   INF("reading from %s", url_con->url);
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
             url_con->event_count++;
             ecore_event_add(ECORE_CON_EVENT_URL_DATA, e,
                             (Ecore_End_Cb)_ecore_con_event_url_free, url_con);
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
   Ecore_Con_Url *url_con = stream;

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
        url_con->event_count++;
        ecore_event_add(ECORE_CON_EVENT_URL_PROGRESS, e,
                        (Ecore_End_Cb)_ecore_con_event_url_free, url_con);
     }
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
        Ecore_Con_Url *url_con = NULL;

        DBG("Curl message: %d", curlmsg->msg);
        if (curlmsg->msg == CURLMSG_DONE)
          {
             EINA_LIST_FOREACH_SAFE(_url_con_list, l, ll, url_con)
               {
                  if (curlmsg->easy_handle == url_con->curl_easy)
                    _ecore_con_url_event_url_complete(url_con, curlmsg);
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
_ecore_con_url_fd_handler(void *data EINA_UNUSED, Ecore_Fd_Handler *fd_handler EINA_UNUSED)
{
   Ecore_Fd_Handler *fdh;
   long ms;

   if (!_c) return ECORE_CALLBACK_CANCEL;
   EINA_LIST_FREE(_fd_hd_list, fdh)
     ecore_main_fd_handler_del(fdh);
   _c->curl_multi_timeout(_c->_curlm, &ms);
   if ((ms >= CURL_MIN_TIMEOUT) || (ms <= 0)) ms = CURL_MIN_TIMEOUT;
   ecore_timer_interval_set(_curl_timer, (double)ms / 1000.0);
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
        long ms;

        _ecore_con_url_fdset();
        _c->curl_multi_timeout(_c->_curlm, &ms);
        DBG("multiperform is still running: %d, timeout: %ld",
            still_running, ms);
        if ((ms >= CURL_MIN_TIMEOUT) || (ms <= 0)) ms = CURL_MIN_TIMEOUT;
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
_ecore_con_url_perform(Ecore_Con_Url *url_con)
{
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
   _url_con_list = eina_list_append(_url_con_list, url_con);
   ecore_timer_thaw(_curl_timer);
   return EINA_TRUE;
}

static void
_ecore_con_event_url_free(Ecore_Con_Url *url_con, void *ev)
{
   free(ev);
   url_con->event_count--;
   if (url_con->dead && (!url_con->event_count)) ecore_con_url_free(url_con);
}

