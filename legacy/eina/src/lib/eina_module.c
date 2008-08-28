/* EINA - EFL data type library
 * Copyright (C) 2007-2008 Jorge Luis Zapata Muga
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library;
 * if not, see <http://www.gnu.org/licenses/>.
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <stdio.h>
#include <sys/types.h>
#include <dirent.h>
#include <string.h>
#include <dlfcn.h>

#include "eina_module.h"
#include "eina_file.h"
#include "eina_private.h"
#include "eina_inlist.h"

/*============================================================================*
 *                                  Local                                     *
 *============================================================================*/

/**
 * @cond LOCAL
 */

#ifdef _WIN32
# define MODULE_EXTENSION ".dll"
#else
# define MODULE_EXTENSION ".so"
#endif /* ! _WIN32 */
#define MODULE_SUBDIR "/modules/"
#define MODULE_BASE_EXTENTION "module"

typedef struct _Eina_Root_Directory Eina_Root_Directory;
typedef struct _Eina_Directory Eina_Directory;
typedef struct _Eina_Dir_List Eina_Dir_List;
typedef struct _Eina_Static Eina_Static;
typedef struct _Eina_App Eina_App;

struct _Eina_Module
{
   Eina_Inlist          __list;

   const char         * path;
   const char         * name;
   const Eina_App     * app;
   Eina_Module_Group  * group;

   void               * handle;
   Eina_Module_Export * export;

   int                  references;
   int                  load_references;

   Eina_Bool            is_static_library;
};

struct _Eina_Directory
{
   Eina_Inlist  __list;

   const char * path;
   const char * extention;

   Eina_App   * app;
};

struct _Eina_App
{
   const char * name;
   const char * type;
   const char * version;
};

struct _Eina_Static
{
   Eina_Inlist        __list;
   Eina_Module_Export static_desc;
};

struct _Eina_Module_Group
{
   Eina_Inlist * lookup_directory;
   Eina_Inlist * root_directory;
   Eina_Inlist * recursive_directory;
   Eina_Inlist * static_libraries;

   Eina_Inlist * loaded_module;
};

struct _Eina_Root_Directory
{
   Eina_Inlist  __list;

   const char * path;
};

struct _Eina_Dir_List
{
   Eina_Module_Group *modules;
   Eina_List *list;
   void *data;

   Eina_Module_Cb cb;
};

static Eina_Inlist * root_directory;
static char *self_name = NULL;
static int _eina_module_count = 0;

static Eina_Module *
_eina_module_build(Eina_Module_Group *modules, Eina_App *app,
		   const char *path, int length_path,
		   const char *name)
{
   Eina_Module *module;
   void *handle;

   handle = dlopen(path, RTLD_LAZY | RTLD_GLOBAL);
   fprintf(stderr, "b [%s] %p\n", path, handle);
   if (!handle) return NULL;

   module = calloc(1, sizeof (Eina_Module) + length_path + strlen(name) + 1);
   if (!module) return NULL;

   module->path = (char*)(module + 1);
   module->name = module->path + length_path;
   module->group = modules;
   module->app = app;
   module->handle = handle;
   module->references = 1;
   module->is_static_library = EINA_FALSE;

   memcpy((char*) module->path, path, length_path);
   strcpy((char*) module->name, name);

   if (eina_module_load(module) == EINA_TRUE)
     return module;

   free(module);
   return NULL;
}

static Eina_Module *
_eina_module_try(Eina_Module_Group *modules,
		 const char *path, int length_path,
		 const char *filename, int length_filename)
{
   Eina_Module *module;

   module = calloc(1, sizeof (Eina_Module) + length_path + length_filename + 2);
   if (!module) return NULL;

   module->path = (char*)(module + 1);
   module->name = NULL;
   module->group = modules;
   module->app = NULL;
   module->references = 1;
   module->is_static_library = EINA_FALSE;

   strcpy((char*) module->path, path);
   strcat((char*) module->path, "/");
   strcat((char*) module->path, filename);

   module->handle = dlopen(module->path, RTLD_LAZY | RTLD_GLOBAL);
   if (!module->handle) goto on_error;

   if (eina_module_load(module) == EINA_TRUE)
     {
	module->name = module->export->name;
	return module;
     }

 on_error:
   free(module);
   return NULL;
}

