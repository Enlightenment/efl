#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif
#include <Efl_Ui.h>
#include <Elementary.h>

static void
_state_changed_cb(void *data EINA_UNUSED, const Efl_Event *ev)
{
   int val = efl_ui_nstate_value_get(ev->object);
   efl_text_set(ev->object, eina_slstr_printf("nstate = %d", val));
}

void
test_nstate(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Evas_Object *win, *bx, *nstate;

   win = efl_add_ref(EFL_UI_WIN_CLASS, NULL,
                 efl_text_set(efl_added, "N-state widget"),
                 elm_win_autodel_set(efl_added, EINA_TRUE));

   bx = efl_add(EFL_UI_BOX_CLASS, win);
   efl_content_set(win, bx);

   nstate = efl_add(EFL_UI_NSTATE_CLASS, win);
   efl_event_callback_add(nstate, EFL_UI_NSTATE_EVENT_CHANGED,
                          _state_changed_cb, NULL);
   efl_ui_nstate_count_set(nstate, 5);
   efl_text_set(nstate, "nstate = 0");
   efl_pack(bx, nstate);

   efl_gfx_entity_size_set(win, EINA_SIZE2D(120, 80));
}
