#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#define EFL_ACCESS_OBJECT_BETA
#include <Elementary.h>
#include "elm_suite.h"

EFL_START_TEST(elm_clock_legacy_type_check)
{
   Evas_Object *win, *clock;
   const char *type;

   win = win_add(NULL, "clock", ELM_WIN_BASIC);

   clock = elm_clock_add(win);

   type = elm_object_widget_type_get(clock);
   ck_assert(type != NULL);
   ck_assert(!strcmp(type, "Elm_Clock"));

   type = evas_object_type_get(clock);
   ck_assert(type != NULL);
   ck_assert(!strcmp(type, "elm_clock"));

}
EFL_END_TEST

EFL_START_TEST(elm_atspi_role_get)
{
   Evas_Object *win, *clk;
   Efl_Access_Role role;

   win = win_add(NULL, "clock", ELM_WIN_BASIC);

   clk = elm_clock_add(win);
   role = efl_access_object_role_get(clk);

   ck_assert(role == EFL_ACCESS_ROLE_TEXT);

}
EFL_END_TEST

void elm_test_clock(TCase *tc)
{
   tcase_add_test(tc, elm_clock_legacy_type_check);
   tcase_add_test(tc, elm_atspi_role_get);
}

