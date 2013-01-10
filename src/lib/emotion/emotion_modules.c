#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "emotion_private.h"

#ifdef EMOTION_STATIC_BUILD_XINE
Eina_Bool xine_module_init(void);
void      xine_module_shutdown(void);
#endif
#ifdef EMOTION_STATIC_BUILD_GSTREAMER
Eina_Bool gstreamer_module_init(void);
void      gstreamer_module_shutdown(void);
#endif
#ifdef EMOTION_STATIC_BUILD_GENERIC
Eina_Bool generic_module_init(void);
void      generic_module_shutdown(void);
#endif

Eina_Hash *_emotion_backends = NULL;
Eina_Array *_emotion_modules = NULL;

static void
_emotion_modules_load(void)
{
   char buf[PATH_MAX];
   char *path;

   if (getenv("EFL_RUN_IN_TREE"))
     {
        struct stat st;
        snprintf(buf, sizeof(buf), "%s/src/modules/emotion",
                 PACKAGE_BUILD_DIR);
        if (stat(buf, &st) == 0)
          {
             const char *built_modules[] = {
#ifdef EMOTION_BUILD_GSTREAMER
               "gstreamer",
#endif
#ifdef EMOTION_BUILD_XINE
               "xine",
#endif
               NULL
             };
             const char **itr;
             for (itr = built_modules; *itr != NULL; itr++)
               {
                  snprintf(buf, sizeof(buf),
                           "%s/src/modules/emotion/%s/.libs",
                           PACKAGE_BUILD_DIR, *itr);
                  _emotion_modules = eina_module_list_get(_emotion_modules, buf,
                                                          EINA_FALSE, NULL, NULL);
               }

             return;
          }
     }

   path = eina_module_environment_path_get("EMOTION_MODULES_DIR",
                                           "/emotion/modules");
   if (path)
     {
        _emotion_modules = eina_module_arch_list_get(_emotion_modules, path, MODULE_ARCH);
        free(path);
     }

   path = eina_module_environment_path_get("HOME", "/.emotion");
   if (path)
     {
        _emotion_modules = eina_module_arch_list_get(_emotion_modules, path, MODULE_ARCH);
        free(path);
     }

   snprintf(buf, sizeof(buf), "%s/emotion/modules", eina_prefix_lib_get(_emotion_pfx));
   _emotion_modules = eina_module_arch_list_get(_emotion_modules, buf, MODULE_ARCH);
}

Eina_Bool
emotion_modules_init(void)
{
   int static_modules = 0;

   _emotion_modules_load();

   /* Init static module */
#ifdef EMOTION_STATIC_BUILD_XINE
   static_modules += xine_module_init();
#endif
#ifdef EMOTION_STATIC_BUILD_GSTREAMER
   static_modules += gstreamer_module_init();
#endif
#ifdef EMOTION_STATIC_BUILD_GENERIC
   static_modules += generic_module_init();
#endif

   if ((!_emotion_modules) && (!static_modules))
     WRN("No emotion modules found!");
   else if (_emotion_modules)
     eina_module_list_load(_emotion_modules);

   return EINA_TRUE;
}

void
emotion_modules_shutdown(void)
{
#ifdef EMOTION_STATIC_BUILD_XINE
   xine_module_shutdown();
#endif
#ifdef EMOTION_STATIC_BUILD_GSTREAMER
   gstreamer_module_shutdown();
#endif
#ifdef EMOTION_STATIC_BUILD_GENERIC
   generic_module_shutdown();
#endif

   if (_emotion_modules)
     {
        eina_module_list_free(_emotion_modules);
        eina_array_free(_emotion_modules);
        _emotion_modules = NULL;
     }

   eina_hash_free(_emotion_backends);
   _emotion_backends = NULL;
}
