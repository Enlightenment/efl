#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif
#include <Elementary.h>

static void
_time_changed_cb(void *data EINA_UNUSED, const Efl_Event *ev)
{
   int hour, min;

   efl_ui_timepicker_time_get(ev->object, &hour, &min);
   printf("Current time is %d %d\n", hour, min);
}


void
test_ui_timepicker(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Eo *win, *bx;

   win = efl_add(EFL_UI_WIN_CLASS, efl_main_loop_get(),
                 efl_ui_win_type_set(efl_added, EFL_UI_WIN_BASIC),
                 efl_text_set(efl_added, "Efl.Ui.Timepicker"),
                 efl_ui_win_autodel_set(efl_added, EINA_TRUE));

   bx = efl_add(EFL_UI_BOX_CLASS, win,
                efl_content_set(win, efl_added),
                efl_ui_direction_set(efl_added, EFL_UI_DIR_DOWN));

   efl_add(EFL_UI_TIMEPICKER_CLASS, bx,
           efl_ui_timepicker_time_set(efl_added, 11, 35),
           efl_event_callback_add(efl_added, EFL_UI_TIMEPICKER_EVENT_CHANGED,_time_changed_cb, NULL),
           efl_pack(bx, efl_added));

   efl_gfx_entity_size_set(win, EINA_SIZE2D(150, 170));
}
