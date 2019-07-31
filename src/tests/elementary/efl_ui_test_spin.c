#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include <Efl_Ui.h>
#include "efl_ui_suite.h"

static Eo *win, *spin;

static void
spin_setup()
{
   win = win_add();

   spin = efl_add(EFL_UI_SPIN_CLASS, win);
   efl_content_set(win, spin);
}

static void
_set_flag(void *data, const Efl_Event *ev)
{
   Eina_Bool *b = data;

   ck_assert_int_eq(*b, EINA_FALSE);

   *b = EINA_TRUE;
   ck_assert_ptr_eq(ev->info, NULL);
}

EFL_START_TEST (spin_value_events)
{
   Eina_Bool changed = EINA_FALSE, min_reached = EINA_FALSE, max_reached = EINA_FALSE;

   efl_ui_range_limits_set(spin, -3.0, 3.0);
   efl_ui_range_value_set(spin, 0.0);
   efl_event_callback_add(spin, EFL_UI_RANGE_EVENT_CHANGED, _set_flag, &changed);
   efl_event_callback_add(spin, EFL_UI_RANGE_EVENT_MIN_REACHED, _set_flag, &min_reached);
   efl_event_callback_add(spin, EFL_UI_RANGE_EVENT_MAX_REACHED, _set_flag, &max_reached);

   efl_ui_range_value_set(spin, 1.0);
   ck_assert_int_eq(changed, EINA_TRUE);
   ck_assert_int_eq(min_reached, EINA_FALSE);
   ck_assert_int_eq(max_reached, EINA_FALSE);
   changed = EINA_FALSE;
   min_reached = EINA_FALSE;
   max_reached = EINA_FALSE;

   efl_ui_range_value_set(spin, 3.0);
   ck_assert_int_eq(changed, EINA_TRUE);
   ck_assert_int_eq(min_reached, EINA_FALSE);
   ck_assert_int_eq(max_reached, EINA_TRUE);
   changed = EINA_FALSE;
   min_reached = EINA_FALSE;
   max_reached = EINA_FALSE;

   efl_ui_range_value_set(spin, -3.0);
   ck_assert_int_eq(changed, EINA_TRUE);
   ck_assert_int_eq(min_reached, EINA_TRUE);
   ck_assert_int_eq(max_reached, EINA_FALSE);
}
EFL_END_TEST

void efl_ui_test_spin(TCase *tc)
{
   tcase_add_checked_fixture(tc, fail_on_errors_setup, fail_on_errors_teardown);
   tcase_add_checked_fixture(tc, spin_setup, NULL);
   tcase_add_test(tc, spin_value_events);
}
