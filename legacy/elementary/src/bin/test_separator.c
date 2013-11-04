#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif
#include <Elementary.h>

void
test_separator(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *win, *bx0, *bx, *bt, *sp;

   win = elm_win_util_standard_add("separators", "Separators");
   elm_win_autodel_set(win, EINA_TRUE);

   bx0 = elm_box_add(win);
   evas_object_size_hint_weight_set(bx0, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_box_horizontal_set(bx0, EINA_TRUE);
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
   elm_separator_horizontal_set(sp, EINA_TRUE); // by default, separator is vertical, we must set it horizontal
   elm_box_pack_end(bx, sp);
   evas_object_show(sp);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Left lower corner");
   elm_object_disabled_set(bt, EINA_TRUE);
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
   elm_object_disabled_set(bt, EINA_TRUE);
   elm_box_pack_end(bx, bt);
   evas_object_show(bt);

   sp = elm_separator_add(win);
   elm_separator_horizontal_set(sp, EINA_TRUE);
   elm_box_pack_end(bx, sp);
   evas_object_show(sp);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Right lower corner");
   elm_box_pack_end(bx, bt);
   evas_object_show(bt);

   evas_object_show(win);
}