static void
_eina_dir_module_cb(const char *name, const char *path, Eina_Dir_List *data)
{
   Eina_Module *module;
   int length_name = strlen(name);
   int length_path = strlen(path);

   if (strcmp(name + length_name - strlen(MODULE_EXTENSION), MODULE_EXTENSION))
     return ;

   module = _eina_module_try(data->modules,
			     path, length_path,
			     name, length_name);
   if (!module) return ;

   if (data->cb(module, data->data) == EINA_TRUE)
     {
	data->modules->loaded_module = eina_inlist_prepend(data->modules->loaded_module, module);
	data->list = eina_list_append(data->list, module);

	return ;
     }

   eina_module_unload(module);
   free(module);
}

/**
 * @endcond
 */

/*============================================================================*
 *                                 Global                                     *
 *============================================================================*/

/*============================================================================*
 *                                   API                                      *
 *============================================================================*/
EAPI int
eina_module_init(void)
{
   char *prefix;
   char *path;

   _eina_module_count++;

   if (_eina_module_count != 1)
     goto end_init;

   /* Add some default path. */

   /* 1. ~/lib/ */
   prefix = getenv("HOME");
   if (prefix)
     {
	#define SUBDIR_LIB "/lib"

	path = malloc(strlen(prefix) + strlen(SUBDIR_LIB) + 1);
	if (path)
	  {
	     strcpy(path, prefix);
	     strcat(path, SUBDIR_LIB);

	     eina_module_root_add(path);

	     free(path);
	  }
     }

   /* 2. $(EINA_MODULE_DIR)/ */
   prefix = getenv("EINA_MODULES_DIR");
   if (prefix) eina_module_root_add(prefix);

#ifdef HAVE_DLADDR
   {
      Dl_info eina_dl;
      /* 3. libeina.so/../ */
      if (dladdr(eina_module_init, &eina_dl))
	{
	   int length;

	   self_name = strdup(eina_dl.dli_fname);

	   prefix = strrchr(eina_dl.dli_fname, '/');

	   if (prefix)
	     {
		length = strlen(prefix);
		path = malloc(strlen(eina_dl.dli_fname) - length + 1);
		if (path)
		  {
		     strncpy(path, eina_dl.dli_fname, strlen(eina_dl.dli_fname) - length);
		     path[strlen(eina_dl.dli_fname) - length] = 0;

		     eina_module_root_add(path);

		     free(path);
		  }
	     }
	}
   }
#else
   self_name = malloc(strlen(PACKAGE_LIB_DIR) + strlen("/libeina") + strlen(MODULE_EXTENSION) + 1);
   if (self_name)
     {
	strcpy(self_name, PACKAGE_LIB_DIR);
	strcat(self_name, "/libeina");
	strcat(self_name, MODULE_EXTENSION);
     }
#endif

   /* 4. PREFIX/ */
   prefix = PACKAGE_LIB_DIR;
   if (prefix) eina_module_root_add(prefix);

 end_init:
   return _eina_module_count;
}

EAPI int
eina_module_shutdown(void)
{
   _eina_module_count--;

   if (_eina_module_count != 0)
     goto end_shutdown;

   free(self_name);
   self_name = NULL;

   while (root_directory)
     {
	Eina_Root_Directory *root;

	root = (Eina_Root_Directory*) root_directory;

	root_directory = eina_inlist_remove(root_directory, root_directory);

	free(root);
     }

 end_shutdown:
   return _eina_module_count;
}

EAPI void
eina_module_root_add(const char *root_path)
{
   Eina_Root_Directory *root;
   unsigned int length;

   if (!root_path) return ;

   /* Lookup if the path is already registered. */
   EINA_INLIST_ITER_NEXT(root_directory, root)
     if (!strcmp(root->path, root_path))
       return ;

   /* Add it. */
   length = strlen(root_path) + 1;

   root = calloc(1, sizeof (Eina_Root_Directory) + length);
   if (!root) return ;

   root->path = (char*)(root + 1);
   memcpy((char*) root->path, root_path, length);

   root_directory = eina_inlist_prepend(root_directory, root);
}

EAPI Eina_Module_Group *
eina_module_group_new(void)
{
   Eina_Module_Group *new;

   new = calloc(1, sizeof (Eina_Module_Group));

   return new;
}

