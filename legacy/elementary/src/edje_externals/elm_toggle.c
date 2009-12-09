#include <string.h>

#include "private.h"

typedef struct _Elm_Params_Toggle
{
   Elm_Params base;
   Evas_Object *icon;
   const char *st_label_from, *st_label_to;
   int state;
} Elm_Params_Toggle;

static void
external_toggle_state_set(void *data, Evas_Object *obj, const void *from_params, const void *to_params, float pos)
{
   const Elm_Params_Toggle *p1 = from_params, *p2 = to_params;

   p1 = from_params;
   p2 = to_params;

   if (!p2)
     {
	elm_toggle_label_set(obj, p1->base.label);
	elm_toggle_icon_set(obj, p1->icon);
	elm_toggle_states_labels_set(obj, p1->st_label_from, p1->st_label_to);
	elm_toggle_state_set(obj, p1->state);
	return;
     }

   elm_toggle_label_set(obj, p2->base.label);
   elm_toggle_icon_set(obj, p2->icon);
   elm_toggle_states_labels_set(obj, p2->st_label_from, p2->st_label_to);
   elm_toggle_state_set(obj, p2->state);
}

static void *
external_toggle_params_parse(void *data, Evas_Object *obj, const Eina_List *params)
{
   Elm_Params_Toggle *mem;
   Edje_External_Param *param;
   const Eina_List *l;

   mem = external_common_params_parse(Elm_Params_Toggle, data, obj, params);
   if (!mem)
     return NULL;

   external_common_icon_param_parse(&mem->icon, obj, params);

   EINA_LIST_FOREACH(params, l, param)
     {
	if (!strcmp(param->name, "state"))
	  mem->state = param->i;
	else if (!strcmp(param->name, "label on"))
	  mem->st_label_from = eina_stringshare_add(param->s);
	else if (!strcmp(param->name, "label off"))
	  mem->st_label_to = eina_stringshare_add(param->s);
     }

   return mem;
}

static void
external_toggle_params_free(void *params)
{
   Elm_Params_Toggle *mem = params;

   if (mem->icon)
     evas_object_del(mem->icon);
   if (mem->st_label_from)
     eina_stringshare_del(mem->st_label_from);
   if (mem->st_label_to)
     eina_stringshare_del(mem->st_label_to);
   external_common_params_free(params);
}

static Edje_External_Param_Info external_toggle_params[] = {
   DEFINE_EXTERNAL_COMMON_PARAMS,
   EDJE_EXTERNAL_PARAM_INFO_STRING("icon"),
   EDJE_EXTERNAL_PARAM_INFO_STRING_DEFAULT("label on", "ON"),
   EDJE_EXTERNAL_PARAM_INFO_STRING_DEFAULT("label off", "OFF"),
   EDJE_EXTERNAL_PARAM_INFO_BOOL("state"),
   EDJE_EXTERNAL_PARAM_INFO_SENTINEL
};

DEFINE_EXTERNAL_TYPE_SIMPLE(toggle, "Toggle")
