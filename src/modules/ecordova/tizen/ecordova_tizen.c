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
_ecordova_tizen_init(void)
{
   _ecordova_log_dom = eina_log_domain_register("ecordova_tizen", EINA_COLOR_CYAN);
   if (_ecordova_log_dom < 0)
     {
        EINA_LOG_ERR("Unable to create an 'ecordova' log domain");
        return EINA_FALSE;
     }

   return EINA_TRUE;
}

static void
_ecordova_tizen_shutdown(void)
{
   eina_log_domain_unregister(_ecordova_log_dom);
   _ecordova_log_dom = -1;
}

EINA_MODULE_INIT(_ecordova_system_tizen_init);
EINA_MODULE_SHUTDOWN(_ecordova_system_tizen_shutdown);
