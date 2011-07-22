#include "widget_preview_tmpl_head.c"

Evas_Object *layout = elm_layout_add(win);
evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
elm_layout_theme_set(layout, "layout", "application", "toolbar-content-back-next");
elm_win_resize_object_add(win, layout);
evas_object_show(layout);

Evas_Object *icon = elm_icon_add(win);
elm_icon_standard_set(icon, "home");
evas_object_size_hint_weight_set(icon, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
evas_object_size_hint_align_set(icon, EVAS_HINT_FILL, EVAS_HINT_FILL);
evas_object_size_hint_min_set(icon, 200, 140);
evas_object_show(icon);

elm_layout_content_set(layout, "elm.swallow.content", icon);
elm_object_text_part_set(layout, "elm.text.title", "Layout");

#include "widget_preview_tmpl_foot.c"
