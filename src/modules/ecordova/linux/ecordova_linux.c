#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <Ecore.h>

int _ecordova_log_dom;

#define CRI(...) EINA_LOG_DOM_CRIT(_ecordova_log_dom, __VA_ARGS__)
#define ERR(...) EINA_LOG_DOM_ERR(_ecordova_log_dom, __VA_ARGS__)
#define WRN(...) EINA_LOG_DOM_WARN(_ecordova_log_dom, __VA_ARGS__)
#define INF(...) EINA_LOG_DOM_INFO(_ecordova_log_dom, __VA_ARGS__)
#define DBG(...) EINA_LOG_DOM_DBG(_ecordova_log_dom, __VA_ARGS__)

static Eina_Bool
_ecordova_linux_init(void)
{
   fprintf(stderr, "%s:%s:%d\n", __func__, __FILE__, __LINE__); fflush(stderr);
    
   _ecordova_log_dom = eina_log_domain_register("ecordova_linux", EINA_COLOR_CYAN);
   if (_ecordova_log_dom < 0)
     {
        EINA_LOG_ERR("Unable to create an 'ecordova' log domain");
        //goto on_error_1;
     }

   if (!ecore_file_init())
     {
        ERR("Unable to initialize ecore_file");
        //goto on_error_3;
     }

   if (!eio_init())
     {
        ERR("Unable to initialize eio");
        //goto on_error_4;
     }
   
   
   return EINA_TRUE;
}

static void
_ecordova_linux_shutdown(void)
{
   fprintf(stderr, "%s:%s:%d\n", __func__, __FILE__, __LINE__); fflush(stderr);
   eio_shutdown();
   ecore_file_shutdown();
}

EINA_MODULE_INIT(_ecordova_linux_init);
EINA_MODULE_SHUTDOWN(_ecordova_linux_shutdown);
