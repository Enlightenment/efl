#include "eina_module.h"
#include "eina_file.h"
#include "eina_private.h"

#include <dirent.h>
#include <string.h>
#include <dlfcn.h>
/*============================================================================*
 *                                  Local                                     * 
 *============================================================================*/
struct _Eina_Module
{
#ifdef DEBUG
	unsigned int magic;
#endif
	void *handle;
	char *file;
};

#define MODULE_EXTENSION ".so"


typedef struct _Dir_List_Get_Cb_Data
{
	Eina_Module_Cb cb;
	void *data;
	Eina_List *list;
} Dir_List_Get_Cb_Data;

typedef struct _Dir_List_Cb_Data
{
	Eina_Module_Cb cb;
	void *data;
} Dir_List_Cb_Data;

static int _dir_list_get_cb(Eina_Module *m, void *data)
{
	Dir_List_Get_Cb_Data *cb_data = data;
	int ret = 1;
	
	if (cb_data->cb)
	{
		ret = cb_data->cb(m, cb_data->data);
	}
	if (ret)
	{
		cb_data->list = eina_list_append(cb_data->list, m);
	}

	return ret;
}

static void _dir_list_cb(const char *name, const char *path, void *data)
{
	Dir_List_Cb_Data *cb_data = data;
	unsigned int length;

	length = strlen(name);
	if (length < strlen(MODULE_EXTENSION) + 1) /* x.so */
		return;
	if (!strcmp(name + length - strlen(MODULE_EXTENSION),
			MODULE_EXTENSION))
	{
		char file[PATH_MAX];
		Eina_Module *m;

		snprintf(file, PATH_MAX, "%s/%s", path, name);
		m = eina_module_new(file);
		if (!m)
			return;
		/* call the user provided cb on this module */
		cb_data->cb(m, cb_data->data);
	}
}
/*============================================================================*
 *                                   API                                      * 
 *============================================================================*/
/**
 * Calls the cb function for every module found on the directory path
 * 
 * @param path The directory's path to search for modules
 * @param recursive Iterate recursively on the path
 * @param cb Callback function to call
 * @param data Data passed to the callback function
 */
EAPI void eina_module_list(const char *path, unsigned int recursive, Eina_Module_Cb cb, void *data)
{
	Dir_List_Cb_Data cb_data;
		
	assert(path);
		
	cb_data.cb = cb;
	cb_data.data = data;
	eina_file_dir_list(path, recursive, &_dir_list_cb, &cb_data);
}
/**
 * Gets a list of modules found on the directory path
 * 
 * @param path The directory's path to search for modules
 * @param recursive Iterate recursively on the path
 * @param cb Callback function to call, if the return value of the callback is zero
 * it won't be added to the list, if it is one, it will.
 * @param data Data passed to the callback function
 */
EAPI Eina_List * eina_module_list_get(const char *path, unsigned int recursive, Eina_Module_Cb cb, void *data)
{
	Dir_List_Get_Cb_Data list_get_cb_data;
	Dir_List_Cb_Data list_cb_data;
	
	assert(path);
	
	list_get_cb_data.list = NULL;
	list_get_cb_data.cb = cb;
	list_get_cb_data.data = data;
	
	list_cb_data.cb = &_dir_list_get_cb;
	list_cb_data.data = &list_get_cb_data;
	
	eina_file_dir_list(path, recursive, &_dir_list_cb, &list_cb_data);
	
	return list_get_cb_data.list;
}
/**
 * Load every module on the list of modules
 * @param list The list of modules
 */
EAPI void eina_module_list_load(Eina_List *list)
{
	Eina_List *l;
	
	for (l = list; l; l = eina_list_next(l))
	{
		Eina_Module *m;
		
		m = eina_list_data(l);
		eina_module_load(m);
	}
}
/**
 * To be documented
 * FIXME: To be fixed
 */
EAPI void eina_module_list_unload(Eina_List *list)
{
	Eina_List *l;
	
	for (l = list; l; l = eina_list_next(l))
	{
		Eina_Module *m;
		
		m = eina_list_data(l);
		eina_module_unload(m);
	}
}
/**
 * Helper function that iterates over the list of modules and calls
 * eina_module_delete on each
 * 
 */
EAPI void eina_module_list_free(Eina_List *list)
{
	Eina_List *l;
	
	for (l = list; l; l = eina_list_next(l))
	{
		Eina_Module *m;
		
		m = eina_list_data(l);
		eina_module_free(m);
	}
}
/**
 * To be documented
 * FIXME: To be fixed
 */
EAPI int eina_module_load(Eina_Module *m)
{
	void *dl_handle;
	
	assert(m);
	
	if (m->handle) return 1;

	dl_handle = dlopen(m->file, RTLD_LAZY);
	if (!dl_handle) return 0;
	
	m->handle = dl_handle;
	return 1;
}
/**
 * To be documented
 * FIXME: To be fixed
 */
EAPI Eina_Module * eina_module_new(const char *file)
{
	Eina_Module *m;
	
	assert(file);
	/* TODO check that the file exists */
	
	m = malloc(sizeof(Eina_Module));
#ifdef DEBUG
	/* TODO add the magic */
#endif
	m->file = strdup(file);
	m->handle = NULL;
	
	return m;
}
/**
 * To be documented
 * FIXME: To be fixed
 */
EAPI void eina_module_free(Eina_Module *m)
{
	assert(m);

#ifdef DEBUG
	/* TODO check the magic */
#endif
	if (m->handle)
		eina_module_unload(m);
	free(m->file);
	free(m);
}
/**
 * To be documented
 * FIXME: To be fixed
 */
EAPI void eina_module_unload(Eina_Module *m)
{
	assert(m);
#ifdef DEBUG
	/* TODO check the magic */
#endif
	if (!m->handle)
		return;
	dlclose(m->handle);
}
/**
 * To be documented
 * FIXME: To be fixed
 */
EAPI void * eina_module_symbol_get(Eina_Module *m, const char *symbol)
{
	assert(m);
#ifdef DEBUG
	/* TODO check the magic */
#endif
	return dlsym(m->handle, symbol);	
}
/**
 * To be documented
 * FIXME: To be fixed
 */
EAPI char * eina_module_path_get(Eina_Module *m)
{
	char *path;
	
	assert(m);
#ifdef DEBUG
	/* TODO check the magic */
#endif
	eina_file_path_nth_get(m->file, -1, &path, NULL);
	
	return path;
}
/**
 * To be documented
 * FIXME: To be fixed
 */
EAPI char * eina_module_name_get(Eina_Module *m)
{
	char *name;
	
	assert(m);
#ifdef DEBUG
	/* TODO check the magic */
#endif
	eina_file_path_nth_get(m->file, -1, NULL, &name);
	
	return name;
}
