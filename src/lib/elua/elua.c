#include <Eina.h>

#include "Elua.h"
#include "elua_private.h"

static Eina_Prefix *_elua_pfx = NULL;

static int _elua_init_counter = 0;
int _elua_log_dom = -1;

EAPI int
elua_init(void)
{
   const char *dom = "elua";
   if (_elua_init_counter > 0) return ++_elua_init_counter;

   eina_init();
   _elua_log_dom = eina_log_domain_register(dom, EINA_COLOR_LIGHTBLUE);
   if (_elua_log_dom < 0)
     {
        EINA_LOG_ERR("Could not register log domain: %s", dom);
        return EINA_FALSE;
     }

   eina_log_timing(_elua_log_dom, EINA_LOG_STATE_STOP, EINA_LOG_STATE_INIT);
   INF("elua init");

   _elua_pfx = eina_prefix_new(NULL, elua_init, "ELUA", "elua", NULL,
                               PACKAGE_BIN_DIR, "", PACKAGE_DATA_DIR,
                               LOCALE_DIR);

   if (!_elua_pfx)
     {
        ERR("coul not find elua prefix");
        return EINA_FALSE;
     }

   return ++_elua_init_counter;
}

EAPI int
elua_shutdown(void)
{
   if (_elua_init_counter <= 0)
     {
        EINA_LOG_ERR("Init count not greater than 0 in shutdown.");
        return EINA_FALSE;
     }
   --_elua_init_counter;

   if (_elua_init_counter > 0)
     return _elua_init_counter;

   INF("shutdown");
   eina_log_timing(_elua_log_dom, EINA_LOG_STATE_START, EINA_LOG_STATE_SHUTDOWN);

   eina_prefix_free(_elua_pfx);
   _elua_pfx = NULL;

   eina_log_domain_unregister(_elua_log_dom);
   _elua_log_dom = -1;

   eina_shutdown();
   return _elua_init_counter;
}
