/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdlib.h>
#include <stdio.h>

#ifdef HAVE_DLFCN_H
# include <dlfcn.h>
#endif

#ifdef HAVE_EVIL_H
# include <evil.h>
#endif

#include "Ecore_Data.h"
#include "Ecore_Str.h"
#include "ecore_private.h"


static Ecore_List *loaded_plugins = NULL;

static Eina_Bool _hash_keys(const Eina_Hash	*hash,
			    const char		*key,
                            void		*list);

/**
 * @defgroup Ecore_Plugin Plugin Functions
 *
 * Functions that load modules of compiled code into memory.
 */

/**
 * Loads the specified plugin from the specified path group.
 * @param   group       The path group to search for the plugin to load
 * @param   plugin_name The name of the plugin to load.
 * @param   version     The interface version of the plugin. With version
 *                      equal to NULL the default will be loaded.
 * @return  A pointer to the newly loaded plugin on success, @c NULL on
 *          failure.
 * @ingroup Ecore_Plugin
 */
EAPI Ecore_Plugin *
ecore_plugin_load(Ecore_Path_Group *group, const char *plugin_name, const char *version)
{
   char *path;
   char temp[PATH_MAX];

   Ecore_Plugin *plugin;
   void *handle = NULL;

   CHECK_PARAM_POINTER_RETURN("plugin_name", plugin_name, NULL);

   if (!version || *version == '\0')
     snprintf(temp, sizeof(temp), "%s" SHARED_LIB_SUFFIX, plugin_name);
   else
     snprintf(temp, sizeof(temp), "%s" SHARED_LIB_SUFFIX ".%s", plugin_name, version);

   path = ecore_path_group_find(group, temp);

   if (!path && version)
     {
	/* if this file doesn't exist try a different order */
	snprintf(temp, sizeof(temp), "%s.%s" SHARED_LIB_SUFFIX, plugin_name, version);
	path = ecore_path_group_find(group, temp);
     }

   if (!path)
     return NULL;

   handle = dlopen(path, RTLD_LAZY);
   if (!handle)
     {
	FREE(path);
	return NULL;
     }

   /*
    * Allocate the new plugin and initialize it's fields
    */
   plugin = malloc(sizeof(Ecore_Plugin));
   if (!plugin)
     {
       dlclose(handle);
       FREE(path);
       return NULL;
     }
   memset(plugin, 0, sizeof(Ecore_Plugin));

   plugin->handle = handle;

   /*
    * Now add it to the list of the groups loaded plugins
    */
   if (!loaded_plugins)
     loaded_plugins = ecore_list_new();

   ecore_list_append(loaded_plugins, plugin);

   FREE(path);

   return plugin;
}

/**
 * Unloads the given plugin from memory.
 * @param   plugin The given plugin.
 * @ingroup Ecore_Plugin
 */
EAPI void
ecore_plugin_unload(Ecore_Plugin *plugin)
{
   CHECK_PARAM_POINTER("plugin", plugin);

   if (!plugin->handle)
     return;

   if (ecore_list_goto(loaded_plugins, plugin))
     ecore_list_remove(loaded_plugins);

   if (ecore_list_empty_is(loaded_plugins))
     {
	ecore_list_destroy(loaded_plugins);
	loaded_plugins = NULL;
     }

   dlclose(plugin->handle);

   FREE(plugin);
}

/*
 * Searches for the specified symbol in the given plugin.
 * @param   plugin      The given plugin.
 * @param   symbol_name The symbol to search for.
 * @return  Address of the given symbol if successful.  Otherwise, @c NULL.
 * @ingroup Ecore_Plugin
 */
EAPI void *
ecore_plugin_symbol_get(Ecore_Plugin *plugin, const char *symbol_name)
{
   void *ret;

   CHECK_PARAM_POINTER_RETURN("plugin", plugin, NULL);
   CHECK_PARAM_POINTER_RETURN("symbol_name", symbol_name, NULL);

   if (!plugin->handle)
     return NULL;

   ret = dlsym(plugin->handle, symbol_name);

   return ret;
}

/**
 * Retrieves a list of all available plugins in the given path.
 * @param   group_id The identifier for the given path.
 * @return  A pointer to a newly allocated list of all plugins found in the
 *          paths identified by @p group_id.  @c NULL otherwise.
 * @ingroup Ecore_Plugin
 */
EAPI Ecore_List *
ecore_plugin_available_get(Ecore_Path_Group *group)
{
   Ecore_List *avail = NULL;
   Eina_Hash *plugins = NULL;
   Eina_Iterator *it = NULL;
   char *path;

   CHECK_PARAM_POINTER_RETURN("group", group, NULL);

   if (!group->paths || ecore_list_empty_is(group->paths))
     return NULL;

   ecore_list_first_goto(group->paths);
   plugins = eina_hash_string_superfast_new(free);

   while ((path = ecore_list_next(group->paths)) != NULL)
     {
	DIR *dir;
	struct stat st;
	struct dirent *d;

	if (stat(path, &st) < 0)
	  continue;

	if (!S_ISDIR(st.st_mode))
	  continue;

	dir = opendir(path);

	if (!dir)
	  continue;

	while ((d = readdir(dir)) != NULL)
	  {
	     char ppath[PATH_MAX];
	     char *ext;

	     if (*d->d_name == '.')
	       continue;

	     if (!ecore_str_has_suffix(d->d_name, SHARED_LIB_SUFFIX))
	       continue;

	     snprintf(ppath, PATH_MAX, "%s/%s", path, d->d_name);

	     stat(ppath, &st);

	     if (!S_ISREG(st.st_mode))
	       continue;

	     ecore_strlcpy(ppath, d->d_name, sizeof(ppath));
	     ext = strrchr(ppath, '.');
	     *ext = '\0';

	     if (!eina_hash_find(plugins, ppath))
	       {
		  char *key;

		  key = strdup(ppath);
		  eina_hash_add(plugins, key, key);
	       }
	  }
	closedir(dir);
     }

   avail = ecore_list_new();
   ecore_list_free_cb_set(avail, free);


   it = eina_hash_iterator_key_new(plugins);
   if (it)
     {
	eina_iterator_foreach(it, EINA_EACH(_hash_keys), avail);
	eina_iterator_free(it);
     }

   eina_hash_free(plugins);


   return avail;
}

static Eina_Bool
_hash_keys(const Eina_Hash *hash, const char *key, void *list)
{
   ecore_list_append(list, key);
   return EINA_TRUE;
}
