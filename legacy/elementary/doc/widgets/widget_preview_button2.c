#include "widget_preview_tmpl_head.c"

Evas_Object *o = elm_button_add(win);
evas_object_size_hint_weight_set(o, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
elm_win_resize_object_add(win, o);
evas_object_show(o);

elm_object_text_set(o, "Button");

Evas_Object *o2 = elm_icon_add(win);
elm_icon_standard_set(o2, "home");
elm_object_content_set(o, o2);
evas_object_show(o2);

#include "widget_preview_tmpl_foot.c"
