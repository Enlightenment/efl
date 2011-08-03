unsigned char _func(void *data);
void *o;

#include "widget_preview_tmpl_head.c"

o = elm_pager_add(win);
evas_object_size_hint_weight_set(o, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
elm_win_resize_object_add(win, o);

Evas_Object *o2 = elm_label_add(win);
elm_object_text_set(o2, "back page");
evas_object_show(o2);
elm_pager_content_push(o, o2);

ecore_timer_add(0.3, _func, o2);

o2 = elm_label_add(win);
elm_object_text_set(o2, "front page");
evas_object_show(o2);
elm_pager_content_push(o, o2);

evas_object_show(o);

#include "widget_preview_tmpl_foot.c"

unsigned char _func(void *data)
{
   elm_pager_content_promote(o, data);
   return 0;
}
