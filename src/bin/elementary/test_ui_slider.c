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
_slider_changed_cb(void *data EINA_UNUSED, const Efl_Event *ev)
{
   double val;

   val = efl_ui_range_value_get(ev->object);

   printf("val = %f\n", val);
   if (val > 100)
     efl_ui_range_value_set(ev->object, 100);
}

void
test_ui_slider(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Eo *win, *bx, *hbx;
   double step;

   win = efl_add_ref(EFL_UI_WIN_CLASS, NULL,
                 efl_ui_win_type_set(efl_added, EFL_UI_WIN_BASIC),
                 efl_text_set(efl_added, "Efl.Ui.Slider"),
                 efl_ui_win_autodel_set(efl_added, EINA_TRUE));

   bx = efl_add(EFL_UI_BOX_CLASS, win,
                efl_content_set(win, efl_added));

   efl_add(EFL_UI_TEXT_CLASS, bx,
           efl_text_set(efl_added, "Horizontal"),
           efl_text_interactive_editable_set(efl_added, EINA_FALSE),
           efl_pack(bx, efl_added));

   efl_add(EFL_UI_SLIDER_CLASS, bx,
           efl_gfx_size_hint_min_set(efl_added, EINA_SIZE2D(160, 0)),
           efl_gfx_size_hint_weight_set(efl_added, EVAS_HINT_EXPAND, 0.0),
           efl_pack(bx, efl_added));

   efl_add(EFL_UI_TEXT_CLASS, bx,
           efl_text_set(efl_added, "Horizontal Inverted"),
           efl_text_interactive_editable_set(efl_added, EINA_FALSE),
           efl_pack(bx, efl_added));

   efl_add(EFL_UI_SLIDER_CLASS, bx,
           efl_gfx_size_hint_min_set(efl_added, EINA_SIZE2D(160, 0)),
           efl_gfx_size_hint_weight_set(efl_added, EVAS_HINT_EXPAND, 0.0),
		   efl_ui_direction_set(efl_added, EFL_UI_DIR_LEFT),
           efl_pack(bx, efl_added));

   efl_add(EFL_UI_TEXT_CLASS, bx,
           efl_text_set(efl_added, "Manual step"),
           efl_text_interactive_editable_set(efl_added, EINA_FALSE),
           efl_pack(bx, efl_added));

   step = _step_size_calculate(0, 9);
   efl_add(EFL_UI_SLIDER_CLASS, bx,
           efl_gfx_size_hint_min_set(efl_added, EINA_SIZE2D(120, 0)),
           efl_gfx_size_hint_align_set(efl_added, 0.5, 0.5),
           efl_ui_range_step_set(efl_added, step),
           efl_pack(bx, efl_added));

   efl_add(EFL_UI_TEXT_CLASS, bx,
           efl_text_set(efl_added, "Disabled"),
           efl_text_interactive_editable_set(efl_added, EINA_FALSE),
           efl_pack(bx, efl_added));

   efl_add(EFL_UI_SLIDER_CLASS, bx,
           efl_gfx_size_hint_min_set(efl_added, EINA_SIZE2D(120, 0)),
           efl_ui_range_min_max_set(efl_added, 10, 145),
           efl_ui_range_step_set(efl_added, step),
           elm_object_disabled_set(efl_added, EINA_TRUE),
           efl_pack(bx, efl_added));

   efl_add(EFL_UI_TEXT_CLASS, bx,
           efl_text_set(efl_added, "Vertical"),
           efl_text_interactive_editable_set(efl_added, EINA_FALSE),
           efl_pack(bx, efl_added));

   hbx = efl_add(EFL_UI_BOX_CLASS, bx,
                 efl_ui_direction_set(efl_added, EFL_UI_DIR_HORIZONTAL),
                 efl_pack(bx, efl_added));

   efl_add(EFL_UI_SLIDER_CLASS, hbx,
           efl_gfx_size_hint_min_set(efl_added, EINA_SIZE2D(0, 160)),
           efl_ui_range_min_max_set(efl_added, 10, 145),
           efl_ui_range_step_set(efl_added, step),
           efl_ui_range_value_set(efl_added, 70),
           efl_ui_direction_set(efl_added, EFL_UI_DIR_VERTICAL),
           efl_pack(hbx, efl_added));

   efl_add(EFL_UI_SLIDER_CLASS, hbx,
           efl_gfx_size_hint_min_set(efl_added, EINA_SIZE2D(0, 160)),
           efl_ui_range_min_max_set(efl_added, 10, 145),
           efl_ui_range_step_set(efl_added, step),
           efl_ui_direction_set(efl_added, EFL_UI_DIR_VERTICAL),
           elm_object_disabled_set(efl_added, EINA_TRUE),
           efl_pack(hbx, efl_added));

   efl_add(EFL_UI_SLIDER_CLASS, hbx,
           efl_gfx_size_hint_min_set(efl_added, EINA_SIZE2D(0, 160)),
           efl_ui_range_min_max_set(efl_added, 10, 145),
           efl_ui_range_value_set(efl_added, 35),
           efl_ui_range_step_set(efl_added, step),
           efl_ui_direction_set(efl_added, EFL_UI_DIR_DOWN),
           efl_pack(hbx, efl_added));

   efl_add(EFL_UI_TEXT_CLASS, bx,
           efl_text_set(efl_added, "Limit"),
           efl_text_interactive_editable_set(efl_added, EINA_FALSE),
           efl_pack(bx, efl_added));

   efl_add(EFL_UI_SLIDER_CLASS, bx,
           efl_gfx_size_hint_min_set(efl_added, EINA_SIZE2D(260, 0)),
           efl_ui_range_min_max_set(efl_added, 0, 150),
           efl_ui_range_step_set(efl_added, step),
           efl_event_callback_add(efl_added, EFL_UI_SLIDER_EVENT_CHANGED, _slider_changed_cb, NULL),
           efl_pack(bx, efl_added));
}
