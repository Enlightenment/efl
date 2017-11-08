#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#define EFL_ACCESS_BETA
#define EFL_ACCESS_COMPONENT_PROTECTED
#define EFL_ACCESS_ACTION_PROTECTED
#include <Elementary.h>
#include "elm_suite.h"

START_TEST (elm_atspi_role_get)
{
   Evas_Object *win, *button;
   Efl_Access_Role role;

   elm_init(1, NULL);
   win = elm_win_add(NULL, "button", ELM_WIN_BASIC);

   button = elm_button_add(win);
   role = efl_access_role_get(button);

   ck_assert(role == EFL_ACCESS_ROLE_PUSH_BUTTON);

   elm_shutdown();
}
END_TEST

START_TEST (elm_atspi_interfaces_check)
{
   Evas_Object *win, *button;

   elm_init(1, NULL);
   win = elm_win_add(NULL, "button", ELM_WIN_BASIC);

   button = elm_button_add(win);

   ck_assert(efl_isa(button, EFL_ACCESS_MIXIN));
   ck_assert(efl_isa(button, EFL_ACCESS_COMPONENT_MIXIN));
   ck_assert(efl_isa(button, EFL_ACCESS_ACTION_MIXIN));

   elm_shutdown();
}
END_TEST

void elm_test_button(TCase *tc)
{
 tcase_add_test(tc, elm_atspi_role_get);
 tcase_add_test(tc, elm_atspi_interfaces_check);
}
