#include "private.h"

typedef struct _Elm_Params_Naviframe
{
   Elm_Params base;
   Eina_Bool preserve_on_pop:1;
   Eina_Bool preserve_on_pop_exists:1;
   Eina_Bool prev_btn_auto_push:1;
   Eina_Bool prev_btn_auto_push_exists:1;
} Elm_Params_Naviframe;

static void
external_naviframe_state_set(void *data __UNUSED__, Evas_Object *obj, const void *from_params,
                             const void *to_params, float pos __UNUSED__)
{
   const Elm_Params_Naviframe *p;

   if (to_params) p = to_params;
   else if (from_params) p = from_params;
   else return;

   if (p->preserve_on_pop_exists)
     elm_naviframe_content_preserve_on_pop_set(obj, p->preserve_on_pop);
   if (p->prev_btn_auto_push_exists)
     elm_naviframe_prev_btn_auto_pushed_set(obj, p->prev_btn_auto_push);
}

static Eina_Bool
external_naviframe_param_set(void *data __UNUSED__, Evas_Object *obj,
                             const Edje_External_Param *param)
{
   if (!strcmp(param->name, "preserve on pop"))
     {
        if (param->type == EDJE_EXTERNAL_PARAM_TYPE_BOOL)
          {
             elm_naviframe_content_preserve_on_pop_set(obj, param->i);
             return EINA_TRUE;
          }
     }
   else if (!strcmp(param->name, "prev btn auto push"))
     {
        if (param->type == EDJE_EXTERNAL_PARAM_TYPE_BOOL)
          {
             elm_naviframe_prev_btn_auto_pushed_set(obj, param->i);
             return EINA_TRUE;
          }
     }

   ERR("unknown parameter '%s' of type '%s'",
       param->name, edje_external_param_type_str(param->type));

   return EINA_FALSE;
}

static Eina_Bool
external_naviframe_param_get(void *data __UNUSED__, const Evas_Object *obj,
                             Edje_External_Param *param)
{
   if (!strcmp(param->name, "preserve on pop"))
     {
        if (param->type == EDJE_EXTERNAL_PARAM_TYPE_BOOL)
          {
             param->i = elm_naviframe_content_preserve_on_pop_get(obj);
             return EINA_TRUE;
          }
     }
   else if (!strcmp(param->name, "prev btn auto push"))
     {
        if (param->type == EDJE_EXTERNAL_PARAM_TYPE_BOOL)
          {
             param->i = elm_naviframe_prev_btn_auto_pushed_get(obj);
             return EINA_TRUE;
          }
     }

   ERR("unknown parameter '%s' of type '%s'",
       param->name, edje_external_param_type_str(param->type));

   return EINA_FALSE;
}

static void *
external_naviframe_params_parse(void *data __UNUSED__, Evas_Object *obj __UNUSED__,
                                const Eina_List *params)
{
   Elm_Params_Naviframe *mem;
   Edje_External_Param *param;
   const Eina_List *l;

   mem = ELM_NEW(Elm_Params_Naviframe);
   if (!mem)
     return NULL;

   EINA_LIST_FOREACH(params, l, param)
     {
        if (!strcmp(param->name, "preserve on pop"))
          {
             mem->preserve_on_pop = !!param->i;
             mem->preserve_on_pop_exists = EINA_TRUE;
          }
        else if (!strcmp(param->name, "prev btn auto push"))
          {
             mem->prev_btn_auto_push = !!param->i;
             mem->prev_btn_auto_push_exists = EINA_TRUE;
          }
     }

   return mem;
}

static Evas_Object *external_naviframe_content_get(void *data __UNUSED__,
      const Evas_Object *obj __UNUSED__, const char *content __UNUSED__)
{
   ERR("No content.");
   return NULL;
}

static void
external_naviframe_params_free(void *params)
{
   Elm_Params_Naviframe *mem = params;
   free(mem);
}

static Edje_External_Param_Info external_naviframe_params[] = {
   DEFINE_EXTERNAL_COMMON_PARAMS,
   EDJE_EXTERNAL_PARAM_INFO_BOOL("preserve on pop"),
   EDJE_EXTERNAL_PARAM_INFO_BOOL("prev btn auto push"),
   EDJE_EXTERNAL_PARAM_INFO_SENTINEL
};

DEFINE_EXTERNAL_ICON_ADD(naviframe, "naviframe");
DEFINE_EXTERNAL_TYPE_SIMPLE(naviframe, "Naviframe");
