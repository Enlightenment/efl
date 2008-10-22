/* EINA - EFL data type library
 * Copyright (C) 2007-2008 Jorge Luis Zapata Muga, Cedric BAIL
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

#include "eina_error.h"
#include "eina_module.h"
#include "eina_file.h"
#include "eina_private.h"

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

#define EINA_MODULE_SYMBOL_INIT "__eina_module_init"
#define EINA_MODULE_SYMBOL_SHUTDOWN "__eina_module_shutdown"

EAPI Eina_Error EINA_ERROR_WRONG_MODULE = 0;
EAPI Eina_Error EINA_ERROR_MODULE_INIT_FAILED = 0;

struct _Eina_Module
{
	char *file;
	void *handle;
	int ref;
};

typedef struct _Dir_List_Get_Cb_Data
{
	Eina_Module_Cb cb;
	void *data;
	Eina_Array *array;
} Dir_List_Get_Cb_Data;

typedef struct _Dir_List_Cb_Data
{
	Eina_Module_Cb cb;
	void *data;
} Dir_List_Cb_Data;

static Eina_Bool _dir_list_get_cb(Eina_Module *m, void *data)
{
	Dir_List_Get_Cb_Data *cb_data = data;
	Eina_Bool ret = EINA_TRUE;

	if (cb_data->cb)
	{
		ret = cb_data->cb(m, cb_data->data);
	}
	if (ret)
	{
		eina_array_push(cb_data->array, m);
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
static int _eina_module_count = 0;
/*============================================================================*
 *                                   API                                      *
 *============================================================================*/
/**
 * To be documented
 * FIXME: To be fixed
 */
EAPI int
eina_module_init(void)
{
	_eina_module_count++;

	if (_eina_module_count != 1)
		goto end_init;

	eina_error_init();

	EINA_ERROR_WRONG_MODULE = eina_error_msg_register("Wrong file format or no file module found");
	EINA_ERROR_MODULE_INIT_FAILED = eina_error_msg_register("Module initialisation function failed");

end_init:
	return _eina_module_count;
}
/**
 * To be documented
 * FIXME: To be fixed
 */
EAPI int
eina_module_shutdown(void)
{
	_eina_module_count--;
	if (_eina_module_count != 0)
		goto end_shutdown;

	eina_error_shutdown();

	/* TODO should we store every module when "new" is called and
	 * delete the list of modules here
	 */

end_shutdown:
	return _eina_module_count;
}
/**
 * To be documented
 * FIXME: To be fixed
 */
EAPI Eina_Bool eina_module_load(Eina_Module *m)
{
	void *dl_handle;
	Eina_Module_Init *initcall;

	if (m->handle) goto loaded;

	eina_error_set(EINA_ERROR_WRONG_MODULE);

	dl_handle = dlopen(m->file, RTLD_NOW);
	if (!dl_handle) return EINA_FALSE;

	eina_error_set(EINA_ERROR_MODULE_INIT_FAILED);

	initcall = dlsym(dl_handle, EINA_MODULE_SYMBOL_INIT);
	if ((!initcall) || (!(*initcall)))
		goto ok;
	if ((*initcall)() == EINA_TRUE)
		goto ok;

	dlclose(dl_handle);
	return EINA_FALSE;
ok:
	m->handle = dl_handle;
loaded:
	m->ref++;

	eina_error_set(0);
	return EINA_TRUE;
}
/**
 * To be documented
 * FIXME: To be fixed
 */
EAPI Eina_Bool eina_module_unload(Eina_Module *m)
{
	Eina_Module_Shutdown *shut;
	if (!m)	return EINA_FALSE;

	m->ref--;
	if (!m->ref)
	{
		shut = dlsym(m->handle, EINA_MODULE_SYMBOL_SHUTDOWN);
		if ((shut) && (*shut))
			(*shut)();
		dlclose(m->handle);
		m->handle = NULL;
		return EINA_TRUE;
	}
	return EINA_FALSE;
}
/**
 * To be documented
 * FIXME: To be fixed
 */
EAPI Eina_Module * eina_module_new(const char *file)
{
	Eina_Module *m;

	/* TODO check that the file exists */

	m = malloc(sizeof(Eina_Module));
	/* TODO add the magic */
	m->file = strdup(file);
	m->ref = 0;
	m->handle = NULL;

	return m;
}
/**
 * To be documented
 * FIXME: To be fixed
 */
EAPI Eina_Bool eina_module_delete(Eina_Module *m)
{
	if (!m)	return EINA_FALSE;

	if (m->handle)
	{
		if (eina_module_unload(m) == EINA_FALSE)
			return EINA_FALSE;
	}
	free(m->file);
	free(m);
	return EINA_TRUE;
}
/**
 * To be documented
 * FIXME: To be fixed
 */
EAPI void * eina_module_symbol_get(Eina_Module *m, const char *symbol)
{
	if ((!m) || (!m->handle))
		return NULL;

	return dlsym(m->handle, symbol);
}
/**
 * To be documented
 * FIXME: To be fixed
 */
EAPI const char * eina_module_file_get(Eina_Module *m)
{
	if (!m) return NULL;

	return m->file;
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
EAPI Eina_Array * eina_module_list_get(const char *path, unsigned int recursive, Eina_Module_Cb cb, void *data)
{
	Dir_List_Get_Cb_Data list_get_cb_data;
	Dir_List_Cb_Data list_cb_data;

	if (!path)
		return NULL;

	list_get_cb_data.array = eina_array_new(4);
	list_get_cb_data.cb = cb;
	list_get_cb_data.data = data;

	list_cb_data.cb = &_dir_list_get_cb;
	list_cb_data.data = &list_get_cb_data;

	eina_file_dir_list(path, recursive, &_dir_list_cb, &list_cb_data);

	return list_get_cb_data.array;
}
/**
 * Load every module on the list of modules
 * @param list The list of modules
 */
EAPI void eina_module_list_load(Eina_Array *array)
{
	Eina_Array_Iterator iterator;
	Eina_Module *m;
	unsigned int i;

	EINA_ARRAY_ITER_NEXT(array, i, m, iterator)
		eina_module_load(m);
}
/**
 * To be documented
 * FIXME: To be fixed
 */
EAPI void eina_module_list_unload(Eina_Array *array)
{
	Eina_Array_Iterator iterator;
	Eina_Module *m;
	unsigned int i;

	EINA_ARRAY_ITER_NEXT(array, i, m, iterator)
		eina_module_unload(m);
}
/**
 * Helper function that iterates over the list of modules and calls
 * eina_module_delete on each
 */
EAPI void eina_module_list_delete(Eina_Array *array)
{
	Eina_Array_Iterator iterator;
	Eina_Module *m;
	unsigned int i;

	EINA_ARRAY_ITER_NEXT(array, i, m, iterator)
		eina_module_delete(m);
}

