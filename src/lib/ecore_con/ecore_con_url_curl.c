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

Ecore_Con_Curl *_c = NULL;
Eina_Bool _c_fail = EINA_FALSE;
double _c_timeout = 0.0;

Eina_Bool
_c_init(void)
{
   long ms = 0;

   if (_c) return EINA_TRUE;
   if (_c_fail)
     {
        ERR("Cannot find libcurl at runtime!");
        return EINA_FALSE;
     }
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
