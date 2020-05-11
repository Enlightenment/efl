#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include <Elementary.h>
#include "elm_entry_eo.h" //needed to check that spin is in text mode
#include <Efl_Ui.h>
#include "efl_ui_suite.h"

static Eo *win, *spin;

static void
spin_setup()
{
   win = win_add();

   spin = efl_add(EFL_UI_SPIN_BUTTON_CLASS, win);
   efl_content_set(win, spin);
   efl_gfx_entity_geometry_set(win, EINA_RECT(0, 0, 200, 50));
}

static void
_set_flag(void *data, const Efl_Event *ev)
{
   Eina_Bool *b = data;

   ck_assert_int_eq(*b, EINA_FALSE);
   *b = EINA_TRUE;
   ck_assert_ptr_eq(ev->info, NULL);
}

static void
_set_flag_quit(void *data, const Efl_Event *ev)
{
   Eina_Bool *b = data;

   ck_assert_int_eq(*b, EINA_FALSE);
   *b = EINA_TRUE;
   ck_assert_ptr_eq(ev->info, NULL);

   efl_loop_quit(efl_main_loop_get(), EINA_VALUE_EMPTY);
}

static void
click_spin_part(Eo *obj, const char *part)
{
   get_me_to_those_events(spin);
   click_part(obj, part);
}

EFL_START_TEST (spin_wheel_test)
{
   Eina_Bool changed = EINA_FALSE, min_reached = EINA_FALSE, max_reached = EINA_FALSE;

   efl_ui_range_limits_set(spin, -100.0, 100.0);
   efl_ui_range_value_set(spin, 0.0);
   efl_ui_range_step_set(spin, 10.0);
   efl_event_callback_add(spin, EFL_UI_RANGE_EVENT_CHANGED, _set_flag, &changed);
   efl_event_callback_add(spin, EFL_UI_RANGE_EVENT_MIN_REACHED, _set_flag, &min_reached);
   efl_event_callback_add(spin, EFL_UI_RANGE_EVENT_MAX_REACHED, _set_flag, &max_reached);

   efl_gfx_entity_size_set(win, EINA_SIZE2D(60, 60));
   get_me_to_those_events(spin);
   evas_event_feed_mouse_move(evas_object_evas_get(spin), 30, 30, 1234, NULL);
   evas_event_feed_mouse_wheel(evas_object_evas_get(spin), -1, -1, 12345, NULL);
   ck_assert(EINA_DBL_EQ(efl_ui_range_value_get(spin), 10.0));
   ck_assert_int_eq(changed, EINA_TRUE);
   ck_assert_int_eq(min_reached, EINA_FALSE);
   ck_assert_int_eq(max_reached, EINA_FALSE);
   changed = EINA_FALSE;
   min_reached = EINA_FALSE;
   max_reached = EINA_FALSE;

   evas_event_feed_mouse_wheel(evas_object_evas_get(spin), -1, 1, 12345, NULL);
   ck_assert(EINA_DBL_EQ(efl_ui_range_value_get(spin), 0.0));
   ck_assert_int_eq(changed, EINA_TRUE);
   ck_assert_int_eq(min_reached, EINA_FALSE);
   ck_assert_int_eq(max_reached, EINA_FALSE);
   changed = EINA_FALSE;
   min_reached = EINA_FALSE;
   max_reached = EINA_FALSE;
}
EFL_END_TEST

EFL_START_TEST (spin_value_inc)
{
   Eina_Bool changed = EINA_FALSE, min_reached = EINA_FALSE, max_reached = EINA_FALSE, delay_changed = EINA_FALSE;

   efl_ui_range_limits_set(spin, 0.0, 3.0);
   efl_ui_range_value_set(spin, 1.0);
   efl_event_callback_add(spin, EFL_UI_RANGE_EVENT_CHANGED, _set_flag, &changed);
   efl_event_callback_add(spin, EFL_UI_RANGE_EVENT_MIN_REACHED, _set_flag, &min_reached);
   efl_event_callback_add(spin, EFL_UI_RANGE_EVENT_MAX_REACHED, _set_flag, &max_reached);
   efl_event_callback_add(spin, EFL_UI_RANGE_EVENT_STEADY, _set_flag_quit, &delay_changed);

   click_spin_part(spin, "efl.inc_button");

   ck_assert_int_eq(changed, EINA_TRUE);
   ck_assert_int_eq(min_reached, EINA_FALSE);
   ck_assert_int_eq(max_reached, EINA_FALSE);
   changed = EINA_FALSE;
   min_reached = EINA_FALSE;
   max_reached = EINA_FALSE;

   efl_loop_begin(efl_main_loop_get());
   ck_assert_int_eq(delay_changed, EINA_TRUE);
}
EFL_END_TEST

