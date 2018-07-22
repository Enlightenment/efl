#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#define EFL_ACCESS_OBJECT_BETA
#include <Elementary.h>
#include "elm_suite.h"

EFL_START_TEST (elm_panes_legacy_type_check)
{
   Evas_Object *win, *panes;
   const char *type;

   win = win_add(NULL, "panes", ELM_WIN_BASIC);

   panes = elm_panes_add(win);

   type = elm_object_widget_type_get(panes);
   ck_assert(type != NULL);
   ck_assert(!strcmp(type, "Elm_Panes"));

   type = evas_object_type_get(panes);
   ck_assert(type != NULL);
   ck_assert(!strcmp(type, "elm_panes"));

}
EFL_END_TEST

EFL_START_TEST (elm_atspi_role_get)
{
   Evas_Object *win, *panes;
   Efl_Access_Role role;

   win = win_add(NULL, "panes", ELM_WIN_BASIC);

   panes = elm_panes_add(win);
   role = efl_access_object_role_get(panes);

   ck_assert(role == EFL_ACCESS_ROLE_SPLIT_PANE);

}
EFL_END_TEST

void elm_test_panes(TCase *tc)
{
   tcase_add_test(tc, elm_panes_legacy_type_check);
   tcase_add_test(tc, elm_atspi_role_get);
}
