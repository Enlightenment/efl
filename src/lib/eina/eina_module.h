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

#ifndef EINA_MODULE_H_
#define EINA_MODULE_H_

#include "eina_types.h"
#include "eina_array.h"
#include "eina_error.h"

/**
 * @addtogroup Eina_Module_Group Module
 *
 * @brief These functions provide module management.
 */

/**
 * @addtogroup Eina_Tools_Group Tools
 *
 * @{
 */

/**
 * @defgroup Eina_Module_Group Module
 *
 * Eina module provides some helpers over POSIX dlopen(). It is not
 * meant to replace, abstract or make a "portable" version of the
 * POSIX, but enhance its usage by defining some good practices.
 *
 * Modules are created with eina_module_new() and later loaded with
 * eina_module_load(). Loads are reference counted and there must be
 * the same number of eina_module_unload() in order to have it to call
 * dlclose(). This makes simple to have different users for the same
 * module.
 *
 * The loaded shared objects may have two visible functions that will
 * be called and might provide initialization and shutdown
 * procedures. The symbols are @c __eina_module_init and
 * @c __eina_module_shutdown and will be defined by the macros
 * EINA_MODULE_INIT() and EINA_MODULE_SHUTDOWN().
 *
 * There are some helpers to automatically create modules based on
 * directory listing. See eina_module_arch_list_get(),
 * eina_module_list_get() and eina_module_find().
 *
 * @{
 */

/**
 * @typedef Eina_Module
 * Dynamic module loader handle.
 */
typedef struct _Eina_Module Eina_Module;

/**
 * @typedef Eina_Module_Cb
 * Dynamic module loader callback.
 */
typedef Eina_Bool         (*Eina_Module_Cb)(Eina_Module *m, void *data);

/**
 * @typedef Eina_Module_Init
 * If a function with such signature is exported by module as
 * __eina_module_init, it will be called on the first load after
 * dlopen() and if #EINA_FALSE is returned, load will fail, #EINA_TRUE
 * means the module was successfully initialized.
 * @see Eina_Module_Shutdown
 */
typedef Eina_Bool (*Eina_Module_Init)(void);

/**
 * @typedef Eina_Module_Shutdown
 * If a function with such signature is exported by module as
 * __eina_module_shutdown, it will be called before calling dlclose()
 * @see Eina_Module_Init
 */
typedef void (*Eina_Module_Shutdown)(void);

/**
 * @def EINA_MODULE_INIT
 * declares the given function as the module initializer (__eina_module_init).
 * It must be of signature #Eina_Module_Init
 */
#define EINA_MODULE_INIT(f) EAPI Eina_Module_Init __eina_module_init = &f

/**
 * @def EINA_MODULE_SHUTDOWN
 * declares the given function as the module shutdownializer
 * (__eina_module_shutdown). It must be of signature #Eina_Module_Shutdown
 */
#define EINA_MODULE_SHUTDOWN(f) EAPI Eina_Module_Shutdown __eina_module_shutdown = &f

extern EAPI Eina_Error EINA_ERROR_WRONG_MODULE;
extern EAPI Eina_Error EINA_ERROR_MODULE_INIT_FAILED;

/**
 * @brief Return a new module.
 *
 * @param file The name of the file module to load.
 * @return A new module. If @p file is @c NULL, the function 
 * returns @c NULL, otherwise, it allocates an Eina_Module, stores
 * a duplicate string of @p file, sets its reference to @c 0 and
 * its handle to @c NULL.
 *
 * When the new module is not needed anymore, use eina_module_free()
 * to free the allocated memory.
 *
 * @see eina_module_load
 */
EAPI Eina_Module *
 eina_module_new(const char *file) EINA_MALLOC EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * @brief Delete a module.
 *
 * @param module The module to delete.
 * @return #EINA_TRUE on success, #EINA_FALSE otherwise.
 *
 * This function calls eina_module_unload() if @p module has been previously
 * loaded and frees the allocated memory. On success this function
 * returns #EINA_TRUE and #EINA_FALSE otherwise. If @p module is @c NULL, the
 * function returns immediately.
 */
EAPI Eina_Bool
 eina_module_free(Eina_Module *module) EINA_ARG_NONNULL(1);

/**
 * @brief Load a module.
 *
 * @param module The module to load.
 * @return #EINA_TRUE on success, #EINA_FALSE otherwise.
 *
 * This function load the shared file object passed in
 * eina_module_new(). If it is a internal Eina module (like the
 * mempools), it also initialize it. If the shared file object can not
 * be loaded, #EINA_FALSE is returned. If it is an internal Eina module and the
 * module can not be initialized, #EINA_FALSE is returned. If the module has
 * already been loaded, it's reference counter is increased by one and
 * #EINA_TRUE is returned. If @p module is @c NULL, the function returns
 * immediately #EINA_FALSE.
 *
 * When the symbols of the shared file objects are not needed
 * anymore, call eina_module_unload() to unload the module.
 */
EAPI Eina_Bool
 eina_module_load(Eina_Module *module) EINA_ARG_NONNULL(1);

/**
 * @brief Unload a module.
 *
 * @param module The module to load.
 * @return #EINA_TRUE on success, #EINA_FALSE otherwise.
 *
 * This function unload the module @p module that has been previously
 * loaded by eina_module_load(). If the reference counter of @p module is
 * strictly greater than @c 1, #EINA_FALSE is returned. Otherwise, the
 * shared object file is closed and if it is a internal Eina module, it
 * is shutted down just before. In that case, #EINA_TRUE is
 * returned. In all case, the reference counter is decreased. If @p module
 * is @c NULL, the function returns immediately #EINA_FALSE.
 */
