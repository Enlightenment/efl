#include "private.h"

typedef struct _Elm_Params_Segment_Control
{
   Elm_Params base;
} Elm_Params_Segment_Control;

static void
external_segment_control_state_set(void *data __UNUSED__,
                                   Evas_Object *obj __UNUSED__,
                                   const void *from_params __UNUSED__,
                                   const void *to_params __UNUSED__,
                                   float pos __UNUSED__)
{
   /* FIXME: no params, no setting */
}

static Eina_Bool
external_segment_control_param_set(void *data __UNUSED__, Evas_Object *obj __UNUSED__, const Edje_External_Param *param)
{
   ERR("unknown parameter '%s' of type '%s'",
       param->name, edje_external_param_type_str(param->type));

   return EINA_FALSE;
}

static Eina_Bool
external_segment_control_param_get(void *data __UNUSED__, const Evas_Object *obj __UNUSED__, Edje_External_Param *param)
{
   ERR("unknown parameter '%s' of type '%s'",
       param->name, edje_external_param_type_str(param->type));

   return EINA_FALSE;
}

static void *
external_segment_control_params_parse(void *data __UNUSED__, Evas_Object *obj __UNUSED__, const Eina_List *params __UNUSED__)
{
   Elm_Params_Segment_Control *mem;
   //Edje_External_Param *param;
   //const Eina_List *l;

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
