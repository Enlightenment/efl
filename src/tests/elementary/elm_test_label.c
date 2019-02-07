#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#define EFL_ACCESS_OBJECT_BETA
#include <Elementary.h>
#include "elm_suite.h"

EFL_START_TEST(elm_label_legacy_type_check)
{
   Evas_Object *win, *label;
   const char *type;

   win = win_add(NULL, "label", ELM_WIN_BASIC);

   label = elm_label_add(win);

   type = elm_object_widget_type_get(label);
   ck_assert(type != NULL);
   ck_assert(!strcmp(type, "Elm_Label"));

   type = evas_object_type_get(label);
   ck_assert(type != NULL);
   ck_assert(!strcmp(type, "elm_label"));

}
EFL_END_TEST

EFL_START_TEST(elm_atspi_role_get)
{
   Evas_Object *win, *label;
   Efl_Access_Role role;

   win = win_add(NULL, "label", ELM_WIN_BASIC);

   label = elm_label_add(win);
   role = efl_access_object_role_get(label);

   ck_assert(role == EFL_ACCESS_ROLE_LABEL);

}
EFL_END_TEST

void elm_test_label(TCase *tc)
{
   tcase_add_test(tc, elm_label_legacy_type_check);
   tcase_add_test(tc, elm_atspi_role_get);
}
