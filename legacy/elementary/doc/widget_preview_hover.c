#include "widget_preview_tmpl_head.c"

Evas_Object *o = elm_hover_add(win);
evas_object_move(o, w/2, h/2);

Evas_Object *lbl = elm_label_add(win);
evas_object_size_hint_weight_set(lbl, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
elm_object_text_set(lbl, "hover");
elm_hover_content_set(o, "middle", lbl);


evas_object_show(o);

#include "widget_preview_tmpl_foot.c"
