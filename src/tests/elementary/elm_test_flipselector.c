#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#define EFL_ACCESS_BETA
#include <Elementary.h>
#include "elm_suite.h"

EFL_START_TEST (elm_flipselector_legacy_type_check)
{
   Evas_Object *win, *flipselector;
   const char *type;

   char *args[] = { "exe" };
   elm_init(1, args);
   win = elm_win_add(NULL, "flipselector", ELM_WIN_BASIC);

   flipselector = elm_flipselector_add(win);

   type = elm_object_widget_type_get(flipselector);
   ck_assert(type != NULL);
   ck_assert(!strcmp(type, "Elm_Flipselector"));

   type = evas_object_type_get(flipselector);
   ck_assert(type != NULL);
   ck_assert(!strcmp(type, "elm_flipselector"));

   elm_shutdown();
}
EFL_END_TEST

EFL_START_TEST (elm_atspi_role_get)
{
   Evas_Object *win, *flipselector;
   Efl_Access_Role role;

   char *args[] = { "exe" };
   elm_init(1, args);
   win = elm_win_add(NULL, "flipselector", ELM_WIN_BASIC);

   flipselector = elm_flipselector_add(win);
   role = efl_access_role_get(flipselector);

   ck_assert(role == EFL_ACCESS_ROLE_LIST);

   elm_shutdown();
}
EFL_END_TEST

void elm_test_flipselector(TCase *tc)
{
   tcase_add_test(tc, elm_flipselector_legacy_type_check);
   tcase_add_test(tc, elm_atspi_role_get);
}
