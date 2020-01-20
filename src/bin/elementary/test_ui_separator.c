#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif
#include <Efl_Ui.h>

void
test_ui_separator(void *data EINA_UNUSED, Eo *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Eo *win, *table, *sep;

   win = efl_add(EFL_UI_WIN_CLASS, efl_main_loop_get(),
                 efl_text_set(efl_added, "Efl.Ui.Separator"),
                 efl_ui_win_autodel_set(efl_added, EINA_TRUE));

   table = efl_add(EFL_UI_TABLE_CLASS, win);
   efl_content_set(win, table);

   sep = efl_add(EFL_UI_SEPARATOR_CLASS, win);
   efl_pack_table(table, sep, 0, 0, 2, 1);

   sep = efl_add(EFL_UI_SEPARATOR_CLASS, win,
                 efl_ui_layout_orientation_set(efl_added, EFL_UI_LAYOUT_ORIENTATION_VERTICAL));
   efl_pack_table(table, sep, 0, 0, 2, 1);

   efl_gfx_entity_size_set(win, EINA_SIZE2D(100, 120));
}