EAPI void
eina_module_group_delete(Eina_Module_Group *group)
{
   if (!group) return ;

   while (group->lookup_directory)
     {
	Eina_Directory *dir = (Eina_Directory*) group->lookup_directory;

	group->lookup_directory = eina_inlist_remove(group->lookup_directory, group->lookup_directory);

	free(dir);
     }

   while (group->static_libraries)
     {
	Eina_Static *slib = (Eina_Static*) group->static_libraries;

	group->static_libraries = eina_inlist_remove(group->static_libraries, group->static_libraries);

	free(slib);
     }

   while (group->loaded_module)
     eina_module_delete((Eina_Module*) group->loaded_module);

   free(group);
}

EAPI void
eina_module_path_register(Eina_Module_Group *modules, const char *path, Eina_Bool recursive)
{
   Eina_Root_Directory *dir;
   int length;

   if (!modules || !path) return ;

   length = strlen(path) + 1;

   dir = calloc(1, sizeof (Eina_Root_Directory) + length);
   if (!dir) return ;

   dir->path = (const char*) (dir + 1);
   memcpy((char*) dir->path, path, length);

   if (recursive)
     modules->recursive_directory = eina_inlist_prepend(modules->recursive_directory, dir);
   else
     modules->root_directory = eina_inlist_prepend(modules->root_directory, dir);
}

EAPI void
eina_module_app_register(Eina_Module_Group *modules, const char *app, const char *type, const char *version)
{
   Eina_Directory *dir;
   int app_length;
   int type_length;
   int version_length;
   int subdir_length;
   int extention_length;

   if (!modules || !app) return ;

   app_length = strlen(app);
   subdir_length = strlen(MODULE_SUBDIR);
   extention_length = strlen(MODULE_ARCH) + strlen(MODULE_BASE_EXTENTION) + strlen(MODULE_EXTENSION) + 3;

   type_length = type ? strlen(type) + 1 : 0;
   version_length = version ? strlen(version) + 1 : 0;

   dir = calloc(1, sizeof (Eina_Directory)
		+ app_length + subdir_length + type_length + 1 /* path length */
		+ extention_length + version_length + 1 /* extention length */);
   if (!dir) return ;

   /* Build app description. */
   dir->app = malloc(sizeof (Eina_App) + app_length + type_length + version_length + 1);
   if (!dir->app)
     {
	free(dir);
	return ;
     }
   dir->app->name = (char*) (dir->app + 1);
   dir->app->type = dir->app->name + app_length + 1;
   dir->app->version = dir->app->type + type_length;
   memcpy((char*) dir->app->name, app, app_length + 1);
   memcpy((char*) dir->app->type, type, type_length);
   memcpy((char*) dir->app->version, version, version_length);

   /* Setup pointer. */
   dir->path = (const char*) (dir + 1);
   dir->extention = dir->path + app_length + subdir_length + type_length + 1;

   /* Build path. "[app]/modules/{[type]/}" */
   strcpy((char*) dir->path, app);
   strcat((char*) dir->path, MODULE_SUBDIR);
   if (type)
     {
	strcat((char*) dir->path, type);
	strcat((char*) dir->path, "/");
     }

   /* Build extention "/[MODULE_ARCH]/module{-[version]}.so" */
   strcpy((char*) dir->extention, "/");
   strcat((char*) dir->extention, MODULE_ARCH);
   strcat((char*) dir->extention, "/");
   strcat((char*) dir->extention, MODULE_BASE_EXTENTION);
   if (version)
     {
	strcat((char*) dir->extention, "-");
	strcat((char*) dir->extention, version);
     }
   strcat((char*) dir->extention, MODULE_EXTENSION);

   modules->lookup_directory = eina_inlist_prepend(modules->lookup_directory, dir);
}

EAPI void
eina_module_register(Eina_Module_Group *modules, const Eina_Module_Export *static_module)
{
   Eina_Static *library;

   if (!modules || !static_module) return ;

   library = malloc(sizeof (Eina_Static));
   if (!library) return ;

   library->static_desc = *static_module;

   modules->static_libraries = eina_inlist_prepend(modules->static_libraries, library);
}

