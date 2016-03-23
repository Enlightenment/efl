#include "private.h"

typedef struct _Elm_Params_Spinner
{
   Elm_Params base;
   const char *label_format;
   double min, max, step, value;
   Eina_Bool min_exists:1;
   Eina_Bool max_exists:1;
   Eina_Bool step_exists:1;
   Eina_Bool value_exists:1;
   Eina_Bool wrap_exists:1;
   Eina_Bool wrap:1;
} Elm_Params_Spinner;

static void
external_spinner_state_set(void *data EINA_UNUSED, Evas_Object *obj,
                           const void *from_params, const void *to_params,
                           float pos EINA_UNUSED)
{
   const Elm_Params_Spinner *p;

   if (to_params) p = to_params;
   else if (from_params) p = from_params;
   else return;

   if (p->label_format)
     elm_spinner_label_format_set(obj, p->label_format);
   if ((p->min_exists) && (p->max_exists))
     elm_spinner_min_max_set(obj, p->min, p->max);
   else if ((p->min_exists) || (p->max_exists))
     {
        double min, max;
        elm_spinner_min_max_get(obj, &min, &max);
        if (p->min_exists)
          elm_spinner_min_max_set(obj, p->min, max);
        else
          elm_spinner_min_max_set(obj, min, p->max);
     }
   if (p->step_exists)
     elm_spinner_step_set(obj, p->step);
   if (p->value_exists)
     elm_spinner_value_set(obj, p->value);
   if (p->wrap_exists)
     elm_spinner_wrap_set(obj, p->wrap);
}

static Eina_Bool
external_spinner_param_set(void *data EINA_UNUSED, Evas_Object *obj,
                           const Edje_External_Param *param)
{
   if (!strcmp(param->name, "label format"))
     {
        if (param->type == EDJE_EXTERNAL_PARAM_TYPE_STRING)
          {
             elm_spinner_label_format_set(obj, param->s);
             return EINA_TRUE;
          }
     }
   else if (!strcmp(param->name, "min"))
     {
        if (param->type == EDJE_EXTERNAL_PARAM_TYPE_DOUBLE)
          {
             double min, max;
             elm_spinner_min_max_get(obj, &min, &max);
             elm_spinner_min_max_set(obj, param->d, max);
             return EINA_TRUE;
          }
     }
   else if (!strcmp(param->name, "max"))
     {
        if (param->type == EDJE_EXTERNAL_PARAM_TYPE_DOUBLE)
          {
             double min, max;
             elm_spinner_min_max_get(obj, &min, &max);
             elm_spinner_min_max_set(obj, min, param->d);
             return EINA_TRUE;
          }
     }
   else if (!strcmp(param->name, "step"))
     {
        if (param->type == EDJE_EXTERNAL_PARAM_TYPE_DOUBLE)
          {
             elm_spinner_step_set(obj, param->d);
             return EINA_TRUE;
          }
     }
   else if (!strcmp(param->name, "value"))
     {
        if (param->type == EDJE_EXTERNAL_PARAM_TYPE_DOUBLE)
          {
             elm_spinner_value_set(obj, param->d);
             return EINA_TRUE;
          }
     }
   else if (!strcmp(param->name, "wrap"))
     {
        if (param->type == EDJE_EXTERNAL_PARAM_TYPE_BOOL)
          {
             elm_spinner_wrap_set(obj, param->i);
             return EINA_TRUE;
          }
     }

   ERR("unknown parameter '%s' of type '%s'",
       param->name, edje_external_param_type_str(param->type));

   return EINA_FALSE;
}

