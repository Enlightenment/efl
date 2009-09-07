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

#ifdef HAVE_DLADDR
# define _GNU_SOURCE
#endif

#ifdef HAVE_ALLOCA_H
# include <alloca.h>
#elif defined __GNUC__
# define alloca __builtin_alloca
#elif defined _AIX
# define alloca __alloca
#elif defined _MSC_VER
# include <malloc.h>
# define alloca _alloca
#else
# include <stddef.h>
# ifdef  __cplusplus
extern "C"
# endif
void *alloca (size_t);
#endif

#include <stdio.h>
#include <sys/types.h>
#include <dirent.h>
#include <string.h>

#include <dlfcn.h>

#ifdef HAVE_EVIL
# include <Evil.h>
#endif

#include "eina_config.h"
#include "eina_private.h"
#include "eina_error.h"
#include "eina_file.h"
#include "eina_log.h"

/* undefs EINA_ARG_NONULL() so NULL checks are not compiled out! */
#include "eina_safety_checks.h"
#include "eina_module.h"

/*============================================================================*
 *                                  Local                                     *
 *============================================================================*/

/**
 * @cond LOCAL
 */

static int EINA_MODULE_LOG_DOM = -1;
#ifdef ERR
#undef ERR
#endif
#define ERR(...) EINA_LOG_DOM_ERR(EINA_MODULE_LOG_DOM, __VA_ARGS__)

#ifdef WRN
#undef WRN
#endif
#define WRN(...) EINA_LOG_DOM_WARN(EINA_MODULE_LOG_DOM, __VA_ARGS__)

#ifdef DBG
#undef DBG
#endif
#define DBG(...) EINA_LOG_DOM_DBG(EINA_MODULE_LOG_DOM, __VA_ARGS__)

#if defined(_WIN32) || defined(__CYGWIN__)
# define MODULE_EXTENSION ".dll"
#else
# define MODULE_EXTENSION ".so"
#endif /* !defined(_WIN32) && !defined(__CYGWIN__) */

#define EINA_MODULE_SYMBOL_INIT "__eina_module_init"
#define EINA_MODULE_SYMBOL_SHUTDOWN "__eina_module_shutdown"

struct _Eina_Module
{
   void *handle;
   int ref;
   const char file[];
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
   size_t length;

