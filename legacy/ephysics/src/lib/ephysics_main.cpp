#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "ephysics_private.h"

#ifdef  __cplusplus
extern "C" {
#endif

int _ephysics_log_dom = -1;
static int _ephysics_init_count = 0;

EAPI int
ephysics_init()
{
   if (++_ephysics_init_count != 1)
     return _ephysics_init_count;

   _ephysics_log_dom = eina_log_domain_register("ephysics", EPHYSICS_LOG_COLOR);
   if (_ephysics_log_dom < 0)
     {
	EINA_LOG_ERR("Couldn't create a log domain for ephysics");
	return --_ephysics_init_count;
     }

   if (!ephysics_world_init())
     {
	ERR("Couldn't initialize worlds");
	eina_log_domain_unregister(_ephysics_log_dom);
	_ephysics_log_dom = -1;
	return --_ephysics_init_count;
     }

   INF("EPhysics initialized");

   return _ephysics_init_count;
}

EAPI int
ephysics_shutdown()
{
   if (--_ephysics_init_count != 0)
     return _ephysics_init_count;

   ephysics_world_shutdown();

   INF("EPhysics shutdown");

   eina_log_domain_unregister(_ephysics_log_dom);
   _ephysics_log_dom = -1;

   return _ephysics_init_count;
}

#ifdef  __cplusplus
}
#endif
