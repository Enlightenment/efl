#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include <Elementary.h>
#include "elm_suite.h"

#define ELM_INTERFACE_ATSPI_ACCESSIBLE_PROTECTED
#include "elm_interface_atspi_accessible.h"
#include "elm_interface_atspi_accessible.eo.h"

START_TEST (elm_check_onoff_text)
{
   Evas_Object *win, *check;

   elm_init(1, NULL);
   win = elm_win_add(NULL, "check", ELM_WIN_BASIC);

   check = elm_check_add(win);
   elm_object_style_set(check, "toggle");
   elm_object_part_text_set(check, "on", "OnText");
   elm_object_part_text_set(check, "off", "OffText");

   ck_assert_str_eq(elm_object_part_text_get(check, "on"), "OnText");
   ck_assert_str_eq(elm_object_part_text_get(check, "off"), "OffText");

   elm_object_style_set(check, "default");
   ck_assert(elm_object_part_text_get(check, "on") == NULL);
   ck_assert(elm_object_part_text_get(check, "off") == NULL);

   elm_shutdown();
}
END_TEST

START_TEST (elm_check_state)
{
   Evas_Object *win, *check;
   Eina_Bool state = EINA_TRUE;

   elm_init(1, NULL);
   win = elm_win_add(NULL, "check", ELM_WIN_BASIC);

   check = elm_check_add(win);
   elm_check_state_pointer_set(check, &state);
   ck_assert(elm_check_state_get(check) == EINA_TRUE);
   ck_assert(state == EINA_TRUE);

   elm_check_state_set(check, EINA_FALSE);
   ck_assert(elm_check_state_get(check) == EINA_FALSE);
   ck_assert(state == EINA_FALSE);

   elm_shutdown();
}
END_TEST

START_TEST (elm_atspi_role_get)
{
   Evas_Object *win, *check;
   Elm_Atspi_Role role;

   elm_init(1, NULL);
   win = elm_win_add(NULL, "check", ELM_WIN_BASIC);

   check = elm_check_add(win);
   eo_do(check, role = elm_interface_atspi_accessible_role_get());

   ck_assert(role == ELM_ATSPI_ROLE_CHECK_BOX);

   elm_shutdown();
}
END_TEST

void elm_test_check(TCase *tc)
{
   tcase_add_test(tc, elm_check_onoff_text);
   tcase_add_test(tc, elm_check_state);
   tcase_add_test(tc, elm_atspi_role_get);
}
