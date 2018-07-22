#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#define EFL_ACCESS_OBJECT_BETA
#include <Elementary.h>
#include "elm_suite.h"

EFL_START_TEST (elm_flip_legacy_type_check)
{
   Evas_Object *win, *flip;
   const char *type;

   win = win_add(NULL, "flip", ELM_WIN_BASIC);

   flip = elm_flip_add(win);

   type = elm_object_widget_type_get(flip);
   ck_assert(type != NULL);
   ck_assert(!strcmp(type, "Elm_Flip"));

   type = evas_object_type_get(flip);
   ck_assert(type != NULL);
   ck_assert(!strcmp(type, "elm_flip"));

}
EFL_END_TEST

EFL_START_TEST (elm_atspi_role_get)
{
   Evas_Object *win, *flip;
   Efl_Access_Role role;

   win = win_add(NULL, "flip", ELM_WIN_BASIC);

   flip = elm_flip_add(win);
   role = efl_access_object_role_get(flip);

   ck_assert(role == EFL_ACCESS_ROLE_PAGE_TAB_LIST);

}
EFL_END_TEST

void elm_test_flip(TCase *tc)
{
   tcase_add_test(tc, elm_flip_legacy_type_check);
   tcase_add_test(tc, elm_atspi_role_get);
}
