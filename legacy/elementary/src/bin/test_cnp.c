#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif
#include <Elementary.h>

static Evas_Object *glb;

static void
_bt_copy_clicked(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *en = (Evas_Object*)(data);
   const char *txt = elm_object_text_get(en);

   elm_object_text_set(glb, txt);
   elm_cnp_selection_set(elm_object_parent_widget_get(en), ELM_SEL_TYPE_CLIPBOARD,
                         ELM_SEL_FORMAT_TEXT, txt, strlen(txt));
}

static void
_bt_paste_clicked(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *en = (Evas_Object*)(data);

   elm_cnp_selection_get(en, ELM_SEL_TYPE_CLIPBOARD, ELM_SEL_FORMAT_TEXT,
                         NULL, NULL);
}

static void
_bt_clear_clicked(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *en = (Evas_Object*)(data);

   elm_object_text_set(glb, "");
   elm_object_cnp_selection_clear(elm_object_parent_widget_get(en), ELM_SEL_TYPE_CLIPBOARD);
}

void
test_cnp(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *win, *gd, *bt, *en, *lb;

   win = elm_win_util_standard_add("copypaste", "CopyPaste");
   elm_win_autodel_set(win, EINA_TRUE);

   gd = elm_grid_add(win);
   elm_grid_size_set(gd, 100, 100);
   evas_object_size_hint_weight_set(gd, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, gd);
   evas_object_show(gd);

   en = elm_entry_add(win);
   elm_entry_scrollable_set(en, EINA_TRUE);
   elm_entry_line_wrap_set(en, ELM_WRAP_CHAR);
   evas_object_size_hint_weight_set(en, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(en, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_object_text_set(en, "Elementary provides ");
   elm_grid_pack(gd, en, 10, 10, 60, 30);
   evas_object_show(en);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Copy from left entry");
   evas_object_smart_callback_add(bt, "clicked", _bt_copy_clicked, en);
   elm_grid_pack(gd, bt, 70, 10, 22, 30);
   evas_object_show(bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Clear clipboard");
   evas_object_smart_callback_add(bt, "clicked", _bt_clear_clicked, en);
   elm_grid_pack(gd, bt, 70, 70, 22, 20);
   evas_object_show(bt);

   en = elm_entry_add(win);
   elm_entry_scrollable_set(en, EINA_TRUE);
   elm_entry_line_wrap_set(en, ELM_WRAP_CHAR);
   evas_object_size_hint_weight_set(en, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(en, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_object_text_set(en, "rich copying and pasting functionality,");
   elm_grid_pack(gd, en, 10, 40, 60, 30);
   evas_object_show(en);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Paste to left entry");
   evas_object_smart_callback_add(bt, "clicked", _bt_paste_clicked, en);
   elm_grid_pack(gd, bt, 70, 40, 22, 30);
   evas_object_show(bt);

   lb = elm_label_add(win);
   elm_object_text_set(lb, "<b>Clipboard:</b>");
   evas_object_size_hint_weight_set(lb, 0.0, 0.0);
   evas_object_size_hint_align_set(lb, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_grid_pack(gd, lb, 10, 70, 60, 10);
   evas_object_show(lb);

   glb = elm_label_add(win);
   elm_object_text_set(glb, "");
   evas_object_size_hint_weight_set(glb, 0.0, 0.0);
   evas_object_size_hint_align_set(glb, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_grid_pack(gd, glb, 10, 80, 60, 10);
   evas_object_show(glb);

   evas_object_resize(win, 480, 200);
   evas_object_show(win);
}
