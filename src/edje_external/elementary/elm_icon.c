#include <assert.h>
#include "private.h"

typedef struct _Elm_Params_Icon
{
   const char *file;
   Eina_Bool scale_up_exists;
   Eina_Bool scale_up : 1;
   Eina_Bool scale_down_exists;
   Eina_Bool scale_down : 1;
   Eina_Bool smooth_exists;
   Eina_Bool smooth : 1;
   Eina_Bool fill_outside_exists;
   Eina_Bool fill_outside : 1;
   Eina_Bool no_scale_exists;
   Eina_Bool no_scale : 1;
   Eina_Bool prescale_size_exists;
   int prescale_size;
   Elm_Params base;
   const char *icon;
} Elm_Params_Icon;


static void
external_icon_state_set(void *data EINA_UNUSED, Evas_Object *obj,
                        const void *from_params, const void *to_params,
                        float pos EINA_UNUSED)
{
   const Elm_Params_Icon *p;
   Evas_Object *edje;
   const char *file;
   Eina_Bool param;

   if (to_params) p = to_params;
   else if (from_params) p = from_params;
   else return;

   if (p->file)
     {
        elm_image_file_set(obj, p->file, NULL);
     }
   if (p->smooth_exists)
     {
        elm_image_smooth_set(obj, p->smooth);
     }
   if (p->no_scale_exists)
     {
        elm_image_no_scale_set(obj, p->no_scale);
     }
   if (p->scale_up_exists && p->scale_down_exists)
     {
        elm_image_resizable_set(obj, p->scale_up, p->scale_down);
     }
   else if (p->scale_up_exists || p->scale_down_exists)
     {
        if (p->scale_up_exists)
          {
             elm_image_resizable_get(obj, NULL, &param);
             elm_image_resizable_set(obj, p->scale_up, param);
          }
        else
          {
             elm_image_resizable_get(obj, &param, NULL);
             elm_image_resizable_set(obj, param, p->scale_down);
          }
     }
   if (p->fill_outside_exists)
     {
        elm_image_fill_outside_set(obj, p->fill_outside);
     }
   if (p->prescale_size_exists)
     {
        elm_image_prescale_set(obj, p->prescale_size);
     }
   if (p->icon)
     {
        edje = evas_object_smart_parent_get(obj);
        edje_object_file_get(edje, &file, NULL);

        if (!edje_file_group_exists(file, p->icon))
          {
            if (!elm_icon_standard_set(obj, p->icon))
              ERR("Failed to set standard icon! (%s)", p->icon);
          }
        else if (!elm_image_file_set(obj, file, p->icon))
          {
            if (!elm_icon_standard_set(obj, p->icon))
              ERR("Failed to set standard icon! (%s)", p->icon);
          }
     }
}

static Eina_Bool
external_icon_param_set(void *data EINA_UNUSED, Evas_Object *obj,
                        const Edje_External_Param *param)
{
   Evas_Object *edje;
   const char *file;
   Eina_Bool p;

   if (!strcmp(param->name, "file")
       && param->type == EDJE_EXTERNAL_PARAM_TYPE_STRING)
     {
        return elm_image_file_set(obj, param->s, NULL);
     }
   else if (!strcmp(param->name, "smooth")
            && param->type == EDJE_EXTERNAL_PARAM_TYPE_BOOL)
     {
        elm_image_smooth_set(obj, param->i);
        return EINA_TRUE;
     }
   else if (!strcmp(param->name, "no scale")
            && param->type == EDJE_EXTERNAL_PARAM_TYPE_BOOL)
     {
        elm_image_no_scale_set(obj, param->i);
        return EINA_TRUE;
     }
   else if (!strcmp(param->name, "scale up")
            && param->type == EDJE_EXTERNAL_PARAM_TYPE_BOOL)
     {
        elm_image_resizable_get(obj, NULL, &p);
        elm_image_resizable_set(obj, param->i, p);
        return EINA_TRUE;
     }
   else if (!strcmp(param->name, "scale down")
            && param->type == EDJE_EXTERNAL_PARAM_TYPE_BOOL)
     {
        elm_image_resizable_get(obj, &p, NULL);
        elm_image_resizable_set(obj, p, param->i);
        return EINA_TRUE;
     }
   else if (!strcmp(param->name, "fill outside")
            && param->type == EDJE_EXTERNAL_PARAM_TYPE_BOOL)
     {
        elm_image_fill_outside_set(obj, param->i);
        return EINA_TRUE;
     }
   else if (!strcmp(param->name, "prescale")
            && param->type == EDJE_EXTERNAL_PARAM_TYPE_INT)
     {
        elm_image_prescale_set(obj, param->i);
        return EINA_TRUE;
     }
   else if (!strcmp(param->name, "icon"))
     {
        if (param->type == EDJE_EXTERNAL_PARAM_TYPE_STRING)
          {
             edje = evas_object_smart_parent_get(obj);
             edje_object_file_get(edje, &file, NULL);

             if (!edje_file_group_exists(file, param->s))
               {
                 if (!elm_icon_standard_set(obj, param->s))
                   ERR("Failed to set standard icon! (%s)", param->s);
               }
             else if (!elm_image_file_set(obj, file, param->s))
               {
                 if (!elm_icon_standard_set(obj, param->s))
                   ERR("Failed to set standard icon as fallback! (%s)", param->s);
               }
             return EINA_TRUE;
          }
     }

   ERR("unknown parameter '%s' of type '%s'",
       param->name, edje_external_param_type_str(param->type));

   return EINA_FALSE;
}

