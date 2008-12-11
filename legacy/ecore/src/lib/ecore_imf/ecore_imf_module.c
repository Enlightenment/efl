/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

#include "config.h"

#include "ecore_imf_private.h"

static void _ecore_imf_module_load_all(void);
static void _ecore_imf_module_append(Ecore_Plugin *plugin, const Ecore_IMF_Context_Info *info, Ecore_IMF_Context *(*imf_module_create)(void));
static void _ecore_imf_module_free(Ecore_IMF_Module *module);
static int _ecore_imf_modules_exists(const char *ctx_id);

typedef struct _Ecore_IMF_Selector
{
  const char	*toselect;
  void		*selected;
} Ecore_IMF_Selector;

static Ecore_Path_Group *ecore_imf_modules_path = NULL;
static Eina_Hash *modules = NULL;

void
ecore_imf_module_init(void)
{
   char pathname[PATH_MAX];
   const char *homedir;

   ecore_imf_modules_path = ecore_path_group_new();
   snprintf(pathname, sizeof(pathname), "%s/ecore/immodules/",
	    PACKAGE_LIB_DIR);
   ecore_path_group_add(ecore_imf_modules_path, pathname);

   homedir = getenv("HOME");
   if (homedir)
     {
	snprintf(pathname, sizeof(pathname), "%s/.ecore/immodules/",
		 homedir);
	ecore_path_group_add(ecore_imf_modules_path, pathname);
     }

   modules = NULL;
   _ecore_imf_module_load_all();
}

void
ecore_imf_module_shutdown(void)
{
   if (modules)
     {
	eina_hash_free(modules);
	modules = NULL;
     }

   ecore_path_group_del(ecore_imf_modules_path);
   ecore_imf_modules_path = NULL;
}

static Eina_Bool
_hash_module_available_get(const Eina_Hash *hash, int *data, void *list)
{
  ecore_list_append(list, data);
  return EINA_TRUE;
}

Ecore_List *
ecore_imf_module_available_get(void)
{
   Ecore_List *values;
   Eina_Iterator *it = NULL;
   unsigned int i = 0;

   if (!modules) return NULL;

   values = ecore_list_new();
   if (!values) return NULL;

   it = eina_hash_iterator_data_new(modules);
   if (!it)
     {
       ecore_list_destroy(values);
       return NULL;
     }

   eina_iterator_foreach(it, EINA_EACH(_hash_module_available_get), values);
   eina_iterator_free(it);

   ecore_list_first_goto(values);

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
_hash_ids_get(const Eina_Hash *hash, const char *key, void *list)
{
  ecore_list_append(list, key);
  return EINA_TRUE;
}

Ecore_List *
ecore_imf_module_context_ids_get(void)
{
   Ecore_List *l = NULL;
   Eina_Iterator *it = NULL;

   if (!modules) return NULL;

   l = ecore_list_new();
   if (!l) return NULL;

   it = eina_hash_iterator_key_new(modules);
   if (!it)
     {
       ecore_list_destroy(l);
       return NULL;
     }

   eina_iterator_foreach(it, EINA_EACH(_hash_ids_get), l);
   eina_iterator_free(it);

   return l;
}

static Eina_Bool
_hash_ids_by_canvas_type_get(const Eina_Hash *hash, int *data, void *fdata)
{
   Ecore_IMF_Module *module = data;
   Ecore_IMF_Selector *selector = fdata;

   if (!strcmp(module->info->canvas_type, selector->toselect))
     ecore_list_append(selector->selected, (void *)module->info->id);

   return EINA_TRUE;
}

Ecore_List *
ecore_imf_module_context_ids_by_canvas_type_get(const char *canvas_type)
{
   Ecore_IMF_Selector selector;
   Ecore_List *values;
   Eina_Iterator *it = NULL;
   unsigned int i = 0;

   if (!modules) return NULL;

   if (!canvas_type)
     return ecore_imf_module_context_ids_get();

   values = ecore_list_new();
   if (!values) return NULL;

   it = eina_hash_iterator_data_new(modules);
   if (!it)
     {
       ecore_list_destroy(values);
       return NULL;
     }

   selector.toselect = canvas_type;
   selector.selected = values;
   eina_iterator_foreach(it, EINA_EACH(_hash_ids_by_canvas_type_get), &selector);
   eina_iterator_free(it);

   ecore_list_first_goto(values);

   return values;
}

static void
_ecore_imf_module_load_all(void)
{
   Ecore_List *avail;
   char *filename;
   Ecore_Plugin *plugin;
   const Ecore_IMF_Context_Info *info = NULL;
   int (*imf_module_init)(const Ecore_IMF_Context_Info **info);
   Ecore_IMF_Context *(*imf_module_create)(void);

   avail = ecore_plugin_available_get(ecore_imf_modules_path);
   if (!avail) return;

   ecore_list_first_goto(avail);
   while ((filename = ecore_list_next(avail)))
     {
	plugin = ecore_plugin_load(ecore_imf_modules_path, filename, NULL);
	if (!plugin)
	  {
	     fprintf(stderr, "** ecore_imf: Error loading input method plugin %s!\n",
		     filename);
	     continue;
	  }

	imf_module_init = ecore_plugin_symbol_get(plugin, "imf_module_init");
	if (!imf_module_init || !imf_module_init(&info) || !info)
	  {
	     fprintf(stderr, "** ecore_imf: Error initializing input method plugin %s! "
			     "'imf_module_init' is missing or failed to run!",
		     filename);
	     ecore_plugin_unload(plugin);
	     continue;
	  }

	if (_ecore_imf_modules_exists(info->id))
	  {
	     fprintf(stderr, "** ecore_imf: Error loading input method plugin %s! "
			     "Plugin with id='%s' already exists!",
		     filename, info->id);
	     ecore_plugin_unload(plugin);
	     continue;
	  }

	imf_module_create = ecore_plugin_symbol_get(plugin, "imf_module_create");
	if (!imf_module_create)
	  {
	     fprintf(stderr, "** ecore_imf: Error setting up input method plugin %s! "
			     "'imf_module_create' is missing!",
		     filename);
	     ecore_plugin_unload(plugin);
	     continue;
	  }

	_ecore_imf_module_append(plugin, info, imf_module_create);
     }

   ecore_list_destroy(avail);
}

static void
_ecore_imf_module_append(Ecore_Plugin *plugin,
			const Ecore_IMF_Context_Info *info,
			Ecore_IMF_Context *(*imf_module_create)(void))
{
   Ecore_IMF_Module *module;

   if (!modules)
     modules = eina_hash_string_superfast_new(_ecore_imf_module_free);

   module = malloc(sizeof(Ecore_IMF_Module));
   module->plugin = plugin;
   module->info = info;
   /* cache imf_module_create as it may be used several times */
   module->create = imf_module_create;

   eina_hash_add(modules, info->id, module);
}

static void
_ecore_imf_module_free(Ecore_IMF_Module *module)
{
   int (*imf_module_exit)(void);

   imf_module_exit = ecore_plugin_symbol_get(module->plugin, "imf_module_exit");
   if (imf_module_exit) imf_module_exit();
   ecore_plugin_unload(module->plugin);
   free(module);
}

static int
_ecore_imf_modules_exists(const char *ctx_id)
{
   Eina_Iterator *it = NULL;

   if (!modules) return 0;
   if (!ctx_id) return 0;

   if (eina_hash_find(modules, ctx_id))
     return 1;

   return 0;
}
