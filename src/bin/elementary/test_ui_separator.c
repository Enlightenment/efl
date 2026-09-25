#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif
#include <Efl_Ui.h>

/* The same arrangement the legacy page uses, because a separator is only
   worth looking at with something on either side of it: two columns of
   buttons, a rule between the buttons in each column, and a rule between
   the columns. The window is left to take its own smallest size, so the
   separators are seen doing their job rather than filling a chosen space. */

static Eo *
_button(Eo *parent, const char *text, Eina_Bool disabled)
{
   return efl_add(EFL_UI_BUTTON_CLASS, parent,
                  efl_text_set(efl_added, text),
                  efl_ui_widget_disabled_set(efl_added, disabled),
                  efl_pack_end(parent, efl_added));
}

static Eo *
_column(Eo *parent)
{
   return efl_add(EFL_UI_BOX_CLASS, parent,
                  efl_ui_layout_orientation_set(efl_added,
                                                EFL_UI_LAYOUT_ORIENTATION_VERTICAL),
                  efl_pack_end(parent, efl_added));
}

void
test_ui_separator(void *data EINA_UNUSED, Eo *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Eo *win, *row, *col;

   win = efl_add(EFL_UI_WIN_CLASS, efl_main_loop_get(),
                 efl_text_set(efl_added, "Efl.Ui.Separator"),
                 efl_ui_win_autodel_set(efl_added, EINA_TRUE));

   row = efl_add(EFL_UI_BOX_CLASS, win,
                 efl_ui_layout_orientation_set(efl_added,
                                               EFL_UI_LAYOUT_ORIENTATION_HORIZONTAL),
                 efl_content_set(win, efl_added));

   col = _column(row);
   _button(col, "Left upper corner", EINA_FALSE);
   efl_add(EFL_UI_SEPARATOR_CLASS, col,
           efl_ui_layout_orientation_set(efl_added,
                                         EFL_UI_LAYOUT_ORIENTATION_HORIZONTAL),
           efl_pack_end(col, efl_added));
   _button(col, "Left lower corner", EINA_TRUE);

   efl_add(EFL_UI_SEPARATOR_CLASS, row,
           efl_ui_layout_orientation_set(efl_added,
                                         EFL_UI_LAYOUT_ORIENTATION_VERTICAL),
           efl_pack_end(row, efl_added));

   col = _column(row);
   _button(col, "Right upper corner", EINA_TRUE);
   efl_add(EFL_UI_SEPARATOR_CLASS, col,
           efl_ui_layout_orientation_set(efl_added,
                                         EFL_UI_LAYOUT_ORIENTATION_HORIZONTAL),
           efl_pack_end(col, efl_added));
   _button(col, "Right lower corner", EINA_FALSE);

   /* The window takes the size the two columns and their rules need, which
      is what the legacy page does by leaving elm_win to size itself on
      show. Nothing here has been laid out yet, so ask for the calculation
      first. */
   efl_canvas_group_calculate(row);
   efl_gfx_entity_size_set(win, efl_gfx_hint_size_combined_min_get(row));
}
