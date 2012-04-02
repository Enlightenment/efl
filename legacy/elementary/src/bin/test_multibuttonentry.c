#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif
#include <Elementary.h>
#ifndef ELM_LIB_QUICKLAUNCH

static void
_item_selected_cb(void *data __UNUSED__,
                  Evas_Object *obj __UNUSED__,
                  void *event_info)
{
   Elm_Object_Item *mbe_it = event_info;
   printf("selected item = %s\n", elm_object_item_text_get(mbe_it));
}

// "item,added" smart callback of multibuttonentry.
static void
_item_added_cb(void *data __UNUSED__,
               Evas_Object *obj __UNUSED__,
               void *event_info)
{
   Elm_Object_Item *mbe_it = event_info;
   printf("added item = %s\n", elm_object_item_text_get(mbe_it));
}

// "item,deleted" smart callback
static void
_item_deleted_cb(void *data __UNUSED__,
                 Evas_Object *obj __UNUSED__,
                 void *event_info __UNUSED__)
{
   printf("deleted item\n");
}

// "item,clicked" smart callback
static void
_item_clicked_cb(void *data __UNUSED__,
                 Evas_Object *obj __UNUSED__,
                 void *event_info )
{
   Elm_Object_Item *mbe_it = event_info;
   printf("clicked item = %s\n", elm_object_item_text_get(mbe_it));
}

static void
_mbe_clicked_cb(void *data __UNUSED__,
                Evas_Object *obj,
                void *event_info __UNUSED__ )
{
   //Unset the multibuttonentry to contracted mode of single line
   elm_multibuttonentry_expanded_set(obj, EINA_TRUE);
   printf("A multibuttonentry is clicked!\n");
   Evas_Object *entry;
   entry = elm_multibuttonentry_entry_get(obj);
   if (!entry)
     {
        printf("%s entry is NULL\n", __func__);
     }

   Evas_Coord x, y, w, h;
   Evas_Coord mbe_x, mbe_y, mbe_w, mbe_h;
   evas_object_geometry_get(obj, &mbe_x, &mbe_y, &mbe_w, &mbe_h);
   evas_object_geometry_get(entry, &x, &y, &w, &h);
   printf("%s mbe x :%d y:%d w :%d h:%d\n", __func__, mbe_x, mbe_y, mbe_w, mbe_h);
   printf("%s wd->entry x :%d y:%d w :%d h:%d\n", __func__, x, y, w, h);
}

static void
_mbe_unfocused_cb(void *data __UNUSED__,
                  Evas_Object *obj,
                  void *event_info __UNUSED__ )
{
   //Set the multibuttonentry to contracted mode of single line
   elm_multibuttonentry_expanded_set(obj, EINA_FALSE);
   printf("multibuttonentry unfocused!\n");
}

static void
_mbe_focused_cb(void *data __UNUSED__,
                Evas_Object *obj __UNUSED__,
                void *event_info __UNUSED__ )
{
   printf("multibuttonentry focused!\n");
}

// "expanded" smart callback
static void
_expanded_cb(void *data __UNUSED__,
             Evas_Object *obj __UNUSED__,
             void *event_info __UNUSED__)
{
   printf("expanded!\n");
}

// "contracted" smart callback
static void
_contracted_cb(void *data __UNUSED__,
           Evas_Object *obj __UNUSED__,
           void *event_info __UNUSED__)
{
   printf("contracted!\n");
}

// "contracted,state,changed" smart callback
static void
_shrink_state_changed_cb(void *data __UNUSED__,
                         Evas_Object *obj __UNUSED__,
                         void *event_info __UNUSED__)
{
   printf("contracted state changed! \n");
}

// "item verified" confirm callback
static Eina_Bool
_item_filter_cb(Evas_Object *obj __UNUSED__,
                const char* item_label,
                void *item_data __UNUSED__,
                void *data __UNUSED__)
{
   printf("%s, label: %s\n", __func__, item_label);

   return EINA_TRUE;
}

