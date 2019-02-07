#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#define EFL_ACCESS_OBJECT_BETA
#include <Elementary.h>
#include "elm_suite.h"

EFL_START_TEST(elm_check_legacy_type_check)
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

EFL_START_TEST(elm_check_onoff_text)
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
   ck_assert(elm_object_part_text_get(check, "on") == NULL);
   ck_assert(elm_object_part_text_get(check, "off") == NULL);

}
EFL_END_TEST

EFL_START_TEST(elm_check_state)
{
   Evas_Object *win, *check;
   Eina_Bool state = EINA_TRUE;

   win = win_add(NULL, "check", ELM_WIN_BASIC);

   check = elm_check_add(win);
   elm_check_state_pointer_set(check, &state);
   ck_assert(elm_check_state_get(check) == EINA_TRUE);
   ck_assert(state == EINA_TRUE);

   elm_check_state_set(check, EINA_FALSE);
   ck_assert(elm_check_state_get(check) == EINA_FALSE);
   ck_assert(state == EINA_FALSE);

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

void elm_test_check(TCase *tc)
{
   tcase_add_test(tc, elm_check_legacy_type_check);
   tcase_add_test(tc, elm_check_onoff_text);
   tcase_add_test(tc, elm_check_state);
   tcase_add_test(tc, elm_atspi_role_get);
}
