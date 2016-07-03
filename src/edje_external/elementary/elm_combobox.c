#include "private.h"

typedef struct
{
   Elm_Params base;
   Eina_Stringshare *guide;
} Elm_Params_Combobox;


static void
external_combobox_state_set(void        *data        EINA_UNUSED,
                            Evas_Object *obj,
                            const void  *from_params,
                            const void  *to_params,
                            float        pos         EINA_UNUSED)
{
   const Elm_Params_Combobox *p;

   if (to_params) p = to_params;
   else if (from_params) p = from_params;
   else return;

   if (p->guide) elm_object_part_text_set(obj, "guide", p->guide);
}

static Eina_Bool
external_combobox_param_set(void                      *data  EINA_UNUSED,
                            Evas_Object               *obj,
                            const Edje_External_Param *param)
{
   if (!strcmp(param->name, "guide"))
     {
        if (param->type == EDJE_EXTERNAL_PARAM_TYPE_STRING)
          {
             elm_object_part_text_set(obj, "guide", param->s);
             return EINA_TRUE;
          }
     }

   ERR("unknown parameter '%s' of type '%s'", param->name,
       edje_external_param_type_str(param->type));
   return EINA_FALSE;
}

static Eina_Bool
external_combobox_param_get(void                *data  EINA_UNUSED,
                            const Evas_Object   *obj,
                            Edje_External_Param *param)
{
   if (!strcmp(param->name, "guide"))
     {
        if (param->type == EDJE_EXTERNAL_PARAM_TYPE_STRING)
          {
             param->s = elm_object_part_text_get(obj, "guide");
             return EINA_TRUE;
          }
     }

   ERR("Unknown parameter '%s' f type %s", param->name,
       edje_external_param_type_str(param->type));
   return EINA_FALSE;
}

static void *
external_combobox_params_parse(void            *data   EINA_UNUSED,
                               Evas_Object     *obj    EINA_UNUSED,
                               const Eina_List *params)
{
   Elm_Params_Combobox *mem;
   Edje_External_Param *param;
   const Eina_List *l;

   mem = calloc(1, sizeof(*mem));
   if (EINA_UNLIKELY(!mem))
     return NULL;

   EINA_LIST_FOREACH(params, l, param)
     {
        if (!strcmp(param->name, "guide"))
          {
             mem->guide = eina_stringshare_add(param->s);
          }
     }

   return mem;
}

static Evas_Object *
external_combobox_content_get(void              *data    EINA_UNUSED,
                              const Evas_Object *obj     EINA_UNUSED,
                              const char        *content EINA_UNUSED)
{
   ERR("No content for combobox");
   return NULL;
}

static void
external_combobox_params_free(void *params)
{
   Elm_Params_Combobox *const mem = params;

   if (mem->guide) eina_stringshare_del(mem->guide);
   free(mem);
}

static Edje_External_Param_Info external_combobox_params[] = {
     DEFINE_EXTERNAL_COMMON_PARAMS,
     EDJE_EXTERNAL_PARAM_INFO_STRING("guide"),
     EDJE_EXTERNAL_PARAM_INFO_SENTINEL
};

DEFINE_EXTERNAL_ICON_ADD(combobox, "combobox");
DEFINE_EXTERNAL_TYPE_SIMPLE(combobox, "Combobox");
