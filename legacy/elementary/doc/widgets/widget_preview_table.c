#include "widget_preview_tmpl_head.c"

Evas_Object *o = elm_table_add(win);
evas_object_size_hint_weight_set(o, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
elm_win_resize_object_add(win, o);
evas_object_show(o);

Evas_Object *o2 = evas_object_rectangle_add(evas_object_evas_get(win));
evas_object_color_set(o2, 255, 0, 0, 255);
evas_object_size_hint_min_set(o2, 50, 50);
evas_object_show(o2);
elm_table_pack(o, o2, 0, 0, 1, 1);

o2 = evas_object_rectangle_add(evas_object_evas_get(win));
evas_object_color_set(o2, 0, 255, 0, 255);
evas_object_size_hint_min_set(o2, 50, 50);
evas_object_show(o2);
elm_table_pack(o, o2, 1, 0, 1, 1);

o2 = evas_object_rectangle_add(evas_object_evas_get(win));
evas_object_color_set(o2, 0, 0, 255, 255);
evas_object_size_hint_min_set(o2, 50, 50);
evas_object_show(o2);
elm_table_pack(o, o2, 0, 1, 1, 1);

o2 = evas_object_rectangle_add(evas_object_evas_get(win));
evas_object_color_set(o2, 255, 255, 0, 255);
evas_object_size_hint_min_set(o2, 50, 50);
evas_object_show(o2);
elm_table_pack(o, o2, 1, 1, 1, 1);

#include "widget_preview_tmpl_foot.c"
