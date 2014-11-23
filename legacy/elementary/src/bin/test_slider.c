#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif
#include <Elementary.h>

void
_delay_change_cb(void *data EINA_UNUSED, Evas_Object *obj, void *event_info EINA_UNUSED)
{
    printf("delay,changed! slider value : %d\n", (int)round(elm_slider_value_get(obj)));
}

void
_change_cb(void *data, Evas_Object *obj, void *event_info EINA_UNUSED)
{
    double val = elm_slider_value_get(obj);
    elm_slider_value_set(data, val);
}

void
_change_print_cb(void *data EINA_UNUSED, Evas_Object *obj, void *event_info EINA_UNUSED)
{
   double val = elm_slider_value_get(obj);
   printf("change to %3.3f\n", val);
}

void
_bt_0(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   elm_slider_value_set(data, 0.0);
}

void
_bt_1(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   elm_slider_value_set(data, 1.0);
}

void
_bt_p1(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   elm_slider_value_set(data, elm_slider_value_get(data) + 0.1);
}

void
_bt_m1(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   elm_slider_value_set(data, elm_slider_value_get(data) - 0.1);
}

static double
_step_size_calculate(double min, double max)
{
   double step = 0.0;
   int steps = 0;

   steps = max - min;
   if (steps) step = (1.0 / steps);
   return step;
}

