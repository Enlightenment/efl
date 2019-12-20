#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include <Efl_Ui.h>
#include "efl_ui_suite.h"

static Eo *win, *check;

static void
check_setup()
{
   win = win_add();

   check = efl_add(EFL_UI_CHECK_CLASS, win);
}

EFL_START_TEST(check_content)
{
   Eo *t = efl_add(EFL_UI_BUTTON_CLASS, win,
    efl_text_set(efl_added, "asdf")
   );
   efl_content_set(check, t);
   ck_assert_ptr_eq(efl_content_get(check), t);
   //TODO verify that signals can be emitted
}
EFL_END_TEST

EFL_START_TEST(check_text)
{
   efl_text_set(check, "Test the Rest");
   ck_assert_str_eq(efl_text_get(check), "Test the Rest");
   //TODO verify that signals can be emitted
}
EFL_END_TEST

EFL_START_TEST(check_selection_state)
{
   efl_ui_selectable_selected_set(check, EINA_TRUE);
   ck_assert_int_eq(efl_ui_selectable_selected_get(check), EINA_TRUE);
   efl_ui_selectable_selected_set(check, EINA_TRUE);
   ck_assert_int_eq(efl_ui_selectable_selected_get(check), EINA_TRUE);
   //TODO verify that signals can be emitted

   efl_ui_selectable_selected_set(check, EINA_FALSE);
   ck_assert_int_eq(efl_ui_selectable_selected_get(check), EINA_FALSE);
   efl_ui_selectable_selected_set(check, EINA_FALSE);
   ck_assert_int_eq(efl_ui_selectable_selected_get(check), EINA_FALSE);
   //TODO verify that signals can be emitted
}
EFL_END_TEST

static void
_check(void *data, const Efl_Event *ev)
{
   Eina_Bool *flag = data;

   *flag = *((Eina_Bool*)ev->info);
}

EFL_START_TEST(check_selection_events)
{
   Eina_Bool bool_event_info = EINA_FALSE;

   efl_event_callback_add(check, EFL_UI_EVENT_SELECTED_CHANGED, _check, &bool_event_info);
   efl_ui_selectable_selected_set(check, EINA_TRUE);
   ck_assert_int_eq(bool_event_info, EINA_TRUE);
   efl_ui_selectable_selected_set(check, EINA_FALSE);
   ck_assert_int_eq(bool_event_info, EINA_FALSE);
}
EFL_END_TEST


EFL_START_TEST(efl_ui_test_check_callbacks)
{
   int called = 0;
   int i;

   efl_gfx_entity_size_set(win, EINA_SIZE2D(500, 500));

   efl_text_set(check, "TEST TEST TEST");
   efl_event_callback_add(check, EFL_UI_EVENT_SELECTED_CHANGED, (void*)event_callback_single_call_int_data, &called);

   efl_gfx_entity_size_set(check, EINA_SIZE2D(200, 100));
   get_me_to_those_events(check);

   for (i = 0; i < 4; i++)
     {
        called = 0;
        click_object(check);
        ecore_main_loop_iterate();
        ck_assert_int_eq(efl_ui_selectable_selected_get(check), !(i % 2));
        ck_assert_int_eq(called, 1);
     }
}
EFL_END_TEST

void efl_ui_test_check(TCase *tc)
{
   tcase_add_checked_fixture(tc, check_setup, NULL);
   tcase_add_test(tc, check_selection_state);
   tcase_add_test(tc, check_text);
   tcase_add_test(tc, check_content);
   tcase_add_test(tc, check_selection_events);
   tcase_add_test(tc, efl_ui_test_check_callbacks);
}