EAPI Eina_Bool
 eina_module_unload(Eina_Module *module) EINA_ARG_NONNULL(1);

/**
 * @brief Retrieve the data associated to a symbol.
 *
 * @param module The module.
 * @param symbol The symbol.
 * @return The data associated to the symbol, or @c NULL on failure.
 *
 * This function returns the data associated to @p symbol of @p module. @p
 * module must have been loaded before with eina_module_load(). If @p module
 * is @c NULL, or if it has not been correctly loaded before, the
 * function returns immediately @c NULL.
 */
EAPI void *
 eina_module_symbol_get(const Eina_Module *module, const char *symbol) EINA_PURE EINA_ARG_NONNULL(1, 2) EINA_WARN_UNUSED_RESULT;

/**
 * @brief Return the file name associated to the module.
 *
 * @param module The module.
 * @return The file name.
 *
 * This function returns the file name passed in eina_module_new(). If
 * @p module is @c NULL, the function returns immediately @c NULL. The
 * returned value must no be freed.
 */
EAPI const char *
 eina_module_file_get(const Eina_Module *module) EINA_PURE EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);


/**
 * @brief Return the path built from the location of a library and a
 * given sub directory.
 *
 * @param symbol The symbol to search for.
 * @param sub_dir The subdirectory to append.
 * @return The built path on success, @c NULL otherwise.
 *
 * This function returns the path built by concatenating the path of
 * the library containing the symbol @p symbol and @p sub_dir. @p sub_dir
 * can be @c NULL. The returned path must be freed when not used
 * anymore. If the symbol is not found, or dl_addr() is not supported,
 * or allocation fails, this function returns @c NULL.
 */
EAPI char *
 eina_module_symbol_path_get(const void *symbol, const char *sub_dir) EINA_PURE EINA_MALLOC EINA_ARG_NONNULL(1, 2);

/**
 * @brief Return the path built from the value of an environment variable and a
 * given sub directory.
 *
 * @param env The environment variable to expand.
 * @param sub_dir The subdirectory to append.
 * @return The built path on success, @c NULL otherwise.
 *
 * This function returns the path built by concatenating the value of
 * the environment variable named @p env and @p sub_dir. @p sub_dir
 * can be @c NULL. The returned path must be freed when not used
 * anymore. If the symbol is not found, or @p env does not exist, or
 * allocation fails, this function returns @c NULL.
 */
EAPI char *
 eina_module_environment_path_get(const char *env, const char *sub_dir) EINA_PURE EINA_MALLOC EINA_ARG_NONNULL(1, 2);


/**
 * @brief Get an array of modules found on the directory path matching an arch type.
 *
 * @param array The array that stores the list of the modules.
 * @param path The directory's path to search for modules.
 * @param arch The architecture string.
 * @return The array of modules found in @p path matching @p arch.
 *
 * This function adds to @p array the module names found in @p path
 * which match the cpu architecture @p arch. If @p path or @p arch is
 * @c NULL, the function returns immediately @p array. @p array can be
 * @c NULL. In that case, it is created with 4 elements.
 */
EAPI Eina_Array *
 eina_module_arch_list_get(Eina_Array *array, const char *path, const char *arch);

/**
 * @brief Get a list of modules found on the directory path.
 *
 * @param array The array that stores the list of the modules.
 * @param path The directory's path to search for modules.
 * @param recursive Iterate recursively on the path.
 * @param cb Callback function to call on each module.
 * @param data Data passed to the callback function.
 * @return The array of modules found in @p path.
 *
 * This function adds to @p array the list of modules found in
 * @p path. If @p recursive is #EINA_TRUE, then recursive search is
 * done. The callback @p cb is called on each module found, and @p data
 * is passed to @p cb. If @p path is @c NULL, the function returns
 * immediately @p array. If the returned value of @p cb is @c 0, the
 * module will not be added to the list, otherwise it will be added.
 * @p array can be @c NULL. In that case, it is created with 4
 * elements. @p cb can be @c NULL.
 */
EAPI Eina_Array *
 eina_module_list_get(Eina_Array *array, const char *path, Eina_Bool recursive, Eina_Module_Cb cb, void *data) EINA_MALLOC EINA_WARN_UNUSED_RESULT;

/**
 * @brief Load every module on the list of modules.
 *
 * @param array The array of modules to load.
 *
 * This function calls eina_module_load() on each element found in
 * @p array. If @p array is @c NULL, this function does nothing.
 */
EAPI void
 eina_module_list_load(Eina_Array *array) EINA_ARG_NONNULL(1);

/**
 * @brief Unload every module on the list of modules.
 *
 * @param array The array of modules to unload.
 *
 * This function calls eina_module_unload() on each element found in
 * @p array. If @p array is @c NULL, this function does nothing.
 */
EAPI void
 eina_module_list_unload(Eina_Array *array) EINA_ARG_NONNULL(1);

/**
 * @p Free every module on the list of modules.
 *
 * @param array The array of modules to free.
 *
 * This function calls eina_module_free() on each element found in
 * @p array. If @p array is @c NULL, this function does nothing.
 */
EAPI void
 eina_module_list_free(Eina_Array *array) EINA_ARG_NONNULL(1);

/**
 * @brief Find an module in array.
 *
 * @param array The array to find the module.
 * @param module The name of module to be searched.
 * @return The module to find on success, @c NULL otherwise.
 *
 * This function finds an @p module in @p array.
 * If the element is found  the function returns the module, else
 * @c NULL is returned.
 */
EAPI Eina_Module *
 eina_module_find(const Eina_Array *array, const char *module) EINA_ARG_NONNULL(1, 2);

/**
 * @}
 */

/**
 * @}
 */

#endif /*EINA_MODULE_H_*/
