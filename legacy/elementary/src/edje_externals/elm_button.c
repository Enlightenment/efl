#include "private.h"

typedef struct _Elm_Params_Button
{
   Elm_Params base;
   Evas_Object *icon;
} Elm_Params_Button;

static void
external_button_state_set(void *data, Evas_Object *obj, const void *from_params, const void *to_params, float pos)
{
   const Elm_Params_Button *p1 = from_params, *p2 = to_params;

   p1 = from_params;
   p2 = to_params;

   if (!p2)
     {
   elm_button_label_set(obj, p1->base.label);
   elm_button_icon_set(obj, p1->icon);
   return;
     }

   elm_button_label_set(obj, p2->base.label);
   elm_button_icon_set(obj, p2->icon);
}

static void *
external_button_params_parse(void *data, Evas_Object *obj, const Eina_List *params)
{
   Elm_Params_Button *mem;
   Edje_External_Param *param;

   mem = external_common_params_parse(Elm_Params_Button, data, obj, params);
   if (!mem)
     return NULL;

   external_common_icon_param_parse(&mem->icon, obj, params);

   return mem;
}

static void
external_button_params_free(void *params)
{
   Elm_Params_Button *mem = params;

   if (mem->icon)
     evas_object_del(mem->icon);
   external_common_params_free(params);
}

static Edje_External_Param_Info external_button_params[] = {
   DEFINE_EXTERNAL_COMMON_PARAMS,
   EDJE_EXTERNAL_PARAM_INFO_STRING("icon"),
   EDJE_EXTERNAL_PARAM_INFO_SENTINEL
};

DEFINE_EXTERNAL_TYPE_SIMPLE(button, "Button");
