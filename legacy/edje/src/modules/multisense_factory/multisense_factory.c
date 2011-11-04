#include "config.h"
#include <edje_private.h>

#define DEFAULT_SAMPLERATE 44100

#ifdef HAVE_LIBREMIX
EAPI RemixBase *
multisense_sound_player_get(Edje_Multisense_Env *msenv)
{
   RemixEnv *env = msenv->remixenv;
   RemixPlugin *player_plugin;
   RemixBase *player;
   
   player_plugin = remix_find_plugin(env, "alsa_snd_player");
   if (!player_plugin)
     {
        printf("ALSA player_plugin init fail\n");
        return remix_monitor_new(env);
     }
   player =  remix_new(env, player_plugin, NULL);
   return player;
}
#endif

EAPI Eina_Bool
multisense_factory_init(Edje_Multisense_Env *env)
{
#ifdef HAVE_LIBREMIX
   remix_set_samplerate(env->remixenv, DEFAULT_SAMPLERATE);
   remix_set_channels(env->remixenv, REMIX_STEREO);
#endif
   return EINA_TRUE;
}
