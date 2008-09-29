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

/**
 * @defgroup Module_Group Module
 * @{
 */

typedef struct _Eina_Module Eina_Module;

typedef Eina_Bool (*Eina_Module_Cb)(Eina_Module *m, void *data);
typedef Eina_Bool (*Eina_Module_Init)(void);
typedef void (*Eina_Module_Shutdown)(void);

#define EINA_MODULE_INIT(f) Eina_Module_Init __eina_module_init = &f;
#define EINA_MODULE_SHUTDOWN(f) Eina_Module_Shutdown __eina_module_shutdown = &f;

EAPI int eina_module_init(void);
EAPI int eina_module_shutdown(void);

EAPI Eina_Module * eina_module_new(const char *file);
EAPI Eina_Bool eina_module_delete(Eina_Module *m);
EAPI Eina_Bool eina_module_load(Eina_Module *module);
EAPI Eina_Bool eina_module_unload(Eina_Module *m);
EAPI void *eina_module_symbol_get(Eina_Module *module, const char *symbol);
EAPI const char * eina_module_file_get(Eina_Module *m);

EAPI Eina_Array * eina_module_list_get(const char *path, unsigned int recursive, Eina_Module_Cb cb, void *data);
EAPI void eina_module_list_load(Eina_Array *list);
EAPI void eina_module_list_unload(Eina_Array *list);
EAPI void eina_module_list_delete(Eina_Array *list);

/** @} */

#endif /*EINA_MODULE_H_*/