void
test_slider(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *win, *fr, *bx, *sl, *ic, *sl1, *bx2, *bt;
   double step;
   char buf[PATH_MAX];

   win = elm_win_util_standard_add("slider", "Slider");
   elm_win_autodel_set(win, EINA_TRUE);

   fr = elm_frame_add(win);
   evas_object_size_hint_weight_set(fr, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, fr);
   elm_object_style_set(fr, "pad_large");
   evas_object_show(fr);

   bx = elm_box_add(fr);
   elm_object_content_set(fr, bx);
   evas_object_show(bx);

   // disabled horizontal slider
   ic = elm_icon_add(bx);
   snprintf(buf, sizeof(buf), "%s/images/logo_small.png", elm_app_data_dir_get());
   elm_image_file_set(ic, buf, NULL);
   evas_object_size_hint_aspect_set(ic, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);
   evas_object_show(ic);

   sl = elm_slider_add(bx);
   elm_object_text_set(sl, "Disabled");
   elm_object_part_content_set(sl, "icon", ic);
   elm_slider_unit_format_set(sl, "%1.1f units");
   elm_slider_span_size_set(sl, 120);
   elm_slider_min_max_set(sl, 50, 150);
   elm_slider_value_set(sl, 80);
   elm_object_disabled_set(sl, EINA_TRUE);
   evas_object_size_hint_align_set(sl, EVAS_HINT_FILL, 0.5);
   evas_object_size_hint_weight_set(sl, EVAS_HINT_EXPAND, 0.0);
   elm_box_pack_end(bx, sl);
   evas_object_show(sl);

   sl = elm_slider_add(bx);
   elm_slider_unit_format_set(sl, "%1.0f units");
   elm_slider_indicator_format_set(sl, "%1.0f");
   elm_slider_span_size_set(sl, 120);
   elm_slider_min_max_set(sl, 0, 9);
   elm_object_text_set(sl, "Manual step");
   step = _step_size_calculate(0, 9);
   elm_slider_step_set(sl, step);
   evas_object_size_hint_align_set(sl, EVAS_HINT_FILL, 0.5);
   evas_object_size_hint_weight_set(sl, EVAS_HINT_EXPAND, 0.0);
   elm_box_pack_end(bx, sl);
   evas_object_show(sl);

   sl = elm_slider_add(bx);
   elm_slider_unit_format_set(sl, "%1.0f units");
   elm_slider_indicator_format_set(sl, "%1.0f");
   elm_slider_span_size_set(sl, 120);
   elm_slider_min_max_set(sl, 0, 100);
   elm_object_text_set(sl, "Show Indicator on Focus: ");
   step = _step_size_calculate(0, 9);
   elm_slider_step_set(sl, step);
   elm_slider_indicator_show_on_focus_set(sl, EINA_TRUE);
   evas_object_size_hint_align_set(sl, EVAS_HINT_FILL, 0.5);
   evas_object_size_hint_weight_set(sl, EVAS_HINT_EXPAND, 0.0);
   elm_box_pack_end(bx, sl);
   evas_object_show(sl);

   // normal horizontal slider
   ic = elm_icon_add(bx);
   snprintf(buf, sizeof(buf), "%s/images/logo_small.png", elm_app_data_dir_get());
   elm_image_file_set(ic, buf, NULL);
   evas_object_size_hint_aspect_set(ic, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);

   sl1 = sl = elm_slider_add(bx);
   elm_object_text_set(sl, "Horizontal");
   elm_object_part_content_set(sl, "icon", ic);
   elm_slider_unit_format_set(sl, "%1.1f units");
   elm_slider_indicator_format_set(sl, "%1.1f");
   elm_slider_span_size_set(sl, 120);
   evas_object_size_hint_align_set(sl, EVAS_HINT_FILL, 0.5);
   evas_object_size_hint_weight_set(sl, EVAS_HINT_EXPAND, 0.0);
   elm_box_pack_end(bx, sl);
   evas_object_show(ic);
   evas_object_show(sl);

   // horizontally inverted slider
   ic = elm_icon_add(bx);
   snprintf(buf, sizeof(buf), "%s/images/logo_small.png", elm_app_data_dir_get());
   elm_image_file_set(ic, buf, NULL);
   evas_object_size_hint_aspect_set(ic, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);
   evas_object_show(ic);

   sl = elm_slider_add(bx);
   elm_object_text_set(sl, "Horizontal inverted");
   elm_object_part_content_set(sl, "end", ic);
   elm_slider_unit_format_set(sl, "%3.0f units");
   elm_slider_span_size_set(sl, 80);
   evas_object_size_hint_align_set(sl, EVAS_HINT_FILL, 0.5);
   evas_object_size_hint_weight_set(sl, EVAS_HINT_EXPAND, 0.0);
   elm_slider_indicator_format_set(sl, "%3.0f");
   elm_slider_min_max_set(sl, 50, 150);
   elm_slider_value_set(sl, 80);
   elm_slider_inverted_set(sl, EINA_TRUE);
   evas_object_size_hint_align_set(sl, 0.5, 0.5);
   evas_object_size_hint_weight_set(sl, 0.0, 0.0);
   evas_object_smart_callback_add(sl, "delay,changed", _delay_change_cb, NULL);
   elm_box_pack_end(bx, sl);
   evas_object_show(sl);

   // disabled horizontally inverted slider
   ic = elm_icon_add(bx);
   snprintf(buf, sizeof(buf), "%s/images/logo_small.png", elm_app_data_dir_get());
   elm_image_file_set(ic, buf, NULL);
   evas_object_size_hint_aspect_set(ic, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);
   evas_object_show(ic);

   sl = elm_slider_add(bx);
   elm_object_text_set(sl, "Disabled inverted");
   elm_object_part_content_set(sl, "end", ic);
   elm_slider_span_size_set(sl, 80);
   evas_object_size_hint_align_set(sl, EVAS_HINT_FILL, 0.5);
   evas_object_size_hint_weight_set(sl, EVAS_HINT_EXPAND, 0.0);
   elm_slider_indicator_format_set(sl, "%3.0f");
   elm_slider_min_max_set(sl, 50, 150);
   elm_slider_value_set(sl, 80);
   elm_slider_inverted_set(sl, EINA_TRUE);
   elm_object_disabled_set(sl, EINA_TRUE);
   evas_object_size_hint_align_set(sl, 0.5, 0.5);
   evas_object_size_hint_weight_set(sl, 0.0, 0.0);
   elm_box_pack_end(bx, sl);
   evas_object_show(sl);

   // scale doubled slider
   sl = elm_slider_add(bx);
   elm_slider_indicator_show_set(sl, EINA_FALSE);
   elm_object_text_set(sl, "Scale doubled");
   elm_slider_unit_format_set(sl, "%3.0f units");
   elm_slider_span_size_set(sl, 40);
   evas_object_size_hint_align_set(sl, EVAS_HINT_FILL, 0.5);
   evas_object_size_hint_weight_set(sl, EVAS_HINT_EXPAND, 0.0);
   elm_slider_indicator_format_set(sl, "%3.0f");
   elm_slider_min_max_set(sl, 50, 150);
   elm_slider_value_set(sl, 80);
   elm_slider_inverted_set(sl, EINA_TRUE);
   elm_object_scale_set(sl, 2.0);
   elm_box_pack_end(bx, sl);
   evas_object_show(sl);

   // horizontal box
   bx2 = elm_box_add(bx);
   evas_object_size_hint_weight_set(bx2, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(bx2, 0.5, EVAS_HINT_FILL);
   elm_box_horizontal_set(bx2, EINA_TRUE);
   elm_box_pack_end(bx, bx2);
   evas_object_show(bx2);

   // vertical inverted slider
   ic = elm_icon_add(bx2);
   snprintf(buf, sizeof(buf), "%s/images/logo_small.png", elm_app_data_dir_get());
   elm_image_file_set(ic, buf, NULL);
   evas_object_size_hint_aspect_set(ic, EVAS_ASPECT_CONTROL_HORIZONTAL, 1, 1);
   evas_object_show(ic);

   sl = elm_slider_add(bx2);
   elm_object_part_content_set(sl, "icon", ic);
   elm_object_text_set(sl, "Vertical inverted");
   elm_slider_inverted_set(sl, EINA_TRUE);
   elm_slider_unit_format_set(sl, "%1.1f units");
   elm_slider_span_size_set(sl, 60);
   evas_object_size_hint_align_set(sl, 0.5, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(sl, 0.0, EVAS_HINT_EXPAND);
   elm_slider_indicator_format_set(sl, "%1.1f");
   elm_slider_value_set(sl, 0.2);
   elm_object_scale_set(sl, 1.0);
   elm_slider_horizontal_set(sl, EINA_FALSE);
   elm_box_pack_end(bx2, sl);
   evas_object_show(sl);

   evas_object_smart_callback_add(sl1, "changed", _change_cb, sl);

   // disabled vertical slider
   ic = elm_icon_add(bx2);
   snprintf(buf, sizeof(buf), "%s/images/logo_small.png", elm_app_data_dir_get());
   elm_image_file_set(ic, buf, NULL);
   evas_object_size_hint_aspect_set(ic, EVAS_ASPECT_CONTROL_HORIZONTAL, 1, 1);
   evas_object_show(ic);

   sl = elm_slider_add(bx2);
   elm_object_part_content_set(sl, "icon", ic);
   elm_object_text_set(sl, "Disabled vertical");
   elm_slider_inverted_set(sl, EINA_TRUE);
   elm_slider_unit_format_set(sl, "%1.1f units");
   elm_slider_span_size_set(sl, 100);
   evas_object_size_hint_align_set(sl, 0.5, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(sl, 0.0, EVAS_HINT_EXPAND);
   elm_slider_indicator_format_set(sl, "%1.1f");
   elm_slider_value_set(sl, 0.2);
   elm_object_scale_set(sl, 1.0);
   elm_slider_horizontal_set(sl, EINA_FALSE);
   elm_object_disabled_set(sl, EINA_TRUE);
   elm_box_pack_end(bx2, sl);
   evas_object_show(sl);

   // normal vertical slider
   sl = elm_slider_add(bx2);
   elm_object_text_set(sl, "Vertical");
   elm_slider_unit_format_set(sl, "%1.1f units");
   elm_slider_span_size_set(sl, 60);
   evas_object_size_hint_align_set(sl, 0.5, EVAS_HINT_FILL);
   evas_object_size_hint_weight_set(sl, 0.0, EVAS_HINT_EXPAND);
   elm_slider_indicator_show_set(sl, EINA_FALSE);
   elm_slider_value_set(sl, 0.2);
   elm_object_scale_set(sl, 1.0);
   elm_slider_horizontal_set(sl, EINA_FALSE);
   evas_object_smart_callback_add(sl, "changed", _change_print_cb, sl);
   elm_box_pack_end(bx2, sl);
   evas_object_show(sl);

   // box for bottom buttons
   bx2 = elm_box_add(win);
   evas_object_size_hint_weight_set(bx2, EVAS_HINT_EXPAND, 0.0);
   elm_box_horizontal_set(bx2, EINA_TRUE);
   elm_box_pack_end(bx, bx2);
   evas_object_show(bx2);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "0");
   evas_object_smart_callback_add(bt, "clicked", _bt_0, sl);
   evas_object_show(bt);
   elm_box_pack_end(bx2, bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "1");
   evas_object_smart_callback_add(bt, "clicked", _bt_1, sl);
   evas_object_show(bt);
   elm_box_pack_end(bx2, bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "+0.1");
   evas_object_smart_callback_add(bt, "clicked", _bt_p1, sl);
   evas_object_show(bt);
   elm_box_pack_end(bx2, bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "-0.1");
   evas_object_smart_callback_add(bt, "clicked", _bt_m1, sl);
   evas_object_show(bt);
   elm_box_pack_end(bx2, bt);

   evas_object_show(win);
}
