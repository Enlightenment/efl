#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#define EFL_ACCESS_BETA
#include <Elementary.h>
#include "elm_suite.h"

START_TEST (elm_radio_legacy_type_check)
{
   Evas_Object *win, *radio;
   const char *type;

   char *args[] = { "exe" };
   elm_init(1, args);
   win = elm_win_add(NULL, "radio", ELM_WIN_BASIC);

   radio = elm_radio_add(win);

   type = elm_object_widget_type_get(radio);
   ck_assert(type != NULL);
   ck_assert(!strcmp(type, "Elm_Radio"));

   type = evas_object_type_get(radio);
   ck_assert(type != NULL);
   ck_assert(!strcmp(type, "elm_radio"));

   elm_shutdown();
}
END_TEST

START_TEST (elm_atspi_role_get)
{
   Evas_Object *win, *radio;
   Efl_Access_Role role;

   char *args[] = { "exe" };
   elm_init(1, args);
   win = elm_win_add(NULL, "radio", ELM_WIN_BASIC);

   radio = elm_radio_add(win);
   role = efl_access_role_get(radio);

   ck_assert(role == EFL_ACCESS_ROLE_RADIO_BUTTON);

   elm_shutdown();
}
END_TEST

void elm_test_radio(TCase *tc)
{
   tcase_add_test(tc, elm_radio_legacy_type_check);
   tcase_add_test(tc, elm_atspi_role_get);
}
