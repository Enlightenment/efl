#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#ifdef _WIN32
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

Ecore_Con_Curl *_c = NULL;
Eina_Bool _c_fail = EINA_FALSE;
double _c_timeout = 0.0;

EWAPI Eina_Error EFL_NET_HTTP_ERROR_BAD_CONTENT_ENCODING = 0;
EWAPI Eina_Error EFL_NET_HTTP_ERROR_BAD_DOWNLOAD_RESUME = 0;
EWAPI Eina_Error EFL_NET_HTTP_ERROR_BAD_FUNCTION_ARGUMENT = 0;
EWAPI Eina_Error EFL_NET_HTTP_ERROR_CHUNK_FAILED = 0;
EWAPI Eina_Error EFL_NET_HTTP_ERROR_CONV_FAILED = 0;
EWAPI Eina_Error EFL_NET_HTTP_ERROR_CONV_REQD = 0;
EWAPI Eina_Error EFL_NET_HTTP_ERROR_FAILED_INIT = 0;
EWAPI Eina_Error EFL_NET_HTTP_ERROR_FILE_COULDNT_READ_FILE = 0;
EWAPI Eina_Error EFL_NET_HTTP_ERROR_FILESIZE_EXCEEDED = 0;
EWAPI Eina_Error EFL_NET_HTTP_ERROR_FUNCTION_NOT_FOUND = 0;
EWAPI Eina_Error EFL_NET_HTTP_ERROR_GOT_NOTHING = 0;
EWAPI Eina_Error EFL_NET_HTTP_ERROR_HTTP2 = 0;
EWAPI Eina_Error EFL_NET_HTTP_ERROR_HTTP2_STREAM = 0;
EWAPI Eina_Error EFL_NET_HTTP_ERROR_HTTP_POST_ERROR = 0;
EWAPI Eina_Error EFL_NET_HTTP_ERROR_HTTP_RETURNED_ERROR = 0;
EWAPI Eina_Error EFL_NET_HTTP_ERROR_INTERFACE_FAILED = 0;
EWAPI Eina_Error EFL_NET_HTTP_ERROR_LOGIN_DENIED = 0;
EWAPI Eina_Error EFL_NET_HTTP_ERROR_NO_CONNECTION_AVAILABLE = 0;
EWAPI Eina_Error EFL_NET_HTTP_ERROR_NOT_BUILT_IN = 0;
EWAPI Eina_Error EFL_NET_HTTP_ERROR_OPERATION_TIMEDOUT = 0;
EWAPI Eina_Error EFL_NET_HTTP_ERROR_PARTIAL_FILE = 0;
EWAPI Eina_Error EFL_NET_HTTP_ERROR_PEER_FAILED_VERIFICATION = 0;
EWAPI Eina_Error EFL_NET_HTTP_ERROR_RANGE_ERROR = 0;
EWAPI Eina_Error EFL_NET_HTTP_ERROR_READ_ERROR = 0;
EWAPI Eina_Error EFL_NET_HTTP_ERROR_RECV_ERROR = 0;
EWAPI Eina_Error EFL_NET_HTTP_ERROR_REMOTE_ACCESS_DENIED = 0;
EWAPI Eina_Error EFL_NET_HTTP_ERROR_REMOTE_DISK_FULL = 0;
EWAPI Eina_Error EFL_NET_HTTP_ERROR_REMOTE_FILE_EXISTS = 0;
EWAPI Eina_Error EFL_NET_HTTP_ERROR_REMOTE_FILE_NOT_FOUND = 0;
EWAPI Eina_Error EFL_NET_HTTP_ERROR_SEND_ERROR = 0;
EWAPI Eina_Error EFL_NET_HTTP_ERROR_SEND_FAIL_REWIND = 0;
EWAPI Eina_Error EFL_NET_HTTP_ERROR_SSL_CACERT = 0;
EWAPI Eina_Error EFL_NET_HTTP_ERROR_SSL_CACERT_BADFILE = 0;
EWAPI Eina_Error EFL_NET_HTTP_ERROR_SSL_CERTPROBLEM = 0;
EWAPI Eina_Error EFL_NET_HTTP_ERROR_SSL_CIPHER = 0;
EWAPI Eina_Error EFL_NET_HTTP_ERROR_SSL_CONNECT_ERROR = 0;
EWAPI Eina_Error EFL_NET_HTTP_ERROR_SSL_CRL_BADFILE = 0;
EWAPI Eina_Error EFL_NET_HTTP_ERROR_SSL_ENGINE_INITFAILED = 0;
EWAPI Eina_Error EFL_NET_HTTP_ERROR_SSL_ENGINE_NOTFOUND = 0;
EWAPI Eina_Error EFL_NET_HTTP_ERROR_SSL_ENGINE_SETFAILED = 0;
EWAPI Eina_Error EFL_NET_HTTP_ERROR_SSL_INVALIDCERTSTATUS = 0;
EWAPI Eina_Error EFL_NET_HTTP_ERROR_SSL_ISSUER_ERROR = 0;
EWAPI Eina_Error EFL_NET_HTTP_ERROR_SSL_PINNEDPUBKEYNOTMATCH = 0;
EWAPI Eina_Error EFL_NET_HTTP_ERROR_SSL_SHUTDOWN_FAILED = 0;
EWAPI Eina_Error EFL_NET_HTTP_ERROR_TOO_MANY_REDIRECTS = 0;
EWAPI Eina_Error EFL_NET_HTTP_ERROR_UNKNOWN_OPTION = 0;
EWAPI Eina_Error EFL_NET_HTTP_ERROR_UNSUPPORTED_PROTOCOL = 0;
EWAPI Eina_Error EFL_NET_HTTP_ERROR_UPLOAD_FAILED = 0;
EWAPI Eina_Error EFL_NET_HTTP_ERROR_URL_MALFORMAT = 0;
EWAPI Eina_Error EFL_NET_HTTP_ERROR_USE_SSL_FAILED = 0;
EWAPI Eina_Error EFL_NET_HTTP_ERROR_WRITE_ERROR = 0;

