/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */

#ifndef _WIN32
# include <dlfcn.h>
#else
# define WIN32_LEAN_AND_MEAN
# include <windows.h>
# undef WIN32_LEAN_AND_MEAN
# include <stdlib.h>
# include <stdio.h>
#endif /* _WIN32 */

#include "ecore_private.h"
#include "Ecore_Data.h"

/* FIXME: that hack is a temporary one. That code will be in MinGW soon */
#ifdef _WIN32

# define RTLD_LAZY 1   /* lazy function call binding */
# define RTLD_NOW 2    /* immediate function call binding */
# define RTLD_GLOBAL 4 /* symbols in this dlopen'ed obj are visible
                          to other dlopen'ed objs */

static char *dlerr_ptr;
static char  dlerr_data[80];

void *dlopen (const char *file, int mode)
{
   HMODULE hmodule;

   hmodule = LoadLibrary(file);
   if (hmodule == NULL)
     {
        int error;

        error = GetLastError();
        sprintf(dlerr_data, "LoadLibraryEx returned %d.", error);
        dlerr_ptr = dlerr_data;
     }
   return hmodule;
}

int dlclose (void *handle)
{
   if (FreeLibrary(handle))
     {
        return 0;
     }
   else
     {
        int error;

        error = GetLastError();
        sprintf(dlerr_data, "FreeLibrary returned %d.", error);
        dlerr_ptr = dlerr_data;
        return -1;
     }
}

void *dlsym (void *handle, const char *name)
{
   FARPROC fp;

   fp = GetProcAddress(handle, name);
   if (fp == NULL)
     {
        int error;

        error = GetLastError();
        sprintf(dlerr_data, "GetProcAddress returned %d.", error);
        dlerr_ptr = dlerr_data;
     }
   return fp;
}

char *dlerror (void)
{
   if (dlerr_ptr != NULL)
     {
        dlerr_ptr = NULL;
        return dlerr_data;
     }
   else
     return NULL;
}

#endif /* _WIN32 */


static Ecore_List *loaded_plugins = NULL;

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

#ifndef _WIN32
   if (!version || *version == '\0')
     snprintf(temp, sizeof(temp), "%s.so", plugin_name);
   else
     snprintf(temp, sizeof(temp), "%s.so.%s", plugin_name, version);
#else
   if (!version || *version == '\0')
     snprintf(temp, sizeof(temp), "%s.dll", plugin_name);
   else
     snprintf(temp, sizeof(temp), "%s-%s.dll", plugin_name, version);
#endif /* _WIN32 */

   path = ecore_path_group_find(group, temp);

#ifndef _WIN32
   if (!path && version)
     {
	/* if this file doesn't exist try a different order */
	snprintf(temp, sizeof(temp), "%s.%s.so", plugin_name, version);
	path = ecore_path_group_find(group, temp);
     }
#endif /* _WIN32 */

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