   length = strlen(name);
   if (length < sizeof(MODULE_EXTENSION)) /* x.so */
     return;
   if (!strcmp(name + length - sizeof(MODULE_EXTENSION) + 1,
	       MODULE_EXTENSION))
     {
	char *file;
	Eina_Module *m;
	size_t length;

	length = strlen(path) + strlen(name) + 2;

	file = alloca(sizeof (char) * length);
	if (!file) return ;

	snprintf(file, length, "%s/%s", path, name);
	m = eina_module_new(file);
	if (!m)
	  return;
	/* call the user provided cb on this module */
	if (!cb_data->cb(m, cb_data->data))
	  eina_module_free(m);
     }
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

/**
 * @addtogroup Eina_Module_Group Module
 *
 * @brief These functions provide module management.
 *
 * @{
 */

/**
 * @cond LOCAL
 */

static const char EINA_ERROR_WRONG_MODULE_STR[] = "Wrong file format or no file module found";
static const char EINA_ERROR_MODULE_INIT_FAILED_STR[] = "Module initialisation function failed";

EAPI Eina_Error EINA_ERROR_WRONG_MODULE = 0;
EAPI Eina_Error EINA_ERROR_MODULE_INIT_FAILED = 0;

/**
 * @endcond
 */

/**
 * @internal
 * @brief Initialize the module loader module.
 *
 * @return #EINA_TRUE on success, #EINA_FALSE on failure.
 *
 * This function sets up the module loader module of Eina. It is
 * called by eina_init().
 *
 * This function sets up the module module of Eina. It also registers
 * the errors #EINA_ERROR_WRONG_MODULE and
 * #EINA_ERROR_MODULE_INIT_FAILED.
 *
 * @see eina_init()
 */
Eina_Bool
eina_module_init(void)
{
   EINA_MODULE_LOG_DOM = eina_log_domain_register
     ("eina_module", EINA_LOG_COLOR_DEFAULT);
   if (EINA_MODULE_LOG_DOM < 0)
     {
	EINA_LOG_ERR("Could not register log domain: eina_module");
	return EINA_FALSE;
     }

#define EEMR(n) n = eina_error_msg_static_register(n##_STR)
   EEMR(EINA_ERROR_WRONG_MODULE);
   EEMR(EINA_ERROR_MODULE_INIT_FAILED);
#undef EEMR

   return EINA_TRUE;
}

/**
 * @internal
 * @brief Shut down the module loader module.
 *
 * @return #EINA_TRUE on success, #EINA_FALSE on failure.
 *
 * This function shuts down the module loader module set up by
 * eina_module_init(). It is called by eina_shutdown().
 *
 * @see eina_shutdown()
 */
Eina_Bool
eina_module_shutdown(void)
{
   /* TODO should we store every module when "new" is called and
    * delete the list of modules here
    */

   eina_log_domain_unregister(EINA_MODULE_LOG_DOM);
   EINA_MODULE_LOG_DOM = -1;
   return EINA_TRUE;
}

/**
 * @brief Return a new module.
 *
 * @param file The name of the file module to load.
 *
 * This function returns a new module. If @p file is @c NULL, the
 * function returns @c NULL, otherwise, it allocates an Eina_Module,
 * stores a duplicate string of @p file, sets its reference to @c 0
 * and its handle to @c NULL.
 *
 * When the new module is not needed anymore, use eina_module_free()
 * to free the allocated memory.
 *
 * @see eina_module_load
 */
EAPI Eina_Module *eina_module_new(const char *file)
{
   Eina_Module *m;
   size_t len;

   EINA_SAFETY_ON_NULL_RETURN_VAL(file, NULL);
   /* TODO check that the file exists. Update doc too */

   len = strlen(file);
   EINA_SAFETY_ON_FALSE_RETURN_VAL(len > 0, NULL);

   m = malloc(sizeof(Eina_Module) + len + 1);
   if (!m) {
      ERR("could not malloc(%lu)", sizeof(Eina_Module) + len + 1);
      return NULL;
   }
   memcpy((char *)m->file, file, len + 1);
   m->ref = 0;
   m->handle = NULL;
   DBG("m=%p, file=%s", m, file);

   return m;
}

/**
 * @brief Delete a module.
 *
 * @param m The module to delete.
 * @return EINA_TRUE on success, EINA_FALSE otherwise.
 *
 * This function calls eina_module_unload() if @p m has been previously
 * loaded and frees the allocated memory. On success this function
 * returns EINA_TRUE and EINA_FALSE otherwise. If @p m is @c NULL, the
 * function returns immediatly.
 */
EAPI Eina_Bool eina_module_free(Eina_Module *m)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(m, EINA_FALSE);

   DBG("m=%p, handle=%p, file=%s, refs=%d", m, m->handle, m->file, m->ref);

   if (m->handle)
     {
	if (eina_module_unload(m) == EINA_FALSE)
	  return EINA_FALSE;
     }
   free(m);
   return EINA_TRUE;
}

/**
 * @brief Load a module.
 *
 * @param m The module to load.
 * @return EINA_TRUE on success, EINA_FALSE otherwise.
 *
 * This function load the shared file object passed in
 * eina_module_new(). If it is a internal Eina module (like the
 * mempools), it also initialize it. It the shared file object can not
 * be loaded, the error #EINA_ERROR_WRONG_MODULE is set and
 * #EINA_FALSE is returned. If it is a internal Eina module and the
 * module can not be initialized, the error
 * #EINA_ERROR_MODULE_INIT_FAILED is set and #EINA_FALSE is
 * returned. If the module has already been loaded, it's refeence
 * counter is increased by one and #EINA_TRUE is returned. If @p m is
 * @c NULL, the function returns immediatly #EINA_FALSE.
 *
 * When the symbols of the shared file objetcts are not needed
 * anymore, call eina_module_unload() to unload the module.
 */
