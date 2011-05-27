#include "private.h"

typedef struct _Elm_Params_Segment_Control
{
   Elm_Params base;
} Elm_Params_Segment_Control;

static void
external_segment_control_state_set(void *data __UNUSED__, Evas_Object *obj, const void *from_params, const void *to_params, float pos __UNUSED__)
{
   const Elm_Params_Segment_Control *p;

   if (to_params) p = to_params;
   else if (from_params) p = from_params;
   else return;
}

static Eina_Bool
external_segment_control_param_set(void *data __UNUSED__, Evas_Object *obj, const Edje_External_Param *param)
{
   ERR("unknown parameter '%s' of type '%s'",
       param->name, edje_external_param_type_str(param->type));

   return EINA_FALSE;
}

static Eina_Bool
external_segment_control_param_get(void *data __UNUSED__, const Evas_Object *obj, Edje_External_Param *param)
{
   ERR("unknown parameter '%s' of type '%s'",
       param->name, edje_external_param_type_str(param->type));

   return EINA_FALSE;
}

static void *
external_segment_control_params_parse(void *data __UNUSED__, Evas_Object *obj __UNUSED__, const Eina_List *params)
{
   Elm_Params_Segment_Control *mem;
   Edje_External_Param *param;
   const Eina_List *l;

   mem = calloc(1, sizeof(Elm_Params_Segment_Control));
   if (!mem)
     return NULL;

   /*
   EINA_LIST_FOREACH(params, l, param)
     {
     }
   */
   return mem;
}

static Evas_Object *external_segment_control_content_get(void *data __UNUSED__,
		const Evas_Object *obj __UNUSED__, const char *content __UNUSED__)
{
	ERR("No content.");
	return NULL;
}

static void
external_segment_control_params_free(void *params)
{
   Elm_Params_Segment_Control *mem = params;
   free(mem);
}

static Edje_External_Param_Info external_segment_control_params[] = {
   DEFINE_EXTERNAL_COMMON_PARAMS,
   EDJE_EXTERNAL_PARAM_INFO_SENTINEL
};

DEFINE_EXTERNAL_ICON_ADD(segment_control, "segment_control");
DEFINE_EXTERNAL_TYPE_SIMPLE(segment_control, "segment_control");
