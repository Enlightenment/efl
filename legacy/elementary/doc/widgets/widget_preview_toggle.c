#include "widget_preview_tmpl_head.c"

Evas_Object *o = elm_check_add(win);
elm_object_style_set(o, "toggle");
evas_object_size_hint_weight_set(o, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
elm_win_resize_object_add(win, o);
evas_object_show(o);

elm_object_text_set(o, "toggle");

#include "widget_preview_tmpl_foot.c"
