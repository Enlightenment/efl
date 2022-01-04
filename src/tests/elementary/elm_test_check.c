#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#define EFL_ACCESS_OBJECT_BETA
#include <Elementary.h>
#include "elm_suite.h"

typedef struct _Check_Data
{
   Evas_Object *check1;
   Evas_Object *check2;
   int value;
} Check_Data;

EFL_START_TEST(elm_test_check_size)
{
   Evas_Object *win, *check, *box;
   int width, height;

   win = elm_win_util_standard_add("check", "Check");

   box = elm_box_add(win);
   evas_object_size_hint_weight_set(box, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, box);
   evas_object_show(box);

   check = elm_check_add(box);
   elm_box_pack_end(box, check);
   evas_object_show(check);

   evas_object_show(win);

   get_me_to_those_events(check);

   evas_object_size_hint_min_get(check, &width, &height);
   ck_assert_int_gt(width, 0);
   ck_assert_int_gt(height, 0);
}
EFL_END_TEST

EFL_START_TEST(elm_test_check_legacy_type_check)
{
   Evas_Object *win, *check;
   const char *type;

   win = win_add(NULL, "check", ELM_WIN_BASIC);

   check = elm_check_add(win);

   type = elm_object_widget_type_get(check);
   ck_assert(type != NULL);
   ck_assert(!strcmp(type, "Elm_Check"));

   type = evas_object_type_get(check);
   ck_assert(type != NULL);
   ck_assert(!strcmp(type, "elm_check"));

}
EFL_END_TEST

EFL_START_TEST(elm_test_check_onoff_text)
{
   Evas_Object *win, *check;

   win = win_add(NULL, "check", ELM_WIN_BASIC);

   check = elm_check_add(win);
   elm_object_style_set(check, "toggle");
   elm_object_part_text_set(check, "on", "OnText");
   elm_object_part_text_set(check, "off", "OffText");

   ck_assert_str_eq(elm_object_part_text_get(check, "on"), "OnText");
   ck_assert_str_eq(elm_object_part_text_get(check, "off"), "OffText");

   elm_object_style_set(check, "default");
   DISABLE_ABORT_ON_CRITICAL_START;
   ck_assert(elm_object_part_text_get(check, "on") == NULL);
   ck_assert(elm_object_part_text_get(check, "off") == NULL);
   DISABLE_ABORT_ON_CRITICAL_END;

}
EFL_END_TEST

EFL_START_TEST(elm_test_check_callbacks)
{
   Evas_Object *win, *check;
   int called = 0;
   int i;

   win = win_add(NULL, "check", ELM_WIN_BASIC);
   evas_object_resize(win, 500, 500);

   check = elm_check_add(win);
// XXX: disable toggle tests and re-dun check tests
//   if (_i)
//     elm_object_style_set(check, "toggle");
   elm_object_text_set(check, "TEST TEST TEST");
   evas_object_smart_callback_add(check, "changed", event_callback_single_call_int_data, &called);

   evas_object_resize(check, 200, 100);
   evas_object_show(win);
   evas_object_show(check);
   get_me_to_those_events(check);

   for (i = 0; i < 4; i++)
     {
        called = 0;
// XXX: disable toggle tests and re-dun check tests
//        if (_i)
//          click_object_at(check, 150, 50);
//        else
          click_object(check);
        ecore_main_loop_iterate();
        ck_assert_int_eq(elm_check_state_get(check), !(i % 2));
        ck_assert_int_eq(called, 1);
     }
}
EFL_END_TEST

EFL_START_TEST(elm_test_check_state)
{
   Evas_Object *win, *check;
   Eina_Bool state = EINA_TRUE;
   int called = 0;

   win = win_add(NULL, "check", ELM_WIN_BASIC);

   check = elm_check_add(win);
   elm_check_state_pointer_set(check, &state);
   ck_assert(elm_check_state_get(check) == EINA_TRUE);
   ck_assert(state == EINA_TRUE);

   evas_object_smart_callback_add(check, "changed", event_callback_single_call_int_data, &called);
   elm_check_state_set(check, EINA_FALSE);
   ck_assert(elm_check_state_get(check) == EINA_FALSE);
   ck_assert(state == EINA_FALSE);
   ck_assert_int_eq(called, 0);

}
EFL_END_TEST

EFL_START_TEST(elm_atspi_role_get)
{
   Evas_Object *win, *check;
   Efl_Access_Role role;

   win = win_add(NULL, "check", ELM_WIN_BASIC);

   check = elm_check_add(win);
   role = efl_access_object_role_get(check);

   ck_assert(role == EFL_ACCESS_ROLE_CHECK_BOX);

}
EFL_END_TEST

static void
_check_changed_cb(void *ptr, Evas_Object *obj, void *e EINA_UNUSED)
{
   Check_Data *data = (Check_Data*) ptr;

   if (obj == data->check1) {
      elm_check_state_set(data->check2, EINA_TRUE);
   } else if ( obj == data->check2) {
      elm_check_state_set(data->check1, EINA_FALSE);
   } else {
      ck_assert(EINA_FALSE);
   }
   data->value ++;
}

EFL_START_TEST(elm_test_check_api_call)
{
   Evas_Object *win, *check1, *check2, *box;
   Check_Data data ={0,};

   win = elm_win_util_standard_add("check", "Check");

   box = elm_box_add(win);
   evas_object_size_hint_weight_set(box, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, box);
   evas_object_show(box);

   data.check1 = check1 = elm_check_add(box);
   elm_object_text_set(check1, "TEST TEST TEST");
   evas_object_smart_callback_add(check1, "changed", _check_changed_cb, &data);
   elm_box_pack_end(box, check1);
   evas_object_show(check1);

   data.check2 = check2 = elm_check_add(box);
   elm_object_text_set(check2, "TEST TEST TEST");
   evas_object_smart_callback_add(check2, "changed", _check_changed_cb, &data);
   elm_box_pack_end(box, check2);
   evas_object_show(check2);

   evas_object_show(win);

   get_me_to_those_events(check1);

   data.value = 0;
   click_object(data.check1);
   ecore_main_loop_iterate();
   ck_assert_int_eq(data.value, 1);

   data.value = 0;
   click_object(data.check2);
   ecore_main_loop_iterate();
   ck_assert_int_eq(data.value, 1);

   data.value = 0;
   click_object(data.check1);
   ecore_main_loop_iterate();
   ck_assert_int_eq(data.value, 1);
}
EFL_END_TEST

void elm_test_check(TCase *tc)
{
   tcase_add_test(tc, elm_test_check_size);
   tcase_add_test(tc, elm_test_check_legacy_type_check);
   tcase_add_test(tc, elm_test_check_onoff_text);
   tcase_add_test(tc, elm_test_check_state);
   tcase_add_loop_test(tc, elm_test_check_callbacks, 0, 2);
   tcase_add_test(tc, elm_atspi_role_get);
   tcase_add_test(tc, elm_test_check_api_call);
}
