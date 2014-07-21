#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include <Elementary.h>
#include "elm_suite.h"

#define ELM_INTERFACE_ATSPI_ACCESSIBLE_PROTECTED
#include "elm_interface_atspi_accessible.h"
#include "elm_interface_atspi_accessible.eo.h"

#define ELM_INTERFACE_ATSPI_COMPONENT_PROTECTED
#include "elm_interface_atspi_component.eo.h"

#define ELM_INTERFACE_ATSPI_ACTION_PROTECTED
#include "elm_interface_atspi_action.eo.h"


START_TEST (elm_atspi_role_get)
{
   Evas_Object *win, *button;
   Elm_Atspi_Role role;

   elm_init(1, NULL);
   win = elm_win_add(NULL, "button", ELM_WIN_BASIC);

   button = elm_button_add(win);
   eo_do(button, role = elm_interface_atspi_accessible_role_get());

   ck_assert(role == ELM_ATSPI_ROLE_PUSH_BUTTON);

   elm_shutdown();
}
END_TEST

START_TEST (elm_atspi_interfaces_check)
{
   Evas_Object *win, *button;

   elm_init(1, NULL);
   win = elm_win_add(NULL, "button", ELM_WIN_BASIC);

   button = elm_button_add(win);

   ck_assert(eo_isa(button, ELM_INTERFACE_ATSPI_ACCESSIBLE_MIXIN));
   ck_assert(eo_isa(button, ELM_INTERFACE_ATSPI_COMPONENT_MIXIN));
   ck_assert(eo_isa(button, ELM_INTERFACE_ATSPI_ACTION_MIXIN));

   elm_shutdown();
}
END_TEST

void elm_test_button(TCase *tc)
{
 tcase_add_test(tc, elm_atspi_role_get);
 tcase_add_test(tc, elm_atspi_interfaces_check);
}
