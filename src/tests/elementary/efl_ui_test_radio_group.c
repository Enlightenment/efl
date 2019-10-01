#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include <Efl_Ui.h>
#include "efl_ui_suite.h"

static Eo *win, *radio_group;

int state_values = 0;

typedef struct {
   int called;
   int value;
} Group_Changed_Called;

Group_Changed_Called changed_evt;

#define RESET_EVT \
  changed_evt.called = 0; \
  changed_evt.value = 200000000;

#define EXPECT_EVT(c, v) \
  ck_assert_int_eq(changed_evt.called, c); \
  ck_assert_int_eq(changed_evt.value, v);

static void
_group_value_changed_cb(void *data EINA_UNUSED, const Efl_Event *ev)
{
   changed_evt.called ++;
   changed_evt.value = *((int*)ev->info);
}

static void
check_setup()
{
   win = win_add();
   radio_group = efl_new(EFL_UI_RADIO_GROUP_IMPL_CLASS, NULL);
   efl_event_callback_add(radio_group, EFL_UI_RADIO_GROUP_EVENT_VALUE_CHANGED, _group_value_changed_cb, NULL);
}

static void
check_teardown()
{
   radio_group = NULL;
   win = NULL;
   state_values = 0;
}

static Eo*
radio(void)
{
   state_values += 1;
   return efl_add(EFL_UI_RADIO_CLASS, win, efl_ui_radio_state_value_set(efl_added, state_values));
}

EFL_START_TEST(active_selected_value_setting)
{
   Eo *r1 = radio();
   Eo *r2 = radio();

   efl_ui_radio_group_register(radio_group, radio());
   efl_ui_radio_group_register(radio_group, radio());
   efl_ui_radio_group_register(radio_group, r1);
   efl_ui_radio_group_register(radio_group, r2);

   ck_assert_int_eq(efl_ui_radio_group_selected_value_get(radio_group), -1);
   ck_assert_ptr_eq(efl_ui_selectable_last_selected_get(radio_group), NULL);

   RESET_EVT
   efl_ui_radio_group_selected_value_set(radio_group, 1);
   ck_assert_int_eq(efl_ui_radio_group_selected_value_get(radio_group), 1);
   ck_assert_ptr_eq(efl_ui_selectable_last_selected_get(radio_group), r1);
   ck_assert_int_eq(efl_ui_selectable_selected_get(r1), EINA_TRUE);
   ck_assert_int_eq(efl_ui_selectable_selected_get(r2), EINA_FALSE);
   EXPECT_EVT(1,1);

   RESET_EVT
   efl_ui_selectable_selected_set(r2, EINA_TRUE);
   ck_assert_int_eq(efl_ui_radio_group_selected_value_get(radio_group), 2);
   ck_assert_ptr_eq(efl_ui_selectable_last_selected_get(radio_group), r2);
   ck_assert_int_eq(efl_ui_selectable_selected_get(r1), EINA_FALSE);
   ck_assert_int_eq(efl_ui_selectable_selected_get(r2), EINA_TRUE);
   EXPECT_EVT(1,2);
}
EFL_END_TEST

EFL_START_TEST(active_selection_setting)
{
   Eo *r1 = radio();
   Eo *r2 = radio();

   efl_ui_radio_group_register(radio_group, radio());
   efl_ui_radio_group_register(radio_group, radio());
   efl_ui_radio_group_register(radio_group, r1);
   efl_ui_radio_group_register(radio_group, r2);

   ck_assert_int_eq(efl_ui_radio_group_selected_value_get(radio_group), -1);
   ck_assert_ptr_eq(efl_ui_selectable_last_selected_get(radio_group), NULL);

   RESET_EVT
   efl_ui_selectable_selected_set(r1, EINA_TRUE);
   ck_assert_int_eq(efl_ui_radio_group_selected_value_get(radio_group), 1);
   ck_assert_ptr_eq(efl_ui_selectable_last_selected_get(radio_group), r1);
   ck_assert_int_eq(efl_ui_selectable_selected_get(r1), EINA_TRUE);
   ck_assert_int_eq(efl_ui_selectable_selected_get(r2), EINA_FALSE);
   EXPECT_EVT(1,1);

   RESET_EVT
   efl_ui_selectable_selected_set(r2, EINA_TRUE);
   ck_assert_int_eq(efl_ui_radio_group_selected_value_get(radio_group), 2);
   ck_assert_ptr_eq(efl_ui_selectable_last_selected_get(radio_group), r2);
   ck_assert_int_eq(efl_ui_selectable_selected_get(r1), EINA_FALSE);
   ck_assert_int_eq(efl_ui_selectable_selected_get(r2), EINA_TRUE);
   EXPECT_EVT(1,2);
}
EFL_END_TEST

EFL_START_TEST(active_selection_object_setting)
{
   Eo *r = radio();
   efl_ui_radio_group_register(radio_group, radio());
   efl_ui_radio_group_register(radio_group, radio());
   efl_ui_radio_group_register(radio_group, r);
   efl_ui_radio_group_register(radio_group, radio());

   RESET_EVT
   ck_assert_int_eq(efl_ui_radio_group_selected_value_get(radio_group), -1);
   ck_assert_ptr_eq(efl_ui_selectable_last_selected_get(radio_group), NULL);
   efl_ui_selectable_selected_set(r, EINA_TRUE);
   ck_assert_int_eq(efl_ui_radio_group_selected_value_get(radio_group), 1);
   ck_assert_ptr_eq(efl_ui_selectable_last_selected_get(radio_group), r);
   EXPECT_EVT(1,1);
}
EFL_END_TEST

