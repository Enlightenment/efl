#include "widget_preview_tmpl_head.c"

Evas_Object *o = elm_label_add(win);
elm_object_text_set(o, "Here be window contents");
elm_win_resize_object_add(win, o);
evas_object_show(o);

#include "widget_preview_tmpl_foot.c"
