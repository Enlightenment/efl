#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif
#include <Elementary.h>
#ifndef ELM_LIB_QUICKLAUNCH

enum _slide_style
{
   SLIDE_SHORT,
   SLIDE_LONG,
   SLIDE_BOUNCE,
   SLIDE_STYLE_LAST
};

void
test_label(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Evas_Object *win, *bx, *lb;

   win = elm_win_util_standard_add("label", "Label");
   elm_win_autodel_set(win, EINA_TRUE);

   bx = elm_box_add(win);
   evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(bx, EVAS_HINT_FILL, EVAS_HINT_FILL);

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
_cb_size_radio_changed(void *data, Evas_Object *obj, void *event __UNUSED__)
{
   Evas_Object *lb = (Evas_Object *)data;
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
_change_cb(void *data, Evas_Object *obj, void *event_info __UNUSED__)
{
   Evas_Object *lb = (Evas_Object *)data;
   double val = elm_slider_value_get(obj);
   elm_label_slide_set(lb, EINA_FALSE);
   elm_label_slide_duration_set(lb, val);
   elm_label_slide_set(lb, EINA_TRUE);
}

void
test_label2(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   Evas_Object *win, *gd, *rect, *lb, *rd, *rdg, *sl;

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
   elm_label_slide_set(lb, EINA_TRUE);
   elm_grid_pack(gd, lb, 5, 5, 90, 10);
   evas_object_show(lb);

   rect = evas_object_rectangle_add(evas_object_evas_get(win));
   elm_grid_pack(gd, rect, 5, 15, 90, 10);
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
   elm_grid_pack(gd, lb, 5, 15, 90, 10);
   evas_object_show(lb);

   /* Test Label Slide */
   lb = elm_label_add(win);
   elm_object_text_set(lb, "Test Label Slide:");
   elm_label_slide_set(lb, EINA_TRUE);
   elm_grid_pack(gd, lb, 5, 30, 90, 10);
   evas_object_show(lb);

   rect = evas_object_rectangle_add(evas_object_evas_get(win));
   elm_grid_pack(gd, rect, 5, 40, 90, 10);
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
   elm_label_slide_set(lb, EINA_TRUE);
   elm_grid_pack(gd, lb, 5, 40, 90, 10);
   evas_object_show(lb);

   rd = elm_radio_add(win);
   elm_radio_state_value_set(rd, SLIDE_SHORT);
   elm_object_text_set(rd, "slide_short");
   evas_object_size_hint_weight_set(rd, EVAS_HINT_EXPAND, EVAS_HINT_FILL);
   evas_object_smart_callback_add(rd, "changed", _cb_size_radio_changed, lb);
   elm_grid_pack(gd, rd, 5, 50, 30, 10);
   evas_object_show(rd);
   rdg = rd;

   rd = elm_radio_add(win);
   elm_radio_state_value_set(rd, SLIDE_LONG);
   elm_radio_group_add(rd, rdg);
   elm_object_text_set(rd, "slide_long");
   evas_object_size_hint_weight_set(rd, EVAS_HINT_EXPAND, EVAS_HINT_FILL);
   evas_object_smart_callback_add(rd, "changed", _cb_size_radio_changed, lb);
   elm_grid_pack(gd, rd, 35, 50, 30, 10);
   evas_object_show(rd);

   rd = elm_radio_add(win);
   elm_radio_state_value_set(rd, SLIDE_BOUNCE);
   elm_radio_group_add(rd, rdg);
   elm_object_text_set(rd, "slide_bounce");
   evas_object_size_hint_weight_set(rd, EVAS_HINT_EXPAND, EVAS_HINT_FILL);
   evas_object_smart_callback_add(rd, "changed", _cb_size_radio_changed, lb);
   elm_grid_pack(gd, rd, 65, 50, 30, 10);
   evas_object_show(rd);

   sl = elm_slider_add(win);
   elm_object_text_set(sl, "Slide Duration");
   elm_slider_unit_format_set(sl, "%1.1f units");
   elm_slider_min_max_set(sl, 1, 20);
   elm_slider_value_set(sl, 10);
   evas_object_size_hint_align_set(sl, EVAS_HINT_FILL, 0.5);
   evas_object_size_hint_weight_set(sl, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_smart_callback_add(sl, "changed", _change_cb, lb);
   elm_grid_pack(gd, sl, 5, 60, 90, 10);
   evas_object_show(sl);

   evas_object_resize(win, 320, 320);
   evas_object_show(win);
}
#endif
