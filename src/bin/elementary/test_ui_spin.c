#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif
#include <Efl_Ui.h>
#include <Elementary.h>

static void
_spin_changed_cb(void *data EINA_UNUSED, const Efl_Event *ev)
{
   printf("Value changed %d\n", (int)efl_ui_range_value_get(ev->object));
}
static void
_spin_min_reached_cb(void *data EINA_UNUSED, const Efl_Event *ev)
{
   printf("Min reached %d\n", (int)efl_ui_range_value_get(ev->object));
}
static void
_spin_max_reached_cb(void *data EINA_UNUSED, const Efl_Event *ev)
{
   printf("Max reached %d\n", (int)efl_ui_range_value_get(ev->object));
}

static void
_inc_clicked(void *data, const Efl_Event *ev EINA_UNUSED)
{
   efl_ui_range_value_set(data, (efl_ui_range_value_get(data) + efl_ui_range_step_get(data)));
}

static void
_dec_clicked(void *data, const Efl_Event *ev EINA_UNUSED)
{
   efl_ui_range_value_set(data, (efl_ui_range_value_get(data) - efl_ui_range_step_get(data)));
}

void
test_ui_spin(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Eo *win, *bx, *sp;

   win = efl_add_ref(EFL_UI_WIN_CLASS, NULL,
                 efl_ui_win_type_set(efl_added, EFL_UI_WIN_TYPE_BASIC),
                 efl_text_set(efl_added, "Efl.Ui.Spin"),
                 efl_ui_win_autodel_set(efl_added, EINA_TRUE));

   bx = efl_add(EFL_UI_BOX_CLASS, win,
                efl_content_set(win, efl_added),
                efl_ui_direction_set(efl_added, EFL_UI_DIR_DOWN));

   sp = efl_add(EFL_UI_SPIN_CLASS, bx,
                efl_ui_range_min_max_set(efl_added, 0, 10),
                efl_ui_range_value_set(efl_added, 6),
                efl_ui_range_step_set(efl_added, 2),
                efl_ui_format_string_set(efl_added, "test %d"),
                efl_event_callback_add(efl_added, EFL_UI_SPIN_EVENT_CHANGED,_spin_changed_cb, NULL),
                efl_event_callback_add(efl_added, EFL_UI_SPIN_EVENT_MIN_REACHED,_spin_min_reached_cb, NULL),
                efl_event_callback_add(efl_added, EFL_UI_SPIN_EVENT_MAX_REACHED,_spin_max_reached_cb, NULL),
                efl_pack(bx, efl_added));

   efl_add(EFL_UI_BUTTON_CLASS, bx,
           efl_text_set(efl_added, "Increse Spinner value"),
           efl_event_callback_add(efl_added, EFL_UI_EVENT_CLICKED, _inc_clicked, sp),
           efl_pack(bx, efl_added));

   efl_add(EFL_UI_BUTTON_CLASS, bx,
           efl_text_set(efl_added, "Decrease Spinner value"),
           efl_event_callback_add(efl_added, EFL_UI_EVENT_CLICKED, _dec_clicked, sp),
           efl_pack(bx, efl_added));

   efl_gfx_entity_size_set(win, EINA_SIZE2D(100, 120));
}
