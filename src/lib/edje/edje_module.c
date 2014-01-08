#include "edje_private.h"

Eina_Hash *_registered_modules = NULL;
Eina_List *_modules_paths = NULL;

Eina_List *_modules_found = NULL;

#if _WIN32
# define EDJE_MODULE_NAME "module.dll"
#else
# define EDJE_MODULE_NAME "module.so"
#endif

EAPI Eina_Bool
edje_module_load(const char *module)
{
   if (_edje_module_handle_load(module)) return EINA_TRUE;
   return EINA_FALSE;
}

Eina_Module *
_edje_module_handle_load(const char *module)
{
   const char *path;
   Eina_List *l;
   Eina_Module *em = NULL;
   Eina_Bool run_in_tree;

   EINA_SAFETY_ON_NULL_RETURN_VAL(module, NULL);

   em =  (Eina_Module *)eina_hash_find(_registered_modules, module);
   if (em) return em;

   run_in_tree = !!getenv("EFL_RUN_IN_TREE");

   EINA_LIST_FOREACH(_modules_paths, l, path)
     {
        char tmp[PATH_MAX] = "";

#if defined(HAVE_GETUID) && defined(HAVE_GETEUID)
        if (getuid() == geteuid())
#endif
          {
             if (run_in_tree)
               {
                  struct stat st;
                  snprintf(tmp, sizeof(tmp), "%s/%s/.libs/%s",
                           path, module, EDJE_MODULE_NAME);
                  if (stat(tmp, &st) != 0)
                  tmp[0] = '\0';
               }
          }

        if (tmp[0] == '\0')
          snprintf(tmp, sizeof(tmp), "%s/%s/%s/%s",
                   path, module, MODULE_ARCH, EDJE_MODULE_NAME);

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
   char *paths[2] = { NULL, NULL };
   unsigned int i;
   unsigned int j;

   _registered_modules = eina_hash_string_small_new(EINA_FREE_CB(eina_module_free));

#if defined(HAVE_GETUID) && defined(HAVE_GETEUID)
   if (getuid() == geteuid())
#endif
     {
        if (getenv("EFL_RUN_IN_TREE"))
          {
             struct stat st;
             const char mp[] = PACKAGE_BUILD_DIR"/src/modules/edje";
             if (stat(mp, &st) == 0)
               {
                  _modules_paths = eina_list_append(_modules_paths, strdup(mp));
                  return;
               }
          }
     }

   /* 1. libedje.so/../edje/modules/ */
   paths[0] = eina_module_symbol_path_get(_edje_module_init, "/edje/modules");
   /* 2. PREFIX/edje/modules/ */
#ifndef _MSC_VER
   paths[1] = strdup(PACKAGE_LIB_DIR "/edje/modules");
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
                           , info->path + info->name_start
#endif
                           );

		  if (ecore_file_exists(tmp))
		    result = eina_list_append(result, eina_stringshare_add(info->path + info->name_start));
	       }

	     eina_iterator_free(it);
	  }
     }

   _modules_found = result;

   return result;
}
