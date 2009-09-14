#include <Elementary.h>
#ifndef ELM_LIB_QUICKLAUNCH
void
test_spinner(void *data, Evas_Object *obj, void *event_info)
{
   Evas_Object *win, *bg, *bx, *sp;

   win = elm_win_add(NULL, "spinner", ELM_WIN_BASIC);
   elm_win_title_set(win, "Spinner");
   elm_win_autodel_set(win, 1);

   bg = elm_bg_add(win);
   elm_win_resize_object_add(win, bg);
   evas_object_size_hint_weight_set(bg, 1.0, 1.0);
   evas_object_show(bg);

   bx = elm_box_add(win);
   evas_object_size_hint_weight_set(bx, 1.0, 1.0);
   elm_win_resize_object_add(win, bx);
   evas_object_show(bx);

   sp = elm_spinner_add(win);
   elm_spinner_label_format_set(sp, "%1.1f units");
   elm_spinner_step_set(sp, 1.3);
   elm_spinner_wrap_set(sp, 1);
   elm_spinner_min_max_set(sp, -50.0, 250.0);
   evas_object_size_hint_align_set(sp, -1.0, 0.5);
   evas_object_size_hint_weight_set(sp, 1.0, 1.0);
   elm_box_pack_end(bx, sp);
   evas_object_show(sp);

   sp = elm_spinner_add(win);
   elm_spinner_label_format_set(sp, "Disabled %.0f");
   elm_object_disabled_set(sp, 1);
   elm_spinner_min_max_set(sp, -50.0, 250.0);
   evas_object_size_hint_align_set(sp, -1.0, 0.5);
   evas_object_size_hint_weight_set(sp, 1.0, 1.0);
   elm_box_pack_end(bx, sp);
   evas_object_show(sp);

   evas_object_show(win);
}
#endif
