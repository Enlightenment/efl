#include "widget_preview_tmpl_head.c"
Evas_Object *bx = elm_box_add(win);
evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
elm_win_resize_object_add(win, bx);
evas_object_show(bx);

Evas_Object *o = elm_colorselector_add(win);
evas_object_size_hint_weight_set(o, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
evas_object_size_hint_align_set(o, EVAS_HINT_FILL, EVAS_HINT_FILL);;
elm_box_pack_end(bx, o);
evas_object_show(o);

#include "widget_preview_tmpl_foot.c"
