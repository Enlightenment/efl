#include "widget_preview_tmpl_head.c"

Evas_Object *o = elm_scroller_add(win);
elm_scroller_policy_set(o, ELM_SCROLLER_POLICY_ON, ELM_SCROLLER_POLICY_ON);
evas_object_size_hint_weight_set(o, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
elm_win_resize_object_add(win, o);
evas_object_show(o);

Evas_Object *o2 = elm_label_add(win);
elm_object_text_set(o2, "This is the content of the scroller");
evas_object_show(o2);
elm_object_content_set(o, o2);

#include "widget_preview_tmpl_foot.c"
