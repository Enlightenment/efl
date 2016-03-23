#include "widget_preview_tmpl_head.c"

Evas_Object *o = elm_toolbar_add(win);
evas_object_size_hint_weight_set(o, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
elm_win_resize_object_add(win, o);
evas_object_show(o);

elm_toolbar_item_append(o, "document-print", "Print", NULL, NULL);
elm_toolbar_item_append(o, "folder-new", "New Folder", NULL, NULL);
elm_toolbar_item_append(o, "mail-send", "Send Mail", NULL, NULL);
elm_toolbar_item_append(o, "clock", "Clock", NULL, NULL);

#include "widget_preview_tmpl_foot.c"