EAPI Eina_Bool eina_module_load(Eina_Module *m)
{
   void *dl_handle;
   Eina_Module_Init *initcall;

   EINA_SAFETY_ON_NULL_RETURN_VAL(m, EINA_FALSE);

   DBG("m=%p, handle=%p, file=%s, refs=%d", m, m->handle, m->file, m->ref);

   if (m->handle) goto loaded;

   dl_handle = dlopen(m->file, RTLD_NOW);
   if (!dl_handle)
     {
	WRN("could not dlopen(\"%s\", RTLD_NOW): %s", m->file, dlerror());
	eina_error_set(EINA_ERROR_WRONG_MODULE);
	return EINA_FALSE;
     }

   initcall = dlsym(dl_handle, EINA_MODULE_SYMBOL_INIT);
   if ((!initcall) || (!(*initcall)))
     goto ok;
   if ((*initcall)() == EINA_TRUE)
     goto ok;

   WRN("could not find eina's entry symbol %s inside module %s",
       EINA_MODULE_SYMBOL_INIT, m->file);
   eina_error_set(EINA_ERROR_MODULE_INIT_FAILED);
   dlclose(dl_handle);
   return EINA_FALSE;
 ok:
   DBG("successfully loaded %s", m->file);
   m->handle = dl_handle;
 loaded:
   m->ref++;
   DBG("ref %d", m->ref);

   eina_error_set(0);
   return EINA_TRUE;
}

/**
 * @brief Unload a module.
 *
 * @param m The module to load.
 * @return EINA_TRUE on success, EINA_FALSE otherwise.
 *
 * This function unload the module @p m that has been previously
 * loaded by eina_module_load(). If the reference counter of @p m is
 * strictly greater than @c 1, #EINA_FALSE is returned. Otherwise, the
 * shared object file is closed and if it is a internal Eina module, it
 * is shutted down just before. In that case, #EINA_TRUE is
 * returned. In all case, the reference counter is decreased. If @p m
 * is @c NULL, the function returns immediatly #EINA_FALSE.
 */
EAPI Eina_Bool eina_module_unload(Eina_Module *m)
{
   Eina_Module_Shutdown *shut;
   EINA_SAFETY_ON_NULL_RETURN_VAL(m, EINA_FALSE);

   DBG("m=%p, handle=%p, file=%s, refs=%d", m, m->handle, m->file, m->ref);

   m->ref--;
   if (!m->ref)
     {
	shut = dlsym(m->handle, EINA_MODULE_SYMBOL_SHUTDOWN);
	if ((shut) && (*shut))
	  (*shut)();
	dlclose(m->handle);
	m->handle = NULL;
	DBG("unloaded module %s", m->file);
	return EINA_TRUE;
     }
   return EINA_FALSE;
}

/**
 * @brief Retrive the data associated to a symbol.
 *
 * @param m The module.
 * @param symbol The symbol.
 * @return The data associated to the symbol, or @c NULL on failure.
 *
 * This function returns the data associated to @p symbol of @p m. @p
 * m must have been loaded before with eina_module_load(). If @p m
 * is @c NULL, or if it has not been correctly loaded before, the
 * function returns immediatly @c NULL.
 */
EAPI void * eina_module_symbol_get(Eina_Module *m, const char *symbol)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(m, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(m->handle, NULL);
   return dlsym(m->handle, symbol);
}

/**
 * @brief Return the file name associated to the module.
 *
 * @param m The module.
 * @return The file name.
 *
 * Return the file name passed in eina_module_new(). If @p m is
 * @c NULL, the function returns immediatly @c NULL. The returned
 * value must no be freed.
 */
EAPI const char * eina_module_file_get(Eina_Module *m)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(m, NULL);
   return m->file;
}

EAPI char *eina_module_symbol_path_get(const void *symbol, const char *sub_dir)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(symbol, NULL);

