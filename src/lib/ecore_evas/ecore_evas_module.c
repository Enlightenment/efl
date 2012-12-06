#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "Ecore_Evas.h"
#include "ecore_evas_private.h"

static Eina_Hash *_registered_engines = NULL;
static Eina_List *_engines_paths = NULL;
static Eina_List *_engines_available = NULL;

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

   EINA_LIST_FREE(_engines_available, path)
     eina_stringshare_del(path);
}

static Eina_Bool
_file_exists(const char *file)
{
   struct stat st;
   if (!file) return EINA_FALSE;

   if (stat(file, &st) < 0) return EINA_FALSE;
   return EINA_TRUE;
}

const Eina_List *
_ecore_evas_available_engines_get(void)
{
   Eina_File_Direct_Info *info;
   Eina_Iterator *it;
   Eina_List *l = NULL, *result = NULL;
   const char *path;

   if (_engines_available) return _engines_available;

   EINA_LIST_FOREACH(_engines_paths, l, path)
     {
	it = eina_file_direct_ls(path);

	if (it)
	  {
	     EINA_ITERATOR_FOREACH(it, info)
	       {
		  char tmp[PATH_MAX];
		  snprintf(tmp, sizeof (tmp), "%s/%s/" ECORE_EVAS_ENGINE_NAME,
			   info->path, MODULE_ARCH);

		  if (_file_exists(tmp))
		    result = eina_list_append(result,
					      eina_stringshare_add(info->path + info->name_start));
	       }
	     eina_iterator_free(it);
	  }
     }

   _engines_available = result;
   return result;
}
