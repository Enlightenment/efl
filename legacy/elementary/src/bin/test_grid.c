#include <Elementary.h>
#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif
#ifndef ELM_LIB_QUICKLAUNCH
static void
_ch_grid(void *data __UNUSED__, Evas_Object *obj, void *event_info __UNUSED__)
{
   int x, y, w, h;

   elm_grid_pack_get(obj, &x, &y, &w, &h);
   elm_grid_pack_set(obj, x - 1, y - 1, w + 2, h + 2);
}

void
test_grid(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Evas_Object *win, *bg, *gd, *bt, *rc, *en;

   win = elm_win_add(NULL, "grid", ELM_WIN_BASIC);
   elm_win_title_set(win, "Grid");
   elm_win_autodel_set(win, 1);

   bg = elm_bg_add(win);
   evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, bg);
   evas_object_show(bg);

   gd = elm_grid_add(win);
   elm_grid_size_set(gd, 100, 100);
   elm_win_resize_object_add(win, gd);
   evas_object_size_hint_weight_set(gd, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_show(gd);

   en = elm_entry_add(win);
   elm_entry_scrollable_set(en, EINA_TRUE);
   elm_entry_entry_set(en, "Entry text");
   elm_entry_single_line_set(en, 1);
   elm_grid_pack(gd, en, 50, 10, 40, 10);
   evas_object_show(en);

   en = elm_entry_add(win);
   elm_entry_scrollable_set(en, EINA_TRUE);
   elm_entry_entry_set(en, "Entry text 2");
   elm_entry_single_line_set(en, 1);
   elm_grid_pack(gd, en, 60, 20, 30, 10);
   evas_object_show(en);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Button");
   elm_grid_pack(gd, bt,  0,  0, 20, 20);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Button");
   elm_grid_pack(gd, bt, 10, 10, 40, 20);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Button");
   elm_grid_pack(gd, bt, 10, 30, 20, 50);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Button");
   elm_grid_pack(gd, bt, 80, 80, 20, 20);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Change");
   elm_grid_pack(gd, bt, 40, 40, 20, 20);
   evas_object_smart_callback_add(bt, "clicked", _ch_grid, gd);
   evas_object_show(bt);

   rc = evas_object_rectangle_add(evas_object_evas_get(win));
   evas_object_color_set(rc, 128, 0, 0, 128);
   elm_grid_pack(gd, rc, 40, 70, 20, 10);
   evas_object_show(rc);

   rc = evas_object_rectangle_add(evas_object_evas_get(win));
   evas_object_color_set(rc, 0, 128, 0, 128);
   elm_grid_pack(gd, rc, 60, 70, 10, 10);
   evas_object_show(rc);

   rc = evas_object_rectangle_add(evas_object_evas_get(win));
   evas_object_color_set(rc, 0, 0, 128, 128);
   elm_grid_pack(gd, rc, 40, 80, 10, 10);
   evas_object_show(rc);

   rc = evas_object_rectangle_add(evas_object_evas_get(win));
   evas_object_color_set(rc, 128, 0, 128, 128);
   elm_grid_pack(gd, rc, 50, 80, 10, 10);
   evas_object_show(rc);

   rc = evas_object_rectangle_add(evas_object_evas_get(win));
   evas_object_color_set(rc, 128, 64, 0, 128);
   elm_grid_pack(gd, rc, 60, 80, 10, 10);
   evas_object_show(rc);

   evas_object_resize(win, 480, 480);
   evas_object_show(win);
}
#endif
