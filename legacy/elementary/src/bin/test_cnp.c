#include <Elementary.h>
#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif
#ifndef ELM_LIB_QUICKLAUNCH
static void
_bt_copy_clicked(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Evas_Object *en = (Evas_Object*)(data);
   const char *txt = elm_object_text_get(en);

   elm_cnp_selection_set(ELM_SEL_TYPE_CLIPBOARD, elm_object_parent_widget_get(en),
                         ELM_SEL_FORMAT_TEXT, txt, strlen(txt));
}

static void
_bt_paste_clicked(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Evas_Object *en = (Evas_Object*)(data);

   elm_cnp_selection_get(ELM_SEL_TYPE_CLIPBOARD, ELM_SEL_FORMAT_TEXT,
                         en, NULL, NULL);
}

void
test_cnp(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Evas_Object *win, *bg, *gd, *bt, *en;

   win = elm_win_add(NULL, "CopyPaste", ELM_WIN_BASIC);
   elm_win_title_set(win, "CopyPaste");
   elm_win_autodel_set(win, EINA_TRUE);

   bg = elm_bg_add(win);
   elm_win_resize_object_add(win, bg);
   evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_show(bg);

   gd = elm_grid_add(win);
   elm_grid_size_set(gd, 100, 100);
   elm_win_resize_object_add(win, gd);
   evas_object_size_hint_weight_set(gd, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_show(gd);

   en = elm_entry_add(win);
   elm_entry_scrollable_set(en, EINA_TRUE);
   elm_entry_line_wrap_set(en, ELM_WRAP_CHAR);
   evas_object_size_hint_weight_set(en, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(en, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_entry_entry_set(en,
				"Elementary provides "
                                );
   elm_grid_pack(gd, en, 10, 10, 60, 40);
   evas_object_show(en);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Copy from left entry");
   evas_object_smart_callback_add(bt, "clicked", _bt_copy_clicked, en);
   elm_grid_pack(gd, bt, 70, 10, 22, 40);
   evas_object_show(bt);

   en = elm_entry_add(win);
   elm_entry_scrollable_set(en, EINA_TRUE);
   elm_entry_line_wrap_set(en, ELM_WRAP_CHAR);
   evas_object_size_hint_weight_set(en, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(en, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_entry_entry_set(en,
				"rich copying and pasting functionality,"
                                );
   elm_grid_pack(gd, en, 10, 50, 60, 40);
   evas_object_show(en);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Paste to left entry");
   evas_object_smart_callback_add(bt, "clicked", _bt_paste_clicked, en);
   elm_grid_pack(gd, bt, 70, 50, 22, 40);
   evas_object_show(bt);

   evas_object_resize(win, 480, 140);
   evas_object_show(win);
}
#endif
