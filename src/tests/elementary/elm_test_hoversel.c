#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#define EFL_ACCESS_OBJECT_BETA
#include <Elementary.h>
#include "elm_suite.h"

EFL_START_TEST (elm_hoversel_legacy_type_check)
{
   Evas_Object *win, *hoversel;
   const char *type;

   win = win_add(NULL, "hoversel", ELM_WIN_BASIC);

   hoversel = elm_hoversel_add(win);

   type = elm_object_widget_type_get(hoversel);
   ck_assert(type != NULL);
   ck_assert(!strcmp(type, "Elm_Hoversel"));

   type = evas_object_type_get(hoversel);
   ck_assert(type != NULL);
   ck_assert(!strcmp(type, "elm_hoversel"));

}
EFL_END_TEST

EFL_START_TEST (elm_atspi_role_get)
{
   Evas_Object *win, *hoversel;
   Efl_Access_Role role;

   win = win_add(NULL, "hoversel", ELM_WIN_BASIC);

   hoversel = elm_hoversel_add(win);
   role = efl_access_object_role_get(hoversel);

   ck_assert(role == EFL_ACCESS_ROLE_PUSH_BUTTON);

}
EFL_END_TEST

void elm_test_hoversel(TCase *tc)
{
   tcase_add_test(tc, elm_hoversel_legacy_type_check);
   tcase_add_test(tc, elm_atspi_role_get);
}
