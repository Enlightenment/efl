#include "private.h"

static Evas_Object *
elm_prefs_vertical_frame_add(const Elm_Prefs_Page_Iface *iface EINA_UNUSED,
                             Evas_Object *prefs)
{
   Evas_Object *bx, *obj = elm_frame_add(prefs);

   bx = elm_box_add(obj);
   evas_object_size_hint_align_set(bx, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

   elm_layout_content_set(obj, NULL, bx);

   evas_object_data_set(obj, "bx_container", bx);

   return obj;
}

static Eina_Bool
elm_prefs_vertical_frame_title_set(Evas_Object *obj,
                                   const char *title)
{
   elm_layout_text_set(obj, NULL, title);

   return EINA_TRUE;
}

static Eina_Bool
elm_prefs_vertical_frame_item_pack(Evas_Object *obj,
                                   Evas_Object *it,
                                   const Elm_Prefs_Item_Type type,
                                   const Elm_Prefs_Item_Iface *iface)
{
   Evas_Object *l, *bx = evas_object_data_get(obj, "bx_container");

   if ((type == ELM_PREFS_TYPE_SEPARATOR) &&
       (!elm_prefs_page_item_value_set(it, iface, EINA_TRUE)))
     return EINA_FALSE;

   l = evas_object_data_get(it, "label_widget");
   if (l)
     evas_object_size_hint_align_set(l, 0.0, 1.0);

   elm_prefs_vertical_page_common_pack(it, bx, iface);

   return EINA_TRUE;
}

static Eina_Bool
elm_prefs_vertical_frame_item_unpack(Evas_Object *obj,
                                     Evas_Object *it)
{
   Evas_Object *bx = evas_object_data_get(obj, "bx_container");

   /* back to defaults */
   evas_object_size_hint_align_set(it, 0.5, 0.5);
   evas_object_size_hint_weight_set(it, 0.0, 0.0);

   elm_prefs_page_common_unpack(it, bx);

   return EINA_TRUE;
}


static Eina_Bool
elm_prefs_vertical_frame_item_pack_before(Evas_Object *obj,
                                          Evas_Object *it,
                                          Evas_Object *it_before,
                                          const Elm_Prefs_Item_Type type,
                                          const Elm_Prefs_Item_Iface *iface)
{
   Evas_Object *l, *bx = evas_object_data_get(obj, "bx_container");

   if ((type == ELM_PREFS_TYPE_SEPARATOR) &&
       (!elm_prefs_page_item_value_set(it, iface, EINA_TRUE)))
     return EINA_FALSE;

   l = evas_object_data_get(it, "label_widget");
   if (l)
     evas_object_size_hint_align_set(l, 0.0, 1.0);

   elm_prefs_vertical_page_common_pack_before(it, it_before, bx, iface);

   return EINA_TRUE;
}

static Eina_Bool
elm_prefs_vertical_frame_item_pack_after(Evas_Object *obj,
                                         Evas_Object *it,
                                         Evas_Object *it_after,
                                         const Elm_Prefs_Item_Type type,
                                         const Elm_Prefs_Item_Iface *iface)
{
   Evas_Object *l, *bx = evas_object_data_get(obj, "bx_container");

   if ((type == ELM_PREFS_TYPE_SEPARATOR) &&
       (!elm_prefs_page_item_value_set(it, iface, EINA_TRUE)))
     return EINA_FALSE;

   l = evas_object_data_get(it, "label_widget");
   if (l)
     evas_object_size_hint_align_set(l, 0.0, 1.0);

   elm_prefs_vertical_page_common_pack_after(it, it_after, bx, iface);

   return EINA_TRUE;
}

PREFS_PAGE_WIDGET_ADD(vertical_frame,
                      elm_prefs_vertical_frame_title_set,
                      NULL,
                      NULL,
                      elm_prefs_vertical_frame_item_pack,
                      elm_prefs_vertical_frame_item_unpack,
                      elm_prefs_vertical_frame_item_pack_before,
                      elm_prefs_vertical_frame_item_pack_after);
