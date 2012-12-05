#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "Ecore_Evas.h"
#include "ecore_evas_private.h"

Eina_Hash *_registered_engines = NULL;
Eina_List *_engines_paths = NULL;

#if defined(__CEGCC__) || defined(__MINGW32CE__) || defined(_WIN32)
# define ECORE_EVAS_ENGINE_NAME "module.dll"
#else
# define ECORE_EVAS_ENGINE_NAME "module.so"
#endif


Eina_Module *
_ecore_evas_engine_load(const char *engine)
{
   const char *path;
   Eina_List *l;
   Eina_Module *em = NULL;

   EINA_SAFETY_ON_NULL_RETURN_VAL(engine, NULL);

   em =  (Eina_Module *)eina_hash_find(_registered_engines, engine);
   if (em) return em;

   EINA_LIST_FOREACH(_engines_paths, l, path)
     {
        char tmp[PATH_MAX];

        snprintf(tmp, sizeof (tmp), "%s/%s/%s/"ECORE_EVAS_ENGINE_NAME, path, engine, MODULE_ARCH);
        em = eina_module_new(tmp);
        if (!em) continue;

        if (!eina_module_load(em))
          {
             eina_module_free(em);
             continue;
          }
        if (eina_hash_add(_registered_engines, engine, em))
          return em;
     }

   return NULL;
}

void
_ecore_evas_engine_init(void)
{
   char *paths[4] = { NULL, NULL, NULL, NULL };
   unsigned int i;
   unsigned int j;

   _registered_engines = eina_hash_string_small_new(EINA_FREE_CB(eina_module_free));

   /* 1. ~/.ecore_evas/modules/ */
   paths[0] = eina_module_environment_path_get("HOME", "/.ecore_evas/engines");
   /* 2. $(ECORE_ENGINE_DIR)/ecore_evas/modules/ */
   paths[1] = eina_module_environment_path_get("ECORE_EVAS_ENGINES_DIR", "/ecore_evas/engines");
   /* 3. libecore_evas.so/../ecore_evas/engines/ */
   paths[2] = eina_module_symbol_path_get(_ecore_evas_engine_init, "/ecore_evas/engines");
   /* 4. PREFIX/ecore_evas/engines/ */
#ifndef _MSC_VER
   paths[3] = strdup(PACKAGE_LIB_DIR "/ecore_evas/engines");
#endif

   for (j = 0; j < ((sizeof (paths) / sizeof (char*)) - 1); ++j)
     for (i = j + 1; i < sizeof (paths) / sizeof (char*); ++i)
       if (paths[i] && paths[j] && !strcmp(paths[i], paths[j]))
	 {
	    free(paths[i]);
	    paths[i] = NULL;
	 }

   for (i = 0; i < sizeof (paths) / sizeof (char*); ++i)
     if (paths[i])
       _engines_paths = eina_list_append(_engines_paths, paths[i]);
}

void
_ecore_evas_engine_shutdown(void)
{
   char *path;

   if (_registered_engines)
     {
       eina_hash_free(_registered_engines);
       _registered_engines = NULL;
     }

   EINA_LIST_FREE(_engines_paths, path)
     free(path);
}
