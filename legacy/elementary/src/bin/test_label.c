#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif
#include <Elementary.h>

enum _slide_style
{
   SLIDE_SHORT,
   SLIDE_LONG,
   SLIDE_BOUNCE,
   SLIDE_STYLE_LAST
};

void
test_label(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *win, *bx, *lb;

   win = elm_win_util_standard_add("label", "Label");
   elm_win_autodel_set(win, EINA_TRUE);

   bx = elm_box_add(win);
   evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

   lb = elm_label_add(win);
   elm_object_text_set(lb,
                       "<b>This is a small label</b>"
                       );
   evas_object_size_hint_weight_set(lb, 0.0, 0.0);
   evas_object_size_hint_align_set(lb, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_box_pack_end(bx, lb);
   evas_object_show(lb);

   lb = elm_label_add(win);
   elm_object_text_set(lb,
                       "This is a larger label with newlines<br/>"
                       "to make it bigger, bit it won't expand or wrap<br/>"
                       "just be a block of text that can't change its<br/>"
                       "formatting as it's fixed based on text<br/>"
                       );
   evas_object_size_hint_weight_set(lb, 0.0, 0.0);
   evas_object_size_hint_align_set(lb, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_box_pack_end(bx, lb);
   evas_object_show(lb);

   lb = elm_label_add(win);
   elm_label_line_wrap_set(lb, ELM_WRAP_CHAR);
   elm_object_text_set(lb,
                       "<b>"
                       "This is more text designed to line-wrap here as "
                       "This object is resized horizontally. As it is "
                       "resized vertically though, nothing should change. "
                       "The amount of space allocated vertically should "
                       "change as horizontal size changes."
                       "</b>"
                       );
   evas_object_size_hint_weight_set(lb, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(lb, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_box_pack_end(bx, lb);
   evas_object_show(lb);

   lb = elm_label_add(win);
   elm_object_text_set(lb,
                       "This small label set to wrap"
                       );
   evas_object_size_hint_weight_set(lb, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(lb, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_box_pack_end(bx, lb);
   evas_object_show(lb);

   evas_object_resize(win, 320, 300);

   evas_object_show(bx);
   evas_object_show(win);
   elm_win_resize_object_add(win, bx);
}

static void
_cb_size_radio_changed(void *data, Evas_Object *obj, void *event EINA_UNUSED)
{
   Evas_Object *lb = data;
   int style =  elm_radio_value_get((Evas_Object *)obj);
   switch (style)
     {
      case SLIDE_SHORT:
         elm_object_style_set(lb, "slide_short");
         break;

      case SLIDE_LONG:
         elm_object_style_set(lb, "slide_long");
         break;

      case SLIDE_BOUNCE:
         elm_object_style_set(lb, "slide_bounce");
         break;

      default:
         return;
     }
}

static void
_duration_change_cb(void *data, Evas_Object *obj, void *event_info EINA_UNUSED)
{
   Evas_Object *lb = data;
   double val = elm_slider_value_get(obj);

   elm_label_slide_duration_set(lb, val);
   elm_label_slide_go(lb);

   Evas_Object *sl = evas_object_data_get(lb, "speed_slider");
   elm_slider_value_set(sl, elm_label_slide_speed_get(lb));
}

static void
_speed_change_cb(void *data, Evas_Object *obj, void *event_info EINA_UNUSED)
{
   Evas_Object *lb = data;
   double val = elm_slider_value_get(obj);

   elm_label_slide_speed_set(lb, val);
   elm_label_slide_go(lb);

   Evas_Object *sl = evas_object_data_get(lb, "duration_slider");
   elm_slider_value_set(sl, elm_label_slide_duration_get(lb));
}

static void
_label_resize_cb(void *data EINA_UNUSED, Evas *e EINA_UNUSED, Evas_Object *obj,
                 void *event_info EINA_UNUSED)
{
   Evas_Object *speed_slider = evas_object_data_get(obj, "speed_slider");
   Evas_Object *duration_slider = evas_object_data_get(obj, "duration_slider");

   elm_slider_value_set(duration_slider, elm_label_slide_duration_get(obj));
   elm_slider_value_set(speed_slider, elm_label_slide_speed_get(obj));
}

void
test_label2(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *win, *gd, *rect, *lb, *lb1, *lb2, *rd, *rdg, *sl;

   win = elm_win_util_standard_add("label2", "Label 2");
   elm_win_autodel_set(win, EINA_TRUE);

   gd = elm_grid_add(win);
   elm_grid_size_set(gd, 100, 100);
   evas_object_size_hint_weight_set(gd, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, gd);
   evas_object_show(gd);

   /* Test Label Ellipsis */
   lb = elm_label_add(win);
   elm_object_text_set(lb, "Test Label Ellipsis:");
   elm_grid_pack(gd, lb, 5, 0, 90, 10);
   evas_object_show(lb);

   rect = evas_object_rectangle_add(evas_object_evas_get(win));
   elm_grid_pack(gd, rect, 5, 10, 90, 10);
   evas_object_color_set(rect, 255, 125, 125, 255);
   evas_object_show(rect);

   lb = elm_label_add(win);
   elm_object_text_set(lb,
                       "This is a label set to ellipsis. "
                       "If set ellipsis to true and the text doesn't fit "
                       "in the label an ellipsis(\"...\") will be shown "
                       "at the end of the widget."
                       );
   elm_label_ellipsis_set(lb, EINA_TRUE);
   elm_grid_pack(gd, lb, 5, 10, 90, 10);
   evas_object_show(lb);

   /* Test Label Slide */
   lb = elm_label_add(win);
   elm_object_text_set(lb, "Test Label Slide:");
   elm_grid_pack(gd, lb, 5, 20, 90, 10);
   evas_object_show(lb);

   rect = evas_object_rectangle_add(evas_object_evas_get(win));
   elm_grid_pack(gd, rect, 5, 30, 90, 10);
   evas_object_color_set(rect, 255, 125, 125, 255);
   evas_object_show(rect);

   lb = elm_label_add(win);
   elm_object_style_set(lb, "slide_short");
   elm_object_text_set(lb,
                       "This is a label set to slide. "
                       "If set slide to true the text of the label "
                       "will slide/scroll through the length of label."
                       "This only works with the themes \"slide_short\", "
                       "\"slide_long\" and \"slide_bounce\"."
                       );
   elm_label_slide_mode_set(lb, ELM_LABEL_SLIDE_MODE_AUTO);
   elm_label_slide_speed_set(lb, 40.0);
   elm_label_slide_go(lb);
   elm_grid_pack(gd, lb, 5, 30, 90, 10);
   evas_object_show(lb);

   /* The speed or the duration of the slide animation will change when the
    * label change size, so we need to update the sliders on resize. */
   evas_object_event_callback_add(lb, EVAS_CALLBACK_RESIZE, _label_resize_cb, NULL);

   rd = elm_radio_add(win);
   elm_radio_state_value_set(rd, SLIDE_SHORT);
   elm_object_text_set(rd, "slide_short");
   evas_object_size_hint_weight_set(rd, EVAS_HINT_EXPAND, EVAS_HINT_FILL);
   evas_object_smart_callback_add(rd, "changed", _cb_size_radio_changed, lb);
   elm_grid_pack(gd, rd, 5, 40, 30, 10);
   evas_object_show(rd);
   rdg = rd;

   rd = elm_radio_add(win);
   elm_radio_state_value_set(rd, SLIDE_LONG);
   elm_radio_group_add(rd, rdg);
   elm_object_text_set(rd, "slide_long");
   evas_object_size_hint_weight_set(rd, EVAS_HINT_EXPAND, EVAS_HINT_FILL);
   evas_object_smart_callback_add(rd, "changed", _cb_size_radio_changed, lb);
   elm_grid_pack(gd, rd, 35, 40, 30, 10);
   evas_object_show(rd);

   rd = elm_radio_add(win);
   elm_radio_state_value_set(rd, SLIDE_BOUNCE);
   elm_radio_group_add(rd, rdg);
   elm_object_text_set(rd, "slide_bounce");
   evas_object_size_hint_weight_set(rd, EVAS_HINT_EXPAND, EVAS_HINT_FILL);
   evas_object_smart_callback_add(rd, "changed", _cb_size_radio_changed, lb);
   elm_grid_pack(gd, rd, 65, 40, 30, 10);
   evas_object_show(rd);

   sl = elm_slider_add(win);
   elm_object_text_set(sl, "Slide Duration");
   elm_slider_unit_format_set(sl, "%1.1f sec");
   elm_slider_min_max_set(sl, 3, 40);
   elm_slider_value_set(sl, elm_label_slide_duration_get(lb));
   evas_object_size_hint_align_set(sl, EVAS_HINT_FILL, 0.5);
   evas_object_size_hint_weight_set(sl, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_smart_callback_add(sl, "changed", _duration_change_cb, lb);
   evas_object_data_set(lb, "duration_slider", sl);
   elm_grid_pack(gd, sl, 5, 50, 90, 10);
   evas_object_show(sl);

   sl = elm_slider_add(win);
   elm_object_text_set(sl, "Slide Speed");
   elm_slider_unit_format_set(sl, "%1.1f px/sec");
   elm_slider_min_max_set(sl, 40, 300);
   elm_slider_value_set(sl, elm_label_slide_speed_get(lb));
   evas_object_size_hint_align_set(sl, EVAS_HINT_FILL, 0.5);
   evas_object_size_hint_weight_set(sl, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_smart_callback_add(sl, "changed", _speed_change_cb, lb);
   evas_object_data_set(lb, "speed_slider", sl);
   elm_grid_pack(gd, sl, 5, 60, 90, 10);
   evas_object_show(sl);

   /* Test 2 label at the same speed */
   lb = elm_label_add(win);
   elm_object_text_set(lb, "Test 2 label with the same speed:");
   elm_grid_pack(gd, lb, 5, 70, 90, 10);
   evas_object_show(lb);

   rect = evas_object_rectangle_add(evas_object_evas_get(win));
   elm_grid_pack(gd, rect, 5, 80, 90, 20);
   evas_object_color_set(rect, 255, 125, 125, 255);
   evas_object_show(rect);

   lb1 = elm_label_add(win);
   elm_object_style_set(lb1, "slide_long");
   elm_object_text_set(lb1, "This is a label set to slide with a fixed speed,"
                            " should match the speed with the below label."
                            " This label has few extra char for testing.");
   elm_label_slide_mode_set(lb1, ELM_LABEL_SLIDE_MODE_ALWAYS);
   elm_label_slide_speed_set(lb1, 40.0);
   elm_label_slide_go(lb1);
   elm_grid_pack(gd, lb1, 5, 80, 90, 10);
   evas_object_show(lb1);

   lb2 = elm_label_add(win);
   elm_object_style_set(lb2, "slide_long");
   elm_object_text_set(lb2, "This is a label set to slide and will"
                            " match the speed of the upper label.");
   elm_label_slide_mode_set(lb2, ELM_LABEL_SLIDE_MODE_ALWAYS);
   elm_label_slide_speed_set(lb2, 40.0);
   elm_label_slide_go(lb2);
   elm_grid_pack(gd, lb2, 5, 90, 90, 10);
   evas_object_show(lb2);

   evas_object_resize(win, 320, 320);
   evas_object_show(win);
}

void
test_label3(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *win, *bx, *lb;

   win = elm_win_util_standard_add("label", "Label");
   elm_win_autodel_set(win, EINA_TRUE);
   bx = elm_box_add(win);

   lb = elm_label_add(win);
   elm_object_text_set(lb,
                       "This is text for our label, that is long but "
                       "not too long. The label is designed to have line-wrap."
                       );
   elm_label_line_wrap_set(lb, ELM_WRAP_CHAR);
   elm_label_wrap_width_set(lb, 200);
   evas_object_size_hint_weight_set(lb, EVAS_HINT_EXPAND, 0.0);
   evas_object_size_hint_align_set(lb, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_box_pack_end(bx, lb);
   evas_object_show(lb);

   evas_object_show(bx);
   evas_object_show(win);
   elm_win_resize_object_add(win, bx);
}
