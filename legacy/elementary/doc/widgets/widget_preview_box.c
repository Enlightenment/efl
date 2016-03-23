#include "widget_preview_tmpl_head.c"

Evas_Object *o;
Evas_Object *box = elm_box_add(win);
evas_object_size_hint_weight_set(box, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
elm_win_resize_object_add(win, box);
evas_object_show(box);

o = elm_button_add(win);
elm_object_text_set(o, "A button");
elm_box_pack_end(box, o);
evas_object_show(o);

o = elm_entry_add(win);
elm_entry_single_line_set(o, EINA_TRUE);
elm_entry_scrollable_set(o, EINA_TRUE);
elm_object_text_set(o, "An entry with text");
evas_object_size_hint_weight_set(o, EVAS_HINT_EXPAND, 0.0);
evas_object_size_hint_align_set(o, EVAS_HINT_FILL, EVAS_HINT_FILL);
elm_box_pack_end(box, o);
evas_object_show(o);

o = elm_button_add(win);
elm_object_text_set(o, "Another button");
evas_object_size_hint_weight_set(o, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
evas_object_size_hint_align_set(o, EVAS_HINT_FILL, EVAS_HINT_FILL);
elm_box_pack_end(box, o);
evas_object_show(o);

#include "widget_preview_tmpl_foot.c"