static Eina_Bool
external_spinner_param_get(void *data EINA_UNUSED, const Evas_Object *obj,
                           Edje_External_Param *param)
{
   if (!strcmp(param->name, "label format"))
     {
        if (param->type == EDJE_EXTERNAL_PARAM_TYPE_STRING)
          {
             param->s = elm_spinner_label_format_get(obj);
             return EINA_TRUE;
          }
     }
   else if (!strcmp(param->name, "min"))
     {
        if (param->type == EDJE_EXTERNAL_PARAM_TYPE_DOUBLE)
          {
             double min, max;
             elm_spinner_min_max_get(obj, &min, &max);
             param->d = min;
             return EINA_TRUE;
          }
     }
   else if (!strcmp(param->name, "max"))
     {
        if (param->type == EDJE_EXTERNAL_PARAM_TYPE_DOUBLE)
          {
             double min, max;
             elm_spinner_min_max_get(obj, &min, &max);
             param->d = max;
             return EINA_TRUE;
          }
     }
   else if (!strcmp(param->name, "step"))
     {
        if (param->type == EDJE_EXTERNAL_PARAM_TYPE_DOUBLE)
          {
             param->d = elm_spinner_step_get(obj);
             return EINA_TRUE;
          }
     }
   else if (!strcmp(param->name, "value"))
     {
        if (param->type == EDJE_EXTERNAL_PARAM_TYPE_DOUBLE)
          {
             param->d = elm_spinner_value_get(obj);
             return EINA_TRUE;
          }
     }
   else if (!strcmp(param->name, "wrap"))
     {
        if (param->type == EDJE_EXTERNAL_PARAM_TYPE_BOOL)
          {
             param->i = elm_spinner_value_get(obj);
             return EINA_TRUE;
          }
     }

   ERR("unknown parameter '%s' of type '%s'",
       param->name, edje_external_param_type_str(param->type));

   return EINA_FALSE;
}

static void *
external_spinner_params_parse(void *data EINA_UNUSED,
                              Evas_Object *obj EINA_UNUSED,
                              const Eina_List *params)
{
   Elm_Params_Spinner *mem;
   Edje_External_Param *param;
   const Eina_List *l;

   mem = calloc(1, sizeof(Elm_Params_Spinner));
   if (!mem)
     return NULL;

   EINA_LIST_FOREACH(params, l, param)
     {
        if (!strcmp(param->name, "label format"))
          mem->label_format = eina_stringshare_add(param->s);
        else if (!strcmp(param->name, "min"))
          {
             mem->min = param->d;
             mem->min_exists = EINA_TRUE;
          }
        else if (!strcmp(param->name, "max"))
          {
             mem->max = param->d;
             mem->max_exists = EINA_TRUE;
          }
        else if (!strcmp(param->name, "step"))
          {
             mem->step = param->d;
             mem->step_exists = EINA_TRUE;
          }
        else if (!strcmp(param->name, "value"))
          {
             mem->value = param->d;
             mem->value_exists = EINA_TRUE;
          }
        else if (!strcmp(param->name, "wrap"))
          {
             mem->wrap = param->i;
             mem->wrap_exists = EINA_TRUE;
          }
     }

   return mem;
}

static Evas_Object *external_spinner_content_get(void *data EINA_UNUSED,
                                                 const Evas_Object *obj EINA_UNUSED,
                                                 const char *content EINA_UNUSED)
{
   ERR("No content.");
   return NULL;
}

static void
external_spinner_params_free(void *params)
{
   Elm_Params_Spinner *mem = params;

   if (mem->label_format)
     eina_stringshare_del(mem->label_format);
   free(mem);
}

static Edje_External_Param_Info external_spinner_params[] = {
     DEFINE_EXTERNAL_COMMON_PARAMS,
     EDJE_EXTERNAL_PARAM_INFO_STRING_DEFAULT("label format", "%1.2f"),
     EDJE_EXTERNAL_PARAM_INFO_DOUBLE("min"),
     EDJE_EXTERNAL_PARAM_INFO_DOUBLE_DEFAULT("max", 100.0),
     EDJE_EXTERNAL_PARAM_INFO_DOUBLE_DEFAULT("step", 1.0),
     EDJE_EXTERNAL_PARAM_INFO_DOUBLE("value"),
     EDJE_EXTERNAL_PARAM_INFO_BOOL("wrap"),
     EDJE_EXTERNAL_PARAM_INFO_SENTINEL
};

DEFINE_EXTERNAL_ICON_ADD(spinner, "spinner");
DEFINE_EXTERNAL_TYPE_SIMPLE(spinner, "Spinner");
