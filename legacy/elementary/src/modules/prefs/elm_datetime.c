#include "private.h"

static Elm_Prefs_Item_Type supported_types[] =
{
   ELM_PREFS_TYPE_DATE,
   ELM_PREFS_TYPE_UNKNOWN
};

static void
_item_changed_cb(void *data,
                 Evas_Object *obj,
                 void *event_info EINA_UNUSED)
{
   Elm_Prefs_Item_Changed_Cb prefs_it_changed_cb = data;

   prefs_it_changed_cb(obj);
}

static Evas_Object *
elm_prefs_datetime_add(const Elm_Prefs_Item_Iface *iface EINA_UNUSED,
                       Evas_Object *prefs,
                       const Elm_Prefs_Item_Type type EINA_UNUSED,
                       const Elm_Prefs_Item_Spec spec,
                       Elm_Prefs_Item_Changed_Cb cb)
{
   Evas_Object *obj = elm_datetime_add(prefs);
   struct tm t;

   memset(&t, 0, sizeof t);

   elm_datetime_field_visible_set(obj, ELM_DATETIME_HOUR, EINA_FALSE);
   elm_datetime_field_visible_set(obj, ELM_DATETIME_MINUTE, EINA_FALSE);
   elm_datetime_field_visible_set(obj, ELM_DATETIME_AMPM, EINA_FALSE);

   evas_object_smart_callback_add(obj, "changed", _item_changed_cb, cb);

   t.tm_year = spec.d.min.y - 1900;
   t.tm_mon = spec.d.min.m - 1;
   t.tm_mday = spec.d.min.d;

   elm_datetime_value_min_set(obj, &t);

   t.tm_year = spec.d.max.y - 1900;
   t.tm_mon = spec.d.max.m - 1;
   t.tm_mday = spec.d.max.d;

   elm_datetime_value_max_set(obj, &t);

   return obj;
}

static Eina_Bool
elm_prefs_datetime_value_set(Evas_Object *obj,
                             Eina_Value *value)
{
   struct timeval val;
   struct tm *t;
   time_t gmt;

   if (eina_value_type_get(value) != EINA_VALUE_TYPE_TIMEVAL)
     return EINA_FALSE;

   eina_value_get(value, &val);
   gmt = val.tv_sec;
   t = gmtime(&gmt);

   if (elm_datetime_value_set(obj, t)) return EINA_TRUE;

   return EINA_FALSE;
}

static Eina_Bool
elm_prefs_datetime_value_get(Evas_Object *obj,
                             Eina_Value *value)
{
   struct timeval val;
   struct tm t;

   memset(&val, 0, sizeof val);

   if (!elm_datetime_value_get(obj, &t)) return EINA_FALSE;

   val.tv_sec = mktime(&t);

   if (!eina_value_setup(value, EINA_VALUE_TYPE_TIMEVAL)) return EINA_FALSE;
   if (!eina_value_set(value, val)) return EINA_FALSE;

   return EINA_TRUE;
}

PREFS_ITEM_WIDGET_ADD(datetime,
                      supported_types,
                      elm_prefs_datetime_value_set,
                      elm_prefs_datetime_value_get,
                      NULL,
                      NULL,
                      NULL,
                      NULL,
                      NULL,
                      NULL);
