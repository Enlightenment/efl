#include "widget_preview_tmpl_head.c"

Evas_Object *o = elm_segment_control_add(win);
evas_object_size_hint_weight_set(o, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
elm_win_resize_object_add(win, o);
evas_object_show(o);

Evas_Object *ic = elm_icon_add(win);
elm_icon_standard_set(ic, "home");
elm_segment_control_item_add(o, ic, "Home");

ic = elm_icon_add(win);
elm_icon_standard_set(ic, "folder");
elm_segment_control_item_add(o, ic, "Downloads");

ic = elm_icon_add(win);
elm_icon_standard_set(ic, "close");
elm_segment_control_item_add(o, ic, "Close");


#include "widget_preview_tmpl_foot.c"
