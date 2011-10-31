#include "private.h"

typedef struct _Elm_Params_Actionslider
{
   Elm_Params base;
   const char *label;
} Elm_Params_Actionslider;

static void
external_actionslider_state_set(void *data __UNUSED__, Evas_Object *obj __UNUSED__, const void *from_params, const void *to_params, float pos __UNUSED__)
{
   const Elm_Params_Actionslider *p;

   if (to_params) p = to_params;
   else if (from_params) p = from_params;
   else return;

   if (p->label)
     elm_object_text_set(obj, p->label);
}

static Eina_Bool
external_actionslider_param_set(void *data __UNUSED__, Evas_Object *obj __UNUSED__, const Edje_External_Param *param)
{
   if ((param->type == EDJE_EXTERNAL_PARAM_TYPE_STRING)
       && (!strcmp(param->name, "label")))
     {
        elm_object_text_set(obj, param->s);
        return EINA_TRUE;
     }

   ERR("unknown parameter '%s' of type '%s'",
       param->name, edje_external_param_type_str(param->type));

   return EINA_FALSE;
}

static Eina_Bool
external_actionslider_param_get(void *data __UNUSED__, const Evas_Object *obj __UNUSED__, Edje_External_Param *param)
{
   if ((param->type == EDJE_EXTERNAL_PARAM_TYPE_STRING)
       && (!strcmp(param->name, "label")))
     {
        param->s = elm_object_text_get(obj);
        return EINA_TRUE;
     }

   ERR("unknown parameter '%s' of type '%s'",
       param->name, edje_external_param_type_str(param->type));

   return EINA_FALSE;
}

static void *
external_actionslider_params_parse(void *data __UNUSED__, Evas_Object *obj __UNUSED__, const Eina_List *params)
{
   Elm_Params_Actionslider *mem;
   Edje_External_Param *param;
   const Eina_List *l;

   mem = ELM_NEW(Elm_Params_Actionslider);
   if (!mem)
     return NULL;

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

static Evas_Object *external_actionslider_content_get(void *data __UNUSED__,
		const Evas_Object *obj __UNUSED__, const char *content __UNUSED__)
{
   ERR("No content.");
   return NULL;
}

static void
external_actionslider_params_free(void *params)
{
   Elm_Params_Actionslider *mem = params;
   if (mem->label)
     eina_stringshare_del(mem->label);
   free(mem);
}

static Edje_External_Param_Info external_actionslider_params[] = {
   DEFINE_EXTERNAL_COMMON_PARAMS,
   EDJE_EXTERNAL_PARAM_INFO_SENTINEL
};

DEFINE_EXTERNAL_ICON_ADD(actionslider, "actionslider")
DEFINE_EXTERNAL_TYPE_SIMPLE(actionslider, "Actionslider");
