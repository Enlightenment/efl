#include <Elementary.h>
#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif
#ifndef ELM_LIB_QUICKLAUNCH
#include <Elementary_Cursor.h>

static void
_clicked_cb(void *data, Evas_Object *obj, void *event_info __UNUSED__)
{
   elm_object_text_part_set(data, "text", elm_object_text_get(obj));
}

void
test_layout(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Evas_Object *win, *bg, *box, *ly, *bt;
   char buf[PATH_MAX];

   win = elm_win_add(NULL, "layout", ELM_WIN_BASIC);
   elm_win_title_set(win, "Layout");
   elm_win_autodel_set(win, EINA_TRUE);

   bg = elm_bg_add(win);
   elm_win_resize_object_add(win, bg);
   evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_show(bg);

   box = elm_box_add(win);
   evas_object_size_hint_weight_set(box, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(box, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_win_resize_object_add(win, box);
   evas_object_show(box);

   ly = elm_layout_add(win);
   elm_layout_theme_set(ly, "layout", "application", "titlebar");
   elm_object_text_set(ly, "Some title");
   evas_object_size_hint_weight_set(ly, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(ly, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_box_pack_end(box, ly);
   evas_object_show(ly);

   bt = elm_icon_add(win);
   elm_icon_standard_set(bt, "chat");
   evas_object_size_hint_min_set(bt, 20, 20);
   elm_layout_icon_set(ly, bt);

   bt = elm_icon_add(win);
   elm_icon_standard_set(bt, "close");
   evas_object_size_hint_min_set(bt, 20, 20);
   elm_layout_end_set(ly, bt);

   ly = elm_layout_add(win);
   snprintf(buf, sizeof(buf), "%s/objects/test.edj", elm_app_data_dir_get());
   elm_layout_file_set(ly, buf, "layout");
   evas_object_size_hint_weight_set(ly, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_box_pack_end(box, ly);
   //elm_win_resize_object_add(win, ly);
   evas_object_show(ly);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Button 1");
   elm_object_content_part_set(ly, "element1", bt);
   evas_object_smart_callback_add(bt, "clicked", _clicked_cb, ly);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Button 2");
   elm_object_content_part_set(ly, "element2", bt);
   evas_object_smart_callback_add(bt, "clicked", _clicked_cb, ly);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Button 3");
   elm_object_content_part_set(ly, "element3", bt);
   evas_object_smart_callback_add(bt, "clicked", _clicked_cb, ly);
   evas_object_show(bt);

   elm_layout_part_cursor_set(ly, "text", ELM_CURSOR_WATCH);

   evas_object_show(win);
}
#endif
