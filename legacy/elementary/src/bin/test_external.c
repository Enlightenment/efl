#include "test.h"
#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif
#include <Elementary.h>
#ifndef ELM_LIB_QUICKLAUNCH


void
test_external_button(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Evas_Object *win, *ly;
   char buf[PATH_MAX];

   win = elm_win_util_standard_add("ext_button", "Edje External Button");
   elm_win_autodel_set(win, EINA_TRUE);

   ly = elm_layout_add(win);
   snprintf(buf, sizeof(buf), "%s/objects/test_external.edj", elm_app_data_dir_get());
   elm_layout_file_set(ly, buf, "external/button");
   evas_object_size_hint_weight_set(ly, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(ly, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_win_resize_object_add(win, ly);
   evas_object_show(ly);

   evas_object_resize(win, 320, 400);
   evas_object_show(win);
}

void
test_external_slider(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Evas_Object *win, *ly;
   char buf[PATH_MAX];

   win = elm_win_util_standard_add("ext_slider", "Edje External Slider");
   elm_win_autodel_set(win, EINA_TRUE);

   ly = elm_layout_add(win);
   snprintf(buf, sizeof(buf), "%s/objects/test_external.edj", elm_app_data_dir_get());
   elm_layout_file_set(ly, buf, "external/slider");
   evas_object_size_hint_weight_set(ly, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(ly, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_win_resize_object_add(win, ly);
   evas_object_show(ly);

   evas_object_resize(win, 320, 400);
   evas_object_show(win);
}

void
test_external_scroller(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Evas_Object *win, *ly;
   char buf[PATH_MAX];

   win = elm_win_util_standard_add("ext_scroller", "Edje External Scroller");
   elm_win_autodel_set(win, EINA_TRUE);

   ly = elm_layout_add(win);
   snprintf(buf, sizeof(buf), "%s/objects/test_external.edj", elm_app_data_dir_get());
   elm_layout_file_set(ly, buf, "external/scroller");
   evas_object_size_hint_weight_set(ly, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(ly, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_win_resize_object_add(win, ly);
   evas_object_show(ly);

   evas_object_resize(win, 320, 400);
   evas_object_show(win);
}


#endif
