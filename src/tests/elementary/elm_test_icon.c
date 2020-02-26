#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#define EFL_ACCESS_OBJECT_BETA
#include <Elementary.h>
#include "elm_suite.h"

EFL_START_TEST(elm_icon_legacy_type_check)
{
   Evas_Object *win, *icon;
   const char *type;

   win = win_add(NULL, "icon", ELM_WIN_BASIC);

   icon = elm_icon_add(win);

   type = elm_object_widget_type_get(icon);
   ck_assert(type != NULL);
   ck_assert(!strcmp(type, "Elm_Icon"));

   type = evas_object_type_get(icon);
   ck_assert(type != NULL);
   ck_assert(!strcmp(type, "elm_icon"));

}
EFL_END_TEST

EFL_START_TEST(elm_atspi_role_get)
{
   Evas_Object *win, *icon;
   Efl_Access_Role role;

   win = win_add(NULL, "icon", ELM_WIN_BASIC);

   icon = elm_icon_add(win);
   role = efl_access_object_role_get(icon);

   ck_assert(role == EFL_ACCESS_ROLE_ICON);

}
EFL_END_TEST

EFL_START_TEST(elm_test_icon_set)
{
   Evas_Object *win, *image;
   Eina_Bool ok;
   const char *icon_name;

   win = win_add(NULL, "icon", ELM_WIN_BASIC);

   image = elm_icon_add(win);
   evas_object_show(image);

   ok = elm_icon_standard_set(image, "folder");
   ck_assert(ok);
   icon_name = elm_icon_standard_get(image);
   ck_assert_str_eq(icon_name, "folder");

   ok = elm_icon_standard_set(image, "None");
   ck_assert(ok == 0);
   icon_name = elm_icon_standard_get(image);
   /* elm_icon only changes internal name on success */
   ck_assert_str_eq(icon_name, "folder");

}
EFL_END_TEST

void elm_test_icon(TCase *tc)
{
   tcase_add_test(tc, elm_icon_legacy_type_check);
   tcase_add_test(tc, elm_atspi_role_get);
   tcase_add_test(tc, elm_test_icon_set);
}