Eina_Error
_curlcode_to_eina_error(const CURLcode code)
{
   switch (code) {
    case CURLE_OK: return 0;
    case CURLE_ABORTED_BY_CALLBACK: return ECONNABORTED;
    case CURLE_AGAIN: return EAGAIN;
    case CURLE_OUT_OF_MEMORY: return ENOMEM;

    case CURLE_COULDNT_CONNECT: return EFL_NET_DIALER_ERROR_COULDNT_CONNECT;
    case CURLE_COULDNT_RESOLVE_PROXY: return EFL_NET_DIALER_ERROR_COULDNT_RESOLVE_PROXY;
    case CURLE_COULDNT_RESOLVE_HOST: return EFL_NET_ERROR_COULDNT_RESOLVE_HOST;

#define _MAP(n) case CURLE_ ## n: return EFL_NET_HTTP_ERROR_ ## n

   _MAP(BAD_CONTENT_ENCODING);
   _MAP(BAD_DOWNLOAD_RESUME);
   _MAP(BAD_FUNCTION_ARGUMENT);
   _MAP(CHUNK_FAILED);
   _MAP(CONV_FAILED);
   _MAP(CONV_REQD);
   _MAP(FAILED_INIT);
   _MAP(FILE_COULDNT_READ_FILE);
   _MAP(FILESIZE_EXCEEDED);
   _MAP(FUNCTION_NOT_FOUND);
   _MAP(GOT_NOTHING);
   _MAP(HTTP2);
   _MAP(HTTP2_STREAM);
   _MAP(HTTP_POST_ERROR);
   _MAP(HTTP_RETURNED_ERROR);
   _MAP(INTERFACE_FAILED);
   _MAP(LOGIN_DENIED);
   _MAP(NO_CONNECTION_AVAILABLE);
   _MAP(NOT_BUILT_IN);
   _MAP(OPERATION_TIMEDOUT);
   _MAP(PARTIAL_FILE);
   _MAP(PEER_FAILED_VERIFICATION);
   _MAP(RANGE_ERROR);
   _MAP(READ_ERROR);
   _MAP(RECV_ERROR);
   _MAP(REMOTE_ACCESS_DENIED);
   _MAP(REMOTE_DISK_FULL);
   _MAP(REMOTE_FILE_EXISTS);
   _MAP(REMOTE_FILE_NOT_FOUND);
   _MAP(SEND_ERROR);
   _MAP(SEND_FAIL_REWIND);
   _MAP(SSL_CACERT);
   _MAP(SSL_CACERT_BADFILE);
   _MAP(SSL_CERTPROBLEM);
   _MAP(SSL_CIPHER);
   _MAP(SSL_CONNECT_ERROR);
   _MAP(SSL_CRL_BADFILE);
   _MAP(SSL_ENGINE_INITFAILED);
   _MAP(SSL_ENGINE_NOTFOUND);
   _MAP(SSL_ENGINE_SETFAILED);
   _MAP(SSL_INVALIDCERTSTATUS);
   _MAP(SSL_ISSUER_ERROR);
   _MAP(SSL_PINNEDPUBKEYNOTMATCH);
   _MAP(SSL_SHUTDOWN_FAILED);
   _MAP(TOO_MANY_REDIRECTS);
   _MAP(UNKNOWN_OPTION);
   _MAP(UNSUPPORTED_PROTOCOL);
   _MAP(UPLOAD_FAILED);
   _MAP(URL_MALFORMAT);
   _MAP(USE_SSL_FAILED);
   _MAP(WRITE_ERROR);
#undef _MAP

    default:
       ERR("unexpected error CURLcode=%d '%s', not mapped",
           code, _c->curl_easy_strerror(code));
       return EINVAL;
   }
}

