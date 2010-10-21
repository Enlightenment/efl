#include "private.h"

typedef struct _Elm_Params_Clock
{
   Elm_Params base;
   int hrs, min, sec;
   Eina_Bool hrs_exists:1;
   Eina_Bool min_exists:1;
   Eina_Bool sec_exists:1;
   Eina_Bool edit:1;
   Eina_Bool ampm:1;
   Eina_Bool seconds:1;
} Elm_Params_Clock;

static void
external_clock_state_set(void *data __UNUSED__, Evas_Object *obj, const void *from_params, const void *to_params, float pos __UNUSED__)
{
   const Elm_Params_Clock *p;

   if (to_params) p = to_params;
   else if (from_params) p = from_params;
   else return;

   if ((p->hrs_exists) && (p->min_exists) && (p->sec_exists))
     elm_clock_time_set(obj, p->hrs, p->min, p->sec);
   else if ((p->hrs_exists) || (p->min_exists) || (p->sec_exists))
     {
	int hrs, min, sec;
	elm_clock_time_get(obj, &hrs, &min, &sec);
	if (p->hrs_exists)
	  hrs = p->hrs;
	if (p->min_exists)
	  min = p->min;
	if (p->sec_exists)
	  sec = p->sec;
	elm_clock_time_set(obj, hrs, min, sec);
     }
   if (p->edit)
     elm_clock_edit_set(obj, p->edit);
   if (p->ampm)
     elm_clock_show_am_pm_set(obj, p->ampm);
   if (p->seconds)
     elm_clock_show_seconds_set(obj, p->seconds);
}

static Eina_Bool
external_clock_param_set(void *data __UNUSED__, Evas_Object *obj, const Edje_External_Param *param)
{
   if (!strcmp(param->name, "hours"))
     {
	if (param->type == EDJE_EXTERNAL_PARAM_TYPE_INT)
	  {
	     int hrs, min, sec;
	     elm_clock_time_get(obj, &hrs, &min, &sec);
	     elm_clock_time_set(obj, param->d, min, sec);
	     return EINA_TRUE;
	  }
     }
   else if (!strcmp(param->name, "minutes"))
     {
	if (param->type == EDJE_EXTERNAL_PARAM_TYPE_INT)
	  {
	     int hrs, min, sec;
	     elm_clock_time_get(obj, &hrs, &min, &sec);
	     elm_clock_time_set(obj, hrs, param->d, sec);
	     return EINA_TRUE;
	  }
     }
   else if (!strcmp(param->name, "seconds"))
     {
	if (param->type == EDJE_EXTERNAL_PARAM_TYPE_INT)
	  {
	     int hrs, min, sec;
	     elm_clock_time_get(obj, &hrs, &min, &sec);
	     elm_clock_time_set(obj, hrs, min, param->d);
	     return EINA_TRUE;
	  }
     }
   else if (!strcmp(param->name, "editable"))
     {
	if (param->type == EDJE_EXTERNAL_PARAM_TYPE_BOOL)
	  {
	     elm_clock_edit_set(obj, param->i);
	     return EINA_TRUE;
	  }
     }
   else if (!strcmp(param->name, "am/pm"))
     {
	if (param->type == EDJE_EXTERNAL_PARAM_TYPE_BOOL)
	  {
	     elm_clock_show_am_pm_set(obj, param->i);
	     return EINA_TRUE;
	  }
     }
   else if (!strcmp(param->name, "show seconds"))
     {
	if (param->type == EDJE_EXTERNAL_PARAM_TYPE_BOOL)
	  {
	     elm_clock_show_seconds_set(obj, param->i);
	     return EINA_TRUE;
	  }
     }

   ERR("unknown parameter '%s' of type '%s'",
       param->name, edje_external_param_type_str(param->type));

   return EINA_FALSE;
}

