#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include <Efl_Ui.h>
#include <Elementary.h>
#include "efl_ui_suite.h"

/* What the theme hears about the parts of a layout while it is made, it hears
 * in the order it happened: an icon given and taken back leaves the text. */
EFL_START_TEST(efl_ui_button_deferred_signals_in_order)
{
   Eo *win = win_add(NULL, "win", EFL_UI_WIN_TYPE_BASIC);
   Eo *icon = efl_add(EFL_UI_BOX_CLASS, win);
   Eo *btn = efl_add(EFL_UI_BUTTON_CLASS, win,
                     efl_text_set(efl_added, "A"),
                     efl_content_set(efl_added, icon),
                     efl_del(efl_content_unset(efl_added)));

   get_me_to_those_events(win);
   ck_assert_ptr_eq(efl_content_get(btn), NULL);
   ck_assert_str_eq(edje_object_part_state_get(elm_layout_edje_get(btn), "efl.text", NULL),
                    "textonly");
}
EFL_END_TEST

void
efl_ui_test_button(TCase *tc)
{
   tcase_add_test(tc, efl_ui_button_deferred_signals_in_order);
}
