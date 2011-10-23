#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#include <Ecore.h>
#include <ecore_private.h>

#include "Ecore_IMF.h"
#include "ecore_imf_private.h"

static void _ecore_imf_module_free(Ecore_IMF_Module *module);
static int _ecore_imf_modules_exists(const char *ctx_id);

typedef struct _Ecore_IMF_Selector
{
   const char *toselect;
   void       *selected;
} Ecore_IMF_Selector;

static Eina_Hash *modules = NULL;
static Eina_Array *module_list = NULL;

void
ecore_imf_module_init(void)
{
   char *homedir;

   module_list = eina_module_list_get(NULL, PACKAGE_LIB_DIR "/ecore/immodules", 0, NULL, NULL);
   homedir = eina_module_environment_path_get("HOME", "/.ecore/immodules");
   if (homedir)
     {
        module_list = eina_module_list_get(module_list, homedir, 0, NULL, NULL);
        free(homedir);
     }
   eina_module_list_load(module_list);
}

void
ecore_imf_module_shutdown(void)
{
   if (modules)
     {
        eina_hash_free(modules);
        modules = NULL;
     }
   if (module_list)
     {
        eina_module_list_free(module_list);
        eina_array_free(module_list);
        module_list = NULL;
     }
}

static Eina_Bool
_hash_module_available_get(const Eina_Hash *hash __UNUSED__, int *data, void *list)
{
   *(Eina_List**)list = eina_list_append(*(Eina_List**)list, data);
   return EINA_TRUE;
}

Eina_List *
ecore_imf_module_available_get(void)
{
   Eina_List *values = NULL;
   Eina_Iterator *it = NULL;

   if (!modules) return NULL;

   it = eina_hash_iterator_data_new(modules);
   if (!it)
     return NULL;

   eina_iterator_foreach(it, EINA_EACH_CB(_hash_module_available_get), &values);
   eina_iterator_free(it);

   return values;
}

Ecore_IMF_Module *
ecore_imf_module_get(const char *ctx_id)
{
   if (!modules) return NULL;
   return eina_hash_find(modules, ctx_id);
}

Ecore_IMF_Context *
ecore_imf_module_context_create(const char *ctx_id)
{
   Ecore_IMF_Module *module;
   Ecore_IMF_Context *ctx = NULL;

   if (!modules) return NULL;
   module = eina_hash_find(modules, ctx_id);
   if (module)
     {
        ctx = module->create();
        if (!ECORE_MAGIC_CHECK(ctx, ECORE_MAGIC_CONTEXT))
          {
             ECORE_MAGIC_FAIL(ctx, ECORE_MAGIC_CONTEXT,
                              "ecore_imf_module_context_create");
             return NULL;
          }
        ctx->module = module;
     }
   return ctx;
}

static Eina_Bool
_hash_ids_get(const Eina_Hash *hash __UNUSED__, const char *key, void *list)
{
   *(Eina_List**)list = eina_list_append(*(Eina_List**)list, key);
   return EINA_TRUE;
}

Eina_List *
ecore_imf_module_context_ids_get(void)
{
   Eina_List *l = NULL;
   Eina_Iterator *it = NULL;

   if (!modules) return NULL;

   it = eina_hash_iterator_key_new(modules);
   if (!it)
     return NULL;

   eina_iterator_foreach(it, EINA_EACH_CB(_hash_ids_get), &l);
   eina_iterator_free(it);

   return l;
}

static Eina_Bool
_hash_ids_by_canvas_type_get(const Eina_Hash *hash __UNUSED__, void *data, void *fdata)
{
   Ecore_IMF_Module *module = data;
   Ecore_IMF_Selector *selector = fdata;

   if (!strcmp(module->info->canvas_type, selector->toselect))
     selector->selected = eina_list_append(selector->selected, (void *)module->info->id);

   return EINA_TRUE;
}

Eina_List *
ecore_imf_module_context_ids_by_canvas_type_get(const char *canvas_type)
{
   Ecore_IMF_Selector selector;
   Eina_List *values = NULL;
   Eina_Iterator *it = NULL;

   if (!modules) return NULL;

   if (!canvas_type)
     return ecore_imf_module_context_ids_get();

   it = eina_hash_iterator_data_new(modules);
   if (!it)
     return NULL;

   selector.toselect = canvas_type;
   selector.selected = values;
   eina_iterator_foreach(it, EINA_EACH_CB(_hash_ids_by_canvas_type_get), &selector);
   eina_iterator_free(it);

   return values;
}

EAPI void
ecore_imf_module_register(const Ecore_IMF_Context_Info *info,
                          Ecore_IMF_Context *(*imf_module_create)(void),
                          Ecore_IMF_Context *(*imf_module_exit)(void))
{
   Ecore_IMF_Module *module;

   if (_ecore_imf_modules_exists(info->id)) return;

   if (!modules)
     modules = eina_hash_string_superfast_new(EINA_FREE_CB(_ecore_imf_module_free));

   module = malloc(sizeof(Ecore_IMF_Module));
   module->info = info;
   /* cache imf_module_create as it may be used several times */
   module->create = imf_module_create;
   module->exit = imf_module_exit;

   eina_hash_add(modules, info->id, module);
}

static void
_ecore_imf_module_free(Ecore_IMF_Module *module)
{
   if (module->exit) module->exit();
   free(module);
}

static int
_ecore_imf_modules_exists(const char *ctx_id)
{
   if (!modules) return 0;
   if (!ctx_id) return 0;

   if (eina_hash_find(modules, ctx_id))
     return 1;

   return 0;
}
