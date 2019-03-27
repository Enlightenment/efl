// gcc -o efl_ui_relative_layout_example_02 efl_ui_relative_layout_example_02.c `pkg-config --cflags --libs elementary`

#ifdef HAVE_CONFIG_H
#include "elementary_config.h"
#else
#define EFL_BETA_API_SUPPORT 1
#endif

#include <Efl_Ui.h>
#include <Elementary.h>
#include <Efl.h>

EAPI_MAIN int
elm_main(int argc, char **argv)
{
   Eo *win, *layout, *btn1, *btn2;

   win =  efl_add(EFL_UI_WIN_CLASS, efl_main_loop_get(),
                  efl_ui_win_type_set(efl_added, EFL_UI_WIN_TYPE_BASIC),
                  efl_text_set(efl_added, "Efl.Ui.Relative_Layout"),
                  efl_ui_win_autodel_set(efl_added, EINA_TRUE));

   layout = efl_add(EFL_UI_RELATIVE_LAYOUT_CLASS, win,
                    efl_content_set(win, efl_added));

   btn1 = efl_add(EFL_UI_BUTTON_CLASS, layout,
                  efl_text_set(efl_added, "btn1"),
                  efl_gfx_hint_size_min_set(efl_added, EINA_SIZE2D(100, 100)));

   btn2 = efl_add(EFL_UI_BUTTON_CLASS, layout,
                  efl_text_set(efl_added, "btn2"),
                  efl_gfx_hint_size_min_set(efl_added, EINA_SIZE2D(0, 100)));

   efl_ui_relative_layout_relation_right_set(layout, btn1, btn2, 0.0);
   efl_ui_relative_layout_relation_bottom_set(layout, btn1, btn2, 0.0);

   efl_ui_relative_layout_relation_left_set(layout, btn2, btn1, 1.0);
   efl_ui_relative_layout_relation_top_set(layout, btn2, btn1, 1.0);

   efl_gfx_entity_size_set(win, EINA_SIZE2D(300, 300));

   elm_run();
   return 0;
}
ELM_MAIN()
