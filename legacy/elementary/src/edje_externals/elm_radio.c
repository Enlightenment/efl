#include "private.h"

typedef struct _Elm_Params_Radio
{
   Elm_Params base;
   Evas_Object *icon;
   const char* group_name;
   int state;
} Elm_Params_Radio;

static void
external_radio_state_set(void *data, Evas_Object *obj, const void *from_params, const void *to_params, float pos)
{
   const Elm_Params_Radio *p1 = from_params, *p2 = to_params;

   p1 = from_params;
   p2 = to_params;

   if (!p2)
     {
	elm_radio_label_set(obj, p1->base.label);
	elm_radio_icon_set(obj, p1->icon);
	elm_radio_state_value_set(obj, p1->state);
	if (p1->group_name)
	  {
	     Evas_Object *ed = evas_object_smart_parent_get(obj);
	     Evas_Object *group = edje_object_part_swallow_get(ed, p1->group_name);
	     if (group)
	       elm_radio_group_add(obj, group);
	  }
	return;
     }

   elm_radio_label_set(obj, p2->base.label);
   elm_radio_icon_set(obj, p2->icon);
   elm_radio_state_value_set(obj, p2->state);
   if (p2->group_name)
     {
	Evas_Object *ed = evas_object_smart_parent_get(obj);
	Evas_Object *group = edje_object_part_swallow_get(ed, p2->group_name);
	elm_radio_group_add(obj, group);
     }
}

static void *
external_radio_params_parse(void *data, Evas_Object *obj, const Eina_List *params)
{
   Elm_Params_Radio *mem;
   Edje_External_Param *param;
   const Eina_List *l;

   mem = external_common_params_parse(Elm_Params_Radio, data, obj, params);
   if (!mem)
     return NULL;

   external_common_icon_param_parse(&mem->icon, obj, params);

   EINA_LIST_FOREACH(params, l, param)
     {
	if (!strcmp(param->name, "group"))
	  mem->group_name = eina_stringshare_add(param->s);
	else if (!strcmp(param->name, "value"))
	  mem->state = param->i;
     }

   return mem;
}

static void
external_radio_params_free(void *params)
{
   Elm_Params_Radio *mem = params;

   if (mem->icon)
     evas_object_del(mem->icon);
   if (mem->group_name)
     eina_stringshare_del(mem->group_name);
   external_common_params_free(params);
}

static Edje_External_Param_Info external_radio_params[] = {
   DEFINE_EXTERNAL_COMMON_PARAMS,
   EDJE_EXTERNAL_PARAM_INFO_STRING("icon"),
   EDJE_EXTERNAL_PARAM_INFO_STRING("group"),
   EDJE_EXTERNAL_PARAM_INFO_INT("value"),
   EDJE_EXTERNAL_PARAM_INFO_SENTINEL
};

DEFINE_EXTERNAL_ICON_ADD(radio, "radio")
DEFINE_EXTERNAL_TYPE_SIMPLE(radio, "Radio")
