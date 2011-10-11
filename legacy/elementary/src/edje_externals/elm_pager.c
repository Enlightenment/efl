#include <assert.h>

#include "private.h"

typedef struct _Elm_Params_Pager
{
   Eina_Bool disable_animation_exists:1;
   Eina_Bool disable_animation:1;
} Elm_Params_Pager;


static void
external_pager_state_set(void *data __UNUSED__, Evas_Object *obj,
                         const void *from_params, const void *to_params,
                         float pos __UNUSED__)
{
   const Elm_Params_Pager *p;

   if (to_params) p = to_params;
   else if (from_params) p = from_params;
   else return;

   if (p->disable_animation_exists)
     elm_pager_animation_disabled_set(obj, p->disable_animation);
}

static Eina_Bool
external_pager_param_set(void *data __UNUSED__, Evas_Object *obj,
                         const Edje_External_Param *param)
{
   if (!strcmp(param->name, "disable animation"))
     {
        if (param->type == EDJE_EXTERNAL_PARAM_TYPE_BOOL)
          {
             elm_pager_animation_disabled_set(obj, param->i);
             return EINA_TRUE;
          }
     }
   ERR("unknown parameter '%s' of type '%s'",
       param->name, edje_external_param_type_str(param->type));

   return EINA_FALSE;
}

static Eina_Bool
external_pager_param_get(void *data __UNUSED__,
                         const Evas_Object *obj __UNUSED__,
                         Edje_External_Param *param)
{
   ERR("unknown parameter '%s' of type '%s'",
       param->name, edje_external_param_type_str(param->type));

   return EINA_FALSE;
}

static void *
external_pager_params_parse(void *data __UNUSED__,
                            Evas_Object *obj __UNUSED__,
                            const Eina_List *params)
{
   Elm_Params_Pager *mem;
   Edje_External_Param *param;
   const Eina_List *l;

   mem = calloc(1, sizeof(Elm_Params_Pager));
   if (!mem)
     return NULL;

   EINA_LIST_FOREACH(params, l, param)
     {
        if (!strcmp(param->name, "disable animation"))
          {
             mem->disable_animation = !!param->i;
             mem->disable_animation_exists = EINA_TRUE;
          }
     }

   return mem;
}

static Evas_Object *
external_pager_content_get(void *data __UNUSED__,
                           const Evas_Object *obj __UNUSED__,
                           const char *content __UNUSED__)
{
   ERR("so content");
   return NULL;
}

static void
external_pager_params_free(void *params)
{
   Elm_Params_Pager* mem = params;
   free(mem);
}

static Edje_External_Param_Info external_pager_params[] = {
   EDJE_EXTERNAL_PARAM_INFO_BOOL("disable animation"),
   EDJE_EXTERNAL_PARAM_INFO_SENTINEL
};

DEFINE_EXTERNAL_ICON_ADD(pager, "pager");
DEFINE_EXTERNAL_TYPE_SIMPLE(pager, "Pager");
