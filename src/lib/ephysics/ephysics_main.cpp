#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "Ecore.h"
#include "Eina.h"

#include "ephysics_private.h"

#ifdef  __cplusplus
extern "C" {
#endif

int _ephysics_log_dom = -1;
static int _ephysics_dom_count = 0;
static int _ephysics_init_count = 0;

void
ephysics_dom_count_inc(void)
{
   _ephysics_dom_count++;
}

void
ephysics_dom_count_dec(void)
{
   _ephysics_dom_count--;
   if (_ephysics_dom_count)
     return;

   INF("EPhysics shutdown");

   eina_log_domain_unregister(_ephysics_log_dom);
   _ephysics_log_dom = -1;

   eina_shutdown();
}

EAPI int
ephysics_init()
{
   if (++_ephysics_init_count != 1)
     return _ephysics_init_count;

   if (!eina_init())
     {
        EINA_LOG_CRIT("Couldn't init eina");
        return --_ephysics_init_count;
     }

   if (!ecore_init())
     {
        EINA_LOG_CRIT("Couldn't init ecore");
        goto no_ecore;
     }

   _ephysics_log_dom = eina_log_domain_register("ephysics", EPHYSICS_LOG_COLOR);
   if (_ephysics_log_dom < 0)
     {
        EINA_LOG_CRIT("Couldn't create a log domain for ephysics");
        goto no_log;
     }

   if (!ephysics_world_init())
     {
        ERR("Couldn't initialize worlds");
        goto no_world;
     }

   ephysics_dom_count_inc();
   INF("EPhysics initialized");

   return _ephysics_init_count;

no_world:
   eina_log_domain_unregister(_ephysics_log_dom);
   _ephysics_log_dom = -1;
no_log:
   ecore_shutdown();
no_ecore:
   eina_shutdown();
   return --_ephysics_init_count;
}

EAPI int
ephysics_shutdown()
{
   if (--_ephysics_init_count != 0)
     return _ephysics_init_count;

   ephysics_world_shutdown();
   ecore_shutdown();
   ephysics_dom_count_dec();

   return _ephysics_init_count;
}

#ifdef  __cplusplus
}
#endif
