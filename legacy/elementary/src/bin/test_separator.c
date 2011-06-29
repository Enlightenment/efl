#include <Elementary.h>
#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif
#ifndef ELM_LIB_QUICKLAUNCH
void
test_separator(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Evas_Object *win, *bg, *bx0, *bx, *bt, *sp;

   win = elm_win_add(NULL, "separators", ELM_WIN_BASIC);
   elm_win_title_set(win, "Separators");
   elm_win_autodel_set(win, 1);

   bg = elm_bg_add(win);
   elm_win_resize_object_add(win, bg);
   evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_show(bg);

   bx0 = elm_box_add(win);
   evas_object_size_hint_weight_set(bx0, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_box_horizontal_set(bx0, 1);
   elm_win_resize_object_add(win, bx0);
   evas_object_show(bx0);

   bx = elm_box_add(win);
   evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_box_pack_end(bx0, bx);
   evas_object_show(bx);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Left upper corner");
   elm_box_pack_end(bx, bt);
   evas_object_show(bt);

   sp = elm_separator_add(win);
   elm_separator_horizontal_set(sp, 1); // by default, separator is vertical, we must set it horizontal
   elm_box_pack_end(bx, sp);
   evas_object_show(sp);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Left lower corner");
   elm_object_disabled_set(bt, 1);
   elm_box_pack_end(bx, bt);
   evas_object_show(bt);

   sp = elm_separator_add(win); // now we need vertical separator
   elm_box_pack_end(bx0, sp);
   evas_object_show(sp);

   bx = elm_box_add(win);
   evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_box_pack_end(bx0, bx);
   evas_object_show(bx);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Right upper corner");
   elm_object_disabled_set(bt, 1);
   elm_box_pack_end(bx, bt);
   evas_object_show(bt);

   sp = elm_separator_add(win);
   elm_separator_horizontal_set(sp, 1);
   elm_box_pack_end(bx, sp);
   evas_object_show(sp);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Right lower corner");
   elm_box_pack_end(bx, bt);
   evas_object_show(bt);

   evas_object_show(win);
}
#endif
