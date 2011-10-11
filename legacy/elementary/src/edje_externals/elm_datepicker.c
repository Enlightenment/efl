#include "private.h"

typedef struct _Elm_Params_Datepicker
{
   const char *format;
   int year;
   int mon;
   int day;
   Eina_Bool year_exists:1;
   Eina_Bool mon_exists:1;
   Eina_Bool day_exists:1;
} Elm_Params_Datepicker;

static void
external_datepicker_state_set(void *data __UNUSED__, Evas_Object *obj, const void *from_params, const void *to_params, float pos __UNUSED__)
{
   const Elm_Params_Datepicker *p;

   if (to_params) p = to_params;
   else if (from_params) p = from_params;
   else return;

   if (p->format) elm_datepicker_date_format_set(obj, p->format);
   if ((p->year_exists) || (p->mon_exists) || (p->day_exists))
     {
        int year, mon, day;
        elm_datepicker_date_get(obj, &year, &mon, &day);

        if (p->year_exists) year = p->year;
        if (p->mon_exists) mon = p->mon;
        if (p->day_exists) day = p->day;
        elm_datepicker_date_set(obj, year, mon, day);
     }
}

static Eina_Bool
external_datepicker_param_set(void *data __UNUSED__, Evas_Object *obj, const Edje_External_Param *param)
{
   if (!strcmp(param->name, "format"))
     {
        if (param->type == EDJE_EXTERNAL_PARAM_TYPE_STRING)
          {
             elm_datepicker_date_format_set(obj, param->s);
             return EINA_TRUE;
          }
     }
   else if (!strcmp(param->name, "years"))
     {
        if (param->type == EDJE_EXTERNAL_PARAM_TYPE_INT)
          {
             int mon, day;
             elm_datepicker_date_get(obj, NULL, &mon, &day);
             elm_datepicker_date_set(obj, param->i, mon, day);
             return EINA_TRUE;
          }
     }
   else if (!strcmp(param->name, "months"))
     {
        if (param->type == EDJE_EXTERNAL_PARAM_TYPE_INT)
          {
             int year, day;
             elm_datepicker_date_get(obj, &year, NULL, &day);
             elm_datepicker_date_set(obj, year, param->i, day);
             return EINA_TRUE;
          }
     }
   else if (!strcmp(param->name, "days"))
     {
        if (param->type == EDJE_EXTERNAL_PARAM_TYPE_INT)
          {
             int year, mon;
             elm_datepicker_date_get(obj, &year, &mon, NULL);
             elm_datepicker_date_set(obj, year, mon, param->i);
             return EINA_TRUE;
          }
     }

   ERR("unknown parameter '%s' of type '%s'",
       param->name, edje_external_param_type_str(param->type));

   return EINA_FALSE;
}

static Eina_Bool
external_datepicker_param_get(void *data __UNUSED__, const Evas_Object *obj, Edje_External_Param *param)
{
   if (!strcmp(param->name, "format"))
     {
        if (param->type == EDJE_EXTERNAL_PARAM_TYPE_STRING)
          {
             param->s = elm_datepicker_date_format_get(obj);
             return EINA_TRUE;
          }
     }
   else if (!strcmp(param->name, "years"))
     {
        if (param->type == EDJE_EXTERNAL_PARAM_TYPE_INT)
          {
             elm_datepicker_date_get(obj, &(param->i), NULL, NULL);
             return EINA_TRUE;
          }
     }
   else if (!strcmp(param->name, "months"))
     {
        if (param->type == EDJE_EXTERNAL_PARAM_TYPE_INT)
          {
             elm_datepicker_date_get(obj, NULL, &(param->i), NULL);
             return EINA_TRUE;
          }
     }
   else if (!strcmp(param->name, "days"))
     {
        if (param->type == EDJE_EXTERNAL_PARAM_TYPE_INT)
          {
             elm_datepicker_date_get(obj, NULL, NULL, &(param->i));
             return EINA_TRUE;
          }
     }

   ERR("unknown parameter '%s' of type '%s'",
       param->name, edje_external_param_type_str(param->type));

   return EINA_FALSE;
}

static void *
external_datepicker_params_parse(void *data, Evas_Object *obj, const Eina_List *params)
{
   Elm_Params_Datepicker *mem;
   Edje_External_Param *param;
   const Eina_List *l;

   mem = calloc(1, sizeof(Elm_Params_Datepicker));
   if (!mem)
     return NULL;

   EINA_LIST_FOREACH(params, l, param)
     {
        if (!strcmp(param->name, "format"))
          mem->format = eina_stringshare_add(param->s);
        else if (!strcmp(param->name, "years"))
          {
             mem->year = param->i;
             mem->year_exists = EINA_TRUE;
          }
        else if (!strcmp(param->name, "months"))
          {
             mem->mon = param->i;
             mem->mon_exists = EINA_TRUE;
          }
        else if (!strcmp(param->name, "days"))
          {
             mem->day = param->i;
             mem->day_exists = EINA_TRUE;
          }
     }

   return mem;
}

static Evas_Object *external_datepicker_content_get(void *data __UNUSED__,
                const Evas_Object *obj, const char *content)
{
   ERR("so content");
   return NULL;
}

static void
external_datepicker_params_free(void *params)
{
   Elm_Params_Datepicker *mem = params;

   if (mem->format)
     eina_stringshare_del(mem->format);

   free(mem);
}

static Edje_External_Param_Info external_datepicker_params[] = {
   EDJE_EXTERNAL_PARAM_INFO_STRING("format"),
   EDJE_EXTERNAL_PARAM_INFO_INT_DEFAULT("years", 1900),
   EDJE_EXTERNAL_PARAM_INFO_INT_DEFAULT("months", 1),
   EDJE_EXTERNAL_PARAM_INFO_INT_DEFAULT("days", 1),
   EDJE_EXTERNAL_PARAM_INFO_SENTINEL
};

DEFINE_EXTERNAL_ICON_ADD(datepicker, "datepicker");
DEFINE_EXTERNAL_TYPE_SIMPLE(datepicker, "Datepicker");
