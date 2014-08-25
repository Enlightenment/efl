#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif
#include <Elementary.h>

void
test_win_dialog(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *win, *bg, *bt;

   win = elm_win_add(data, "window-dialog", ELM_WIN_DIALOG_BASIC);
   elm_win_title_set(win, "Window Dialog");
   elm_win_autodel_set(win, EINA_TRUE);

   bg = elm_bg_add(win);
   evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, bg);
   evas_object_show(bg);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "OK");
   evas_object_size_hint_fill_set(bt, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(bt, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, bt);
   evas_object_show(bt);

   evas_object_resize(win, 320, 160);
   evas_object_show(win);
}
