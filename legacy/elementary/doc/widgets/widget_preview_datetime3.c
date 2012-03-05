#include "widget_preview_tmpl_head.c"

Evas_Object *bx = elm_box_add(win);
evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
elm_win_resize_object_add(win, bx);
evas_object_show(bx);
evas_object_size_hint_min_set(bx, 200, 60);

Evas_Object *datetime = elm_datetime_add(win);
evas_object_size_hint_weight_set(datetime, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
evas_object_size_hint_align_set(datetime, EVAS_HINT_FILL, EVAS_HINT_FILL);
elm_datetime_field_visible_set(datetime, ELM_DATETIME_YEAR, EINA_FALSE);
elm_datetime_field_visible_set(datetime, ELM_DATETIME_MONTH, EINA_FALSE);
elm_datetime_field_visible_set(datetime, ELM_DATETIME_DATE, EINA_FALSE);
elm_box_pack_end(bx, datetime);
evas_object_show(datetime);

#include "widget_preview_tmpl_foot.c"
