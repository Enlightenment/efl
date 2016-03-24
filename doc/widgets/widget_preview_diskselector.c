#include "widget_preview_tmpl_head.c"

Evas_Object *bx = elm_box_add(win);
evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
evas_object_size_hint_align_set(bx, EVAS_HINT_FILL, EVAS_HINT_FILL);
elm_win_resize_object_add(win, bx);
evas_object_show(bx);

Evas_Object *ds = elm_diskselector_add(win);
evas_object_size_hint_weight_set(ds, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
evas_object_size_hint_align_set(ds, EVAS_HINT_FILL, EVAS_HINT_FILL);
elm_box_pack_end(bx, ds);
elm_diskselector_item_append(ds, "item", NULL, NULL, NULL);
elm_diskselector_item_append(ds, "item2", NULL, NULL, NULL);
evas_object_show(ds);

#include "widget_preview_tmpl_foot.c"
