#include "private.h"

static Elm_Prefs_Item_Type supported_types[] =
{
   ELM_PREFS_TYPE_ACTION,
   ELM_PREFS_TYPE_RESET,
   ELM_PREFS_TYPE_SAVE,
   ELM_PREFS_TYPE_UNKNOWN
};

static void
_item_changed_cb(void *data, Evas_Object *obj, void *event_info EINA_UNUSED)
{
   Elm_Prefs_Item_Changed_Cb prefs_it_changed_cb = data;

   prefs_it_changed_cb(obj);
}

static Evas_Object *
elm_prefs_button_add(const Elm_Prefs_Item_Iface *iface EINA_UNUSED,
                     Evas_Object *prefs,
                     const Elm_Prefs_Item_Type type EINA_UNUSED,
                     const Elm_Prefs_Item_Spec spec EINA_UNUSED,
                     Elm_Prefs_Item_Changed_Cb cb)
{
   Evas_Object *obj = elm_button_add(prefs);

   evas_object_smart_callback_add(obj, "clicked", _item_changed_cb, cb);

   return obj;
}

static Eina_Bool
elm_prefs_button_label_set(Evas_Object *obj,
                           const char *label)
{
   return elm_layout_text_set(obj, NULL, label);
}

static Eina_Bool
elm_prefs_button_icon_set(Evas_Object *obj,
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

PREFS_ITEM_WIDGET_ADD(button,
                      supported_types,
                      NULL,
                      NULL,
                      NULL,
                      elm_prefs_button_label_set,
                      elm_prefs_button_icon_set,
                      NULL,
                      NULL,
                      NULL);
