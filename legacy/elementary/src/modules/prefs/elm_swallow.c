#include "private.h"

static const char LAYOUT_EDJ[] = PACKAGE_LIB_DIR\
                                 "/elementary/modules/prefs/"\
                                 MODULE_ARCH\
                                 "/elm_prefs_swallow.edj";

static Elm_Prefs_Item_Type supported_types[] =
{
   ELM_PREFS_TYPE_SWALLOW,
   ELM_PREFS_TYPE_UNKNOWN
};

static Evas_Object *
elm_prefs_swallow_add(const Elm_Prefs_Item_Iface *iface EINA_UNUSED,
                      Evas_Object *prefs,
                      const Elm_Prefs_Item_Type type EINA_UNUSED,
                      const Elm_Prefs_Item_Spec spec EINA_UNUSED,
                      Elm_Prefs_Item_Changed_Cb cb EINA_UNUSED)
{
   Evas_Object *obj = elm_layout_add(prefs);

   elm_layout_file_set(obj, LAYOUT_EDJ, "elm_prefs_swallow");

   return obj;
}

static Eina_Bool
elm_prefs_swallow_swallow(Evas_Object *obj,
                          Eina_Value *value)
{
   Evas_Object *subobj;

   if (eina_value_type_get(value) != EINA_VALUE_TYPE_UINT64 ||
       !eina_value_get(value, &subobj))
     return EINA_FALSE;

   elm_layout_content_set(obj, "content", subobj);

   return EINA_TRUE;
}

static Eina_Bool
elm_prefs_swallow_unswallow(Evas_Object *obj,
                            Eina_Value *value)
{
   Evas_Object *subobj = elm_layout_content_unset(obj, "content");

   if (!eina_value_setup(value, EINA_VALUE_TYPE_UINT64) ||
       !eina_value_set(value, subobj))
     return EINA_FALSE;

   return EINA_TRUE;
}

PREFS_ITEM_WIDGET_ADD(swallow,
                      supported_types,
                      elm_prefs_swallow_swallow,
                      elm_prefs_swallow_unswallow,
                      NULL,
                      NULL,
                      NULL,
                      NULL,
                      NULL,
                      NULL);
