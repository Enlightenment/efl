#include "private.h"

typedef struct _Elm_Params_Multibuttonentry
{
   const char *label;
   const char *guide_text;
} Elm_Params_Multibuttonentry;

static void
external_multibuttonentry_state_set(void *data __UNUSED__, Evas_Object *obj, const void *from_params, const void *to_params, float pos __UNUSED__)
{
   const Elm_Params_Multibuttonentry *p;

   if (to_params) p = to_params;
   else if (from_params) p = from_params;
   else return;

   if (p->label)
     elm_object_text_set(obj, p->label);
   if (p->guide_text)
     elm_object_part_text_set(obj, "guide", p->guide_text);
}

static Eina_Bool
external_multibuttonentry_param_set(void *data __UNUSED__, Evas_Object *obj, const Edje_External_Param *param)
{
   if (!strcmp(param->name, "label"))
     {
        if (param->type == EDJE_EXTERNAL_PARAM_TYPE_STRING)
          {
             elm_object_text_set(obj, param->s);
             return EINA_TRUE;
          }
     }
   else if (!strcmp(param->name, "guide text"))
     {
        if (param->type == EDJE_EXTERNAL_PARAM_TYPE_STRING)
          {
             elm_object_part_text_set(obj, "guide", param->s);
             return EINA_TRUE;
          }
     }

   ERR("unknown parameter '%s' of type '%s'",
       param->name, edje_external_param_type_str(param->type));

   return EINA_FALSE;
}

static Eina_Bool
external_multibuttonentry_param_get(void *data __UNUSED__, const Evas_Object *obj, Edje_External_Param *param)
{
   if (!strcmp(param->name, "label"))
     {
        if (param->type == EDJE_EXTERNAL_PARAM_TYPE_STRING)
          {
             param->s = elm_object_text_get(obj);
             return EINA_TRUE;
          }
     }
   else if (!strcmp(param->name, "guide text"))
     {
        if (param->type == EDJE_EXTERNAL_PARAM_TYPE_STRING)
          {
             param->s = elm_object_part_text_get(obj, "guide");
             return EINA_TRUE;
          }
     }

   ERR("unknown parameter '%s' of type '%s'",
       param->name, edje_external_param_type_str(param->type));

   return EINA_FALSE;
}

static void *
external_multibuttonentry_params_parse(void *data __UNUSED__, Evas_Object *obj __UNUSED__, const Eina_List *params)
{
   Elm_Params_Multibuttonentry *mem = NULL;
   Edje_External_Param *param;
   const Eina_List *l;

   //mem = external_common_params_parse(Elm_Params_Multibuttonentry, data, obj, params);
   if (!mem)
     return NULL;

   EINA_LIST_FOREACH(params, l, param)
     {
        if (!strcmp(param->name, "label"))
          mem->label = eina_stringshare_add(param->s);
        else if (!strcmp(param->name, "guide text"))
          mem->guide_text = eina_stringshare_add(param->s);
     }

   return mem;
}

static Evas_Object *external_multibuttonentry_content_get(void *data __UNUSED__, const Evas_Object *obj __UNUSED__, const char *content __UNUSED__)
{
   ERR("so content");
   return NULL;
}

static void
external_multibuttonentry_params_free(void *params)
{
   Elm_Params_Multibuttonentry *mem = params;

   if (mem->label)
     eina_stringshare_del(mem->label);
   if (mem->guide_text)
     eina_stringshare_del(mem->guide_text);
   external_common_params_free(params);
}

static Edje_External_Param_Info external_multibuttonentry_params[] = {
   DEFINE_EXTERNAL_COMMON_PARAMS,
   EDJE_EXTERNAL_PARAM_INFO_STRING("label"),
   EDJE_EXTERNAL_PARAM_INFO_STRING("guide text"),
   EDJE_EXTERNAL_PARAM_INFO_SENTINEL
};

DEFINE_EXTERNAL_ICON_ADD(multibuttonentry, "multibuttonentry")
DEFINE_EXTERNAL_TYPE_SIMPLE(multibuttonentry, "Multibuttonentry")