static void
_button_clicked_cb(void *data __UNUSED__,
                   Evas_Object *obj __UNUSED__,
                   void *event_info __UNUSED__)
{
   printf("%s button is clicked\n", __func__);
}

static Evas_Object*
_add_multibuttonentry(Evas_Object *parent)
{
   Evas_Object *scr = NULL;
   Evas_Object *mbe = NULL;
   void *data = NULL;

   scr = elm_scroller_add(parent);
   elm_scroller_bounce_set(scr, EINA_FALSE, EINA_TRUE);
   elm_scroller_policy_set(scr, ELM_SCROLLER_POLICY_OFF,ELM_SCROLLER_POLICY_AUTO);
   evas_object_show(scr);

   mbe = elm_multibuttonentry_add(parent);
   elm_object_text_set(mbe, "To: ");
   elm_object_part_text_set(mbe, "guide", "Tap to add recipient");
   evas_object_size_hint_weight_set(mbe, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(mbe, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_object_content_set(scr, mbe);

   // Add item verify callback to Multibuttonentry
   elm_multibuttonentry_item_filter_append(mbe, _item_filter_cb, data);

   // Add "item,selected","item,added", "item,deleted", "clicked", "unfocused",
   // "expanded", "contracted" and "contracted,state,changed" smart callback
   evas_object_smart_callback_add(mbe, "item,selected", _item_selected_cb, NULL);
   evas_object_smart_callback_add(mbe, "item,added", _item_added_cb, NULL);
   evas_object_smart_callback_add(mbe, "item,deleted", _item_deleted_cb, NULL);
   evas_object_smart_callback_add(mbe, "item,clicked", _item_clicked_cb, NULL);

   evas_object_smart_callback_add(mbe, "clicked", _mbe_clicked_cb, NULL);
   evas_object_smart_callback_add(mbe, "focused", _mbe_focused_cb, NULL);
   evas_object_smart_callback_add(mbe, "unfocused", _mbe_unfocused_cb, NULL);

   evas_object_smart_callback_add(mbe, "expanded", _expanded_cb, NULL);
   evas_object_smart_callback_add(mbe, "contracted", _contracted_cb, NULL);
   evas_object_smart_callback_add(mbe, "shrink,state,changed", _shrink_state_changed_cb, NULL);

   evas_object_resize(mbe, 220, 300);
   elm_object_focus_set(mbe, EINA_TRUE);

   return scr;
}

static Evas_Object*
_add_buttons(Evas_Object *parent)
{
   Evas_Object *bx = NULL;
   Evas_Object *btn;

   bx = elm_box_add(parent);
   elm_box_horizontal_set(bx, EINA_TRUE);
   elm_box_homogeneous_set(bx, EINA_TRUE);

   btn = elm_button_add(parent);
   evas_object_smart_callback_add(btn, "clicked", _button_clicked_cb, NULL);
   elm_object_text_set(btn, "click");
   evas_object_size_hint_weight_set(btn, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(btn, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_box_pack_end(bx, btn);
   evas_object_show(btn);

   return bx;
}

void
test_multibuttonentry(void *data __UNUSED__,
                      Evas_Object *obj __UNUSED__,
                      void *event_info __UNUSED__)
{
   Evas_Object *win, *sc, *bx;
   Evas_Object *ly;
   char buf[PATH_MAX];

   win = elm_win_util_standard_add("multibuttonentry", "MultiButtonEntry");
   elm_win_autodel_set(win, EINA_TRUE);

   ly = elm_layout_add(win);
   snprintf(buf, sizeof(buf), "%s/objects/multibuttonentry.edj", elm_app_data_dir_get());
   elm_layout_file_set(ly, buf, "multibuttonentry_test");
   evas_object_size_hint_weight_set(ly, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, ly);
   evas_object_show(ly);

   sc = _add_multibuttonentry(ly);
   elm_object_part_content_set(ly, "multibuttonentry", sc);

   bx = _add_buttons(ly);
   elm_object_part_content_set(ly, "box", bx);

   evas_object_resize(win, 320, 480);
   evas_object_show(win);
   elm_object_focus_set(sc, EINA_TRUE);
}
#endif
