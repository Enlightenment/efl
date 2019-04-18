#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#define EFL_ACCESS_OBJECT_BETA
#define EFL_ACCESS_COMPONENT_PROTECTED
#define EFL_ACCESS_ACTION_PROTECTED
#include <Elementary.h>
#include "elm_suite.h"

EFL_START_TEST(elm_button_legacy_type_check)
{
   Evas_Object *win, *button;
   const char *type;

   win = win_add(NULL, "button", ELM_WIN_BASIC);

   button = elm_button_add(win);

   type = elm_object_widget_type_get(button);
   ck_assert(type != NULL);
   ck_assert(!strcmp(type, "Elm_Button"));

   type = evas_object_type_get(button);
   ck_assert(type != NULL);
   ck_assert(!strcmp(type, "elm_button"));

}
EFL_END_TEST

EFL_START_TEST(elm_atspi_role_get)
{
   Evas_Object *win, *button;
   Efl_Access_Role role;

   win = win_add(NULL, "button", ELM_WIN_BASIC);

   button = elm_button_add(win);
   role = efl_access_object_role_get(button);

   ck_assert(role == EFL_ACCESS_ROLE_PUSH_BUTTON);

}
EFL_END_TEST

EFL_START_TEST(elm_atspi_interfaces_check)
{
   Evas_Object *win, *button;

   win = win_add(NULL, "button", ELM_WIN_BASIC);

   button = elm_button_add(win);

   ck_assert(efl_isa(button, EFL_ACCESS_OBJECT_MIXIN));
   ck_assert(efl_isa(button, EFL_ACCESS_COMPONENT_MIXIN));
   ck_assert(efl_isa(button, EFL_ACCESS_ACTION_MIXIN));

}
EFL_END_TEST

void elm_test_button(TCase *tc)
{
   tcase_add_test(tc, elm_button_legacy_type_check);
   tcase_add_test(tc, elm_atspi_role_get);
   tcase_add_test(tc, elm_atspi_interfaces_check);
}
