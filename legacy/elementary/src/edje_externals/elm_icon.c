#include "private.h"

typedef struct _Elm_Params_Icon
{
   Elm_Params base;
   const char *icon;
} Elm_Params_Icon;

static void
external_icon_state_set(void *data __UNUSED__, Evas_Object *obj, const void *from_params, const void *to_params, float pos __UNUSED__)
{
   const Elm_Params_Icon *p;
   Evas_Object *edje;
   const char *file;

   if (to_params) p = to_params;
   else if (from_params) p = from_params;
   else return;

   if (p->icon)
     {
        edje = evas_object_smart_parent_get(obj);
        edje_object_file_get(edje, &file, NULL);

        if (!elm_icon_file_set(obj, file, p->icon))
          elm_icon_standard_set(obj, p->icon);
     }
}

static Eina_Bool
external_icon_param_set(void *data __UNUSED__, Evas_Object *obj, const Edje_External_Param *param)
{
   Evas_Object *edje;
   const char *file;

   if (!strcmp(param->name, "icon"))
     {
        if (param->type == EDJE_EXTERNAL_PARAM_TYPE_STRING)
          {
             edje = evas_object_smart_parent_get(obj);
             edje_object_file_get(edje, &file, NULL);

             if (!elm_icon_file_set(obj, file, param->s))
               elm_icon_standard_set(obj, param->s);
             return EINA_TRUE;
          }
     }

   ERR("unknown parameter '%s' of type '%s'",
       param->name, edje_external_param_type_str(param->type));

   return EINA_FALSE;
}

static Eina_Bool
external_icon_param_get(void *data __UNUSED__, const Evas_Object *obj __UNUSED__, Edje_External_Param *param)
{
   if (!strcmp(param->name, "icon"))
     {
        /* not easy to get icon name back from live object */
        return EINA_FALSE;
     }

   ERR("unknown parameter '%s' of type '%s'",
       param->name, edje_external_param_type_str(param->type));

   return EINA_FALSE;
}

static void *
external_icon_params_parse(void *data __UNUSED__, Evas_Object *obj __UNUSED__, const Eina_List *params)
{
   Elm_Params_Icon *mem;
   Edje_External_Param *param;
   const Eina_List *l;

   mem = ELM_NEW(Elm_Params_Icon);
   if (!mem)
     return NULL;

   EINA_LIST_FOREACH(params, l, param)
     {
        if (!strcmp(param->name, "icon"))
          {
             mem->icon = eina_stringshare_add(param->s);
          }
     }

   return mem;
}

static Evas_Object *external_icon_content_get(void *data __UNUSED__,
                                              const Evas_Object *obj __UNUSED__, const char *content __UNUSED__)
{
   ERR("no content");
   return NULL;
}

static void
external_icon_params_free(void *params)
{
   Elm_Params_Icon *mem = params;

   if (mem->icon)
     eina_stringshare_del(mem->icon);
   external_common_params_free(params);
}

static Edje_External_Param_Info external_icon_params[] = {
   DEFINE_EXTERNAL_COMMON_PARAMS,
   EDJE_EXTERNAL_PARAM_INFO_STRING("icon"),
   EDJE_EXTERNAL_PARAM_INFO_SENTINEL
};

DEFINE_EXTERNAL_ICON_ADD(icon, "icon");
DEFINE_EXTERNAL_TYPE_SIMPLE(icon, "Icon");
