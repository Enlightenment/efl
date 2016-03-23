#include "widget_preview_tmpl_head.c"

Evas_Object *o = elm_fileselector_add(win);
evas_object_resize(o, 300, 300);
evas_object_show(o);
elm_fileselector_path_set(o, PACKAGE_DATA_DIR);

#include "widget_preview_tmpl_foot.c"
