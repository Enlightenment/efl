unsigned char _func(void *data);

#include "widget_preview_tmpl_head.c"

Evas_Object *o = elm_flip_add(win);
evas_object_size_hint_weight_set(o, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
elm_win_resize_object_add(win, o);
evas_object_show(o);

Evas_Object *ic = elm_icon_add(win);
elm_icon_standard_set(ic, "home");
evas_object_show(ic);
elm_flip_content_front_set(o, ic);
ic = elm_icon_add(win);
elm_icon_standard_set(ic, "file");
evas_object_show(ic);
elm_flip_content_back_set(o, ic);

ecore_timer_add(0.35, _func, o);

#include "widget_preview_tmpl_foot.c"

unsigned char _func(void *data)
{
   elm_flip_go(data, ELM_FLIP_CUBE_RIGHT);
   return 0;
}