EFL_START_TEST(unregister_setted)
{
   Eo *r = radio();
   efl_ui_radio_group_register(radio_group, radio());
   efl_ui_radio_group_register(radio_group, radio());
   efl_ui_radio_group_register(radio_group, r);
   efl_ui_radio_group_register(radio_group, radio());

   RESET_EVT
   efl_ui_selectable_selected_set(r, EINA_TRUE);
   ck_assert_int_eq(efl_ui_radio_group_selected_value_get(radio_group), 1);
   ck_assert_ptr_eq(efl_ui_selectable_last_selected_get(radio_group), r);
   EXPECT_EVT(1,1);
   RESET_EVT
   efl_ui_radio_group_unregister(radio_group, r);
   ck_assert_int_eq(efl_ui_radio_group_selected_value_get(radio_group), -1);
   ck_assert_ptr_eq(efl_ui_selectable_last_selected_get(radio_group), NULL);
   EXPECT_EVT(1,-1);
}
EFL_END_TEST

EFL_START_TEST(delete_setted)
{
   Eo *r = radio();
   efl_ui_radio_group_register(radio_group, radio());
   efl_ui_radio_group_register(radio_group, radio());
   efl_ui_radio_group_register(radio_group, r);
   efl_ui_radio_group_register(radio_group, radio());
   RESET_EVT
   efl_ui_selectable_selected_set(r, EINA_TRUE);
   ck_assert_int_eq(efl_ui_radio_group_selected_value_get(radio_group), 1);
   ck_assert_ptr_eq(efl_ui_selectable_last_selected_get(radio_group), r);
   EXPECT_EVT(1,1);
   RESET_EVT
   efl_del(r);
   ck_assert_int_eq(efl_ui_radio_group_selected_value_get(radio_group), -1);
   ck_assert_ptr_eq(efl_ui_selectable_last_selected_get(radio_group), NULL);
   EXPECT_EVT(1,-1);
}
EFL_END_TEST

EFL_START_TEST(external_setting)
{
   Eo *r = radio();
   efl_ui_radio_group_register(radio_group, radio());
   efl_ui_radio_group_register(radio_group, radio());
   efl_ui_radio_group_register(radio_group, r);
   efl_ui_radio_group_register(radio_group, radio());
   RESET_EVT
   efl_ui_selectable_selected_set(r, EINA_TRUE);
   ck_assert_int_eq(efl_ui_radio_group_selected_value_get(radio_group), 1);
   ck_assert_ptr_eq(efl_ui_selectable_last_selected_get(radio_group), r);
   EXPECT_EVT(1,1);
   RESET_EVT
   efl_ui_selectable_selected_set(r, EINA_FALSE);
   ck_assert_int_eq(efl_ui_radio_group_selected_value_get(radio_group), -1);
   ck_assert_ptr_eq(efl_ui_selectable_last_selected_get(radio_group), NULL);
   EXPECT_EVT(1,-1);
}
EFL_END_TEST

EFL_START_TEST(pairwise_different_error)
{
   Eo *r = radio();
   Eo *r1 = radio();
   Eo *r2 = radio();
   efl_ui_radio_state_value_set(r, -1);
   efl_ui_radio_state_value_set(r1, 14);
   efl_ui_radio_state_value_set(r2, 14);
   EXPECT_ERROR_START;
   efl_ui_radio_group_register(radio_group, r);
   EXPECT_ERROR_END;
   efl_ui_radio_group_register(radio_group, r1);
   EXPECT_ERROR_START;
   efl_ui_radio_group_register(radio_group, r2);
   EXPECT_ERROR_END;
}
EFL_END_TEST

EFL_START_TEST(group_double_registeration)
{
   Eo *r = radio();
   efl_ui_radio_group_register(radio_group, r);
   EXPECT_ERROR_START;
   efl_ui_radio_group_register(radio_group, r);
   EXPECT_ERROR_END;
}
EFL_END_TEST

EFL_START_TEST(group_registeration_in_two_groups)
{
   Eo *radio_group2 = efl_new(EFL_UI_RADIO_GROUP_IMPL_CLASS, NULL);
   Eo *r = radio();
   efl_ui_radio_group_register(radio_group, r);
   EXPECT_ERROR_START;
   efl_ui_radio_group_register(radio_group2, r);
   EXPECT_ERROR_END;
}
EFL_END_TEST

EFL_START_TEST(group_invalid_value)
{
   Eo *radio_group2 = efl_new(EFL_UI_RADIO_GROUP_IMPL_CLASS, NULL);
   Eo *r = radio();
   efl_ui_radio_group_register(radio_group, r);
   EXPECT_ERROR_START;
   efl_ui_radio_group_selected_value_set(radio_group2, 1000);
   EXPECT_ERROR_END;
}
EFL_END_TEST

void efl_ui_test_radio_group(TCase *tc)
{
   tcase_add_checked_fixture(tc, fail_on_errors_setup, fail_on_errors_teardown);
   tcase_add_checked_fixture(tc, check_setup, check_teardown);
   tcase_add_test(tc, active_selected_value_setting);
   tcase_add_test(tc, active_selection_setting);
   tcase_add_test(tc, active_selection_object_setting);
   tcase_add_test(tc, pairwise_different_error);
   tcase_add_test(tc, group_double_registeration);
   tcase_add_test(tc, group_registeration_in_two_groups);
   tcase_add_test(tc, unregister_setted);
   tcase_add_test(tc, external_setting);
   tcase_add_test(tc, delete_setted);
   tcase_add_test(tc, group_invalid_value);
}
