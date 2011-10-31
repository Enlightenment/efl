#include "private.h"

typedef struct _Elm_Params_Pager
{
   Elm_Params base;
} Elm_Params_Pager;

static void
external_pager_state_set(void *data __UNUSED__, Evas_Object *obj __UNUSED__, const void *from_params __UNUSED__, const void *to_params __UNUSED__, float pos __UNUSED__)
{
}

static Eina_Bool
external_pager_param_set(void *data __UNUSED__, Evas_Object *obj __UNUSED__, const Edje_External_Param *param)
{

   ERR("unknown parameter '%s' of type '%s'",
       param->name, edje_external_param_type_str(param->type));

   return EINA_FALSE;
}

static Eina_Bool
external_pager_param_get(void *data __UNUSED__, const Evas_Object *obj __UNUSED__, Edje_External_Param *param)
{
   ERR("unknown parameter '%s' of type '%s'",
       param->name, edje_external_param_type_str(param->type));

   return EINA_FALSE;
}

static void *
external_pager_params_parse(void *data __UNUSED__, Evas_Object *obj __UNUSED__, const Eina_List *params __UNUSED__)
{
   Elm_Params_Pager *mem;

   mem = ELM_NEW(Elm_Params_Pager);
   if (!mem)
     return NULL;

   return mem;
}

static Evas_Object *external_pager_content_get(void *data __UNUSED__,
		const Evas_Object *obj __UNUSED__, const char *content __UNUSED__)
{
   ERR("No content.");
   return NULL;
}

static void
external_pager_params_free(void *params)
{
   Elm_Params_Pager *mem = params;

   free(mem);
}

static Edje_External_Param_Info external_pager_params[] = {
   DEFINE_EXTERNAL_COMMON_PARAMS,
   EDJE_EXTERNAL_PARAM_INFO_SENTINEL
};

DEFINE_EXTERNAL_ICON_ADD(pager, "pager")
DEFINE_EXTERNAL_TYPE_SIMPLE(pager, "Pager");
