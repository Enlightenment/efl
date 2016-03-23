#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif
#include <Elementary.h>



void
_bt_clicked_cb(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *dia, *lb;
   Evas_Object *parent = data;

   dia = elm_win_util_dialog_add(parent, "window-dia", "A Dialog Window");
   elm_win_autodel_set(dia, EINA_TRUE);

   lb = elm_label_add(dia);
   elm_object_text_set(lb, "This is a Dialog Window");
   evas_object_size_hint_weight_set(lb, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(dia, lb);
   evas_object_show(lb);

   evas_object_resize(dia, 200, 150);
   evas_object_show(dia);
}

void
test_win_dialog(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *win, *box, *bt;

   win = elm_win_util_standard_add("window-dia", "A Standard Window");
   elm_win_autodel_set(win, EINA_TRUE);

   box = elm_box_add(win);
   evas_object_size_hint_weight_set(box, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, box);
   evas_object_show(box);
   
   bt = elm_button_add(win);
   elm_object_text_set(bt, "Create a new dialog");
   evas_object_smart_callback_add(bt, "clicked", _bt_clicked_cb, win);
   elm_box_pack_end(box, bt);
   evas_object_show(bt);

   evas_object_resize(win, 400, 400);
   evas_object_show(win);
}
