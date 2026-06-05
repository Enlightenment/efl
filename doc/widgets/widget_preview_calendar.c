#include "widget_preview_tmpl_head.c"

Evas_Object *o = elm_calendar_add(win);
evas_object_size_hint_weight_set(o, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
elm_win_resize_object_add(win, o);

Efl_Time t = {};
t.tm_year = 125;
t.tm_mon = 5;
t.tm_mday = 23;

elm_calendar_selected_time_set(o, &t);
evas_object_show(o);

#include "widget_preview_tmpl_foot.c"
