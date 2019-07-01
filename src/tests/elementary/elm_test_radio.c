#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#define EFL_ACCESS_OBJECT_BETA
#include <Elementary.h>
#include "elm_suite.h"

EFL_START_TEST(elm_radio_legacy_type_check)
{
   Evas_Object *win, *radio;
   const char *type;

   win = win_add(NULL, "radio", ELM_WIN_BASIC);

   radio = elm_radio_add(win);

   type = elm_object_widget_type_get(radio);
   ck_assert(type != NULL);
   ck_assert(!strcmp(type, "Elm_Radio"));

   type = evas_object_type_get(radio);
   ck_assert(type != NULL);
   ck_assert(!strcmp(type, "elm_radio"));

}
EFL_END_TEST

EFL_START_TEST(elm_atspi_role_get)
{
   Evas_Object *win, *radio;
   Efl_Access_Role role;

   win = win_add(NULL, "radio", ELM_WIN_BASIC);

   radio = elm_radio_add(win);
   role = efl_access_object_role_get(radio);

   ck_assert(role == EFL_ACCESS_ROLE_RADIO_BUTTON);

}
EFL_END_TEST

static unsigned int radio_change_count = 0;

static void
_rdg_changed_cb()
{
   radio_change_count++;
   ecore_main_loop_quit();
}

EFL_START_TEST(elm_test_radio_selection)
{
   radio_change_count = 0;
   Evas_Object *rd, *rdg;
   Evas_Object *win = win_add(NULL, "radio", ELM_WIN_BASIC);
   evas_object_resize(win, 100, 100);

   // radio 1
   rd = elm_radio_add(win);
   evas_object_resize(rd, 100, 100);

   // rdg radio group
   rdg = rd;
   evas_object_smart_callback_add(rdg, "changed", _rdg_changed_cb, NULL);

   elm_radio_state_value_set(rd, 1);
   elm_object_text_set(rd, "Radio Group Test #1");
   evas_object_show(rd);


   // radio 2
   rd = elm_radio_add(win);
   evas_object_smart_callback_add(rd, "changed", _rdg_changed_cb, NULL);
   evas_object_resize(rd, 100, 100);
   elm_radio_state_value_set(rd, 2);
   elm_object_text_set(rd, "Radio Group Test #2");
   elm_radio_group_add(rd, rdg);
   evas_object_show(rd);
   evas_object_show(win);

   elm_layout_signal_emit(rdg, "elm,action,radio,toggle", "elm");
   ecore_main_loop_begin();
   ck_assert_int_eq(elm_radio_value_get(rdg), 1);
   ck_assert_int_eq(radio_change_count, 1);

   elm_layout_signal_emit(rd, "elm,action,radio,toggle", "elm");
   ecore_main_loop_begin();
   ck_assert_int_eq(elm_radio_value_get(rdg), 2);
   ck_assert_int_eq(radio_change_count, 2);

   elm_layout_signal_emit(rdg, "elm,action,radio,toggle", "elm");
   ecore_main_loop_begin();
   ck_assert_int_eq(elm_radio_value_get(rdg), 1);
   ck_assert_int_eq(radio_change_count, 3);
}
EFL_END_TEST

EFL_START_TEST(elm_test_radio_callback)
{
   radio_change_count = 0;
   Evas_Object *rd, *rdg;
   Evas_Object *win = win_add(NULL, "radio", ELM_WIN_BASIC);
   evas_object_resize(win, 100, 100);

   // radio 1
   rd = elm_radio_add(win);
   evas_object_resize(rd, 100, 100);

   // rdg radio group
   rdg = rd;
   evas_object_smart_callback_add(rdg, "changed", _rdg_changed_cb, NULL);

   elm_radio_state_value_set(rd, 1);
   elm_object_text_set(rd, "Radio Group Test #1");
   evas_object_show(rd);


   // radio 2
   rd = elm_radio_add(win);
   evas_object_resize(rd, 100, 100);
   elm_radio_state_value_set(rd, 2);
   elm_object_text_set(rd, "Radio Group Test #2");
   elm_radio_group_add(rd, rdg);
   evas_object_show(rd);
   evas_object_show(win);

   elm_radio_value_set(rdg, 2);
   elm_radio_value_set(rdg, 1);
   elm_radio_value_set(rdg, 2);
   ck_assert_int_eq(radio_change_count, 0);
}
EFL_END_TEST

void elm_test_radio(TCase *tc)
{
   tcase_add_test(tc, elm_radio_legacy_type_check);
   tcase_add_test(tc, elm_atspi_role_get);
   tcase_add_test(tc, elm_test_radio_selection);
   tcase_add_test(tc, elm_test_radio_callback);
}
