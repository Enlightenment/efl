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


static Eina_Bool
_timer_cb(void *data)
{
   Evas_Object *edje = data;
   Evas_Object *bt1, *bt2, *bt3, *pb1, *pb2, *pb3, *pb4, *pb5, *pb6, *pb7;
   double progress;

   pb1 = edje_object_part_external_object_get(edje, "ext_pbar1");
   pb2 = edje_object_part_external_object_get(edje, "ext_pbar2");
   pb3 = edje_object_part_external_object_get(edje, "ext_pbar3");
   pb4 = edje_object_part_external_object_get(edje, "ext_pbar4");
   pb6 = edje_object_part_external_object_get(edje, "ext_pbar6");
   progress = elm_progressbar_value_get(pb1) + 0.0123;

   elm_progressbar_value_set(pb1, progress);
   elm_progressbar_value_set(pb2, progress);
   elm_progressbar_value_set(pb3, progress);
   elm_progressbar_value_set(pb4, progress);
   elm_progressbar_value_set(pb6, progress);

   if (progress < 1.0)
     return ECORE_CALLBACK_RENEW;

   pb5 = edje_object_part_external_object_get(edje, "ext_pbar5");
   pb7 = edje_object_part_external_object_get(edje, "ext_pbar7");
   elm_progressbar_pulse(pb2, EINA_FALSE);
   elm_progressbar_pulse(pb5, EINA_FALSE);
   elm_progressbar_pulse(pb7, EINA_FALSE);

   bt1 = edje_object_part_external_object_get(edje, "ext_button1");
   bt2 = edje_object_part_external_object_get(edje, "ext_button2");
   bt3 = edje_object_part_external_object_get(edje, "ext_button3");
   elm_object_disabled_set(bt1, EINA_FALSE);
   elm_object_disabled_set(bt2, EINA_FALSE);
   elm_object_disabled_set(bt3, EINA_FALSE);
   
   return ECORE_CALLBACK_CANCEL;
}

static void
_bt_clicked(void *data, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Evas_Object *edje = data;
   Evas_Object *bt1, *bt2, *bt3, *pb1, *pb2, *pb3, *pb4, *pb5, *pb6, *pb7;

   bt1 = edje_object_part_external_object_get(edje, "ext_button1");
   bt2 = edje_object_part_external_object_get(edje, "ext_button2");
   bt3 = edje_object_part_external_object_get(edje, "ext_button3");
   elm_object_disabled_set(bt1, EINA_TRUE);
   elm_object_disabled_set(bt2, EINA_TRUE);
   elm_object_disabled_set(bt3, EINA_TRUE);

   pb1 = edje_object_part_external_object_get(edje, "ext_pbar1");
   pb1 = edje_object_part_external_object_get(edje, "ext_pbar1");
   pb2 = edje_object_part_external_object_get(edje, "ext_pbar2");
   pb3 = edje_object_part_external_object_get(edje, "ext_pbar3");
   pb4 = edje_object_part_external_object_get(edje, "ext_pbar4");
   pb5 = edje_object_part_external_object_get(edje, "ext_pbar5");
   pb6 = edje_object_part_external_object_get(edje, "ext_pbar6");
   pb7 = edje_object_part_external_object_get(edje, "ext_pbar7");

   elm_progressbar_value_set(pb1, 0.0);
   elm_progressbar_value_set(pb3, 0.0);
   elm_progressbar_value_set(pb4, 0.0);
   elm_progressbar_value_set(pb6, 0.0);

   elm_progressbar_pulse(pb2, EINA_TRUE);
   elm_progressbar_pulse(pb5, EINA_TRUE);
   elm_progressbar_pulse(pb7, EINA_TRUE);

   ecore_timer_add(0.1, _timer_cb, edje);
}

void
test_external_pbar(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Evas_Object *win, *ly, *edje, *bt;
   char buf[PATH_MAX];

   win = elm_win_util_standard_add("ext_pbar", "Edje External ProgressBar");
   elm_win_autodel_set(win, EINA_TRUE);

   ly = elm_layout_add(win);
   snprintf(buf, sizeof(buf), "%s/objects/test_external.edj", elm_app_data_dir_get());
   elm_layout_file_set(ly, buf, "external/pbar");
   evas_object_size_hint_weight_set(ly, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(ly, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_win_resize_object_add(win, ly);
   evas_object_show(ly);

   edje = elm_layout_edje_get(ly);
   bt = edje_object_part_external_object_get(edje, "ext_button3");
   evas_object_smart_callback_add(bt, "clicked", _bt_clicked, edje);

   evas_object_resize(win, 320, 400);
   evas_object_show(win);
}

void
test_external_video(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Evas_Object *win, *ly;
   char buf[PATH_MAX];

   win = elm_win_util_standard_add("ext_video", "Edje External Video");
   elm_win_autodel_set(win, EINA_TRUE);

   ly = elm_layout_add(win);
   snprintf(buf, sizeof(buf), "%s/objects/test_external.edj", elm_app_data_dir_get());
   elm_layout_file_set(ly, buf, "external/video");
   evas_object_size_hint_weight_set(ly, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(ly, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_win_resize_object_add(win, ly);
   evas_object_show(ly);

   evas_object_resize(win, 320, 400);
   evas_object_show(win);
}


#endif
