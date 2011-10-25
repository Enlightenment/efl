#include "private.h"

typedef struct _Elm_Params_Button
{
   Elm_Params base;
   const char *label;
   Evas_Object *icon;
} Elm_Params_Button;

static void
external_button_state_set(void *data __UNUSED__, Evas_Object *obj, const void *from_params, const void *to_params, float pos __UNUSED__)
{
   const Elm_Params_Button *p;

   if (to_params) p = to_params;
   else if (from_params) p = from_params;
   else return;

   if (p->label)
     elm_object_text_set(obj, p->label);
   if (p->icon)
     elm_object_content_set(obj, p->icon);
}

static Eina_Bool
external_button_param_set(void *data __UNUSED__, Evas_Object *obj, const Edje_External_Param *param)
{
   if (!strcmp(param->name, "label"))
     {
	if (param->type == EDJE_EXTERNAL_PARAM_TYPE_STRING)
	  {
	     elm_object_text_set(obj, param->s);
	     return EINA_TRUE;
	  }
     }
   else if (!strcmp(param->name, "icon"))
     {
	if (param->type == EDJE_EXTERNAL_PARAM_TYPE_STRING)
	  {
	     Evas_Object *icon = external_common_param_icon_get(obj, param);
	     if ((strcmp(param->s, "")) && (!icon)) return EINA_FALSE;
	     elm_object_content_set(obj, icon);
	     return EINA_TRUE;
	  }
     }

   ERR("unknown parameter '%s' of type '%s'",
       param->name, edje_external_param_type_str(param->type));

   return EINA_FALSE;
}

static Eina_Bool
external_button_param_get(void *data __UNUSED__, const Evas_Object *obj, Edje_External_Param *param)
{
   if (!strcmp(param->name, "label"))
     {
	if (param->type == EDJE_EXTERNAL_PARAM_TYPE_STRING)
	  {
	     param->s = elm_object_text_get(obj);
	     return EINA_TRUE;
	  }
     }
   else if (!strcmp(param->name, "icon"))
     {
	/* not easy to get icon name back from live object */
	return EINA_FALSE;
     }

   ERR("unknown parameter '%s' of type '%s'",
       param->name, edje_external_param_type_str(param->type));

   return EINA_FALSE;
}

static void *
external_button_params_parse(void *data __UNUSED__, Evas_Object *obj, const Eina_List *params)
{
   Elm_Params_Button *mem;
   Edje_External_Param *param;
   const Eina_List *l;

   mem = ELM_NEW(Elm_Params_Button);
   if (!mem)
     return NULL;

   external_common_icon_param_parse(&mem->icon, obj, params);

   EINA_LIST_FOREACH(params, l, param)
     {
        if (!strcmp(param->name, "label"))
          {
             mem->label = eina_stringshare_add(param->s);
             break;
          }
     }

   return mem;
}

static Evas_Object *external_button_content_get(void *data __UNUSED__,
		const Evas_Object *obj __UNUSED__, const char *content __UNUSED__)
{
	ERR("No content.");
	return NULL;
}

static void
external_button_params_free(void *params)
{
   Elm_Params_Button *mem = params;
   if (mem->label)
      eina_stringshare_del(mem->label);
   free(params);
}

static Edje_External_Param_Info external_button_params[] = {
   DEFINE_EXTERNAL_COMMON_PARAMS,
   EDJE_EXTERNAL_PARAM_INFO_STRING("label"),
   EDJE_EXTERNAL_PARAM_INFO_STRING("icon"),
   EDJE_EXTERNAL_PARAM_INFO_SENTINEL
};

DEFINE_EXTERNAL_ICON_ADD(button, "button");
DEFINE_EXTERNAL_TYPE_SIMPLE(button, "Button");
