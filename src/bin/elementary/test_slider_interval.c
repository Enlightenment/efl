#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif
#include <Elementary.h>

static double
_step_size_calculate(double min, double max)
{
   double step = 0.0;
   int steps = 0;

   steps = max - min;
   if (steps) step = (1.0 / steps);
   return step;
}

static void
_intv_slider_changed_cb(void *data EINA_UNUSED, const Efl_Event *ev)
{
   double from, to;

   efl_ui_slider_interval_value_get(ev->object, &from, &to);
   if (from < 100)
     efl_ui_slider_interval_value_set(ev->object, 100, to);
   if (to > 500)
     efl_ui_slider_interval_value_set(ev->object, from, 500);
}

void
test_slider_interval(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Eo *win, *bx, *hbx, *sl;
   double step;

   win = efl_add(EFL_UI_WIN_CLASS, NULL,
                 efl_ui_win_type_set(efl_added, EFL_UI_WIN_BASIC),
                 efl_text_set(efl_added, "Efl.Ui.Slider_Interval"),
                 efl_ui_win_autodel_set(efl_added, EINA_TRUE));

   bx = efl_add(EFL_UI_BOX_CLASS, win,
                efl_content_set(win, efl_added));

   sl = efl_add(EFL_UI_SLIDER_INTERVAL_CLASS, bx,
                efl_ui_slider_indicator_visible_mode_set(efl_added, EFL_UI_SLIDER_INDICATOR_VISIBLE_MODE_NONE),
                efl_ui_range_span_size_set(efl_added, 160),
                efl_ui_slider_indicator_format_set(efl_added, "%1.5f"),
                efl_ui_slider_interval_value_set(efl_added, 0.4, 0.9),
                efl_pack(bx, efl_added));
   elm_object_text_set(sl, "horizontal");

   step = _step_size_calculate(0, 9);
   sl = efl_add(EFL_UI_SLIDER_INTERVAL_CLASS, bx,
                efl_ui_range_span_size_set(efl_added, 120),
                efl_ui_range_unit_format_set(efl_added, "%1.1f units"),
                efl_ui_slider_indicator_format_set(efl_added, "%1.1f"),
                efl_ui_slider_interval_value_set(efl_added, 0.4, 0.9),
                efl_ui_slider_step_set(efl_added, step),
                efl_pack(bx, efl_added));
   elm_object_text_set(sl, "Manual step");

   sl = efl_add(EFL_UI_SLIDER_INTERVAL_CLASS, bx,
                efl_ui_range_span_size_set(efl_added, 120),
                efl_ui_range_unit_format_set(efl_added, "%1.0f units"),
                efl_ui_slider_indicator_format_set(efl_added, "%1.0f"),
                efl_ui_range_min_max_set(efl_added, 10, 145),
                efl_ui_slider_interval_value_set(efl_added, 50, 100),
                efl_ui_slider_step_set(efl_added, step),
                efl_pack(bx, efl_added));
   elm_object_disabled_set(sl, EINA_TRUE);
   elm_object_text_set(sl, "Disabled");

   hbx = efl_add(EFL_UI_BOX_CLASS, bx,
                 efl_ui_direction_set(efl_added, EFL_UI_DIR_HORIZONTAL),
                 efl_pack(bx, efl_added));

   sl = efl_add(EFL_UI_SLIDER_INTERVAL_CLASS, hbx,
                efl_ui_range_span_size_set(efl_added, 120),
                efl_ui_range_unit_format_set(efl_added, "%1.0f units"),
                efl_ui_slider_indicator_format_set(efl_added, "%1.0f"),
                efl_ui_range_min_max_set(efl_added, 10, 145),
                efl_ui_slider_interval_value_set(efl_added, 50, 100),
                efl_ui_slider_step_set(efl_added, step),
                efl_ui_direction_set(efl_added, EFL_UI_DIR_VERTICAL),
                efl_pack(hbx, efl_added));
   elm_object_text_set(sl, "vertical");

   sl = efl_add(EFL_UI_SLIDER_INTERVAL_CLASS, hbx,
                efl_ui_range_span_size_set(efl_added, 120),
                efl_ui_range_unit_format_set(efl_added, "%1.0f units"),
                efl_ui_slider_indicator_format_set(efl_added, "%1.0f"),
                efl_ui_range_min_max_set(efl_added, 10, 145),
                efl_ui_slider_interval_value_set(efl_added, 50, 100),
                efl_ui_slider_step_set(efl_added, step),
                efl_ui_direction_set(efl_added, EFL_UI_DIR_VERTICAL),
                efl_pack(hbx, efl_added));
   elm_object_text_set(sl, "vertical");
   elm_object_disabled_set(sl, EINA_TRUE);

   sl = efl_add(EFL_UI_SLIDER_INTERVAL_CLASS, bx,
                efl_ui_slider_indicator_visible_mode_set(efl_added, EFL_UI_SLIDER_INDICATOR_VISIBLE_MODE_ON_FOCUS),
                efl_ui_range_span_size_set(efl_added, 120),
                efl_ui_range_unit_format_set(efl_added, "%1.0f units"),
                efl_ui_slider_indicator_format_set(efl_added, "%1.0f"),
                efl_ui_range_min_max_set(efl_added, 0, 600),
                efl_ui_slider_interval_value_set(efl_added, 100, 500),
                efl_ui_slider_step_set(efl_added, step),
                efl_event_callback_add(efl_added, EFL_UI_SLIDER_EVENT_CHANGED, _intv_slider_changed_cb, NULL),
                efl_pack(bx, efl_added));

   elm_object_text_set(sl, "Limited(100-500)");
}

