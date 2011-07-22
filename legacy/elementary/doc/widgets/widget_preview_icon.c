#include "widget_preview_tmpl_head.c"

Evas_Object *o = elm_icon_add(win);
evas_object_size_hint_weight_set(o, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
elm_win_resize_object_add(win, o);

elm_icon_standard_set(o, "home");
elm_icon_scale_set(o, EINA_FALSE, EINA_FALSE);
evas_object_show(o);

#include "widget_preview_tmpl_foot.c"
