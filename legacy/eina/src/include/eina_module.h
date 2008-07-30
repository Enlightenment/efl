#ifndef EINA_MODULE_H_
#define EINA_MODULE_H_

#include "eina_types.h"
#include "eina_list.h"

/**
 * @defgroup Module_Group Module
 * @{
 */

typedef struct _Eina_Module Eina_Module;

typedef int (*Eina_Module_Cb)(Eina_Module *m, void *data);

EAPI Eina_Module * eina_module_new(const char *file);
EAPI void eina_module_free(Eina_Module *m);
EAPI void eina_module_unload(Eina_Module *m);
EAPI char * eina_module_path_get(Eina_Module *m);
EAPI char * eina_module_name_get(Eina_Module *m);
EAPI void * eina_module_symbol_get(Eina_Module *m, const char *symbol);

EAPI Eina_List * eina_module_list_get(const char *path, unsigned int recursive, Eina_Module_Cb cb, void *data);
EAPI void eina_module_list_load(Eina_List *list);
EAPI void eina_module_list_unload(Eina_List *list);
EAPI void eina_module_list_free(Eina_List *list);

/** @} */

#endif /*EINA_MODULE_H_*/
