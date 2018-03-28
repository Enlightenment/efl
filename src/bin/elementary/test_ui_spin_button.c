#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif
#include <Elementary.h>

static void
_spin_delay_changed_cb(void *data EINA_UNUSED, const Efl_Event *ev)
{
   printf("Value delay changed %d\n", (int)efl_ui_range_value_get(ev->object));
}

void
test_ui_spin_button(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Eo *win, *bx;

   win = efl_add(EFL_UI_WIN_CLASS, efl_main_loop_get(),
                 efl_ui_win_type_set(efl_added, EFL_UI_WIN_BASIC),
                 efl_text_set(efl_added, "Efl.Ui.Spin_Button"),
                 efl_ui_win_autodel_set(efl_added, EINA_TRUE));

   bx = efl_add(EFL_UI_BOX_CLASS, win,
                efl_content_set(win, efl_added),
                efl_ui_direction_set(efl_added, EFL_UI_DIR_DOWN));

   efl_add(EFL_UI_SPIN_BUTTON_CLASS, bx,
           efl_ui_range_min_max_set(efl_added, 0, 10),
           efl_ui_range_value_set(efl_added, 6),
           efl_ui_range_step_set(efl_added, 2),
           efl_ui_spin_button_circulate_set(efl_added, EINA_TRUE),
           efl_ui_spin_button_editable_set(efl_added, EINA_TRUE),
           efl_event_callback_add(efl_added, EFL_UI_SPIN_BUTTON_EVENT_DELAY_CHANGED,_spin_delay_changed_cb, NULL),
           efl_pack(bx, efl_added));

   efl_add(EFL_UI_SPIN_BUTTON_CLASS, bx,
           efl_ui_range_min_max_set(efl_added, -100.0, 100.0),
           efl_ui_range_value_set(efl_added, 0),
           efl_ui_format_string_set(efl_added, "test float %0.2f"),
           efl_ui_spin_button_editable_set(efl_added, EINA_FALSE),
           efl_pack(bx, efl_added));

   efl_gfx_size_set(win, EINA_SIZE2D(180, 100));
}
