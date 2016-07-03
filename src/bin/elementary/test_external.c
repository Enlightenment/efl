#include "test.h"
#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif
#include <Elementary.h>

void
test_external_button(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *win, *ly;
   char buf[PATH_MAX];

   win = elm_win_util_standard_add("ext_button", "Edje External Button");
   elm_win_autodel_set(win, EINA_TRUE);

   ly = elm_layout_add(win);
   snprintf(buf, sizeof(buf), "%s/objects/test_external.edj", elm_app_data_dir_get());
   elm_layout_file_set(ly, buf, "external/button");
   evas_object_size_hint_weight_set(ly, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, ly);
   evas_object_show(ly);

   evas_object_resize(win, 320, 400);
   evas_object_show(win);
}

void
test_external_slider(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *win, *ly;
   char buf[PATH_MAX];

   win = elm_win_util_standard_add("ext_slider", "Edje External Slider");
   elm_win_autodel_set(win, EINA_TRUE);

   ly = elm_layout_add(win);
   snprintf(buf, sizeof(buf), "%s/objects/test_external.edj", elm_app_data_dir_get());
   elm_layout_file_set(ly, buf, "external/slider");
   evas_object_size_hint_weight_set(ly, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, ly);
   evas_object_show(ly);

   evas_object_resize(win, 320, 400);
   evas_object_show(win);
}

void
test_external_scroller(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *win, *ly;
   char buf[PATH_MAX];

   win = elm_win_util_standard_add("ext_scroller", "Edje External Scroller");
   elm_win_autodel_set(win, EINA_TRUE);

   ly = elm_layout_add(win);
   snprintf(buf, sizeof(buf), "%s/objects/test_external.edj", elm_app_data_dir_get());
   elm_layout_file_set(ly, buf, "external/scroller");
   evas_object_size_hint_weight_set(ly, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
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
_bt_clicked(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
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
test_external_pbar(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *win, *ly, *edje, *bt;
   char buf[PATH_MAX];

   win = elm_win_util_standard_add("ext_pbar", "Edje External ProgressBar");
   elm_win_autodel_set(win, EINA_TRUE);

   ly = elm_layout_add(win);
   snprintf(buf, sizeof(buf), "%s/objects/test_external.edj", elm_app_data_dir_get());
   elm_layout_file_set(ly, buf, "external/pbar");
   evas_object_size_hint_weight_set(ly, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, ly);
   evas_object_show(ly);

   edje = elm_layout_edje_get(ly);
   bt = edje_object_part_external_object_get(edje, "ext_button3");
   evas_object_smart_callback_add(bt, "clicked", _bt_clicked, edje);

   evas_object_resize(win, 320, 400);
   evas_object_show(win);
}

void
test_external_video(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *win, *ly;
   char buf[PATH_MAX];

   win = elm_win_util_standard_add("ext_video", "Edje External Video");
   elm_win_autodel_set(win, EINA_TRUE);

   ly = elm_layout_add(win);
   snprintf(buf, sizeof(buf), "%s/objects/test_external.edj", elm_app_data_dir_get());
   elm_layout_file_set(ly, buf, "external/video");
   evas_object_size_hint_weight_set(ly, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, ly);
   evas_object_show(ly);

   evas_object_resize(win, 320, 400);
   evas_object_show(win);
}

void
test_external_icon(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *win, *ly;
   char buf[PATH_MAX];

   win = elm_win_util_standard_add("ext_icon", "Edje External Icon");
   elm_win_autodel_set(win, EINA_TRUE);

   ly = elm_layout_add(win);
   snprintf(buf, sizeof(buf), "%s/objects/test_external.edj", elm_app_data_dir_get());
   elm_layout_file_set(ly, buf, "external/icon");
   evas_object_size_hint_weight_set(ly, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, ly);
   evas_object_show(ly);

   elm_layout_signal_emit(ly, "elm_test,animations,start", "elm_test");

   evas_object_resize(win, 320, 400);
   evas_object_show(win);
}

static char *
text_get_cb(void        *data,
            Evas_Object *obj  EINA_UNUSED,
            const char  *part EINA_UNUSED)
{
   return strdup(data);
}

static void
action_cb(void        *data,
          Evas_Object *obj  EINA_UNUSED,
          void        *info)
{
   Evas_Object *const lay = data;
   Elm_Object_Item *const item = info;

   elm_layout_text_set(lay, "info", elm_object_item_data_get(item));
}

static void
_cb_pressed_cb(void        *data EINA_UNUSED,
               Evas_Object *obj,
               void        *info)
{
   const char *txt;

   txt = elm_object_item_text_get(info);
   elm_object_text_set(obj, txt);
   elm_combobox_hover_end(obj);
   elm_entry_cursor_end_set(obj);
}

void
test_external_combobox(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *win, *ly, *cb, *edj;
   char buf[PATH_MAX];
   Elm_Genlist_Item_Class *itc;
   const char *info[] = {
      "Label", "Button", "Combobox", "Icon", "Scroller", "Layout",
      "Naviframe", "Bubble"
   };
   const unsigned int size = EINA_C_ARRAY_LENGTH(info);
   unsigned int i;

   win = elm_win_util_standard_add("ext_combobox", "Edje External Combobox");
   elm_win_autodel_set(win, EINA_TRUE);

   ly = elm_layout_add(win);
   snprintf(buf, sizeof(buf), "%s/objects/test_external.edj", elm_app_data_dir_get());
   elm_layout_file_set(ly, buf, "external/combobox");
   evas_object_size_hint_weight_set(ly, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, ly);
   evas_object_show(ly);

   itc = elm_genlist_item_class_new();
   itc->item_style = "default";
   itc->func.text_get = text_get_cb;

   edj = elm_layout_edje_get(ly);
   cb = edje_object_part_external_object_get(edj, "combobox");
   evas_object_smart_callback_add(cb, "item,pressed", _cb_pressed_cb, NULL);

   for (i = 0; i < size; i++)
     {
        elm_genlist_item_append(cb, itc, info[i], NULL,
                                ELM_GENLIST_ITEM_NONE, action_cb, ly);
     }

   elm_genlist_item_class_free(itc);

   evas_object_resize(win, 320, 400);
   evas_object_show(win);
}
