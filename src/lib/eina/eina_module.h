/* EINA - EFL data type library
 * Copyright (C) 2007-2008 Jorge Luis Zapata Muga
 *
 * This library is a free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
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
 * @defgroup Eina_Module_Group Module
 * @ingroup Eina_Tools_Group
 *
 * @brief This group of functions provide module management.
 *
 * Eina module provides some helpers over POSIX dlopen(). It is not
 * meant to replace, abstract, or make a "portable" version of the
 * POSIX, but enhance its usage by defining some good practices.
 *
 * Modules are created with eina_module_new() and later loaded with
 * eina_module_load(). Loads are reference counted and there must be
 * the same number of eina_module_unload() in order to have it to call
 * dlclose(). This makes it simple to have different users for the same
 * module.
 *
 * The loaded shared objects may have two visible functions that are
 * called and might provide initialization and shutdown
 * procedures. The symbols are @c __eina_module_init and
 * @c __eina_module_shutdown and are defined by the macros
 * EINA_MODULE_INIT() and EINA_MODULE_SHUTDOWN().
 *
 * There are some helpers to automatically create modules based on
 * directory listing. See eina_module_arch_list_get(),
 * eina_module_list_get(), and eina_module_find().
 *
 * @{
 */

/**
 * @typedef Eina_Module
 * @brief The structure type for the dynamic module loader handle.
 */
typedef struct _Eina_Module Eina_Module;

/**
 * @typedef Eina_Module_Cb
 * @brief The dynamic module loader callback.
 */
typedef Eina_Bool         (*Eina_Module_Cb)(Eina_Module *m, void *data);

/**
 * @typedef Eina_Module_Init
 * @brief If a function with such a signature is exported by the module as
 *        __eina_module_init, it is called on the first load after
 *        dlopen() and if @c EINA_FALSE is returned, the load fails, @c EINA_TRUE
 *        means the module is successfully initialized.
 * @see Eina_Module_Shutdown
 */
typedef Eina_Bool (*Eina_Module_Init)(void);

/**
 * @typedef Eina_Module_Shutdown
 * @brief If a function with such a signature is exported by the module as
 *        __eina_module_shutdown, it is called before calling dlclose()
 * @see Eina_Module_Init
 */
typedef void (*Eina_Module_Shutdown)(void);

/**
 * @def EINA_MODULE_INIT
 * @brief Definition that declares the given function as the module initializer (__eina_module_init).
 *        It must be of signature #Eina_Module_Init
 */
#define EINA_MODULE_INIT(f) EAPI Eina_Module_Init __eina_module_init = &f

/**
 * @def EINA_MODULE_SHUTDOWN
 * @brief Defintion that declares the given function as the module shutdown initializer (__eina_module_shutdown).
 *        It must be of signature #Eina_Module_Shutdown
 */
#define EINA_MODULE_SHUTDOWN(f) EAPI Eina_Module_Shutdown __eina_module_shutdown = &f

/**
 * @var EINA_ERROR_WRONG_MODULE
 * @brief The error identifier corresponding to a wrong module.
 */
extern EAPI Eina_Error EINA_ERROR_WRONG_MODULE;

/**
 * @var EINA_ERROR_MODULE_INIT_FAILED
 * @brief The error identifier corresponding to a failure during the initialisation of a module.
 */
extern EAPI Eina_Error EINA_ERROR_MODULE_INIT_FAILED;

/**
 * @brief Returns a new module.
 *
 * @since_tizen 2.3
 *
 * @remarks When the new module is not needed anymore, use eina_module_free()
 *          to free the allocated memory.
 *
 * @param[in] file The name of the file module to load
 * @return A new module \n
 *         If @a file is @c NULL, the function returns @c NULL,
 *         otherwise it allocates an Eina_Module, stores
 *         a duplicate string of @a file, sets its reference to @c 0 and
 *         its handle to @c NULL.
 *
 * @see eina_module_load
 */
EAPI Eina_Module *eina_module_new(const char *file) EINA_MALLOC EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * @brief Deletes a module.
 *
 * @details This function calls eina_module_unload() if @a module has been previously
 *          loaded and frees the allocated memory. On success this function
 *          returns @c EINA_TRUE, otherwise it returns @c EINA_FALSE. If @a module is @c NULL, the
 *          function returns immediately.
 *
 * @since_tizen 2.3
 *
 * @param[in] module The module to delete
 * @return @c EINA_TRUE on success, otherwise @c EINA_FALSE
 *
 */
