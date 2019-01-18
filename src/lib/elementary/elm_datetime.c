#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#define EFL_ACCESS_OBJECT_PROTECTED

#include <Elementary.h>
#include "elm_priv.h"

#ifdef HAVE_LOCALE_H
# include <locale.h>
#endif

#ifdef HAVE_LANGINFO_H
# include <langinfo.h>
#endif

#include "elm_datetime.h"

#include "efl_ui_clock_legacy.eo.h"

#define MY_CLASS_NAME_LEGACY "elm_datetime"

static void
_efl_ui_clock_legacy_class_constructor(Efl_Class *klass)
{
   evas_smart_legacy_type_register(MY_CLASS_NAME_LEGACY, klass);
}

EOLIAN static Eo *
_efl_ui_clock_legacy_efl_object_constructor(Eo *obj, void *pd EINA_UNUSED)
{
   obj = efl_constructor(efl_super(obj, EFL_UI_CLOCK_LEGACY_CLASS));
   efl_canvas_object_type_set(obj, MY_CLASS_NAME_LEGACY);
   legacy_child_focus_handle(obj);
   return obj;
}

EAPI Evas_Object *
elm_datetime_add(Evas_Object *parent)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(parent, NULL);
   return elm_legacy_add(EFL_UI_CLOCK_LEGACY_CLASS, parent);
}

EAPI void
elm_datetime_format_set(Evas_Object *obj,
                        const char *fmt)
{
   EINA_SAFETY_ON_NULL_RETURN(obj);

   efl_ui_clock_format_set(obj, fmt);
}

EAPI const char *
elm_datetime_format_get(const Evas_Object *obj)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(obj, NULL);

   return efl_ui_clock_format_get(obj);
}

static Efl_Ui_Clock_Type
adjust_field_type(Elm_Datetime_Field_Type type)
{
   Efl_Ui_Clock_Type ctype;

   switch(type)
     {
      case ELM_DATETIME_MONTH:
         ctype = EFL_UI_CLOCK_TYPE_MONTH;
         break;
      case ELM_DATETIME_DATE:
         ctype = EFL_UI_CLOCK_TYPE_DATE;
         break;
      case ELM_DATETIME_HOUR:
         ctype = EFL_UI_CLOCK_TYPE_HOUR;
         break;
      case ELM_DATETIME_MINUTE:
         ctype = EFL_UI_CLOCK_TYPE_MINUTE;
         break;
      case ELM_DATETIME_AMPM:
         ctype = EFL_UI_CLOCK_TYPE_AMPM;
         break;
      case ELM_DATETIME_YEAR:
      default:
         ctype = EFL_UI_CLOCK_TYPE_YEAR;
     }

   return ctype;
}

EAPI void
elm_datetime_field_limit_set(Evas_Object *obj, Elm_Datetime_Field_Type type, int min, int max)
{
   EINA_SAFETY_ON_NULL_RETURN(obj);

   if ((type < ELM_DATETIME_YEAR) || (type >= ELM_DATETIME_AMPM)) return;

   efl_ui_clock_field_limit_set(obj, adjust_field_type(type), min, max);
}

EAPI void
elm_datetime_field_limit_get(const Evas_Object *obj, Elm_Datetime_Field_Type fieldtype, int *min, int *max)
{
   EINA_SAFETY_ON_NULL_RETURN(obj);

   if ((fieldtype < ELM_DATETIME_YEAR) || (fieldtype >= ELM_DATETIME_AMPM)) return;

   efl_ui_clock_field_limit_get(obj, adjust_field_type(fieldtype), min, max);
}

EAPI Eina_Bool
elm_datetime_value_min_set(Evas_Object *obj, const Efl_Time *mintime)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(obj, EINA_FALSE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(mintime, EINA_FALSE);

   efl_ui_clock_time_min_set(obj, *mintime);

   return EINA_TRUE;
}

EAPI Eina_Bool
elm_datetime_value_min_get(const Evas_Object *obj, Efl_Time *mintime)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(obj, EINA_FALSE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(mintime, EINA_FALSE);

   *mintime = efl_ui_clock_time_min_get(obj);

   return EINA_TRUE;
}

EAPI Eina_Bool
elm_datetime_value_set(Evas_Object *obj, const Efl_Time *newtime)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(obj, EINA_FALSE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(newtime, EINA_FALSE);

   efl_ui_clock_time_set(obj, *newtime);

   return EINA_TRUE;
}

EAPI Eina_Bool
elm_datetime_value_get(const Evas_Object *obj, Efl_Time *currtime)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(obj, EINA_FALSE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(currtime, EINA_FALSE);

   *currtime = efl_ui_clock_time_get(obj);

   return EINA_TRUE;
}

EAPI void
elm_datetime_field_visible_set(Evas_Object *obj, Elm_Datetime_Field_Type fieldtype, Eina_Bool visible)
{
   EINA_SAFETY_ON_NULL_RETURN(obj);

   if ((fieldtype < ELM_DATETIME_YEAR) || (fieldtype > ELM_DATETIME_AMPM)) return;

   efl_ui_clock_field_visible_set(obj, adjust_field_type(fieldtype), visible);
}

EAPI Eina_Bool elm_datetime_field_visible_get(const Evas_Object *obj, Elm_Datetime_Field_Type fieldtype)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(obj, EINA_FALSE);

   if ((fieldtype < ELM_DATETIME_YEAR) || (fieldtype > ELM_DATETIME_AMPM)) return EINA_FALSE;

   return efl_ui_clock_field_visible_get(obj, adjust_field_type(fieldtype));
}

EAPI Eina_Bool
elm_datetime_value_max_set(Evas_Object *obj, const Efl_Time *maxtime)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(obj, EINA_FALSE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(maxtime, EINA_FALSE);

   efl_ui_clock_time_max_set(obj, *maxtime);

   return EINA_TRUE;
}

EAPI Eina_Bool
elm_datetime_value_max_get(const Evas_Object *obj, Efl_Time *maxtime)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(obj, EINA_FALSE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(maxtime, EINA_FALSE);

   *maxtime = efl_ui_clock_time_max_get(obj);

   return EINA_TRUE;
}

#include "efl_ui_clock_legacy.eo.c"
