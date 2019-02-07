#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#define EFL_ACCESS_OBJECT_BETA
#include <Elementary.h>
#include "elm_suite.h"

EFL_START_TEST(elm_multibuttonentry_legacy_type_check)
{
   Evas_Object *win, *multibuttonentry;
   const char *type;

   win = win_add(NULL, "multibuttonentry", ELM_WIN_BASIC);

   multibuttonentry = elm_multibuttonentry_add(win);

   type = elm_object_widget_type_get(multibuttonentry);
   ck_assert(type != NULL);
   ck_assert_str_eq(type, "Elm_Multibuttonentry");

   type = evas_object_type_get(multibuttonentry);
   ck_assert(type != NULL);
   ck_assert_str_eq(type, "elm_multibuttonentry");

}
EFL_END_TEST

EFL_START_TEST(elm_atspi_role_get)
{
   Evas_Object *win, *multibuttonentry;
   Efl_Access_Role role;

   win = win_add(NULL, "multibuttonentry", ELM_WIN_BASIC);

   multibuttonentry = elm_multibuttonentry_add(win);
   role = efl_access_object_role_get(multibuttonentry);

   ck_assert_int_eq(role, EFL_ACCESS_ROLE_PANEL);

}
EFL_END_TEST

void elm_test_multibuttonentry(TCase *tc)
{
   tcase_add_test(tc, elm_multibuttonentry_legacy_type_check);
   tcase_add_test(tc, elm_atspi_role_get);
}
