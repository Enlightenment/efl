#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#ifdef STDC_HEADERS
# include <stdlib.h>
# include <stddef.h>
#else
# ifdef HAVE_STDLIB_H
#  include <stdlib.h>
# endif
#endif

#include <stdio.h>

#include <Ecore.h>

#include "emotion_private.h"

static Emotion_Version _version = { VMAJ, VMIN, VMIC, VREV };
static int emotion_pending_objects = 0;
EAPI Emotion_Version *emotion_version = &_version;

Eina_Prefix *_emotion_pfx = NULL;
int _emotion_log_domain = -1;

static Eet_File *_emotion_config_file = NULL;

struct ext_match_s
{
   unsigned int length;
   const char *extension;
};

#define MATCHING(Ext)                           \
  { sizeof (Ext), Ext }

static const struct ext_match_s matchs[] =
{ /* map extensions to know if it's a emotion playable content for good first-guess tries */
   MATCHING(".264"),
   MATCHING(".3g2"),
   MATCHING(".3gp"),
   MATCHING(".3gp2"),
   MATCHING(".3gpp"),
   MATCHING(".3gpp2"),
   MATCHING(".3p2"),
   MATCHING(".asf"),
   MATCHING(".avi"),
   MATCHING(".bdm"),
   MATCHING(".bdmv"),
   MATCHING(".clpi"),
   MATCHING(".clp"),
   MATCHING(".fla"),
   MATCHING(".flv"),
   MATCHING(".m1v"),
   MATCHING(".m2v"),
   MATCHING(".m2t"),
   MATCHING(".m4v"),
   MATCHING(".mkv"),
   MATCHING(".mov"),
   MATCHING(".mp2"),
   MATCHING(".mp2ts"),
   MATCHING(".mp4"),
   MATCHING(".mpe"),
   MATCHING(".mpeg"),
   MATCHING(".mpg"),
   MATCHING(".mpl"),
   MATCHING(".mpls"),
   MATCHING(".mts"),
   MATCHING(".mxf"),
   MATCHING(".nut"),
   MATCHING(".nuv"),
   MATCHING(".ogg"),
   MATCHING(".ogm"),
   MATCHING(".ogv"),
   MATCHING(".rm"),
   MATCHING(".rmj"),
   MATCHING(".rmm"),
   MATCHING(".rms"),
   MATCHING(".rmx"),
   MATCHING(".rmvb"),
   MATCHING(".swf"),
   MATCHING(".ts"),
   MATCHING(".weba"),
   MATCHING(".webm"),
   MATCHING(".wmv")
};

Eina_Bool
_emotion_object_extension_can_play_generic_get(const void *data EINA_UNUSED, const char *file)
{
   unsigned int length;
   unsigned int i;

   length = eina_stringshare_strlen(file) + 1;
   if (length < 5) return EINA_FALSE;

   for (i = 0; i < sizeof (matchs) / sizeof (struct ext_match_s); ++i)
     {
        if (matchs[i].length > length) continue;

        if (!strcasecmp(matchs[i].extension,
                        file + length - matchs[i].length))
          return EINA_TRUE;
     }

   return EINA_FALSE;
}

EAPI Eina_Bool
emotion_object_extension_may_play_fast_get(const char *file)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(file, EINA_FALSE);
   return _emotion_object_extension_can_play_generic_get(NULL, file);
}

EAPI Eina_Bool
emotion_object_extension_may_play_get(const char *file)
{
   const char *tmp;
   Eina_Bool result;

   EINA_SAFETY_ON_NULL_RETURN_VAL(file, EINA_FALSE);
   tmp = eina_stringshare_add(file);
   result = emotion_object_extension_may_play_fast_get(tmp);
   eina_stringshare_del(tmp);

   return result;
}

static int _emotion_init_count = 0;

EAPI Eina_Bool
emotion_init(void)
{
   char buffer[PATH_MAX];

   if (_emotion_init_count > 0)
     {
        _emotion_init_count++;
        return EINA_TRUE;
     }

   eina_init();

   _emotion_log_domain = eina_log_domain_register("emotion", EINA_COLOR_LIGHTCYAN);
   if (_emotion_log_domain < 0)
     {
        EINA_LOG_CRIT("Could not register log domain 'emotion'");
        eina_shutdown();
        return EINA_FALSE;
     }

   _emotion_pfx = eina_prefix_new(NULL, emotion_init,
                                  "EMOTION", "emotion", "checkme",
                                  PACKAGE_BIN_DIR, PACKAGE_LIB_DIR,
                                  PACKAGE_DATA_DIR, PACKAGE_DATA_DIR);
   EINA_SAFETY_ON_NULL_GOTO(_emotion_pfx, error);

   ecore_init();
   eet_init();

   snprintf(buffer, sizeof(buffer), "%s/emotion.cfg",
            eina_prefix_data_get(_emotion_pfx));
   _emotion_config_file = eet_open(buffer, EET_FILE_MODE_READ);

   if (!emotion_webcam_init()) goto error_webcam;
   emotion_webcam_config_load(_emotion_config_file);

   if (!emotion_modules_init()) goto error_modules;

   _emotion_init_count = 1;
   return EINA_TRUE;

 error_modules:
   emotion_webcam_shutdown();

 error_webcam:
   eina_prefix_free(_emotion_pfx);
   _emotion_pfx = NULL;

 error:
   eina_log_domain_unregister(_emotion_log_domain);
   _emotion_log_domain = -1;

   eina_shutdown();
   return EINA_FALSE;
}

static int emotion_pendig_events = 0;

EAPI void
_emotion_pending_ecore_begin(void)
{
   emotion_pendig_events++;
}

EAPI void
_emotion_pending_ecore_end(void)
{
   emotion_pendig_events--;
}

EAPI Eina_Bool
emotion_shutdown(void)
{
   double start;

   if (_emotion_init_count <= 0)
     {
        ERR("Init count not greater than 0 in emotion shutdown.");
        return EINA_FALSE;
     }
   if (--_emotion_init_count) return EINA_TRUE;

   start = ecore_time_get();
   while (((emotion_pending_objects > 0) ||
           (emotion_pendig_events > 0)) &&
          ((ecore_time_get() - start) < 0.5))
     ecore_main_loop_iterate();

   if (emotion_pending_objects > 0)
     {
        ERR("There is still %i Emotion pipeline running", emotion_pending_objects);
     }
   if (emotion_pendig_events > 0)
     {
        ERR("There is still %i Emotion events queued", emotion_pendig_events);
     }

   emotion_modules_shutdown();

   emotion_webcam_shutdown();

   if (_emotion_config_file)
     {
        /* As long as there is no one reference any pointer, you are safe */
        eet_close(_emotion_config_file);
        _emotion_config_file = NULL;
     }

   eet_shutdown();
   ecore_shutdown();

   eina_prefix_free(_emotion_pfx);
   _emotion_pfx = NULL;

   eina_log_domain_unregister(_emotion_log_domain);
   _emotion_log_domain = -1;

   eina_shutdown();

   return EINA_TRUE;
}

EAPI void
_emotion_pending_object_ref(void)
{
   emotion_pending_objects++;
}

EAPI void
_emotion_pending_object_unref(void)
{
   emotion_pending_objects--;
}
