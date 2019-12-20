#define EFL_NOLEGACY_API_SUPPORT
#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif
#define EFL_LAYOUT_CALC_PROTECTED
#include <Efl_Ui.h>
#include "efl_ui_suite.h"

static void
increment_int_changed(void *data EINA_UNUSED, const Efl_Event *ev EINA_UNUSED)
{
   int *value = data;
   (*value)++;
}

EFL_START_TEST(text_cnp)
{
   Eo *txt;
   Eo *win = win_add();

   int i_copy = 0, i_paste = 0, i_cut = 0;

   txt = efl_add(EFL_UI_TEXTBOX_CLASS, win,
                efl_event_callback_add(efl_added, EFL_UI_TEXTBOX_EVENT_SELECTION_COPY, increment_int_changed, &i_copy),
                efl_event_callback_add(efl_added, EFL_UI_TEXTBOX_EVENT_SELECTION_PASTE, increment_int_changed, &i_paste),
                efl_event_callback_add(efl_added, EFL_UI_TEXTBOX_EVENT_SELECTION_CUT, increment_int_changed, &i_cut)
                );

   efl_text_set(txt, "Hello");
   efl_text_interactive_all_select(txt);
   efl_ui_textbox_selection_copy(txt);
   efl_text_interactive_all_select(txt);
   efl_ui_textbox_selection_cut(txt);
   efl_ui_textbox_selection_paste(txt);
   efl_ui_textbox_selection_paste(txt);
   ecore_main_loop_iterate();
   ck_assert_int_eq(i_copy, 1);
   ck_assert_int_eq(i_cut, 1);
   ck_assert_int_eq(i_paste, 2);
   //FIXME
   //fail_if(strcmp(efl_text_get(txt),"HelloHello"));
   efl_del(txt);
}
EFL_END_TEST

EFL_START_TEST(text_all_select_all_unselect)
{
   Eo *txt;
   Eo *win = win_add();

   int i_have_selection = 0, i_selection = 0;

   txt = efl_add(EFL_UI_TEXTBOX_CLASS, win,
                efl_event_callback_add(efl_added, EFL_TEXT_INTERACTIVE_EVENT_HAVE_SELECTION_CHANGED,
                            increment_int_changed, &i_have_selection),
                efl_event_callback_add(efl_added, EFL_TEXT_INTERACTIVE_EVENT_SELECTION_CHANGED,
                            increment_int_changed, &i_selection)
                );

   efl_text_set(txt, "Hello");
   efl_text_interactive_all_select(txt);
   Efl_Text_Cursor *c1=NULL, *c2 =NULL;
   efl_text_interactive_selection_cursors_get(txt, &c1, &c2);
   ck_assert_ptr_ne(c1, NULL);
   ck_assert_ptr_ne(c2, NULL);
   fail_if(!efl_text_interactive_have_selection_get(txt));
   ck_assert_int_eq(efl_text_cursor_position_get(c1), 0);
   ck_assert_int_eq(efl_text_cursor_position_get(c2), 5);
   efl_text_interactive_all_unselect(txt);
   fail_if(efl_text_interactive_have_selection_get(txt));
   ecore_main_loop_iterate();
   ck_assert_int_eq(i_have_selection, 2);
   ck_assert_int_eq(i_selection, 1);
   efl_del(txt);
   efl_del(win);
}
EFL_END_TEST

EFL_START_TEST(text_selection)
{
   Eo *txt;
   Eo *win = win_add();

   txt = efl_add(EFL_UI_TEXTBOX_CLASS, win);
   efl_gfx_entity_size_set(txt, EINA_SIZE2D(300, 300));
   efl_text_set(txt, "Hello");
   get_me_to_those_events(txt);
   drag_object(txt, 2, 2, 295, 295, EINA_TRUE);
   ecore_main_loop_iterate();
   Efl_Text_Cursor *c1=NULL, *c2 =NULL;
   efl_text_interactive_selection_cursors_get(txt, &c1, &c2);
   ck_assert_ptr_ne(c1, NULL);
   ck_assert_ptr_ne(c2, NULL);
   fail_if(!efl_text_interactive_have_selection_get(txt));
   ck_assert_int_eq(efl_text_cursor_position_get(c1), 0);
   ck_assert_int_eq(efl_text_cursor_position_get(c2), 5);
   efl_text_interactive_all_unselect(txt);
   fail_if(efl_text_interactive_have_selection_get(txt));
   efl_text_interactive_selection_allowed_set(txt, EINA_FALSE);
   drag_object(txt, 2, 2, 295, 295, EINA_TRUE);
   ecore_main_loop_iterate();
   fail_if(efl_text_interactive_have_selection_get(txt));
   efl_del(txt);
}
EFL_END_TEST

static void
user_changed(void *data, const Efl_Event *ev)
{
   Efl_Text_Change_Info *info = (Efl_Text_Change_Info *)ev->info;
   Efl_Text_Change_Info *stored = (Efl_Text_Change_Info *)data;
   *stored = *info;
}

EFL_START_TEST(text_user_change)
{
   Eo *txt;
   Eo *win = win_add();
   Efl_Text_Change_Info info = {0};
   txt = efl_add(EFL_UI_TEXTBOX_CLASS, win,
                efl_event_callback_add(efl_added, EFL_TEXT_INTERACTIVE_EVENT_CHANGED_USER, user_changed, &info)
                );

   efl_text_set(txt, "Hello");
   efl_gfx_entity_size_set(txt, EINA_SIZE2D(300, 300));
   efl_text_interactive_all_select(txt);
   efl_ui_textbox_selection_cut(txt);
   ck_assert_int_eq(info.position, 0);
   ck_assert_int_eq(info.length, 5);
   efl_del(txt);
   efl_del(win);
}
EFL_END_TEST

void efl_ui_test_text(TCase *tc)
{
   tcase_add_test(tc, text_cnp);
   tcase_add_test(tc, text_all_select_all_unselect);
   tcase_add_test(tc, text_selection);
   tcase_add_test(tc, text_user_change);
}
