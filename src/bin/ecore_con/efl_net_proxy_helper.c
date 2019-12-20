#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>

#include "Eina.h"

#ifdef ERR
# undef ERR
#endif
#define ERR(...) EINA_LOG_DOM_ERR(EINA_LOG_DOMAIN_GLOBAL, __VA_ARGS__)

#ifdef DBG
# undef DBG
#endif
#define DBG(...) EINA_LOG_DOM_DBG(EINA_LOG_DOMAIN_GLOBAL, __VA_ARGS__)

#ifdef INF
# undef INF
#endif
#define INF(...) EINA_LOG_DOM_INFO(EINA_LOG_DOMAIN_GLOBAL, __VA_ARGS__)

#ifdef WRN
# undef WRN
#endif
#define WRN(...) EINA_LOG_DOM_WARN(EINA_LOG_DOMAIN_GLOBAL, __VA_ARGS__)

#ifdef CRI
# undef CRI
#endif
#define CRI(...) EINA_LOG_DOM_CRIT(EINA_LOG_DOMAIN_GLOBAL, __VA_ARGS__)

typedef struct pxProxyFactory_  pxProxyFactory;
typedef struct _Libproxy
{
   pxProxyFactory    *factory;
   char           **(*px_proxy_factory_get_proxies) (pxProxyFactory *factory, const char *url);
   void            *(*px_proxy_factory_new)         (void);
   void             (*px_proxy_factory_free)        (pxProxyFactory *);
   Eina_Module       *mod;
} Libproxy;
static Libproxy _libproxy = { 0 };

static Eina_Spinlock pending_lock;
static int pending = 0;
static int opcount = 0;
static Eina_List *join_list = NULL;

static Eina_Bool
init(void)
{
   if (!_libproxy.mod)
     {
#define LOAD(x) \
   if (!_libproxy.mod) { \
      _libproxy.mod = eina_module_new(x); \
      if (_libproxy.mod) { \
         if (!eina_module_load(_libproxy.mod)) { \
            eina_module_free(_libproxy.mod); \
            _libproxy.mod = NULL; \
         } \
      } \
   }
#if defined(_WIN32) || defined(__CYGWIN__)
        LOAD("libproxy-1.dll");
        LOAD("libproxy.dll");
#elif defined(__APPLE__) && defined(__MACH__)
        LOAD("libproxy.1.dylib");
        LOAD("libproxy.dylib");
#else
        LOAD("libproxy.so.1");
        LOAD("libproxy.so");
#endif
#undef LOAD
        if (!_libproxy.mod)
          {
             DBG("Couldn't find libproxy in your system. Continue without it");
             return EINA_FALSE;
          }

#define SYM(x) \
   if ((_libproxy.x = eina_module_symbol_get(_libproxy.mod, #x)) == NULL) { \
      ERR("libproxy (%s) missing symbol %s", \
          eina_module_file_get(_libproxy.mod), #x); \
      eina_module_free(_libproxy.mod); \
      _libproxy.mod = NULL; \
      return EINA_FALSE; \
   }

        SYM(px_proxy_factory_new);
        SYM(px_proxy_factory_free);
        SYM(px_proxy_factory_get_proxies);
#undef SYM
        DBG("using libproxy=%s", eina_module_file_get(_libproxy.mod));
     }

   if (!_libproxy.factory)
     _libproxy.factory = _libproxy.px_proxy_factory_new();

   return !!_libproxy.factory;
}

static void
shutdown(void)
{
   if (_libproxy.factory)
     {
        _libproxy.px_proxy_factory_free(_libproxy.factory);
        _libproxy.factory = NULL;
     }
   if (_libproxy.mod)
     {
        eina_module_free(_libproxy.mod);
        _libproxy.mod = NULL;
     }
}

static void *
proxy_lookup(void *data, Eina_Thread t)
{
   char *cmd = data;
   char **proxies, **itr;
   const char *p, *url;
   int id = atoi(cmd + 2);
   int pending_local, opcount_prev;

   if (id > 0)
     {
        for (p = cmd + 2; *p && (*p != ' '); p++);
        if (*p == ' ')
          {
             url = p + 1;
             proxies = _libproxy.px_proxy_factory_get_proxies
               (_libproxy.factory, url);
             if (proxies)
               {
                  for (itr = proxies; *itr != NULL; itr++)
                    {
                       fprintf(stdout, "P %i P %s\n", id, *itr);
                       free(*itr);
                    }
                  free(proxies);
               }
             fprintf(stdout, "P %i E\n", id);
             fflush(stdout);
          }
     }
   free(cmd);

   eina_spinlock_take(&pending_lock);
     {
        pending--;
        pending_local = pending;
        opcount_prev = opcount;
     }
   eina_spinlock_release(&pending_lock);
   // if there are no more pending threads doing work - sleep for the
   // timeout then check if we still are and if so - exit;
   if (pending_local == 0) sleep(10);
   eina_spinlock_take(&pending_lock);
     {
        Eina_Thread *tt;

        if ((pending == 0) & (opcount == opcount_prev)) exit(0);
        tt = calloc(1, sizeof(Eina_Thread));
        if (tt)
          {
             *tt = t;
             join_list = eina_list_append(join_list, tt);
          }
     }
   eina_spinlock_release(&pending_lock);
   return NULL;
}

static void
handle(const char *cmd)
{
   // "P 1234 URL" -> Get Proxy, id=1234, url=URL
   if ((cmd[0] == 'P') && (cmd[1] == ' '))
     {
        char *dup = strdup(cmd);

        if (dup)
          {
             Eina_Thread t;

             eina_spinlock_take(&pending_lock);
               {
                  pending++;
                  opcount++;
               }
             eina_spinlock_release(&pending_lock);
             if (!eina_thread_create(&t, EINA_THREAD_BACKGROUND, -1,
                                     proxy_lookup, dup))
               {
                  abort();
               }
          }
        return;
     }
}

static void
clean_threads(void)
{
   eina_spinlock_take(&pending_lock);
     {
        Eina_Thread *t;

        EINA_LIST_FREE(join_list, t)
          {
             eina_thread_join(*t);
             free(t);
          }
     }
   eina_spinlock_release(&pending_lock);
}

int
main(int argc EINA_UNUSED, char **argv EINA_UNUSED)
{
   char inbuf[8192];
   eina_init();
   if (init())
     {
        eina_spinlock_new(&pending_lock);
        // 1 command per stdin line
        while (fgets(inbuf, sizeof(inbuf) - 1, stdin))
          {
             // strip off newline and ensure the string is 0 terminated
             int len = strlen(inbuf);
             if (len > 0)
               {
                  if (inbuf[len -1 ] == '\n') inbuf[len - 1] = 0;
                  else inbuf[len] = 0;
                  handle(inbuf);
               }
             clean_threads();
          }
        eina_spinlock_free(&pending_lock);
        shutdown();
     }
   else
     {
        // Failed to init libproxy so report this before exit
        fprintf(stdout, "F\n");
        fflush(stdout);
        for (;;) sleep(60 * 60 * 24);
     }
   eina_shutdown();
   return 0;
}
