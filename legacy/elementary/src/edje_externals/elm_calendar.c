#include "private.h"

typedef struct _Elm_Params_Calendar
{
   Elm_Params base;
   int year_min;
   int year_max;
   Eina_Bool sel_enable;
   Eina_Bool sel_exists:1;
} Elm_Params_Calendar;

static void
external_calendar_state_set(void *data __UNUSED__, Evas_Object *obj,
                            const void *from_params, const void *to_params,
                            float pos __UNUSED__)
{
   const Elm_Params_Calendar *p;
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
   if (p->sel_exists)
     elm_calendar_day_selection_enabled_set(obj, p->sel_enable);
}

static Eina_Bool
external_calendar_param_set(void *data __UNUSED__, Evas_Object *obj,
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
   else if (!strcmp(param->name, "sel_enable"))
     {
	if (param->type == EDJE_EXTERNAL_PARAM_TYPE_BOOL)
	  {
	     elm_calendar_day_selection_enabled_set(obj,param->i );
	     return EINA_TRUE;
	  }
     }

   ERR("unknown parameter '%s' of type '%s'",
       param->name, edje_external_param_type_str(param->type));

   return EINA_FALSE;
}

static Eina_Bool
external_calendar_param_get(void *data __UNUSED__, const Evas_Object *obj,
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
   else if (!strcmp(param->name, "sel_enable"))
     {
	if (param->type == EDJE_EXTERNAL_PARAM_TYPE_BOOL)
	  {
	     param->i = elm_calendar_day_selection_enabled_get(obj);
	     return EINA_TRUE;
	  }
     }

   ERR("unknown parameter '%s' of type '%s'",
       param->name, edje_external_param_type_str(param->type));

   return EINA_FALSE;
}

static void *
external_calendar_params_parse(void *data __UNUSED__,
                               Evas_Object *obj __UNUSED__,
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

	else if(!strcmp(param->name, "year_max"))
          mem->year_max = param->i;

	else if (!strcmp(param->name, "sel_enable"))
          {
	     mem->sel_enable = param->i;
	     mem->sel_exists = EINA_TRUE;
          }
     }

   return mem;
}

static Evas_Object *
external_calendar_content_get(void *data __UNUSED__,
                              const Evas_Object *obj __UNUSED__,
                              const char *content __UNUSED__)
{
   ERR("No content.");
   return NULL;
}

static void
external_calendar_params_free(void *params)
{
   free(params);
}

static Edje_External_Param_Info external_calendar_params[] = {
   DEFINE_EXTERNAL_COMMON_PARAMS,
   EDJE_EXTERNAL_PARAM_INFO_INT("year_min"),
   EDJE_EXTERNAL_PARAM_INFO_INT("year_max"),
   EDJE_EXTERNAL_PARAM_INFO_BOOL("sel_enable"),
   EDJE_EXTERNAL_PARAM_INFO_SENTINEL
};

DEFINE_EXTERNAL_ICON_ADD(calendar, "calendar");
DEFINE_EXTERNAL_TYPE_SIMPLE(calendar, "Calendar");