EAPI Eina_Module *
eina_module_new(Eina_Module_Group *modules, const char *name)
{
   Eina_Root_Directory *root;
   Eina_Directory *dir;
   Eina_Module *module;
   Eina_Static *slib;
   char path[PATH_MAX];
   int length;

   if (!modules) return NULL;

   /* Lookup in loaded module. */
   EINA_INLIST_ITER_NEXT(modules->loaded_module, module)
     if (!strcmp(module->name, name))
       {
	  module->references++;

	  return module;
       }

   /* Lookup in statically provided module. */
   EINA_INLIST_ITER_NEXT(modules->static_libraries, slib)
     if (!strcmp(slib->static_desc.name, name))
       {
	  module = calloc(1, sizeof (Eina_Module));
	  if (!module) return NULL;

	  /* FIXME: Must retrieve the library path that provide slib->static_desc */
	  module->path = self_name;
	  module->name = slib->static_desc.name;
	  module->export = &slib->static_desc;
	  module->group = modules;
	  module->references = 1;
	  module->is_static_library = EINA_TRUE;

	  modules->loaded_module = eina_inlist_prepend(modules->loaded_module, module);

	  return module;
       }

   /* Lookup for apps in each root_directory. */
   EINA_INLIST_ITER_NEXT(root_directory, root)
     EINA_INLIST_ITER_NEXT(modules->lookup_directory, dir)
       {
	  length = snprintf(path, PATH_MAX, "%s/%s/%s/%s",
			    root->path, dir->path, name, dir->extention) + 1;
	  if (length > PATH_MAX) return NULL;

	  module = _eina_module_build(modules, dir->app,
				      path, length,
				      name);
	  if (!module) continue ;

	  modules->loaded_module = eina_inlist_prepend(modules->loaded_module, module);
	  return module;
       }

   /* Lookup for apps with full path from root_dir. */
   EINA_INLIST_ITER_NEXT(modules->root_directory, root)
     {
	length = snprintf(path, PATH_MAX, "%s"MODULE_SUBDIR"%s/%s",
			  root->path, name, MODULE_BASE_EXTENTION""MODULE_EXTENSION) + 1;
	if (length > PATH_MAX) return NULL;

	module = _eina_module_build(modules, NULL,
				    path, length,
				    name);
	if (!module) continue ;
	if (strcmp(module->export->name, name) != 0)
	  {
	     eina_module_unload(module);
	     free(module);
	     continue ;
	  }

	modules->loaded_module = eina_inlist_prepend(modules->loaded_module, module);
	return module;
     }

   return NULL;
}

EAPI void
eina_module_delete(Eina_Module *module)
{
   if (!module) return ;

   module->references--;

   if (module->references != 0) return ;

   module->group->loaded_module = eina_inlist_remove(module->group->loaded_module, module);

   if (module->handle) dlclose(module->handle);
   free(module);
}

EAPI Eina_List *
eina_module_list_new(Eina_Module_Group *modules, Eina_Module_Cb cb, void *data)
{
   Eina_List *list = NULL;
   Eina_Root_Directory *root;
   Eina_Directory *dir;
   Eina_Module *module;
   Eina_Static *slib;
   struct dirent *rent;
   DIR *look;
   Eina_Dir_List edl;
   char path[PATH_MAX];
   char filename[PATH_MAX];
   int path_length;

   /* Test against already loaded module. */
   EINA_INLIST_ITER_NEXT(modules->loaded_module, module)
     if (cb(module, data) == EINA_TRUE)
       {
	  module->references++;

	  list = eina_list_append(list, module);
       }

   /* Test against static module. */
   EINA_INLIST_ITER_NEXT(modules->static_libraries, slib)
     {
	module = calloc(1, sizeof (Eina_Module));
	if (!module) return NULL;

	module->path = self_name;
	module->name = slib->static_desc.name;
	module->export = &slib->static_desc;
	module->group = modules;
	module->references = 1;
	module->is_static_library = EINA_TRUE;

	if (cb(module, data) == EINA_TRUE)
	  {
	     modules->loaded_module = eina_inlist_prepend(modules->loaded_module, module);
	     list = eina_list_append(list, module);
	  }
	else
	  free(module);
     }

   /* Lookup for apps in each root_directory. */
   EINA_INLIST_ITER_NEXT(root_directory, root)
     EINA_INLIST_ITER_NEXT(modules->lookup_directory, dir)
       {
	  strcpy(path, root->path);
	  strcat(path, "/");
	  strcat(path, dir->path);

	  look = opendir(path);
	  if (!look) continue ;

	  path_length = strlen(path);

	  while ((rent = readdir(look)))
	    {
	       int length;

	       length = snprintf(filename, PATH_MAX, "%s/%s/%s",
				 path, rent->d_name, dir->extention) + 1;
	       if (length > PATH_MAX) continue ;

	       module = _eina_module_build(modules, dir->app,
					   filename, length,
					   rent->d_name);
	       if (!module) continue ;

	       if (cb(module, data) == EINA_TRUE)
		 {
		    modules->loaded_module = eina_inlist_prepend(modules->loaded_module, module);
		    list = eina_list_append(list, module);
		 }
	       else
		 {
		    eina_module_unload(module);
		    free(module);
		 }
	    }

	  closedir(look);
       }

   edl.list = list;
   edl.modules = modules;
   edl.data = data;
   edl.cb = cb;

   /* Lookup for apps with full path from root_dir. */
   EINA_INLIST_ITER_NEXT(modules->root_directory, root)
     eina_file_dir_list(root->path, EINA_FALSE, EINA_FILE_DIR_LIST_CB(_eina_dir_module_cb), &edl);

   /* Lookup for library in a directory or recursively. */
   EINA_INLIST_ITER_NEXT(modules->recursive_directory, root)
     eina_file_dir_list(root->path, EINA_TRUE, EINA_FILE_DIR_LIST_CB(_eina_dir_module_cb), &edl);

   return edl.list;
}

