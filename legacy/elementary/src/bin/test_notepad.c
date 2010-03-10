#include <Elementary.h>
#ifndef ELM_LIB_QUICKLAUNCH
static void
my_notepad_bt_1(void *data, Evas_Object *obj, void *event_info)
{
//   Evas_Object *np = data;
}

static void
my_notepad_bt_2(void *data, Evas_Object *obj, void *event_info)
{
//   Evas_Object *np = data;
}

static void
my_notepad_bt_3(void *data, Evas_Object *obj, void *event_info)
{
//   Evas_Object *np = data;
}

void
test_notepad(void *data, Evas_Object *obj, void *event_info)
{
   Evas_Object *win, *bg, *bx, *bx2, *bt, *ic, *np;

   win = elm_win_add(NULL, "notepad", ELM_WIN_BASIC);
   elm_win_title_set(win, "Notepad");
   elm_win_autodel_set(win, 1);

   bg = elm_bg_add(win);
   elm_win_resize_object_add(win, bg);
   evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_show(bg);

   bx = elm_box_add(win);
   evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, bx);
   evas_object_show(bx);

   np = elm_notepad_add(win);
   elm_notepad_file_set(np, "note.txt", ELM_TEXT_FORMAT_PLAIN_UTF8);
   evas_object_size_hint_weight_set(np, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(np, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_box_pack_end(bx, np);
   evas_object_show(np);

   bx2 = elm_box_add(win);
   elm_box_horizontal_set(bx2, 1);
   elm_box_homogenous_set(bx2, 1);
   evas_object_size_hint_weight_set(bx2, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(bx2, EVAS_HINT_FILL, EVAS_HINT_FILL);

   bt = elm_button_add(win);
   ic = elm_icon_add(win);
   elm_icon_standard_set(ic, "arrow_left");
   elm_icon_scale_set(ic, 1, 0);
   elm_button_icon_set(bt, ic);
   evas_object_show(ic);
   evas_object_smart_callback_add(bt, "clicked", my_notepad_bt_1, np);
   evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, 0.0);
   elm_box_pack_end(bx2, bt);
   evas_object_show(bt);

   bt = elm_button_add(win);
   ic = elm_icon_add(win);
   elm_icon_standard_set(ic, "close");
   elm_icon_scale_set(ic, 1, 0);
   elm_button_icon_set(bt, ic);
   evas_object_show(ic);
   evas_object_smart_callback_add(bt, "clicked", my_notepad_bt_2, np);
   evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, 0.0);
   elm_box_pack_end(bx2, bt);
   evas_object_show(bt);

   bt = elm_button_add(win);
   ic = elm_icon_add(win);
   elm_icon_standard_set(ic, "arrow_right");
   elm_icon_scale_set(ic, 1, 0);
   elm_button_icon_set(bt, ic);
   evas_object_show(ic);
   evas_object_smart_callback_add(bt, "clicked", my_notepad_bt_3, np);
   evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, 0.0);
   elm_box_pack_end(bx2, bt);
   evas_object_show(bt);

   elm_box_pack_end(bx, bx2);
   evas_object_show(bx2);

   evas_object_resize(win, 320, 300);

   elm_object_focus(win);
   evas_object_show(win);
}
#endif
