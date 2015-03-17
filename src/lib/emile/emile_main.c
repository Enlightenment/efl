#include <Eina.h>

#include "Emile.h"

static unsigned int _emile_init_count = 0;
int _emile_log_dom_global = -1;

EAPI int
emile_init(void)
{
   if (++_emile_init_count != 1)
     return _emile_init_count;

   if (!eina_init())
     return --_emile_init_count;

   _emile_log_dom_global = eina_log_domain_register("emile", EINA_COLOR_CYAN);
   if (_emile_log_dom_global < 0)
     {
        EINA_LOG_ERR("Emile can not create a general log domain.");
        goto shutdown_eina;
     }

   // FIXME: Init the rest here.

   eina_log_timing(_emile_log_dom_global,
                   EINA_LOG_STATE_STOP,
                   EINA_LOG_STATE_INIT);

   return _emile_init_count;

 shutdown_eina:
   eina_shutdown();

   return --_emile_init_count;
}

EAPI int
emile_shutdown(void)
{
   if (--_emile_init_count != 0)
     return _emile_init_count;

   eina_log_timing(_emile_log_dom_global,
                   EINA_LOG_STATE_START,
                   EINA_LOG_STATE_SHUTDOWN);

   // FIXME: Shutdown the rest here.

   eina_log_domain_unregister(_emile_log_dom_global);
   _emile_log_dom_global = -1;

   eina_shutdown();

   return _emile_init_count;
}
