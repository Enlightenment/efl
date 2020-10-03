#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <Eina.h>

#include "Embryo.h"
#include "embryo_private.h"

static Embryo_Version _version = { VMAJ, VMIN, VMIC, VREV };
EAPI Embryo_Version * embryo_version = &_version;

static int _embryo_init_count = 0;
int _embryo_default_log_dom = -1;

/*** EXPORTED CALLS ***/

EAPI int
embryo_init(void)
{
   if (++_embryo_init_count != 1)
     return _embryo_init_count;

   if (!eina_init())
     return --_embryo_init_count;

   _embryo_default_log_dom = eina_log_domain_register
       ("embryo", EMBRYO_DEFAULT_LOG_COLOR);
   if (_embryo_default_log_dom < 0)
     {
        EINA_LOG_ERR("Embryo Can not create a general log domain.");
        goto shutdown_eina;
     }

   eina_log_timing(_embryo_default_log_dom,
                   EINA_LOG_STATE_STOP,
                   EINA_LOG_STATE_INIT);

   return _embryo_init_count;

shutdown_eina:
   eina_shutdown();
   return --_embryo_init_count;
}

EAPI int
embryo_shutdown(void)
{
   if (_embryo_init_count <= 0)
     {
        printf("%s:%i Init count not greater than 0 in shutdown.", __func__, __LINE__);
        return 0;
     }
   if (--_embryo_init_count != 0)
     return _embryo_init_count;

   eina_log_timing(_embryo_default_log_dom,
                   EINA_LOG_STATE_START,
                   EINA_LOG_STATE_SHUTDOWN);

   eina_log_domain_unregister(_embryo_default_log_dom);
   _embryo_default_log_dom = -1;
   eina_shutdown();

   return _embryo_init_count;
}

