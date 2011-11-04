#include "edje_private.h"

Eina_Hash *_registered_modules = NULL;
Eina_List *_modules_paths = NULL;

Eina_List *_modules_found = NULL;

#if defined(__CEGCC__) || defined(__MINGW32CE__)
# define EDJE_MODULE_NAME "edje_%s.dll"
# define EDJE_EXTRA_MODULE_NAME 1
#elif _WIN32
# define EDJE_MODULE_NAME "module.dll"
#else
# define EDJE_MODULE_NAME "module.so"
#endif

EAPI Eina_Module *
edje_module_load(const char *module)
{
   const char *path;
   Eina_List *l;
   Eina_Module *em = NULL;

   EINA_SAFETY_ON_NULL_RETURN_VAL(module, NULL);

   em =  (Eina_Module *)eina_hash_find(_registered_modules, module);
   if (em) return em;

   EINA_LIST_FOREACH(_modules_paths, l, path)
     {
        char tmp[PATH_MAX];

        snprintf(tmp, sizeof (tmp), "%s/%s/%s/" EDJE_MODULE_NAME, path, module, MODULE_ARCH
#ifdef EDJE_EXTRA_MODULE_NAME
                 , module
#endif
                );
        em = eina_module_new(tmp);
        if (!em) continue;

        if (!eina_module_load(em))
          {
             eina_module_free(em);
             continue;
          }
        if (eina_hash_add(_registered_modules, module, em))
          return em;
     }

   return NULL;
}

void
_edje_module_init(void)
{
   char *paths[4] = { NULL, NULL, NULL, NULL };
   unsigned int i;
   unsigned int j;

   _registered_modules = eina_hash_string_small_new(EINA_FREE_CB(eina_module_free));

   /* 1. ~/.edje/modules/ */
   paths[0] = eina_module_environment_path_get("HOME", "/.edje/modules");
   /* 2. $(EDJE_MODULE_DIR)/edje/modules/ */
   paths[1] = eina_module_environment_path_get("EDJE_MODULES_DIR", "/edje/modules");
   /* 3. libedje.so/../edje/modules/ */
   paths[2] = eina_module_symbol_path_get(_edje_module_init, "/edje/modules");
   /* 4. PREFIX/edje/modules/ */
#ifndef _MSC_VER
   paths[3] = strdup(PACKAGE_LIB_DIR "/edje/modules");
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
       _modules_paths = eina_list_append(_modules_paths, paths[i]);
}

void
_edje_module_shutdown(void)
{
   char *path;

   if (_registered_modules)
     {
       eina_hash_free(_registered_modules);
       _registered_modules = NULL;
     }

   EINA_LIST_FREE(_modules_paths, path)
     free(path);

   EINA_LIST_FREE(_modules_found, path)
     eina_stringshare_del(path);
}

EAPI const Eina_List *
edje_available_modules_get(void)
{
   Eina_File_Direct_Info *info;
   Eina_Iterator *it;
   Eina_List *l;
   const char *path;
   Eina_List *result = NULL;

   /* FIXME: Stat each possible dir and check if they did change, before starting a huge round of readdir/stat */
   if (_modules_found)
     {
	EINA_LIST_FREE(_modules_found, path)
	  eina_stringshare_del(path);
     }

   EINA_LIST_FOREACH(_modules_paths, l, path)
     {
	it = eina_file_direct_ls(path);

	if (it)
	  {
	     EINA_ITERATOR_FOREACH(it, info)
	       {
		  char tmp[PATH_MAX];

		  snprintf(tmp, sizeof (tmp), "%s/%s/" EDJE_MODULE_NAME, info->path, MODULE_ARCH
#ifdef EDJE_EXTRA_MODULE_NAME
                           , ecore_file_file_get(info->path)
#endif
                           );

		  if (ecore_file_exists(tmp))
		    result = eina_list_append(result, eina_stringshare_add(ecore_file_file_get(info->path)));
	       }

	     eina_iterator_free(it);
	  }
     }

   _modules_found = result;

   return result;
}
