#include <Elementary.h>
#ifndef ELM_LIB_QUICKLAUNCH

void 
test_conformant(void *data, Evas_Object *obj, void *event) 
{
   Evas_Object *win, *bg, *conform, *btn;

   win = elm_win_add(NULL, "conformant", ELM_WIN_BASIC);
   elm_win_title_set(win, "Conformant");
   elm_win_autodel_set(win, 1);
   elm_win_conformant_set(win, 1);

   bg = elm_bg_add(win);
   elm_win_resize_object_add(win, bg);
   evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_show(bg);

   conform = elm_conformant_add(win);
   elm_win_resize_object_add(win, conform);
   evas_object_size_hint_weight_set(conform, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_show(conform);

   btn = elm_button_add(win);
   elm_button_label_set(btn, "Test Conformant");
   evas_object_show(btn);

   elm_conformant_content_set(conform, btn);

   evas_object_resize(win, 200, 200);
   evas_object_show(win);
}

#endif