EAPI Eina_Bool eina_module_free(Eina_Module *module) EINA_ARG_NONNULL(1);

/**
 * @brief Loads a module.
 *
 * @details This function loads the shared file object passed in
 *          eina_module_new(). If it is an internal Eina module (like the
 *          mempools), it also initializes it. If the shared file object cannot
 *          be loaded, the error #EINA_ERROR_WRONG_MODULE is set and
 *          and @c EINA_FALSE is returned. If it is an internal Eina module and the
 *          module cannot be initialized, the error #EINA_ERROR_MODULE_INIT_FAILED
 *          is set and @c EINA_FALSE is returned. If the module has already been loaded,
 *          its reference counter is increased by one and @c EINA_TRUE is returned.
 *          If @a module is @c NULL, the function returns @c EINA_FALSE immediately.
 *
 * @since_tizen 2.3
 *
 * @remarks When the symbols of the shared file objects are not needed
 *          anymore, call eina_module_unload() to unload the module.
 *
 * @param[in] module The module to load
 * @return @c EINA_TRUE on success, otherwise @c EINA_FALSE
 *
 */
EAPI Eina_Bool eina_module_load(Eina_Module *module) EINA_ARG_NONNULL(1);

/**
 * @brief Unloads a module.
 *
 * @details This function unloads the module @a module that has been previously
 *          loaded by eina_module_load(). If the reference counter of @a module is
 *          strictly greater than @c 1, @c EINA_FALSE is returned. Otherwise, the
 *          shared object file is closed and if it is an internal Eina module, it
 *          is shut down just before. In that case, @c EINA_TRUE is
 *          returned. In all cases, the reference counter is decreased. If @a module
 *          is @c NULL, the function returns @c EINA_FALSE immediately.
 *
 * @since_tizen 2.3
 *
 * @param[in] module The module to unload
 * @return @c EINA_TRUE on success, otherwise EINA_FALSE
 *
 */
EAPI Eina_Bool eina_module_unload(Eina_Module *module) EINA_ARG_NONNULL(1);

/**
 * @brief Gets the data associated to a symbol.
 *
 * @details This function returns the data associated to @a symbol of @a module. @a
 *          module must have been loaded earlier with eina_module_load(). If @a module
 *          is @c NULL, or if it has not been correctly loaded before, the
 *          function returns @c NULL immediately.
 *
 * @since_tizen 2.3
 *
 * @param[in] module The module
 * @param[in] symbol The symbol
 * @return The data associated to the symbol, otherwise @c NULL on failure
 *
 */
EAPI void *eina_module_symbol_get(const Eina_Module *module, const char *symbol) EINA_PURE EINA_ARG_NONNULL(1, 2) EINA_WARN_UNUSED_RESULT;

/**
 * @brief Gets the file name associated to the module.
 *
 * @details This function returns the file name passed in eina_module_new(). If
 *          @a module is @c NULL, the function returns @c NULL immediately. The
 *          returned value must no be freed.
 *
 * @since_tizen 2.3
 *
 * @param[in] module The module
 * @return The file name
 *
 */
EAPI const char *eina_module_file_get(const Eina_Module *module) EINA_PURE EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);


/**
 * @brief Gets the path built from the location of a library and a
 *        given sub directory.
 *
 * @details This function returns the path built by concatenating the path of
 *          the library containing the symbol @a symbol and @a sub_dir. @a sub_dir
 *          can be @c NULL. The returned path must be freed when not used
 *          anymore. If the symbol is not found, or dl_addr() is not supported,
 *          or allocation fails, this function returns @c NULL.
 *
 * @since_tizen 2.3
 *
 * @param[in] symbol The symbol to search for
 * @param[in] sub_dir The subdirectory to append
 * @return The built path on success, otherwise @c NULL
 *
 */
EAPI char *eina_module_symbol_path_get(const void *symbol, const char *sub_dir) EINA_PURE EINA_MALLOC EINA_ARG_NONNULL(1, 2);