EFL_START_TEST (spin_value_inc_max)
{
   Eina_Bool changed = EINA_FALSE, min_reached = EINA_FALSE, max_reached = EINA_FALSE, delay_changed = EINA_FALSE;

   efl_ui_range_limits_set(spin, 0.0, 3.0);
   efl_ui_range_value_set(spin, 2.0);
   efl_event_callback_add(spin, EFL_UI_RANGE_EVENT_CHANGED, _set_flag, &changed);
   efl_event_callback_add(spin, EFL_UI_RANGE_EVENT_MIN_REACHED, _set_flag, &min_reached);
   efl_event_callback_add(spin, EFL_UI_RANGE_EVENT_MAX_REACHED, _set_flag, &max_reached);
   efl_event_callback_add(spin, EFL_UI_RANGE_EVENT_STEADY, _set_flag_quit, &delay_changed);

   click_spin_part(spin, "efl.inc_button");

   ck_assert_int_eq(changed, EINA_TRUE);
   ck_assert_int_eq(min_reached, EINA_FALSE);
   ck_assert_int_eq(max_reached, EINA_TRUE);
   changed = EINA_FALSE;
   min_reached = EINA_FALSE;
   max_reached = EINA_FALSE;

   efl_loop_begin(efl_main_loop_get());
   ck_assert_int_eq(delay_changed, EINA_TRUE);
}
EFL_END_TEST

EFL_START_TEST (spin_value_dec_min)
{
   Eina_Bool changed = EINA_FALSE, min_reached = EINA_FALSE, max_reached = EINA_FALSE, delay_changed = EINA_FALSE;

   efl_ui_range_limits_set(spin, 0.0, 3.0);
   efl_ui_range_value_set(spin, 1.0);
   efl_event_callback_add(spin, EFL_UI_RANGE_EVENT_CHANGED, _set_flag, &changed);
   efl_event_callback_add(spin, EFL_UI_RANGE_EVENT_MIN_REACHED, _set_flag, &min_reached);
   efl_event_callback_add(spin, EFL_UI_RANGE_EVENT_MAX_REACHED, _set_flag, &max_reached);
   efl_event_callback_add(spin, EFL_UI_RANGE_EVENT_STEADY, _set_flag_quit, &delay_changed);

   click_spin_part(spin, "efl.dec_button");

   ck_assert_int_eq(changed, EINA_TRUE);
   ck_assert_int_eq(min_reached, EINA_TRUE);
   ck_assert_int_eq(max_reached, EINA_FALSE);
   changed = EINA_FALSE;
   min_reached = EINA_FALSE;
   max_reached = EINA_FALSE;

   efl_loop_begin(efl_main_loop_get());
   ck_assert_int_eq(delay_changed, EINA_TRUE);
}
EFL_END_TEST

EFL_START_TEST (spin_wraparound)
{
   efl_ui_spin_button_wraparound_set(spin, EINA_TRUE);
   efl_ui_range_limits_set(spin, 10, 30);
   efl_ui_range_step_set(spin, 20);

   efl_ui_range_value_set(spin, 20);
   click_spin_part(spin, "efl.inc_button");
   ck_assert_int_eq(efl_ui_range_value_get(spin), 10);

   efl_ui_range_value_set(spin, 20);
   click_spin_part(spin, "efl.dec_button");
   ck_assert_int_eq(efl_ui_range_value_get(spin), 30);

}
EFL_END_TEST