static Eina_Bool
external_icon_param_get(void *data EINA_UNUSED,
                        const Evas_Object *obj,
                        Edje_External_Param *param)
{

   if (!strcmp(param->name, "file")
       && param->type == EDJE_EXTERNAL_PARAM_TYPE_STRING)
     {
        elm_image_file_get(obj, &param->s, NULL);
        return EINA_TRUE;
     }
   else if (!strcmp(param->name, "smooth")
            && param->type == EDJE_EXTERNAL_PARAM_TYPE_BOOL)
     {
        param->i = elm_image_smooth_get(obj);
        return EINA_TRUE;
     }
   else if (!strcmp(param->name, "no scale")
            && param->type == EDJE_EXTERNAL_PARAM_TYPE_BOOL)
     {
        param->i = elm_image_no_scale_get(obj);
        return EINA_TRUE;
     }
   else if (!strcmp(param->name, "scale up")
            && param->type == EDJE_EXTERNAL_PARAM_TYPE_BOOL)
     {
        elm_image_resizable_get(obj, NULL, (Eina_Bool *)(&param->i));
        return EINA_TRUE;
     }
   else if (!strcmp(param->name, "scale down")
            && param->type == EDJE_EXTERNAL_PARAM_TYPE_BOOL)
     {
        elm_image_resizable_get(obj, (Eina_Bool *)(&param->i), NULL);
        return EINA_TRUE;
     }
   else if (!strcmp(param->name, "fill outside")
            && param->type == EDJE_EXTERNAL_PARAM_TYPE_BOOL)
     {
        param->i = elm_image_fill_outside_get(obj);
        return EINA_TRUE;
     }
   else if (!strcmp(param->name, "prescale")
            && param->type == EDJE_EXTERNAL_PARAM_TYPE_INT)
     {
        param->i = elm_image_prescale_get(obj);
        return EINA_TRUE;
     }
   else if (!strcmp(param->name, "icon"))
     {
        /* not easy to get icon name back from live object */
        return EINA_FALSE;
     }

   ERR("unknown parameter '%s' of type '%s'",
       param->name, edje_external_param_type_str(param->type));

   return EINA_FALSE;
}

static void *
external_icon_params_parse(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED,
                           const Eina_List *params)
{
   Elm_Params_Icon *mem;
   Edje_External_Param *param;
   const Eina_List *l;
   mem = ELM_NEW(Elm_Params_Icon);
   if (EINA_UNLIKELY(!mem))
     return NULL;

   EINA_LIST_FOREACH(params, l, param)
     {
        if (!strcmp(param->name, "file"))
          mem->file = eina_stringshare_add(param->s);
        else if (!strcmp(param->name, "smooth"))
          {
             mem->smooth = param->i;
             mem->smooth_exists = EINA_TRUE;
          }
        else if (!strcmp(param->name, "no scale"))
          {
             mem->no_scale = param->i;
             mem->no_scale_exists = EINA_TRUE;
          }
        else if (!strcmp(param->name, "scale up"))
          {
             mem->scale_up = param->i;
             mem->scale_up_exists = EINA_TRUE;
          }
        else if (!strcmp(param->name, "scale down"))
          {
             mem->scale_down = param->i;
             mem->scale_down_exists = EINA_TRUE;
          }
        else if (!strcmp(param->name, "fill outside"))
          {
             mem->fill_outside = param->i;
             mem->fill_outside_exists = EINA_TRUE;
          }
        else if (!strcmp(param->name, "prescale"))
          {
             mem->prescale_size = param->i;
             mem->prescale_size_exists = EINA_TRUE;
          }
        else if (!strcmp(param->name, "icon"))
          {
             mem->icon = eina_stringshare_add(param->s);
          }
     }

   return mem;
}

static Evas_Object *
external_icon_content_get(void *data EINA_UNUSED,
                          const Evas_Object *obj EINA_UNUSED,
                          const char *content EINA_UNUSED)
{
   ERR("no content");
   return NULL;
}

static void
external_icon_params_free(void *params)
{
   Elm_Params_Icon *mem = params;

   if (mem->file)
     eina_stringshare_del(mem->file);

   if (mem->icon)
     eina_stringshare_del(mem->icon);
   free(mem);
}

static Edje_External_Param_Info external_icon_params[] = {
   DEFINE_EXTERNAL_COMMON_PARAMS,
   EDJE_EXTERNAL_PARAM_INFO_STRING("icon"),
   EDJE_EXTERNAL_PARAM_INFO_STRING("file"),
   EDJE_EXTERNAL_PARAM_INFO_BOOL("smooth"),
   EDJE_EXTERNAL_PARAM_INFO_BOOL("no scale"),
   EDJE_EXTERNAL_PARAM_INFO_BOOL("scale up"),
   EDJE_EXTERNAL_PARAM_INFO_BOOL("scale down"),
   EDJE_EXTERNAL_PARAM_INFO_BOOL("fill outside"),
   EDJE_EXTERNAL_PARAM_INFO_INT("prescale"),
   EDJE_EXTERNAL_PARAM_INFO_SENTINEL
};

DEFINE_EXTERNAL_ICON_ADD(icon, "icon");
DEFINE_EXTERNAL_TYPE_SIMPLE(icon, "Icon");
