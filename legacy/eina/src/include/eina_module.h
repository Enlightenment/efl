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
#include "eina_list.h"

/**
 * @defgroup Module_Group Module
 * @{
 */
typedef struct _Eina_Module Eina_Module;
typedef struct _Eina_Module_Group Eina_Module_Group;
typedef struct _Eina_Module_Export Eina_Module_Export;

typedef Eina_Bool (*Eina_Module_Cb)(Eina_Module *m, void *data);
struct _Eina_Module_Export
{
   const char *name;
   const char *version;
   const char *type;
   const void *object;
};

EAPI int eina_module_init(void);
EAPI int eina_module_shutdown(void);
EAPI void eina_module_root_add(const char *root_path);

EAPI Eina_Module_Group *eina_module_group_new(void);
EAPI void eina_module_group_delete(Eina_Module_Group *group);

EAPI void eina_module_path_register(Eina_Module_Group *modules, const char *path, Eina_Bool recursive);
EAPI void eina_module_app_register(Eina_Module_Group *modules, const char *app, const char *types, const char *version);
EAPI void eina_module_register(Eina_Module_Group *modules, const Eina_Module_Export *static_module);

EAPI Eina_List *eina_module_list_new(Eina_Module_Group *modules, Eina_Module_Cb cb, void *data);
EAPI void eina_module_list_delete(Eina_List *modules);

EAPI Eina_Module *eina_module_new(Eina_Module_Group *modules, const char *name);
EAPI void eina_module_delete(Eina_Module *modules);

EAPI Eina_Bool eina_module_load(Eina_Module *module);
EAPI void eina_module_unload(Eina_Module *module);

EAPI void eina_module_list_load(const Eina_List *list);
EAPI void eina_module_list_unload(const Eina_List *list);

EAPI const char *eina_module_path_get(Eina_Module *module);
EAPI void *eina_module_symbol_get(Eina_Module *module, const char *symbol);

EAPI void *eina_module_export_object_get(Eina_Module *module);
EAPI const char *eina_module_export_type_get(Eina_Module *module);
EAPI const char *eina_module_export_version_get(Eina_Module *module);
EAPI const char *eina_module_export_name_get(Eina_Module *module);

#define EINA_MODULE(Name, Type, Version, Object) EAPI Eina_Module_Export Eina_Export = { Name, Version, Type, Object };

/** @} */

#endif /*EINA_MODULE_H_*/
