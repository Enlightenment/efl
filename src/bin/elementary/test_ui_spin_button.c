#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif
#include <Efl_Ui.h>
#include <Elementary.h>

#define NUM_OF_VALS 12

static void
_spin_delay_changed_cb(void *data EINA_UNUSED, const Efl_Event *ev)
{
   printf("Value delay changed %d\n", (int)efl_ui_range_value_get(ev->object));
}

static void
_spin_changed_cb(void *data EINA_UNUSED, const Efl_Event *ev)
{
   printf("Value changed %d\n", (int)efl_ui_range_value_get(ev->object));
}

void
test_ui_spin_button(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Eo *win, *bx;
   Efl_Ui_Format_Value special_values[] = {
     {1, "January"}, {2, "February"}, {3, "March"}, {4, "April"},
     {5, "May"}, {6, "June"}, {7, "July"}, {8, "August"},
     {9, "September"}, {10, "October"}, {11, "November"}, {12, "December"}
   };

   win = efl_add(EFL_UI_WIN_CLASS, efl_main_loop_get(),
                 efl_text_set(efl_added, "Efl.Ui.Spin_Button"),
                 efl_ui_win_autodel_set(efl_added, EINA_TRUE));

   bx = efl_add(EFL_UI_BOX_CLASS, win,
                efl_content_set(win, efl_added),
                efl_ui_layout_orientation_set(efl_added, EFL_UI_LAYOUT_ORIENTATION_VERTICAL));

   efl_add(EFL_UI_SPIN_BUTTON_CLASS, bx,
           efl_ui_range_limits_set(efl_added, 0, 10),
           efl_ui_range_value_set(efl_added, 6),
           efl_ui_range_step_set(efl_added, 2),
           efl_ui_spin_button_wraparound_set(efl_added, EINA_TRUE),
           efl_ui_spin_button_direct_text_input_set(efl_added, EINA_TRUE),
           efl_event_callback_add(efl_added, EFL_UI_RANGE_EVENT_STEADY,_spin_delay_changed_cb, NULL),
           efl_event_callback_add(efl_added, EFL_UI_RANGE_EVENT_CHANGED, _spin_changed_cb, NULL),
           efl_pack(bx, efl_added));

   efl_add(EFL_UI_SPIN_BUTTON_CLASS, bx,
           efl_ui_range_limits_set(efl_added, -100.0, 100.0),
           efl_ui_range_value_set(efl_added, 0),
           efl_ui_format_string_set(efl_added, "test float %0.2f", EFL_UI_FORMAT_STRING_TYPE_SIMPLE),
           efl_ui_spin_button_direct_text_input_set(efl_added, EINA_FALSE),
           efl_pack(bx, efl_added));

   efl_add(EFL_UI_SPIN_BUTTON_CLASS, bx,
           efl_ui_range_limits_set(efl_added, 1, 12),
           efl_ui_range_value_set(efl_added, 1),
           efl_ui_spin_button_direct_text_input_set(efl_added, EINA_FALSE),
           efl_ui_format_values_set(efl_added, EINA_C_ARRAY_ACCESSOR_PTR_NEW(special_values)),
           efl_ui_layout_orientation_set(efl_added, EFL_UI_LAYOUT_ORIENTATION_VERTICAL),
           efl_pack(bx, efl_added));

   efl_gfx_entity_size_set(win, EINA_SIZE2D(180, 140));
}
