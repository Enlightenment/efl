unsigned char _func(void *data);

#include "widget_preview_tmpl_head.c"

Evas_Object *o = elm_hoversel_add(win);
evas_object_size_hint_weight_set(o, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
evas_object_resize(o, 90, 30);
evas_object_show(o);

elm_object_text_set(o, "Hover selector");
elm_hoversel_item_add(o, "first", NULL, 0, NULL, NULL);
elm_hoversel_item_add(o, "second", NULL, 0, NULL, NULL);
elm_hoversel_item_add(o, "third", NULL, 0, NULL, NULL);

ecore_timer_add(0, _func, o);

#include "widget_preview_tmpl_foot.c"

unsigned char _func(void *data)
{
   elm_hoversel_hover_begin(data);
   return 0;
}