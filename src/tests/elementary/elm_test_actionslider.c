#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#define EFL_ACCESS_OBJECT_BETA
#include <Elementary.h>
#include "elm_suite.h"

EFL_START_TEST (elm_actionslider_legacy_type_check)
{
   Evas_Object *win, *actionslider;
   const char *type;

   win = win_add(NULL, "actionslider", ELM_WIN_BASIC);

   actionslider = elm_actionslider_add(win);

   type = elm_object_widget_type_get(actionslider);
   ck_assert(type != NULL);
   ck_assert(!strcmp(type, "Elm_Actionslider"));

   type = evas_object_type_get(actionslider);
   ck_assert(type != NULL);
   ck_assert(!strcmp(type, "elm_actionslider"));

}
EFL_END_TEST

EFL_START_TEST (elm_atspi_role_get)
{
   Evas_Object *win, *actionslider;
   Efl_Access_Role role;

   win = win_add(NULL, "actionslider", ELM_WIN_BASIC);

   actionslider = elm_actionslider_add(win);
   role = efl_access_object_role_get(actionslider);

   ck_assert(role == EFL_ACCESS_ROLE_SLIDER);

}
EFL_END_TEST

void elm_test_actionslider(TCase *tc)
{
   tcase_add_test(tc, elm_actionslider_legacy_type_check);
   tcase_add_test(tc, elm_atspi_role_get);
}
