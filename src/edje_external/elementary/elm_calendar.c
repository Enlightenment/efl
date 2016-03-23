#include <assert.h>
#include "private.h"

typedef struct _Elm_Params_Calendar
{
   Elm_Params base;
   int year_min;
   int year_max;
   const char *select_mode;

} Elm_Params_Calendar;

#define SELECT_MODE_GET(CHOICES, STR)                                \
   unsigned int i;                                              \
   for (i = 0; i < (sizeof(CHOICES) / sizeof(CHOICES[0])); ++i) \
     if (!strcmp(STR, CHOICES[i]))                              \
       return i;

static const char *_calendar_select_modes[] =
{
   "default", "always", "none", "ondemand", NULL
};

static Elm_Calendar_Select_Mode
_calendar_select_mode_get(const char *select_mode)
{
   assert(sizeof(_calendar_select_modes) /
          sizeof(_calendar_select_modes[0])
          == ELM_CALENDAR_SELECT_MODE_ONDEMAND + 2);
   SELECT_MODE_GET(_calendar_select_modes, select_mode);
   return -1;
}

static void
external_calendar_state_set(void *data EINA_UNUSED, Evas_Object *obj,
                            const void *from_params, const void *to_params,
                            float pos EINA_UNUSED)
{
   const Elm_Params_Calendar *p;
   Elm_Calendar_Select_Mode select_mode;
   int min,max;

   if (to_params) p = to_params;
   else if (from_params) p = from_params;
   else return;

   if (p->year_min)
     {
        elm_calendar_min_max_year_get(obj, NULL, &max);
        elm_calendar_min_max_year_set(obj, p->year_min, max);
     }
   if (p->year_max)
     {
        elm_calendar_min_max_year_get(obj, &min, NULL);
        elm_calendar_min_max_year_set(obj, min, p->year_max);
     }
   if (p->select_mode)
     {
        select_mode = _calendar_select_mode_get(p->select_mode);
        elm_calendar_select_mode_set(obj, select_mode);
     }
}

static Eina_Bool
external_calendar_param_set(void *data EINA_UNUSED, Evas_Object *obj,
                            const Edje_External_Param *param)
{
   int min,max;

   if (!strcmp(param->name, "year_min"))
     {
        if (param->type == EDJE_EXTERNAL_PARAM_TYPE_INT)
          {
             elm_calendar_min_max_year_get(obj, NULL, &max);
             elm_calendar_min_max_year_set(obj, param->i, max);
             return EINA_TRUE;
          }
     }
   else if (!strcmp(param->name, "year_max"))
     {
        if (param->type == EDJE_EXTERNAL_PARAM_TYPE_INT)
          {
             elm_calendar_min_max_year_get(obj, &min, NULL);
             elm_calendar_min_max_year_set(obj, min,param->i);
             return EINA_TRUE;
          }
     }
   else if (!strcmp(param->name, "select_mode"))
     {
        if (param->type == EDJE_EXTERNAL_PARAM_TYPE_STRING)
          {
             Elm_Calendar_Select_Mode select_mode;
             select_mode = _calendar_select_mode_get(param->s);
             elm_calendar_select_mode_set(obj, select_mode);
             return EINA_TRUE;
          }
     }

   ERR("unknown parameter '%s' of type '%s'",
       param->name, edje_external_param_type_str(param->type));

   return EINA_FALSE;
}

static Eina_Bool
external_calendar_param_get(void *data EINA_UNUSED, const Evas_Object *obj,
                            Edje_External_Param *param)
{
   int min, max;

   if (!strcmp(param->name, "year_min"))
     {
        if (param->type == EDJE_EXTERNAL_PARAM_TYPE_INT)
          {
             elm_calendar_min_max_year_get(obj, &(param->i) ,&max);
             return EINA_TRUE;
          }
     }
   else if (!strcmp(param->name, "year_max"))
     {
        if (param->type == EDJE_EXTERNAL_PARAM_TYPE_INT)
          {
             elm_calendar_min_max_year_get(obj, &min,&(param->i));
             return EINA_TRUE;
          }
     }
   else if (!strcmp(param->name, "select_mode"))
     {
        if (param->type == EDJE_EXTERNAL_PARAM_TYPE_STRING)
          {
             Elm_Calendar_Select_Mode mode;
             mode = elm_calendar_select_mode_get(obj);
             param->s = _calendar_select_modes[mode];
             return EINA_TRUE;
          }
     }


   ERR("unknown parameter '%s' of type '%s'",
       param->name, edje_external_param_type_str(param->type));

   return EINA_FALSE;
}

static void *
external_calendar_params_parse(void *data EINA_UNUSED,
                               Evas_Object *obj EINA_UNUSED,
                               const Eina_List *params)
{
   Elm_Params_Calendar *mem;
   Edje_External_Param *param;
   const Eina_List *l;

   mem = calloc(1, sizeof(Elm_Params_Calendar));
   if (!mem)
     return NULL;

   EINA_LIST_FOREACH(params, l, param)
     {
        if (!strcmp(param->name, "year_min"))
          mem->year_min = param->i;

        else if (!strcmp(param->name, "year_max"))
          mem->year_max = param->i;

        else if (!strcmp(param->name, "select_mode"))
          mem->select_mode = eina_stringshare_add(param->s);
     }

   return mem;
}

static Evas_Object *
external_calendar_content_get(void *data EINA_UNUSED,
                              const Evas_Object *obj EINA_UNUSED,
                              const char *content EINA_UNUSED)
{
   ERR("No content.");
   return NULL;
}

static void
external_calendar_params_free(void *params)
{
   Elm_Params_Calendar *mem = params;
   if (mem->select_mode)
     eina_stringshare_del(mem->select_mode);
   free(params);
}

static Edje_External_Param_Info external_calendar_params[] = {
   DEFINE_EXTERNAL_COMMON_PARAMS,
   EDJE_EXTERNAL_PARAM_INFO_INT("year_min"),
   EDJE_EXTERNAL_PARAM_INFO_INT("year_max"),
   EDJE_EXTERNAL_PARAM_INFO_STRING("select_mode"),
   EDJE_EXTERNAL_PARAM_INFO_SENTINEL
};

DEFINE_EXTERNAL_ICON_ADD(calendar, "calendar");
DEFINE_EXTERNAL_TYPE_SIMPLE(calendar, "Calendar");