static Eina_Bool
external_clock_param_get(void *data __UNUSED__, const Evas_Object *obj, Edje_External_Param *param)
{
   if (!strcmp(param->name, "hours"))
     {
	if (param->type == EDJE_EXTERNAL_PARAM_TYPE_INT)
	  {
	     int hrs, min, sec;
	     elm_clock_time_get(obj, &hrs, &min, &sec);
	     param->i = hrs;
	     return EINA_TRUE;
	  }
     }
   else if (!strcmp(param->name, "minutes"))
     {
	if (param->type == EDJE_EXTERNAL_PARAM_TYPE_INT)
	  {
	     int hrs, min, sec;
	     elm_clock_time_get(obj, &hrs, &min, &sec);
	     param->i = min;
	     return EINA_TRUE;
	  }
     }
   else if (!strcmp(param->name, "seconds"))
     {
	if (param->type == EDJE_EXTERNAL_PARAM_TYPE_INT)
	  {
	     int hrs, min, sec;
	     elm_clock_time_get(obj, &hrs, &min, &sec);
	     param->i = sec;
	     return EINA_TRUE;
	  }
     }
   else if (!strcmp(param->name, "editable"))
     {
	if (param->type == EDJE_EXTERNAL_PARAM_TYPE_BOOL)
	  {
	     param->i = elm_clock_edit_get(obj);
	     return EINA_TRUE;
	  }
     }
   else if (!strcmp(param->name, "am/pm"))
     {
	if (param->type == EDJE_EXTERNAL_PARAM_TYPE_BOOL)
	  {
	     param->i = elm_clock_show_am_pm_get(obj);
	     return EINA_TRUE;
	  }
     }
   else if (!strcmp(param->name, "show seconds"))
     {
	if (param->type == EDJE_EXTERNAL_PARAM_TYPE_BOOL)
	  {
	     param->i = elm_clock_show_seconds_get(obj);
	     return EINA_TRUE;
	  }
     }


   ERR("unknown parameter '%s' of type '%s'",
       param->name, edje_external_param_type_str(param->type));

   return EINA_FALSE;
}

static void *
external_clock_params_parse(void *data __UNUSED__, Evas_Object *obj __UNUSED__, const Eina_List *params)
{
   Elm_Params_Clock *mem;
   Edje_External_Param *param;
   const Eina_List *l;

   mem = calloc(1, sizeof(Elm_Params_Clock));
   if (!mem)
     return NULL;

   EINA_LIST_FOREACH(params, l, param)
     {
	if (!strcmp(param->name, "hours"))
	  {
	     mem->hrs = param->i;
	     mem->hrs_exists = EINA_TRUE;
	  }
	else if (!strcmp(param->name, "minutes"))
	  {
	     mem->min = param->i;
	     mem->min_exists = EINA_TRUE;
	  }
	else if (!strcmp(param->name, "seconds"))
	  {
	     mem->sec = param->i;
	     mem->sec_exists = EINA_TRUE;
	  }
	else if (!strcmp(param->name, "editable"))
	  mem->edit = !!param->i;
	else if (!strcmp(param->name, "am/pm"))
	  mem->ampm = !!param->i;
	else if (!strcmp(param->name, "show seconds"))
	  mem->seconds = !!param->i;
     }

   return mem;
}

static Evas_Object *external_clock_content_get(void *data __UNUSED__,
		const Evas_Object *obj __UNUSED__, const char *content __UNUSED__)
{
	ERR("No content.");
	return NULL;
}

static void
external_clock_params_free(void *params)
{
   Elm_Params_Clock *mem = params;

   free(mem);
}

static Edje_External_Param_Info external_clock_params[] = {
   DEFINE_EXTERNAL_COMMON_PARAMS,
   EDJE_EXTERNAL_PARAM_INFO_INT("hours"),
   EDJE_EXTERNAL_PARAM_INFO_INT("minutes"),
   EDJE_EXTERNAL_PARAM_INFO_INT("seconds"),
   EDJE_EXTERNAL_PARAM_INFO_BOOL("editable"),
   EDJE_EXTERNAL_PARAM_INFO_BOOL("am/pm"),
   EDJE_EXTERNAL_PARAM_INFO_BOOL("show seconds"),
   EDJE_EXTERNAL_PARAM_INFO_SENTINEL
};

DEFINE_EXTERNAL_ICON_ADD(clock, "clock")
DEFINE_EXTERNAL_TYPE_SIMPLE(clock, "Clock")
