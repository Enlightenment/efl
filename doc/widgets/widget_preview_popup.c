#include "widget_preview_tmpl_head.c"

Evas_Object *o = elm_popup_add(win);
evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
evas_object_size_hint_weight_set(o, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
elm_win_resize_object_add(win, o);
elm_object_part_text_set(o, "title,text", "Title");
elm_object_text_set(o, "Content");

Evas_Object *o2 = elm_button_add(win);
elm_object_text_set(o2, "Close");
elm_object_part_content_set(o, "button1", o2);
evas_object_show(o);
#include "widget_preview_tmpl_foot.c"

