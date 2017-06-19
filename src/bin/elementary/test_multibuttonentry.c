#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif
#include <Elementary.h>

static Elm_Multibuttonentry_Format_Cb format_func = NULL;

static char *
_custom_format(int count, void *data EINA_UNUSED)
{
   char buf[32];

   if (!snprintf(buf, sizeof(buf), "+ %d rabbits", count)) return NULL;
   return strdup(buf);
}

static void
_item_selected_cb(void *data EINA_UNUSED,
                  Evas_Object *obj EINA_UNUSED,
                  void *event_info)
{
   Elm_Object_Item *mbe_it = event_info;
   printf("selected item = %s\n", elm_object_item_text_get(mbe_it));
}

// "item,added" smart callback of multibuttonentry.
static void
_item_added_cb(void *data EINA_UNUSED,
               Evas_Object *obj EINA_UNUSED,
               void *event_info)
{
   Elm_Object_Item *mbe_it = event_info;
   printf("added item = %s\n", elm_object_item_text_get(mbe_it));
}

// "item,deleted" smart callback
static void
_item_deleted_cb(void *data EINA_UNUSED,
                 Evas_Object *obj EINA_UNUSED,
                 void *event_info EINA_UNUSED)
{
   printf("deleted item\n");
}

// "item,clicked" smart callback
static void
_item_clicked_cb(void *data EINA_UNUSED,
                 Evas_Object *obj EINA_UNUSED,
                 void *event_info )
{
   Elm_Object_Item *mbe_it = event_info;
   printf("clicked item = %s\n", elm_object_item_text_get(mbe_it));
}

static void
_mbe_clicked_cb(void *data EINA_UNUSED,
                Evas_Object *obj,
                void *event_info EINA_UNUSED )
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
_mbe_unfocused_cb(void *data EINA_UNUSED,
                  Evas_Object *obj,
                  void *event_info EINA_UNUSED )
{
   //Set the multibuttonentry to contracted mode of single line
   elm_multibuttonentry_expanded_set(obj, EINA_FALSE);
   printf("multibuttonentry unfocused!\n");
}

static void
_mbe_focused_cb(void *data EINA_UNUSED,
                Evas_Object *obj EINA_UNUSED,
                void *event_info EINA_UNUSED )
{
   printf("multibuttonentry focused!\n");
}

// "expanded" smart callback
static void
_expanded_cb(void *data EINA_UNUSED,
             Evas_Object *obj EINA_UNUSED,
             void *event_info EINA_UNUSED)
{
   printf("expanded!\n");
}

// "contracted" smart callback
static void
_contracted_cb(void *data EINA_UNUSED,
           Evas_Object *obj EINA_UNUSED,
           void *event_info EINA_UNUSED)
{
   printf("contracted!\n");
}

// "contracted,state,changed" smart callback
static void
_expand_state_changed_cb(void *data EINA_UNUSED,
                         Evas_Object *obj EINA_UNUSED,
                         void *event_info EINA_UNUSED)
{
   if (elm_multibuttonentry_expanded_get(obj))
     printf("expand state changed: EXPANDED \n");
   else
     printf("expand state changed: SHRANK \n");
}

// "longpressed" smart callback
static void
_longpressed_cb(void *data EINA_UNUSED,
                Evas_Object *obj EINA_UNUSED,
                void *event_info)
{
   printf("item = %p longpressed! \n", event_info);
}


// "item verified" confirm callback
static Eina_Bool
_item_filter_cb(Evas_Object *obj EINA_UNUSED,
                const char* item_label,
                void *item_data EINA_UNUSED,
                void *data EINA_UNUSED)
{
   printf("%s, label: %s\n", __func__, item_label);

   return EINA_TRUE;
}

static void
_format_change_cb(void *data,
                   Evas_Object *obj EINA_UNUSED,
                   void *event_info EINA_UNUSED)
{
   Evas_Object *mbe = data;

   if (format_func) format_func = NULL;
   else format_func = _custom_format;

   elm_multibuttonentry_format_function_set(mbe, format_func, NULL);

   printf("Changing format function to %p\n", format_func);
}