EFL_START_TEST (spin_double_values)
{
   double step = 0.1;
   efl_ui_range_limits_set(spin, 10, 30);
   efl_ui_range_value_set(spin, 20);
   efl_ui_range_step_set(spin, step);
   ck_assert(EINA_DBL_EQ(efl_ui_range_step_get(spin), step));
   get_me_to_those_events(spin);
   ck_assert(EINA_DBL_EQ(efl_ui_range_value_get(spin), 20.0));

   for (int i = 0; i < 5; ++i)
     {
        click_part(spin, "efl.inc_button");
        get_me_to_those_events(spin);
     }
   ck_assert_int_eq(EINA_DBL_EQ(efl_ui_range_value_get(spin), 20.5), 1);
}
EFL_END_TEST

EFL_START_TEST (spin_double_values_hitting_max_with_step)
{
   //idea is to check that spin button can hit max with inc, even if value is not multiple is 2.7
   efl_ui_range_limits_set(spin, 10, 30);
   efl_ui_range_value_set(spin, 27);
   efl_ui_range_step_set(spin, 2.7);
   get_me_to_those_events(spin);

   for (int i = 0; i < 2; ++i)
     {
        click_part(spin, "efl.inc_button");
        get_me_to_those_events(spin);
     }
   ck_assert_int_eq(EINA_DBL_EQ(efl_ui_range_value_get(spin), 30), 1);
}
EFL_END_TEST

static inline void
_try_direct_text_input(const char *text, double result)
{
   Eo *entry;

   efl_ui_focus_util_focus(efl_content_get(efl_part(spin, "efl.text_button")));
   get_me_to_those_events(spin);
   entry = efl_content_get(efl_part(spin, "efl.entry"));
   elm_object_text_set(entry, "");
   ck_assert_int_eq(efl_isa(entry, ELM_ENTRY_CLASS), 1);
   efl_ui_focus_util_focus(entry);
   get_me_to_those_events(spin);

   write_key_sequence(spin, text);
   get_me_to_those_events(spin);

   efl_ui_focus_util_focus(efl_content_get(efl_part(spin, "efl.inc_button")));
   ck_assert_msg(EINA_DBL_EQ(efl_ui_range_value_get(spin), result), "Values do not match %f - %f (%s)", efl_ui_range_value_get(spin), result, elm_object_text_get(entry));
}

EFL_START_TEST (spin_direct_text_input)
{
   efl_ui_spin_button_direct_text_input_set(spin, EINA_TRUE);
   efl_ui_range_limits_set(spin, -30, 30);
   efl_ui_range_value_set(spin, 20);
   get_me_to_those_events(spin);
   _try_direct_text_input("1asdf2", 12);
   _try_direct_text_input("1-2", 12);
   _try_direct_text_input("-12", -12);
   _try_direct_text_input("-100", -30);
   _try_direct_text_input("1.8", 18);
   _try_direct_text_input("12342435", 30);
}
EFL_END_TEST

EFL_START_TEST (spin_direct_text_input_comma_value)
{
   efl_ui_spin_button_direct_text_input_set(spin, EINA_TRUE);
   efl_ui_range_limits_set(spin, -30, 30);
   efl_ui_range_value_set(spin, 20);
   efl_ui_format_string_set(spin, "%.2f", EFL_UI_FORMAT_STRING_TYPE_SIMPLE);
   efl_ui_focus_util_focus(efl_content_get(efl_part(spin, "efl.text_button")));
   get_me_to_those_events(spin);
   _try_direct_text_input("1asdf2.1", 12.1);
   _try_direct_text_input("1-2.2", 12.2);
   _try_direct_text_input("-12.8", -12.8);
   _try_direct_text_input("-100", -30);
   _try_direct_text_input("10.8", 10.8);
   _try_direct_text_input("12342435.12312341342", 30);
}
EFL_END_TEST

void efl_ui_test_spin_button(TCase *tc)
{
   tcase_add_checked_fixture(tc, fail_on_errors_setup, fail_on_errors_teardown);
   tcase_add_checked_fixture(tc, spin_setup, NULL);
   tcase_add_test(tc, spin_wheel_test);
   tcase_add_test(tc, spin_value_inc);
   tcase_add_test(tc, spin_value_inc_max);
   tcase_add_test(tc, spin_value_dec_min);
   tcase_add_test(tc, spin_wraparound);
   tcase_add_test(tc, spin_double_values);
   tcase_add_test(tc, spin_double_values_hitting_max_with_step);
   tcase_add_test(tc, spin_direct_text_input);
   tcase_add_test(tc, spin_direct_text_input_comma_value);
}