Eina_Error
_curlmcode_to_eina_error(const CURLMcode code)
{
   switch (code) {
    case CURLM_OK: return 0;
    case CURLM_BAD_HANDLE: return EBADF;
    case CURLM_BAD_EASY_HANDLE: return EBADF;
    case CURLM_OUT_OF_MEMORY: return ENOMEM;
    case CURLM_INTERNAL_ERROR: EIO; /* not exact, but the error should happen so not much of a problem */
    case CURLM_BAD_SOCKET: return ENOTSOCK;
    case CURLM_UNKNOWN_OPTION: return EINVAL;
    case CURLM_ADDED_ALREADY: return EALREADY;

    default:
       ERR("unexpected error CURLMcode=%d '%s', not mapped",
           code, _c->curl_multi_strerror(code));
       return EINVAL;
   }
}

static void
_c_init_errors(void)
{
   /* use from system: */
   // CURLE_ABORTED_BY_CALLBACK = ECONNABORTED
   // CURLE_AGAIN = EAGAIN
   // CURLE_OUT_OF_MEMORY = ENOMEM

   if (EFL_NET_HTTP_ERROR_BAD_CONTENT_ENCODING) return; /* only once in the whole runtime */

#define _MAP(n) EFL_NET_HTTP_ERROR_ ## n = eina_error_msg_static_register(_c->curl_easy_strerror(CURLE_ ## n))
   _MAP(BAD_CONTENT_ENCODING);
   _MAP(BAD_DOWNLOAD_RESUME);
   _MAP(BAD_FUNCTION_ARGUMENT);
   _MAP(CHUNK_FAILED);
   _MAP(CONV_FAILED);
   _MAP(CONV_REQD);
   _MAP(FAILED_INIT);
   _MAP(FILE_COULDNT_READ_FILE);
   _MAP(FILESIZE_EXCEEDED);
   _MAP(FUNCTION_NOT_FOUND);
   _MAP(GOT_NOTHING);
   _MAP(HTTP2);
   _MAP(HTTP2_STREAM);
   _MAP(HTTP_POST_ERROR);
   _MAP(HTTP_RETURNED_ERROR);
   _MAP(INTERFACE_FAILED);
   _MAP(LOGIN_DENIED);
   _MAP(NO_CONNECTION_AVAILABLE);
   _MAP(NOT_BUILT_IN);
   _MAP(OPERATION_TIMEDOUT);
   _MAP(PARTIAL_FILE);
   _MAP(PEER_FAILED_VERIFICATION);
   _MAP(RANGE_ERROR);
   _MAP(READ_ERROR);
   _MAP(RECV_ERROR);
   _MAP(REMOTE_ACCESS_DENIED);
   _MAP(REMOTE_DISK_FULL);
   _MAP(REMOTE_FILE_EXISTS);
   _MAP(REMOTE_FILE_NOT_FOUND);
   _MAP(SEND_ERROR);
   _MAP(SEND_FAIL_REWIND);
   _MAP(SSL_CACERT);
   _MAP(SSL_CACERT_BADFILE);
   _MAP(SSL_CERTPROBLEM);
   _MAP(SSL_CIPHER);
   _MAP(SSL_CONNECT_ERROR);
   _MAP(SSL_CRL_BADFILE);
   _MAP(SSL_ENGINE_INITFAILED);
   _MAP(SSL_ENGINE_NOTFOUND);
   _MAP(SSL_ENGINE_SETFAILED);
   _MAP(SSL_INVALIDCERTSTATUS);
   _MAP(SSL_ISSUER_ERROR);
   _MAP(SSL_PINNEDPUBKEYNOTMATCH);
   _MAP(SSL_SHUTDOWN_FAILED);
   _MAP(TOO_MANY_REDIRECTS);
   _MAP(UNKNOWN_OPTION);
   _MAP(UNSUPPORTED_PROTOCOL);
   _MAP(UPLOAD_FAILED);
   _MAP(URL_MALFORMAT);
   _MAP(USE_SSL_FAILED);
   _MAP(WRITE_ERROR);
#undef _MAP
}