#ifdef HAVE_DLADDR
   Dl_info eina_dl;

   if (dladdr(symbol, &eina_dl))
     {
	if (strrchr(eina_dl.dli_fname, '/'))
	  {
	     char *path;
	     int l0;
	     int l1;
	     int l2 = 0;

	     l0 = strlen(eina_dl.dli_fname);
	     l1 = strlen(strrchr(eina_dl.dli_fname, '/'));
	     if (sub_dir && (*sub_dir != '\0'))
	       l2 = strlen(sub_dir);
	     path = malloc(l0 - l1 + l2 + 1);
	     if (path)
	       {
		  memcpy(path, eina_dl.dli_fname, l0 - l1);
		  if (sub_dir && (*sub_dir != '\0'))
		    memcpy(path + l0 - l1, sub_dir, l2);
		  path[l0 - l1 + l2] = '\0';
		  return path;
	       }
	  }
     }
#endif /* ! HAVE_DLADDR */

   return NULL;
}

EAPI char *eina_module_environment_path_get(const char *env, const char *sub_dir)
{
   const char *env_dir;

   EINA_SAFETY_ON_NULL_RETURN_VAL(env, NULL);

   env_dir = getenv(env);
   if (env_dir)
     {
	char  *path;
	size_t l1;
	size_t l2 = 0;

	l1 = strlen(env_dir);
	if (sub_dir && (*sub_dir != '\0'))
	  l2 = strlen(sub_dir);

	path = (char *)malloc(l1 + l2 + 1);
	if (path)
	  {
	     memcpy(path, env_dir, l1);
	     if (sub_dir && (*sub_dir != '\0'))
	       memcpy(path + l1, sub_dir, l2);
	     path[l1 + l2] = '\0';

	     return path;
	  }
     }

   return NULL;
}

/**
 * Get a list of modules found on the directory path
 *
 * @param array The array that stores the list of the modules.
 * @param path The directory's path to search for modules
 * @param recursive Iterate recursively on the path
 * @param cb Callback function to call, if the return value of the callback is zero
 * it won't be added to the list, if it is one, it will.
 * @param data Data passed to the callback function
 */
EAPI Eina_Array * eina_module_list_get(Eina_Array *array, const char *path, unsigned int recursive, Eina_Module_Cb cb, void *data)
{
   Dir_List_Get_Cb_Data list_get_cb_data;
   Dir_List_Cb_Data list_cb_data;

   if (!path) return array;

   list_get_cb_data.array = array ? array : eina_array_new(4);
   list_get_cb_data.cb = cb;
   list_get_cb_data.data = data;

   list_cb_data.cb = &_dir_list_get_cb;
   list_cb_data.data = &list_get_cb_data;

   eina_file_dir_list(path, recursive, &_dir_list_cb, &list_cb_data);

   return list_get_cb_data.array;
}

/**
 * Load every module on the list of modules
 * @param array The array of modules to load
 */
EAPI void eina_module_list_load(Eina_Array *array)
{
   Eina_Array_Iterator iterator;
   Eina_Module *m;
   unsigned int i;

   EINA_SAFETY_ON_NULL_RETURN(array);
   DBG("array %p, count %u", array, array->count);
   EINA_ARRAY_ITER_NEXT(array, i, m, iterator)
     eina_module_load(m);
}

/**
 * Unload every module on the list of modules
 * @param array The array of modules to unload
 */
EAPI void eina_module_list_unload(Eina_Array *array)
{
   Eina_Array_Iterator iterator;
   Eina_Module *m;
   unsigned int i;

   EINA_SAFETY_ON_NULL_RETURN(array);
   DBG("array %p, count %u", array, array->count);
   EINA_ARRAY_ITER_NEXT(array, i, m, iterator)
     eina_module_unload(m);
}

/**
 * Helper function that iterates over the list of modules and calls
 * eina_module_free on each
 */
EAPI void eina_module_list_flush(Eina_Array *array)
{
   Eina_Array_Iterator iterator;
   Eina_Module *m;
   unsigned int i;

   EINA_SAFETY_ON_NULL_RETURN(array);
   DBG("array %p, count %u", array, array->count);
   EINA_ARRAY_ITER_NEXT(array, i, m, iterator)
     eina_module_free(m);

   eina_array_flush(array);
}

/**
 * @}
 */
