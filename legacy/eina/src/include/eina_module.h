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
 * @defgroup Module_Group Module
 * @{
 */

typedef struct _Eina_Module Eina_Module;

typedef Eina_Bool (*Eina_Module_Cb)(Eina_Module *m, void *data);
typedef Eina_Bool (*Eina_Module_Init)(void);
typedef void (*Eina_Module_Shutdown)(void);

#define EINA_MODULE_INIT(f) EAPI Eina_Module_Init __eina_module_init = &f;
#define EINA_MODULE_SHUTDOWN(f) EAPI Eina_Module_Shutdown __eina_module_shutdown = &f;

extern EAPI Eina_Error EINA_ERROR_WRONG_MODULE;
extern EAPI Eina_Error EINA_ERROR_MODULE_INIT_FAILED;

EAPI int eina_module_init(void);
EAPI int eina_module_shutdown(void);

EAPI Eina_Module * eina_module_new(const char *file) EINA_MALLOC EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);
EAPI Eina_Bool eina_module_delete(Eina_Module *m) EINA_ARG_NONNULL(1);
EAPI Eina_Bool eina_module_load(Eina_Module *module) EINA_ARG_NONNULL(1);
EAPI Eina_Bool eina_module_unload(Eina_Module *m) EINA_ARG_NONNULL(1);
EAPI void *eina_module_symbol_get(Eina_Module *module, const char *symbol) EINA_PURE EINA_ARG_NONNULL(1, 2) EINA_WARN_UNUSED_RESULT;
EAPI const char * eina_module_file_get(Eina_Module *m) EINA_PURE EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

EAPI char *eina_module_symbol_path_get(const void *symbol, const char *sub_dir) EINA_PURE EINA_MALLOC EINA_ARG_NONNULL(1, 2);
EAPI char *eina_module_environment_path_get(const char *env, const char *sub_dir) EINA_PURE EINA_MALLOC EINA_ARG_NONNULL(1, 2);

EAPI Eina_Array * eina_module_list_get(Eina_Array *array, const char *path, unsigned int recursive, Eina_Module_Cb cb, void *data) EINA_MALLOC EINA_ARG_NONNULL(2) EINA_WARN_UNUSED_RESULT;
EAPI void eina_module_list_load(Eina_Array *list) EINA_ARG_NONNULL(1);
EAPI void eina_module_list_unload(Eina_Array *list) EINA_ARG_NONNULL(1);
EAPI void eina_module_list_delete(Eina_Array *list) EINA_ARG_NONNULL(1);

/** @} */

#endif /*EINA_MODULE_H_*/
