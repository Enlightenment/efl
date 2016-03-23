#include "widget_preview_tmpl_head.c"

Evas_Object *o = elm_entry_add(win);
elm_entry_scrollable_set(o, EINA_TRUE);
evas_object_size_hint_weight_set(o, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
elm_win_resize_object_add(win, o);
evas_object_show(o);

elm_object_text_set(o, "Some text with <a href=nothing>Anchors</a> and<ps>"
                    "also items taken from the default theme"
                    "<item size=16x16 vsize=full href=emoticon/evil></item>"
                    "showing theme with different sizes<ps>and how they affect"
                    "line heights <item absize=64x64 vsize=full "
                    "href=emoticon/love></item>. This will also scroll.");

#include "widget_preview_tmpl_foot.c"
