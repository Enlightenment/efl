#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#ifdef HAVE_FEATURES_H
#include <features.h>
#endif
#include <ctype.h>
#include <errno.h>

#include "ecore_audio_private.h"

int _ecore_audio_log_dom = -1;
static int _ecore_audio_init_count = 0;
Eina_List *ecore_audio_modules;


/* externally accessible functions */

EAPI int
ecore_audio_init(void)
{

   if (++_ecore_audio_init_count != 1)
     return _ecore_audio_init_count;

   if (!ecore_init())
     return --_ecore_audio_init_count;

   if (!eo_init()) {
     ecore_shutdown();
     return --_ecore_audio_init_count;
   }

   _ecore_audio_log_dom = eina_log_domain_register("ecore_audio", ECORE_AUDIO_DEFAULT_LOG_COLOR);
   if (_ecore_audio_log_dom < 0)
     {
        EINA_LOG_ERR("Impossible to create a log domain for the ecore audio module.");
        return --_ecore_audio_init_count;
     }

   DBG("Ecore_Audio init");
   ecore_audio_modules = NULL;


   eina_log_timing(_ecore_audio_log_dom,
		   EINA_LOG_STATE_STOP,
		   EINA_LOG_STATE_INIT);

   return _ecore_audio_init_count;
}

EAPI int
ecore_audio_shutdown(void)
{
   DBG("Ecore_Audio shutdown");
   if (--_ecore_audio_init_count != 0)
     return _ecore_audio_init_count;

   /* FIXME: Shutdown all the inputs and outputs first */
   eina_log_timing(_ecore_audio_log_dom,
		   EINA_LOG_STATE_START,
		   EINA_LOG_STATE_SHUTDOWN);


   eina_list_free(ecore_audio_modules);

   eina_log_domain_unregister(_ecore_audio_log_dom);
   _ecore_audio_log_dom = -1;

   eo_shutdown();
   ecore_shutdown();

   return _ecore_audio_init_count;
}


/**
 * @}
 */
