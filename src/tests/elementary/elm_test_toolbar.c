#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#define EFL_ACCESS_OBJECT_BETA
#include <Elementary.h>
#include "elm_suite.h"

EFL_START_TEST(elm_toolbar_legacy_type_check)
{
   Evas_Object *win, *toolbar;
   const char *type;

   win = win_add(NULL, "toolbar", ELM_WIN_BASIC);

   toolbar = elm_toolbar_add(win);

   type = elm_object_widget_type_get(toolbar);
   ck_assert(type != NULL);
   ck_assert(!strcmp(type, "Elm_Toolbar"));

   type = evas_object_type_get(toolbar);
   ck_assert(type != NULL);
   ck_assert(!strcmp(type, "elm_toolbar"));

}
EFL_END_TEST

EFL_START_TEST(elm_atspi_role_get)
{
   Evas_Object *win, *toolbar;
   Efl_Access_Role role;

   win = win_add(NULL, "toolbar", ELM_WIN_BASIC);

   toolbar = elm_toolbar_add(win);
   role = efl_access_object_role_get(toolbar);

   ck_assert(role == EFL_ACCESS_ROLE_TOOL_BAR);

}
EFL_END_TEST

void elm_test_toolbar(TCase *tc)
{
   tcase_add_test(tc, elm_toolbar_legacy_type_check);
   tcase_add_test(tc, elm_atspi_role_get);
}
