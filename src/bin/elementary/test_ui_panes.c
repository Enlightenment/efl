#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif
#include <Elementary.h>

void
test_panes_minsize(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Eo *win, *panes, *panes_h;

   win = efl_add_ref(EFL_UI_WIN_CLASS, NULL,
                 efl_ui_win_type_set(efl_added, EFL_UI_WIN_BASIC),
                 efl_text_set(efl_added, "Efl.Ui.Panes"),
                 efl_ui_win_autodel_set(efl_added, EINA_TRUE)
                );

   panes = efl_add(EFL_UI_PANES_CLASS, win,
                   efl_content_set(win, efl_added),
                   efl_ui_panes_split_ratio_set(efl_added, 0.7)
                  );

   efl_add(EFL_UI_BUTTON_CLASS, win,
           efl_text_set(efl_added, "Left - user set min size(110,110)"),
           efl_gfx_hint_size_min_set(efl_added, EINA_SIZE2D(110, 110)),
           efl_content_set(efl_part(panes, "first"), efl_added)
          );

   panes_h = efl_add(EFL_UI_PANES_CLASS, win,
                     efl_ui_direction_set(efl_added, EFL_UI_DIR_HORIZONTAL),
                     efl_content_set(efl_part(panes, "second"), efl_added)
                    );
   efl_add(EFL_UI_BUTTON_CLASS, win,
           efl_text_set(efl_added, "Up - user set min size(10,0)"),
           efl_gfx_hint_size_min_set(efl_added, EINA_SIZE2D(10, 0)),
           efl_content_set(efl_part(panes_h, "first"), efl_added)
          );
   efl_ui_panes_part_hint_min_allow_set(efl_part(panes_h, "first"), EINA_TRUE);

   efl_add(EFL_UI_BUTTON_CLASS, win,
           efl_text_set(efl_added, "Down - min size 50 40"),
           efl_gfx_hint_size_min_set(efl_added, EINA_SIZE2D(50, 40)),
           efl_content_set(efl_part(panes_h, "second"), efl_added)
          );

   efl_gfx_entity_size_set(win, EINA_SIZE2D(320,  400));
}

