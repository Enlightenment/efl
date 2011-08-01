#include "widget_preview_tmpl_head.c"

elm_need_ethumb();

Evas_Object *o = elm_thumb_add(win);
evas_object_size_hint_weight_set(o, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
elm_win_resize_object_add(win, o);

elm_thumb_file_set(o, PACKAGE_DATA_DIR"/images/plant_01.jpg", NULL);
evas_object_show(o);

#include "widget_preview_tmpl_foot.c"
