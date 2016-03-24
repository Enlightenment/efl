#include "private.h"

static Elm_Prefs_Item_Type supported_types[] =
{
   ELM_PREFS_TYPE_LABEL,
   ELM_PREFS_TYPE_UNKNOWN
};

static Evas_Object *
elm_prefs_label_add(const Elm_Prefs_Item_Iface *iface EINA_UNUSED,
                    Evas_Object *prefs,
                    const Elm_Prefs_Item_Type type EINA_UNUSED,
                    const Elm_Prefs_Item_Spec spec EINA_UNUSED,
                    Elm_Prefs_Item_Changed_Cb it_changed_cb EINA_UNUSED)
{
   Evas_Object *obj = elm_label_add(prefs);

   return obj;
}

static Eina_Bool
elm_prefs_label_label_set(Evas_Object *obj,
                          const char *label)
{
   return elm_layout_text_set(obj, NULL, label);
}

PREFS_ITEM_WIDGET_ADD(label,
                      supported_types,
                      NULL,
                      NULL,
                      NULL,
                      elm_prefs_label_label_set,
                      NULL,
                      NULL,
                      NULL,
                      NULL);
