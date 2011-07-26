#include "widget_preview_tmpl_head.c"

Evas_Object *ic, *pb, *bx;

bx = elm_box_add(win);
evas_object_show(bx);
elm_win_resize_object_add(win, bx);

ic = elm_icon_add(win);
elm_icon_standard_set(ic, "file");
evas_object_size_hint_aspect_set(ic, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);
evas_object_show(ic);

/* pb with label, icon, custom unit label and span size set */
pb = elm_progressbar_add(win);
elm_object_text_set(pb, "Label");
elm_progressbar_span_size_set(pb, 100);
elm_progressbar_icon_set(pb, ic);
elm_progressbar_unit_format_set(pb, "%1.1f units");
evas_object_size_hint_align_set(pb, EVAS_HINT_FILL, EVAS_HINT_FILL);
evas_object_size_hint_weight_set(pb, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
evas_object_show(pb);
elm_progressbar_value_set(pb, 0.5);
elm_box_pack_end(bx, pb);

#include "widget_preview_tmpl_foot.c"
