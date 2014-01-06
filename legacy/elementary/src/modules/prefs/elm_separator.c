#include "private.h"

static Elm_Prefs_Item_Type supported_types[] =
{
   ELM_PREFS_TYPE_SEPARATOR,
   ELM_PREFS_TYPE_UNKNOWN
};

static Evas_Object *
elm_prefs_separator_add(const Elm_Prefs_Item_Iface *iface EINA_UNUSED,
                        Evas_Object *prefs,
                        const Elm_Prefs_Item_Type type EINA_UNUSED,
                        const Elm_Prefs_Item_Spec spec EINA_UNUSED,
                        Elm_Prefs_Item_Changed_Cb cb EINA_UNUSED)
{
   Evas_Object *obj = elm_separator_add(prefs);

   return obj;
}

static Eina_Bool
elm_prefs_separator_value_set(Evas_Object *obj,
                              Eina_Value *value)
{
   Eina_Bool val;

   if (eina_value_type_get(value) != EINA_VALUE_TYPE_UCHAR)
     return EINA_FALSE;

   eina_value_get(value, &val);
   elm_separator_horizontal_set(obj, val);

   return EINA_TRUE;
}

static Eina_Bool
elm_prefs_separator_value_get(Evas_Object *obj,
                              Eina_Value *value)
{
   Eina_Bool val = elm_separator_horizontal_get(obj);

   if (!eina_value_setup(value, EINA_VALUE_TYPE_UCHAR))
     return EINA_FALSE;

   if (!eina_value_set(value, val)) return EINA_FALSE;

   return EINA_TRUE;
}

PREFS_ITEM_WIDGET_ADD(separator,
                      supported_types,
                      elm_prefs_separator_value_set,
                      elm_prefs_separator_value_get,
                      NULL,
                      NULL,
                      NULL,
                      NULL,
                      NULL,
                      NULL);
