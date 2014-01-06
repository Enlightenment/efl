#include "private.h"

static Elm_Prefs_Item_Type supported_types[] =
{
   ELM_PREFS_TYPE_BOOL,
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
elm_prefs_check_add(const Elm_Prefs_Item_Iface *iface EINA_UNUSED,
                    Evas_Object *prefs,
                    const Elm_Prefs_Item_Type type EINA_UNUSED,
                    const Elm_Prefs_Item_Spec spec,
                    Elm_Prefs_Item_Changed_Cb cb)
{
   Evas_Object *obj = elm_check_add(prefs);

   evas_object_smart_callback_add(obj, "changed", _item_changed_cb, cb);

   elm_check_state_set(obj, spec.b.def);

   return obj;
}

static Eina_Bool
elm_prefs_check_value_set(Evas_Object *obj,
                          Eina_Value *value)
{
   Eina_Bool val;

   if (eina_value_type_get(value) != EINA_VALUE_TYPE_UCHAR) return EINA_FALSE;

   eina_value_get(value, &val);
   elm_check_state_set(obj, val);

   return EINA_TRUE;
}

static Eina_Bool
elm_prefs_check_value_get(Evas_Object *obj,
                          Eina_Value *value)
{
   Eina_Bool val;

   val = elm_check_state_get(obj);

   if (!eina_value_setup(value, EINA_VALUE_TYPE_UCHAR)) return EINA_FALSE;
   if (!eina_value_set(value, val)) return EINA_FALSE;

   return EINA_TRUE;
}

static Eina_Bool
elm_prefs_check_label_set(Evas_Object *obj,
                          const char *label)
{
   return elm_layout_text_set(obj, NULL, label);
}

static Eina_Bool
elm_prefs_check_icon_set(Evas_Object *obj,
                         const char *icon)
{
   Evas_Object *ic = elm_icon_add(obj);
   Eina_Bool ret;

   if (!elm_icon_standard_set(ic, icon)) goto err;

   elm_image_resizable_set(ic, EINA_FALSE, EINA_FALSE);

   ret = elm_layout_content_set(obj, "icon", ic);
   if (!ret) goto err;

   return ret;

err:
   evas_object_del(ic);
   return EINA_FALSE;
}

PREFS_ITEM_WIDGET_ADD(check,
                      supported_types,
                      elm_prefs_check_value_set,
                      elm_prefs_check_value_get,
                      NULL,
                      elm_prefs_check_label_set,
                      elm_prefs_check_icon_set,
                      NULL,
                      NULL,
                      NULL);
