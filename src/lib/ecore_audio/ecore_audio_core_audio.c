#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef HAVE_COREAUDIO
#include "Ecore.h"
#include "ecore_private.h"
#include "Ecore_Audio.h"
#include "ecore_audio_private.h"

static Ecore_Audio_Module *_module = NULL;

EAPI Ecore_Audio_Module *
ecore_audio_core_audio_init(void)
{
   /* Don't call this twice */
   if (_module != NULL) return _module;

   _module = calloc(1, sizeof(Ecore_Audio_Module));
   if (EINA_UNLIKELY(_module == NULL))
     {
        CRI("Failed to allocate Ecore_Audio_Module");
        goto ret_null;
     }

   ECORE_MAGIC_SET(_module, ECORE_MAGIC_AUDIO_MODULE);
   _module->type = ECORE_AUDIO_TYPE_CORE_AUDIO;
   _module->name = "CoreAudio";

   return _module;

ret_null:
   return NULL;
}

EAPI void
ecore_audio_nssound_shutdown(void)
{
   free(_module);
   _module = NULL;
}

#endif /* HAVE_COREAUDIO */

