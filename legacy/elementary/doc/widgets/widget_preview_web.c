#include "widget_preview_tmpl_head.c"

elm_need_web();

Evas_Object *o = elm_web_add(win);
evas_object_size_hint_weight_set(o, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
elm_win_resize_object_add(win, o);
evas_object_show(o);

elm_web_uri_set(o, "http://www.enlightenment.org");

#include "widget_preview_tmpl_foot.c"
