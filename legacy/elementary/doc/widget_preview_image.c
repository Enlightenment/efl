#include "widget_preview_tmpl_head.c"

Evas_Object *o = elm_image_add(win);
evas_object_size_hint_weight_set(o, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
elm_win_resize_object_add(win, o);

elm_image_file_set(o, PACKAGE_DATA_DIR"/images/logo_small.png", NULL);
elm_image_scale_set(o, EINA_FALSE, EINA_FALSE);
evas_object_show(o);

#include "widget_preview_tmpl_foot.c"
