#include "private.h"

typedef struct _Elm_Params_Index
{
   Elm_Params base;
   Eina_Bool active:1;
   Eina_Bool active_exists:1;

} Elm_Params_Index;

static void
external_index_state_set(void *data __UNUSED__, Evas_Object *obj, const void *from_params, const void *to_params, float pos __UNUSED__)
{
   const Elm_Params_Index *p;

   if (to_params) p = to_params;
   else if (from_params) p = from_params;
   else return;

   if (p->active_exists)
     elm_index_autohide_disabled_set(obj, p->active_exists);
}

static Eina_Bool
external_index_param_set(void *data __UNUSED__, Evas_Object *obj, const Edje_External_Param *param)
{
   if (!strcmp(param->name, "active"))
     {
        if (param->type == EDJE_EXTERNAL_PARAM_TYPE_BOOL)
          {
             elm_index_autohide_disabled_set(obj, param->i);
             return EINA_TRUE;
          }
     }
   ERR("unknown parameter '%s' of type '%s'",
       param->name, edje_external_param_type_str(param->type));

   return EINA_FALSE;
}

static Eina_Bool
external_index_param_get(void *data __UNUSED__, const Evas_Object *obj, Edje_External_Param *param)
{
   if (!strcmp(param->name, "active"))
     {
        if (param->type == EDJE_EXTERNAL_PARAM_TYPE_BOOL)
          {
             param->i = elm_index_autohide_disabled_get(obj);
             return EINA_TRUE;
          }
     }

   ERR("unknown parameter '%s' of type '%s'",
       param->name, edje_external_param_type_str(param->type));

   return EINA_FALSE;
}

static void *
external_index_params_parse(void *data __UNUSED__, Evas_Object *obj __UNUSED__, const Eina_List *params)
{
   Elm_Params_Index *mem;
   Edje_External_Param *param;
   const Eina_List *l;

   mem = calloc(1, sizeof(Elm_Params_Index));
   if (!mem)
     return NULL;

   EINA_LIST_FOREACH(params, l, param)
     {
        if (!strcmp(param->name, "active"))
          {
             mem->active = !!param->i;
             mem->active_exists = EINA_TRUE;
          }
     }

   return mem;
}

static Evas_Object *external_index_content_get(void *data __UNUSED__,
		const Evas_Object *obj __UNUSED__, const char *content __UNUSED__)
{
   ERR("No content.");
   return NULL;
}

static void
external_index_params_free(void *params)
{
   Elm_Params_Index *mem = params;
   free(mem);
}

static Edje_External_Param_Info external_index_params[] = {
   DEFINE_EXTERNAL_COMMON_PARAMS,
    EDJE_EXTERNAL_PARAM_INFO_BOOL("active"),
   EDJE_EXTERNAL_PARAM_INFO_SENTINEL
};

DEFINE_EXTERNAL_ICON_ADD(index, "index");
DEFINE_EXTERNAL_TYPE_SIMPLE(index, "index");
