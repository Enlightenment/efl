#include "Elementary.h"
#include "private.h"

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
   elm_icon_add(parent_widget);

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
   edje_external_type_array_register(elm_external_types);
   return EINA_TRUE;
}

static void
elm_mod_shutdown(void)
{
   edje_external_type_array_unregister(elm_external_types);
}

EINA_MODULE_INIT(elm_mod_init);
EINA_MODULE_SHUTDOWN(elm_mod_shutdown);
