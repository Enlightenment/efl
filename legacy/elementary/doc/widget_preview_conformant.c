#include "widget_preview_tmpl_head.c"

evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

Evas_Object *conform = elm_conformant_add(win);
elm_win_resize_object_add(win, conform);
evas_object_size_hint_weight_set(conform, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
evas_object_show(conform);

Evas_Object *bx = elm_box_add(win);
evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
evas_object_size_hint_align_set(bx, EVAS_HINT_FILL, EVAS_HINT_FILL);
evas_object_show(bx);

Evas_Object *btn = elm_button_add(win);
elm_object_text_set(btn, "Conformant");
evas_object_size_hint_weight_set(btn, EVAS_HINT_EXPAND, 0);
evas_object_size_hint_align_set(btn, EVAS_HINT_FILL, 0);
evas_object_show(btn);
elm_box_pack_end(bx, btn);

Evas_Object *en = elm_entry_add(win);
elm_entry_scrollable_set(en, EINA_TRUE);
elm_entry_entry_set(en,
                    "This is a multi-line entry<br>"
                    "This can contain more than 1 line of text and be "
                    "scrolled around to allow for entering of lots of "
                    "content. It is also to test to see that autoscroll "
                    "moves to the right part of a larger multi-line "
                    "text entry that is inside of a scroller than can be "
                    "scrolled around, thus changing the expected position "
                    "as well as cursor changes updating auto-scroll when "
                    "it is enabled.");

evas_object_size_hint_weight_set(en, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
evas_object_size_hint_align_set(en, EVAS_HINT_FILL, EVAS_HINT_FILL);
evas_object_show(en);
elm_box_pack_end(bx, en);

elm_conformant_content_set(conform, bx);

#include "widget_preview_tmpl_foot.c"
