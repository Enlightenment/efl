#include "widget_preview_tmpl_head.c"

Evas_Object *panes = elm_panes_add(win);
evas_object_size_hint_weight_set(panes, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
elm_win_resize_object_add(win, panes);
evas_object_show(panes);

Evas_Object *bt = elm_button_add(win);
elm_object_text_set(bt, "Left");
evas_object_size_hint_weight_set(bt, 1.0, 1.0);
evas_object_size_hint_align_set(bt, -1.0, -1.0);
evas_object_show(bt);
elm_panes_content_left_set(panes, bt);

bt = elm_button_add(win);
elm_object_text_set(bt, "Right");
evas_object_size_hint_weight_set(bt, 1.0, 1.0);
evas_object_size_hint_align_set(bt, -1.0, -1.0);
evas_object_show(bt);
elm_panes_content_right_set(panes, bt);

#include "widget_preview_tmpl_foot.c"
