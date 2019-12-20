#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif
#include <Efl_Ui.h>
#include <Elementary.h>

static void
_inc_clicked(void *data, const Efl_Event *ev EINA_UNUSED)
{
   efl_ui_frame_collapse_go(data, EINA_TRUE);
}

void
test_ui_frame(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Eo *win, *bx, *f, *txt;

   win = efl_add(EFL_UI_WIN_CLASS, efl_main_loop_get(),
                                  efl_text_set(efl_added, "Efl.Ui.Frame"),
                 efl_ui_win_autodel_set(efl_added, EINA_TRUE));

   bx = efl_add(EFL_UI_BOX_CLASS, win,
                efl_content_set(win, efl_added),
                efl_ui_layout_orientation_set(efl_added, EFL_UI_LAYOUT_ORIENTATION_VERTICAL));

   f = efl_add(EFL_UI_FRAME_CLASS, win,
               efl_pack_end(bx, efl_added),
               efl_ui_frame_autocollapse_set(efl_added, EINA_FALSE));
   efl_text_set(f, "Test 1");

   txt = efl_add(EFL_UI_TEXTBOX_CLASS, f);
   efl_text_set(txt, "Not collapseable");
   efl_content_set(f, txt);

   f = efl_add(EFL_UI_FRAME_CLASS, win,
               efl_pack_end(bx, efl_added),
               efl_ui_frame_autocollapse_set(efl_added, EINA_TRUE));
   efl_text_set(f, "Test2");

   txt = efl_add(EFL_UI_TEXTBOX_CLASS, f);
   efl_text_set(txt, "Collapseable");
   efl_content_set(f, txt);

   f = efl_add(EFL_UI_FRAME_CLASS, win,
               efl_pack_end(bx, efl_added));
   efl_text_set(f, "Test 3");

   efl_add(EFL_UI_BUTTON_CLASS, bx,
           efl_text_set(efl_added, "frame collapse externally"),
           efl_event_callback_add(efl_added, EFL_INPUT_EVENT_CLICKED, _inc_clicked, f),
           efl_content_set(f, efl_added));

   efl_gfx_entity_size_set(win, EINA_SIZE2D(100, 120));
}