static Evas_Object*
_format_change_btn_add(Evas_Object *mbe)
{
   Evas_Object *btn;

   btn = elm_button_add(mbe);
   evas_object_smart_callback_add(btn, "clicked", _format_change_cb, mbe);
   elm_object_text_set(btn, "Change format function");
   evas_object_size_hint_weight_set(btn, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(btn, EVAS_HINT_FILL, EVAS_HINT_FILL);

   return btn;
}

void
_select_cb(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info)
{
   Elm_Object_Item *it = (Elm_Object_Item *)event_info;
   printf("select function called, item = %s\n", elm_object_item_text_get(it));
}

static Evas_Object*
_add_multibuttonentry(Evas_Object *parent)
{
   Evas_Object *scr = NULL;
   Evas_Object *mbe = NULL;
   Evas_Object *btn = NULL;
   Elm_Object_Item *item = NULL;
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
   item = elm_multibuttonentry_item_append(mbe, "mbe3", _select_cb, NULL);
   elm_multibuttonentry_item_prepend(mbe, "mbe1", _select_cb, NULL);
   elm_multibuttonentry_item_insert_before(mbe, item, "mbe2", _select_cb, NULL);
   elm_multibuttonentry_item_insert_after(mbe, item, "mbe4", _select_cb, NULL);

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
   evas_object_smart_callback_add(mbe, "expand,state,changed", _expand_state_changed_cb, NULL);
   evas_object_smart_callback_add(mbe, "item,longpressed", _longpressed_cb, NULL);

   btn = _format_change_btn_add(mbe);
   elm_object_part_content_set(parent, "box", btn);

   evas_object_resize(mbe, 220, 300);
   elm_object_focus_set(mbe, EINA_TRUE);

   return scr;
}

void
test_multibuttonentry(void *data EINA_UNUSED,
                      Evas_Object *obj EINA_UNUSED,
                      void *event_info EINA_UNUSED)
{
   Evas_Object *win, *sc;
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

   evas_object_resize(win, 320, 480);
   evas_object_show(win);
}

static Evas_Object*
_add_multibuttonentry_shrink(Evas_Object *parent)
{
   Evas_Object *scr = NULL;
   Evas_Object *mbe = NULL;
   Evas_Object *btn = NULL;
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
   elm_multibuttonentry_expanded_set(mbe, EINA_FALSE);

   elm_multibuttonentry_item_append(mbe, "mbe1", _select_cb, NULL);
   elm_multibuttonentry_item_append(mbe, "mbe2", _select_cb, NULL);
   elm_multibuttonentry_item_append(mbe, "mbe3", _select_cb, NULL);
   elm_multibuttonentry_item_append(mbe, "mbe4", _select_cb, NULL);
   elm_multibuttonentry_item_append(mbe, "mbe5", _select_cb, NULL);
   elm_multibuttonentry_item_append(mbe, "mbe6", _select_cb, NULL);
   elm_multibuttonentry_item_append(mbe, "mbe7", _select_cb, NULL);
   elm_multibuttonentry_item_append(mbe, "mbe8", _select_cb, NULL);
   elm_multibuttonentry_item_append(mbe, "mbe9", _select_cb, NULL);
   elm_multibuttonentry_item_append(mbe, "mbe10", _select_cb, NULL);

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
   evas_object_smart_callback_add(mbe, "expand,state,changed", _expand_state_changed_cb, NULL);
   evas_object_smart_callback_add(mbe, "item,longpressed", _longpressed_cb, NULL);

   btn = _format_change_btn_add(mbe);
   elm_object_part_content_set(parent, "box", btn);

   evas_object_resize(mbe, 220, 300);
   elm_object_focus_set(mbe, EINA_TRUE);

   return scr;
}

void
test_multibuttonentry2(void *data EINA_UNUSED,
                      Evas_Object *obj EINA_UNUSED,
                      void *event_info EINA_UNUSED)
{
   Evas_Object *win, *sc;
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

   sc = _add_multibuttonentry_shrink(ly);
   elm_object_part_content_set(ly, "multibuttonentry", sc);

   evas_object_resize(win, 320, 480);
   evas_object_show(win);
}
