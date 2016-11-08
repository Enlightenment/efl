#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#define ELM_INTERFACE_ATSPI_ACCESSIBLE_PROTECTED

#include <Elementary.h>
#include "elm_priv.h"

#ifdef HAVE_LOCALE_H
# include <locale.h>
#endif

#ifdef HAVE_LANGINFO_H
# include <langinfo.h>
#endif

#include "elm_datetime.h"

EAPI Evas_Object *
elm_datetime_add(Evas_Object *parent)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(parent, NULL);
   Evas_Object *obj = efl_add(EFL_UI_CLOCK_CLASS, parent);
   return obj;
}

EAPI void
elm_datetime_format_set(Evas_Object *obj,
                        const char *fmt)
{
   efl_ui_clock_format_set(obj, fmt);
}

EAPI const char *
elm_datetime_format_get(const Evas_Object *obj)
{
   return efl_ui_clock_format_get(obj);
}

EAPI void
elm_datetime_field_limit_set(Evas_Object *obj, Elm_Datetime_Field_Type type, int min, int max)
{
   efl_ui_clock_field_limit_set(obj, type, min, max);
}

EAPI void
elm_datetime_field_limit_get(const Evas_Object *obj, Elm_Datetime_Field_Type fieldtype, int *min, int *max)
{
   efl_ui_clock_field_limit_get(obj, fieldtype, min, max);
}

EAPI Eina_Bool
elm_datetime_value_min_set(Evas_Object *obj, const Efl_Time *mintime)
{
   return efl_ui_clock_value_min_set(obj, (Efl_Time *)mintime);
}

EAPI Eina_Bool
elm_datetime_value_min_get(const Evas_Object *obj, Efl_Time *mintime)
{
   return efl_ui_clock_value_min_get(obj, mintime);
}

EAPI Eina_Bool
elm_datetime_value_set(Evas_Object *obj, const Efl_Time *newtime)
{
   return efl_ui_clock_value_set(obj, (Efl_Time *)newtime);
}

EAPI Eina_Bool elm_datetime_value_get(const Evas_Object *obj, Efl_Time *currtime)
{
   return efl_ui_clock_value_get(obj, currtime);
}

EAPI void
elm_datetime_field_visible_set(Evas_Object *obj, Elm_Datetime_Field_Type fieldtype, Eina_Bool visible)
{
   efl_ui_clock_field_visible_set(obj, fieldtype, visible);
}

EAPI Eina_Bool elm_datetime_field_visible_get(const Evas_Object *obj, Elm_Datetime_Field_Type fieldtype)
{
   return efl_ui_clock_field_visible_get(obj, fieldtype);
}

EAPI Eina_Bool
elm_datetime_value_max_set(Evas_Object *obj, const Efl_Time *maxtime)
{
   return efl_ui_clock_value_max_set(obj, (Efl_Time *)maxtime);
}

EAPI Eina_Bool
elm_datetime_value_max_get(const Evas_Object *obj, Efl_Time *maxtime)
{
   return efl_ui_clock_value_max_get(obj, maxtime);
}
