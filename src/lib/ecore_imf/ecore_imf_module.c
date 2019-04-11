#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <unistd.h>

#include <Ecore.h>
#include <ecore_private.h>

#include "Ecore_IMF.h"
#include "ecore_imf_private.h"

#include "../../static_libs/buildsystem/buildsystem.h"

static void _ecore_imf_module_free(Ecore_IMF_Module *module);
static int _ecore_imf_modules_exists(const char *ctx_id);

typedef struct _Ecore_IMF_Selector
{
   const char *toselect;
   void       *selected;
} Ecore_IMF_Selector;

static Eina_Hash *modules = NULL;
static Eina_Array *module_list = NULL;
static Eina_Prefix *pfx = NULL;

void
ecore_imf_module_init(void)
{
   const char *built_modules[] = {
#ifdef BUILD_ECORE_IMF_XIM
      "xim",
#endif
#ifdef BUILD_ECORE_IMF_IBUS
      "ibus",
#endif
#ifdef BUILD_ECORE_IMF_SCIM
      "scim",
#endif
#ifdef BUILD_ECORE_IMF_WAYLAND
      "wayland",
#endif
      NULL
   };
   const char *env;
   char buf[PATH_MAX] = "";

   pfx = eina_prefix_new(NULL, ecore_imf_init,
                         "ECORE_IMF", "ecore_imf", "checkme",
                         PACKAGE_BIN_DIR, PACKAGE_LIB_DIR,
                         PACKAGE_DATA_DIR, PACKAGE_DATA_DIR);
#ifdef NEED_RUN_IN_TREE
#if defined(HAVE_GETUID) && defined(HAVE_GETEUID)
   if (getuid() == geteuid())
#endif
     {
        if (getenv("EFL_RUN_IN_TREE"))
          {
             struct stat st;
             snprintf(buf, sizeof(buf), "%s/src/modules/ecore_imf",
                      PACKAGE_BUILD_DIR);
             if (stat(buf, &st) == 0)
               {
                  const char **itr;
                  const char **modules_load;
                  const char *modules_one[2] = { NULL, NULL };

                  modules_load = built_modules;
                  env = getenv("ECORE_IMF_MODULE");
                  if ((env) && (env[0]))
                    {
                       modules_one[0] = env;
                       modules_load = modules_one;
                    }
                  for (itr = modules_load; *itr != NULL; itr++)
                    {
                       bs_mod_dir_get(buf, sizeof(buf), "ecore_imf", *itr);
                       module_list = eina_module_list_get
                         (module_list, buf, EINA_FALSE, NULL, NULL);
                    }

                  if (module_list) eina_module_list_load(module_list);
                  return;
               }
          }
     }
#endif

   env = getenv("ECORE_IMF_MODULE");
#ifdef BUILD_ECORE_IMF_WAYLAND
   // if not set and we are sure we're on wayland....
   if ((!env) && (getenv("WAYLAND_DISPLAY")) && (!getenv("DISPLAY")))
     env = "wayland";
#endif
#ifdef ENABLE_XIM
   if ((!env) && (!getenv("WAYLAND_DISPLAY")) && (getenv("DISPLAY")))
     env = "xim";
#endif
   if ((env) && (env[0]))
     {
        const char **itr;
        Eina_Bool ok = EINA_FALSE;

        for (itr = built_modules; *itr != NULL; itr++)
          {
             if (!strcmp(env, *itr))
               {
                  ok = EINA_TRUE;
                  break;
               }
          }
        if (ok)
          {
             Eina_Module *m;

             snprintf(buf, sizeof(buf),
                      "%s/ecore_imf/modules/%s/%s/module" SHARED_LIB_SUFFIX,
                      eina_prefix_lib_get(pfx), env, MODULE_ARCH);
             m = eina_module_new(buf);
             if (m)
               {
                  module_list = eina_array_new(1);
                  if (module_list) eina_array_push(module_list, m);
                  else eina_module_free(m);
               }
          }
     }
   else
     {
        Eina_Module *m;
        const char **itr;

        for (itr = built_modules; *itr != NULL; itr++)
          {
             snprintf(buf, sizeof(buf),
                      "%s/ecore_imf/modules/%s/%s/module" SHARED_LIB_SUFFIX,
                      eina_prefix_lib_get(pfx), *itr, MODULE_ARCH);

             m = eina_module_new(buf);
             if (m)
               {
                  module_list = eina_array_new(1);
                  if (module_list)
                    {
                       eina_array_push(module_list, m);
                       break;
                    }
                  else
                    eina_module_free(m);
               }
          }
     }

   if (module_list) eina_module_list_load(module_list);
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

   eina_prefix_free(pfx);
   pfx = NULL;
}

static Eina_Bool
_hash_module_available_get(const Eina_Hash *hash EINA_UNUSED, int *data, void *list)
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
        if (!(ctx = module->create())) return NULL;
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
_hash_ids_get(const Eina_Hash *hash EINA_UNUSED, const char *key, void *list)
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
_hash_ids_by_canvas_type_get(const Eina_Hash *hash EINA_UNUSED, void *data, void *fdata)
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

   if (!info || _ecore_imf_modules_exists(info->id)) return;

   if (!modules)
     modules = eina_hash_string_superfast_new(EINA_FREE_CB(_ecore_imf_module_free));

   module = malloc(sizeof(Ecore_IMF_Module));
   EINA_SAFETY_ON_NULL_RETURN(module);

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