EAPI void
eina_module_list_delete(Eina_List *modules)
{
   Eina_Module *module;

   while (modules)
     {
	module = eina_list_data(modules);
	eina_module_delete(module);

	modules = eina_list_remove_list(modules, modules);
     }
}

EAPI Eina_Bool
eina_module_load(Eina_Module *module)
{
   if (!module) return EINA_FALSE;

   module->load_references++;

   if (module->load_references != 1) return EINA_FALSE;

   if (module->is_static_library == EINA_FALSE)
     {
	if (!module->handle)
	  module->handle = dlopen(module->path, RTLD_LAZY | RTLD_GLOBAL);
	if (!module->handle)
	  return EINA_FALSE;

	module->export = dlsym(module->handle, "Eina_Export");
     }

   if (!module->export) goto on_error;

   if (!module->export->name)
     goto on_error;

   if (module->app)
     {
	if (strcmp(module->app->name, module->export->name))
	  goto on_error;

	if (module->app->type && module->export->type)
	  if (strcmp(module->app->type, module->export->type))
	    goto on_error;

	if (module->export->version && module->app->version)
	  if (strcmp(module->app->version, module->export->version))
	    goto on_error;
     }

   return EINA_TRUE;

 on_error:
   if (module->handle)
     dlclose(module->handle);
   module->handle = NULL;

   if (module->is_static_library == EINA_FALSE)
     module->export = NULL;

   module->load_references--;

   return EINA_FALSE;
}

EAPI void
eina_module_unload(Eina_Module *module)
{
   if (!module) return ;

   module->load_references--;

   if (module->load_references != 0) return ;

   if (module->handle)
     dlclose(module->handle);
   module->handle = NULL;

   if (module->is_static_library == EINA_FALSE)
     module->export = NULL;
}

EAPI void
eina_module_list_load(const Eina_List *list)
{
   while (list)
     {
	eina_module_load(eina_list_data(list));

	list = eina_list_next(list);
     }
}

EAPI void
eina_module_list_unload(const Eina_List *list)
{
   while (list)
     {
	eina_module_unload(eina_list_data(list));

	list = eina_list_next(list);
     }
}

EAPI const char *
eina_module_path_get(Eina_Module *module)
{
   if (!module) return NULL;
   return module->path;
}

EAPI void *
eina_module_symbol_get(Eina_Module *module, const char *symbol)
{
   if (!module) return NULL;
   if (module->load_references <= 0) return NULL;
   /* dlopen of NULL give a handler to the main program. */
   if (!module->handle) module->handle = dlopen(NULL, RTLD_LAZY | RTLD_NODELETE | RTLD_GLOBAL);
   if (!module->handle) return NULL;

   return dlsym(module->handle, symbol);
}

EAPI void *
eina_module_export_object_get(Eina_Module *module)
{
   if (!module) return NULL;
   if (module->load_references <= 0) return NULL;

   return (void*) module->export->object;
}

EAPI const char *
eina_module_export_type_get(Eina_Module *module)
{
   if (!module) return NULL;
   if (module->load_references <= 0) return NULL;

   return module->export->type;
}

EAPI const char *
eina_module_export_version_get(Eina_Module *module)
{
   if (!module) return NULL;
   if (module->load_references <= 0) return NULL;

   return module->export->version;
}

EAPI const char *
eina_module_export_name_get(Eina_Module *module)
{
   if (!module) return NULL;
   if (module->load_references <= 0) return NULL;

   return module->export->name;
}