/**
 * @brief Gets the path built from the value of an environment variable and a
 *        given sub directory.
 *
 * @details This function returns the path built by concatenating the value of
 *          the environment variable named @a env and @a sub_dir. @a sub_dir
 *          can be @c NULL. The returned path must be freed when not used
 *          anymore. If the symbol is not found, or @a env does not exist, or
 *          allocation fails, this function returns @c NULL.
 *
 * @since_tizen 2.3
 *
 * @param[in] env The environment variable to expand
 * @param[in] sub_dir The subdirectory to append
 * @return The built path on success, otherwise @c NULL
 *
 */
EAPI char *eina_module_environment_path_get(const char *env, const char *sub_dir) EINA_PURE EINA_MALLOC EINA_ARG_NONNULL(1, 2);


/**
 * @brief Gets an array of modules found on the directory path matching an arch type.
 *
 * @details This function adds to @a array the module names found in @a path
 *          which match the CPU architecture @a arch. If @a path or @a arch is
 *          @c NULL, the function returns @a array immediately. @a array can be
 *          @c NULL. In that case, it is created with 4 elements.
 *
 * @since_tizen 2.3
 *
 * @param[in] array The array that stores the list of the modules
 * @param[in] path The directory path to search for modules
 * @param[in] arch The architecture string
 * @return The array of modules found in @a path matching @a arch
 *
 */
EAPI Eina_Array *eina_module_arch_list_get(Eina_Array *array, const char *path, const char *arch);

/**
 * @brief Gets a list of modules found on the directory path.
 *
 * @details This function adds to @a array the list of modules found in
 *          @a path. If @a recursive is @c EINA_TRUE, then recursive search is
 *          done. The callback @a cb is called on each module found, and @a data
 *          is passed to @a cb. If @a path is @c NULL, the function returns
 *          @a array immediately. If the returned value of @a cb is @c 0, the
 *          module is not added to the list, otherwise it is added.
 *          @a array can be @c NULL. In that case, it is created with 4
 *          elements. @a cb can be @c NULL.
 *
 * @since_tizen 2.3
 *
 * @param[in] array The array that stores the list of the modules
 * @param[in] path The directory path to search for modules
 * @param[in] recursive The boolean value to iterate recursively on the path
 * @param[in] cb The callback function to call on each module
 * @param[in] data The data passed to the callback function
 * @return The array of modules found in @a path
 *
 */
EAPI Eina_Array *eina_module_list_get(Eina_Array *array, const char *path, Eina_Bool recursive, Eina_Module_Cb cb, void *data) EINA_MALLOC EINA_WARN_UNUSED_RESULT;

/**
 * @brief Loads every module on the list of modules.
 *
 * @details This function calls eina_module_load() on each element found in
 *          @a array. If @a array is @c NULL, this function does nothing.
 *
 * @since_tizen 2.3
 *
 * @param[in] array The array of modules to load
 *
 */
EAPI void eina_module_list_load(Eina_Array *array) EINA_ARG_NONNULL(1);

/**
 * @brief Unloads every module on the list of modules.
 *
 * @details This function calls eina_module_unload() on each element found in
 *          @a array. If @a array is @c NULL, this function does nothing.
 *
 * @since_tizen 2.3
 *
 * @param[in] array The array of modules to unload
 *
 */
EAPI void eina_module_list_unload(Eina_Array *array) EINA_ARG_NONNULL(1);

/**
 * @brief Frees every module on the list of modules.
 *
 * @details This function calls eina_module_free() on each element found in
 *          @a array. If @a array is @c NULL, this function does nothing.
 *
 * @since_tizen 2.3
 *
 * @param[in] array The array of modules to free
 *
 */
EAPI void eina_module_list_free(Eina_Array *array) EINA_ARG_NONNULL(1);

/**
 * @brief Finds a module in an array.
 *
 * @details This function finds a @a module in @a array.
 *          If the element is found the function returns the module, otherwise
 *          @c NULL is returned.
 *
 * @since_tizen 2.3
 *
 * @param[in] array The array to find the module in
 * @param[in] module The name of the module to be searched
 * @return The module to find on success, otherwise @c NULL
 *
 */
EAPI Eina_Module *eina_module_find(const Eina_Array *array, const char *module) EINA_ARG_NONNULL(1, 2);

/**
 * @}
 */

#endif /*EINA_MODULE_H_*/
