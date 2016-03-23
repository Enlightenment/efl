#include "widget_preview_tmpl_head.c"

static const char LAYOUT_EPB[] = DOCW_DIR\
                                 "/widget_preview_prefs.epb";

static const char LAYOUT_CFG[] = DOCW_DIR\
                                 "/widget_preview_prefs.cfg";

Evas_Object *prefs;
Elm_Prefs_Data *prefs_data;

prefs = elm_prefs_add(win);
evas_object_size_hint_weight_set(prefs, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
elm_win_resize_object_add(win, prefs);
evas_object_show(prefs);

elm_prefs_autosave_set(prefs, EINA_TRUE);

prefs_data = elm_prefs_data_new(LAYOUT_CFG, NULL, EET_FILE_MODE_READ_WRITE);

elm_prefs_file_set(prefs, LAYOUT_EPB, NULL);
elm_prefs_data_set(prefs, prefs_data);

#include "widget_preview_tmpl_foot.c"
