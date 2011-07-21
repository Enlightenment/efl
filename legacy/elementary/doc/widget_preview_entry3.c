#include "widget_preview_tmpl_head.c"

Evas_Object *o = elm_entry_add(win);
elm_entry_single_line_set(o, EINA_TRUE);
elm_entry_scrollable_set(o, EINA_TRUE);
evas_object_size_hint_weight_set(o, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
elm_win_resize_object_add(win, o);
evas_object_show(o);

elm_object_text_set(o, "A single line entry <b>with some format</b> that"
                    "is so long it won't fit in the small screenshot we are"
                    "generating for the documentation.");

#include "widget_preview_tmpl_foot.c"
