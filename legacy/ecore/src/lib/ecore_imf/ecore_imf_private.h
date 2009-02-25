#ifndef _ECORE_IMF_PRIVATE_H
#define _ECORE_IMF_PRIVATE_H

#include "Ecore_IMF.h"

#include "ecore_private.h"

#define ECORE_MAGIC_CONTEXT 0x56c1b39a

typedef struct _Ecore_IMF_Module Ecore_IMF_Module;

struct _Ecore_IMF_Context
{
   ECORE_MAGIC;
   const Ecore_IMF_Module        *module;
   const Ecore_IMF_Context_Class *klass;
   void                          *data;
   int                            input_mode;
   int                          (*retrieve_surrounding_func)(void *data, Ecore_IMF_Context *ctx, char **text, int *cursor_pos);
   void                          *retrieve_surrounding_data;
};

struct _Ecore_IMF_Module
{
   Ecore_Plugin                 *plugin;
   const Ecore_IMF_Context_Info *info;
   Ecore_IMF_Context            *(*create)(void);
};

void               ecore_imf_module_init(void);
void               ecore_imf_module_shutdown(void);
Eina_List         *ecore_imf_module_available_get(void);
Ecore_IMF_Module  *ecore_imf_module_get(const char *ctx_id);
Ecore_IMF_Context *ecore_imf_module_context_create(const char *ctx_id);
Eina_List         *ecore_imf_module_context_ids_get(void);
Eina_List         *ecore_imf_module_context_ids_by_canvas_type_get(const char *canvas_type);

#endif
