/* Test for Focus Chain Linear*/
#include <Elementary.h>
#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif
#ifndef ELM_LIB_QUICKLAUNCH

static Evas_Object *foc = NULL;

static void
_foc(void *data __UNUSED__, Evas_Object *obj, void *event_info __UNUSED__)
{
   foc = obj;
   printf("foc -> %p\n", foc);
}

static void
_unfoc(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   foc = NULL;
   printf("foc -> %p\n", foc);
}

static void
_add(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Evas_Object *bx = data, *en;

   en = elm_entry_add(elm_object_top_widget_get(bx));
   elm_entry_scrollable_set(en, EINA_TRUE);
   elm_entry_entry_set(en, "An entry");
   evas_object_smart_callback_add(en, "focused", _foc, NULL);
   evas_object_smart_callback_add(en, "unfocused", _unfoc, NULL);
   evas_object_size_hint_weight_set(en, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(en, EVAS_HINT_FILL, 0.5);
   elm_entry_single_line_set(en, 1);
   elm_box_pack_start(bx, en);
   evas_object_show(en);
}

static void
_del(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   if (foc) evas_object_del(foc);
}

static void
_hide(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   if (foc) evas_object_hide(foc);
}

void
test_focus3(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Evas_Object *win, *bg, *bx, *bt, *en;

   win = elm_win_add(NULL, "focus3", ELM_WIN_BASIC);
   elm_win_title_set(win, "Focus 3");
   elm_win_focus_highlight_enabled_set(win, EINA_TRUE);
   evas_object_resize(win, 320, 480);
   elm_win_autodel_set(win, EINA_TRUE);

   bg = elm_bg_add(win);
   elm_win_resize_object_add(win, bg);
   evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_show(bg);

   bx = elm_box_add(win);
   evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, bx);
   evas_object_show(bx);

   en = elm_entry_add(win);
   elm_entry_scrollable_set(en, EINA_TRUE);
   elm_entry_entry_set(en, "An entry");
   evas_object_smart_callback_add(en, "focused", _foc, NULL);
   evas_object_smart_callback_add(en, "unfocused", _unfoc, NULL);
   evas_object_size_hint_weight_set(en, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(en, EVAS_HINT_FILL, 0.5);
   elm_entry_single_line_set(en, 1);
   elm_box_pack_end(bx, en);
   evas_object_show(en);

   bt = elm_button_add(win);
   elm_object_focus_allow_set(bt, 0);
   elm_button_label_set(bt, "Add");
   evas_object_smart_callback_add(bt, "clicked", _add, bx);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, 0.5);
   elm_box_pack_end(bx, bt);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_object_focus_allow_set(bt, 0);
   elm_button_label_set(bt, "Del");
   evas_object_smart_callback_add(bt, "clicked", _del, NULL);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, 0.5);
   elm_box_pack_end(bx, bt);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_object_focus_allow_set(bt, 0);
   elm_button_label_set(bt, "Hide");
   evas_object_smart_callback_add(bt, "clicked", _hide, NULL);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(bt, EVAS_HINT_FILL, 0.5);
   elm_box_pack_end(bx, bt);
   evas_object_show(bt);

   evas_object_show(win);
}
#endif
