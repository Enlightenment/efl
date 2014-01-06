#include "private.h"

static Evas_Object *
elm_prefs_vertical_box_add(const Elm_Prefs_Page_Iface *iface EINA_UNUSED,
                           Evas_Object *prefs)
{
   Evas_Object *obj = elm_box_add(prefs);

   elm_box_horizontal_set(obj, EINA_FALSE);

   return obj;
}

static Eina_Bool
elm_prefs_vertical_box_item_pack(Evas_Object *obj,
                                 Evas_Object *it,
                                 const Elm_Prefs_Item_Type type,
                                 const Elm_Prefs_Item_Iface *iface)
{
   Evas_Object *l;

   if ((type == ELM_PREFS_TYPE_SEPARATOR) &&
       (!elm_prefs_page_item_value_set(it, iface, EINA_TRUE)))
     return EINA_FALSE;

   l = evas_object_data_get(it, "label_widget");
   if (l)
     evas_object_size_hint_align_set(l, 0.0, 1.0);

   elm_prefs_vertical_page_common_pack(it, obj, iface);

   return EINA_TRUE;
}

static Eina_Bool
elm_prefs_vertical_box_item_unpack(Evas_Object *obj,
                                   Evas_Object *it)
{
   /* back to defaults */
   evas_object_size_hint_align_set(it, 0.5, 0.5);
   evas_object_size_hint_weight_set(it, 0.0, 0.0);

   elm_prefs_page_common_unpack(it, obj);

   return EINA_TRUE;
}

static Eina_Bool
elm_prefs_vertical_box_item_pack_before(Evas_Object *obj,
                                        Evas_Object *it,
                                        Evas_Object *it_before,
                                        const Elm_Prefs_Item_Type type,
                                        const Elm_Prefs_Item_Iface *iface)
{
   Evas_Object *l;

   if ((type == ELM_PREFS_TYPE_SEPARATOR) &&
       (!elm_prefs_page_item_value_set(it, iface, EINA_TRUE)))
     return EINA_FALSE;

   l = evas_object_data_get(it, "label_widget");
   if (l)
     evas_object_size_hint_align_set(l, 0.0, 1.0);

   elm_prefs_vertical_page_common_pack_before(it, it_before, obj, iface);

   return EINA_TRUE;
}

static Eina_Bool
elm_prefs_vertical_box_item_pack_after(Evas_Object *obj,
                                       Evas_Object *it,
                                       Evas_Object *it_after,
                                       const Elm_Prefs_Item_Type type,
                                       const Elm_Prefs_Item_Iface *iface)
{
   Evas_Object *l;

   if ((type == ELM_PREFS_TYPE_SEPARATOR) &&
       (!elm_prefs_page_item_value_set(it, iface, EINA_TRUE)))
     return EINA_FALSE;

   l = evas_object_data_get(it, "label_widget");
   if (l)
     evas_object_size_hint_align_set(l, 0.0, 1.0);

   elm_prefs_vertical_page_common_pack_after(it, it_after, obj, iface);

   return EINA_TRUE;
}

PREFS_PAGE_WIDGET_ADD(vertical_box,
                      NULL,
                      NULL,
                      NULL,
                      elm_prefs_vertical_box_item_pack,
                      elm_prefs_vertical_box_item_unpack,
                      elm_prefs_vertical_box_item_pack_before,
                      elm_prefs_vertical_box_item_pack_after);
