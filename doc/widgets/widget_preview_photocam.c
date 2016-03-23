#include "widget_preview_tmpl_head.c"

Evas_Object *o = elm_photocam_add(win);
elm_photocam_file_set(o, PACKAGE_DATA_DIR"/images/insanely_huge_test_image.jpg");
elm_photocam_zoom_mode_set(o, ELM_PHOTOCAM_ZOOM_MODE_AUTO_FIT);
evas_object_size_hint_weight_set(o, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
elm_win_resize_object_add(win, o);
evas_object_show(o);

#include "widget_preview_tmpl_foot.c"
