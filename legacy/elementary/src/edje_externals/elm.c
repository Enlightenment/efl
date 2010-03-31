#include "Elementary.h"
#include "private.h"

int _elm_log_dom = -1;

void
external_signal(void *data __UNUSED__, Evas_Object *obj __UNUSED__, const char *signal, const char *source)
{
   printf("External Signal received: '%s' '%s'\n", signal, source);
}

const char *
external_translate(void *data __UNUSED__, const char *orig)
{
   // in future, mark all params as translatable and use dgettext()
   // with "elementary" text domain here.
   return orig;
}

typedef struct {
   const char *emission;
   const char *source;
   Evas_Object *edje;
} Elm_External_Signals_Proxy_Context;

static void
_external_signal_proxy_free_cb(void *data, Evas *e __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Elm_External_Signals_Proxy_Context *ctxt = data;
   free(ctxt);
}

static void
_external_signal_proxy_cb(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Elm_External_Signals_Proxy_Context *ctxt = data;
   // TODO: Is it worth to check Evas_Smart_Cb_Description and do something
   // TODO: with event_info given its description?
   edje_object_signal_emit(ctxt->edje, ctxt->emission, ctxt->source);
}

void
external_signals_proxy(Evas_Object *obj, Evas_Object *edje, const char *part_name)
{
   const Evas_Smart_Cb_Description **cls_descs, **inst_descs;
   unsigned int cls_count, inst_count, total;
   Elm_External_Signals_Proxy_Context *ctxt;

   evas_object_smart_callbacks_descriptions_get
     (obj, &cls_descs, &cls_count, &inst_descs, &inst_count);

   total = cls_count + inst_count;
   if (!total) return;
   ctxt = malloc(sizeof(Elm_External_Signals_Proxy_Context) * total);
   if (!ctxt) return;
   evas_object_event_callback_add
     (obj, EVAS_CALLBACK_DEL, _external_signal_proxy_free_cb, ctxt);

   for (; cls_count > 0; cls_count--,  cls_descs++, ctxt++)
     {
	const Evas_Smart_Cb_Description *d = *cls_descs;
	ctxt->emission = d->name;
	ctxt->source = part_name;
	ctxt->edje = edje;
	evas_object_smart_callback_add
	  (obj, d->name, _external_signal_proxy_cb, ctxt);
     }

   for (; inst_count > 0; inst_count--,  inst_descs++, ctxt++)
     {
	const Evas_Smart_Cb_Description *d = *inst_descs;
	ctxt->emission = d->name;
	ctxt->source = part_name;
	ctxt->edje = edje;
	evas_object_smart_callback_add
	  (obj, d->name, _external_signal_proxy_cb, ctxt);
     }
}

void *
external_common_params_parse_internal(size_t params_size, void *data __UNUSED__, Evas_Object *obj __UNUSED__, const Eina_List *params)
{
   Elm_Params *p;
   const Eina_List *l;
   Edje_External_Param *param;

   if (params_size < sizeof(Elm_Params))
     return NULL;

   p = calloc(1, params_size);
   if (!p)
     return NULL;

   EINA_LIST_FOREACH(params, l, param)
     {
	if (!strcmp(param->name, "label"))
	  p->label = param->s;
     }
   return p;
}

Evas_Object *
external_common_param_icon_get(Evas_Object *obj, const Edje_External_Param *p)
{
   Evas_Object *edje, *parent_widget, *icon;
   const char *file;

   if ((!p) || (!p->s) || (p->type != EDJE_EXTERNAL_PARAM_TYPE_STRING))
       return NULL;

   edje = evas_object_smart_parent_get(obj);
   edje_object_file_get(edje, &file, NULL);

   parent_widget = elm_widget_parent_widget_get(obj);
   if (!parent_widget)
     parent_widget = edje;
   icon = elm_icon_add(parent_widget);

   if (elm_icon_file_set(icon, file, p->s))
     return icon;
   if (elm_icon_standard_set(icon, p->s))
     return icon;

   evas_object_del(icon);
   return NULL;
}

void
external_common_icon_param_parse(Evas_Object **icon, Evas_Object *obj, const Eina_List *params)
{
   Edje_External_Param *p = edje_external_param_find(params, "icon");
   *icon = external_common_param_icon_get(obj, p);
}

Evas_Object *
external_common_param_edje_object_get(Evas_Object *obj, const Edje_External_Param *p)
{
   Evas_Object *edje, *parent_widget, *ret;
   const char *file;

   if ((!p) || (!p->s) || (p->type != EDJE_EXTERNAL_PARAM_TYPE_STRING))
       return NULL;

   edje = evas_object_smart_parent_get(obj);
   edje_object_file_get(edje, &file, NULL);

   parent_widget = elm_widget_parent_widget_get(obj);
   if (!parent_widget)
     parent_widget = edje;

   ret = edje_object_add(evas_object_evas_get(parent_widget));

   if (edje_object_file_set(ret, file, p->s))
     return ret;

   evas_object_del(ret);
   return NULL;
}

void
external_common_params_free(void *params)
{
   Elm_Params *p = params;
   free(p);
};

#define DEFINE_TYPE(type_name) \
  extern const Edje_External_Type external_##type_name##_type;
#include "modules.inc"
#undef DEFINE_TYPE

static Edje_External_Type_Info elm_external_types[] =
{
#define DEFINE_TYPE(type_name)              \
  {"elm/"#type_name, &external_##type_name##_type},
#include "modules.inc"
#undef DEFINE_TYPE
   {NULL, NULL}
};

static Eina_Bool
elm_mod_init(void)
{
   int argc = 0;
   char **argv = NULL;

   ecore_app_args_get(&argc, &argv);
   elm_init(argc, argv);

   _elm_log_dom = eina_log_domain_register("elm-externals", EINA_COLOR_LIGHTBLUE);
   edje_external_type_array_register(elm_external_types);
   return EINA_TRUE;
}

static void
elm_mod_shutdown(void)
{
   edje_external_type_array_unregister(elm_external_types);

   eina_log_domain_unregister(_elm_log_dom);
   _elm_log_dom = -1;

   elm_shutdown();
}

EINA_MODULE_INIT(elm_mod_init);
EINA_MODULE_SHUTDOWN(elm_mod_shutdown);
