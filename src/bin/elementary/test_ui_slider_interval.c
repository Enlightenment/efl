#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif
#include <Efl_Ui.h>
#include <Elementary.h>
#include "elm_priv.h" //FIXME remove this once efl.ui.text doesn't need elm_general.h
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
   Eo *win, *bx, *hbx;
   double step;

   win = efl_add_ref(EFL_UI_WIN_CLASS, NULL,
                                  efl_text_set(efl_added, "Efl.Ui.Slider_Interval"),
                 efl_ui_win_autodel_set(efl_added, EINA_TRUE));

   bx = efl_add(EFL_UI_BOX_CLASS, win,
                efl_content_set(win, efl_added));

   efl_add(EFL_UI_TEXT_CLASS, bx,
           efl_text_set(efl_added, "Horizontal"),
           efl_text_interactive_editable_set(efl_added, EINA_FALSE),
           efl_pack(bx, efl_added));

   efl_add(EFL_UI_SLIDER_INTERVAL_CLASS, bx,
           efl_gfx_hint_size_min_set(efl_added, EINA_SIZE2D(160, 0)),
           efl_ui_slider_interval_value_set(efl_added, 0.4, 0.9),
           efl_pack(bx, efl_added));

   efl_add(EFL_UI_TEXT_CLASS, bx,
           efl_text_set(efl_added, "Manual step"),
           efl_text_interactive_editable_set(efl_added, EINA_FALSE),
           efl_pack(bx, efl_added));

   step = _step_size_calculate(0, 9);
   efl_add(EFL_UI_SLIDER_INTERVAL_CLASS, bx,
           efl_gfx_hint_size_min_set(efl_added, EINA_SIZE2D(120, 0)),
           efl_ui_slider_interval_value_set(efl_added, 0.4, 0.9),
           efl_ui_range_step_set(efl_added, step),
           efl_pack(bx, efl_added));

   efl_add(EFL_UI_TEXT_CLASS, bx,
           efl_text_set(efl_added, "Disabled"),
           efl_text_interactive_editable_set(efl_added, EINA_FALSE),
           efl_pack(bx, efl_added));

   efl_add(EFL_UI_SLIDER_INTERVAL_CLASS, bx,
           efl_gfx_hint_size_min_set(efl_added, EINA_SIZE2D(120, 0)),
           efl_ui_range_limits_set(efl_added, 10, 145),
           efl_ui_slider_interval_value_set(efl_added, 50, 100),
           efl_ui_range_step_set(efl_added, step),
           elm_object_disabled_set(efl_added, EINA_TRUE),
           efl_pack(bx, efl_added));

   efl_add(EFL_UI_TEXT_CLASS, bx,
           efl_text_set(efl_added, "Vertical"),
           efl_text_interactive_editable_set(efl_added, EINA_FALSE),
           efl_pack(bx, efl_added));

   hbx = efl_add(EFL_UI_BOX_CLASS, bx,
                 efl_ui_layout_orientation_set(efl_added, EFL_UI_LAYOUT_ORIENTATION_HORIZONTAL),
                 efl_pack(bx, efl_added));

   efl_add(EFL_UI_SLIDER_INTERVAL_CLASS, hbx,
           efl_gfx_hint_size_min_set(efl_added, EINA_SIZE2D(0, 160)),
           efl_ui_range_limits_set(efl_added, 10, 145),
           efl_ui_slider_interval_value_set(efl_added, 50, 100),
           efl_ui_range_step_set(efl_added, step),
           efl_ui_layout_orientation_set(efl_added, EFL_UI_LAYOUT_ORIENTATION_VERTICAL),
           efl_pack(hbx, efl_added));

   efl_add(EFL_UI_SLIDER_INTERVAL_CLASS, hbx,
           efl_gfx_hint_size_min_set(efl_added, EINA_SIZE2D(0, 160)),
           efl_ui_range_limits_set(efl_added, 10, 145),
           efl_ui_slider_interval_value_set(efl_added, 50, 100),
           efl_ui_range_step_set(efl_added, step),
           efl_ui_layout_orientation_set(efl_added, EFL_UI_LAYOUT_ORIENTATION_VERTICAL),
           elm_object_disabled_set(efl_added, EINA_TRUE),
           efl_pack(hbx, efl_added));

   efl_add(EFL_UI_TEXT_CLASS, bx,
           efl_text_set(efl_added, "Limited (100-500)"),
           efl_text_interactive_editable_set(efl_added, EINA_FALSE),
           efl_pack(bx, efl_added));

   efl_add(EFL_UI_SLIDER_INTERVAL_CLASS, bx,
           efl_gfx_hint_size_min_set(efl_added, EINA_SIZE2D(260, 0)),
           efl_ui_range_limits_set(efl_added, 0, 600),
           efl_ui_slider_interval_value_set(efl_added, 100, 500),
           efl_ui_range_step_set(efl_added, step),
           efl_event_callback_add(efl_added, EFL_UI_RANGE_EVENT_CHANGED, _intv_slider_changed_cb, NULL),
           efl_pack(bx, efl_added));
}
