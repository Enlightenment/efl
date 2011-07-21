#include "widget_preview_tmpl_head.c"

Evas_Object *o = elm_bg_add(win);
evas_object_size_hint_weight_set(o, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
elm_win_resize_object_add(win, o);

elm_bg_file_set(o, PACKAGE_DATA_DIR"/images/logo_small.png", NULL);
evas_object_show(o);

#include "widget_preview_tmpl_foot.c"