Eina_Bool
_c_init(void)
{
   long ms = 0;

   if (_c)
     {
        _c->ref++;
        return EINA_TRUE;
     }
   if (_c_fail)
     {
        ERR("Cannot find libcurl at runtime!");
        return EINA_FALSE;
     }
   _c = calloc(1, sizeof(Ecore_Con_Curl));
   if (!_c) goto error;
   _c->ref++;

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
   LOAD("libcurl-5.dll"); // try correct dll first
   LOAD("libcurl-4.dll"); // try correct dll first
   LOAD("libcurl.dll"); // try 1
   LOAD("curllib.dll"); // if fail try 2
   if (!_c->mod)
     ERR("Could not find libcurl-5.dll, libcurl-4.dll, libcurl.dll, curllib.dll");
#elif defined(__APPLE__) && defined(__MACH__)
   LOAD("libcurl.5.dylib"); // try 1
   LOAD("libcurl.4.dylib"); // try 1
   LOAD("libcurl.so.5"); // if fail try 2
   LOAD("libcurl.so.4"); // if fail try 2
   if (!_c->mod)
     ERR("Could not find libcurl.5.dylib, libcurl.4.dylib, libcurl.so.5, libcurl.so.4");
#else
   LOAD("libcurl.so.5"); // try only
   LOAD("libcurl.so.4"); // try only
   if (!_c->mod)
     ERR("Could not find libcurl.so.5, libcurl.so.4");
#endif
   if (!_c->mod) goto error;

#define SYM(x) \
   if (!(_c->x = eina_module_symbol_get(_c->mod, #x))) { \
      ERR("Cannot find symbol '%s' in'%s", #x, eina_module_file_get(_c->mod)); \
      goto error; \
   }
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
   SYM(curl_multi_socket_action);
   SYM(curl_multi_assign);
   SYM(curl_easy_init);
   SYM(curl_easy_setopt);
   SYM(curl_easy_strerror);
   SYM(curl_easy_cleanup);
   SYM(curl_easy_getinfo);
   SYM(curl_easy_pause);
   SYM(curl_slist_free_all);
   SYM(curl_slist_append);
   SYM(curl_version_info);
   SYM(curl_getdate);

   _c_init_errors();

   // curl_global_init() is not thread safe!
   if (_c->curl_global_init(CURL_GLOBAL_ALL)) goto error;
   _c->_curlm = _c->curl_multi_init();
   if (!_c->_curlm)
     {
        _c->curl_global_cleanup();
        goto error;
     }
   _c->curl_multi_timeout(_c->_curlm, &ms);
   if ((ms >= CURL_MIN_TIMEOUT) || (ms < 0)) ms = CURL_MIN_TIMEOUT;
   _c_timeout = ms / 1000.0;
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

void
_c_shutdown(void)
{
   if (!_c || _c->ref--) return;
   if (_c->_curlm)
     {
        _c->curl_multi_cleanup(_c->_curlm);
     }
   _c->curl_global_cleanup();
   if (_c->mod) eina_module_free(_c->mod);
   free(_c);
   _c = NULL;
}
