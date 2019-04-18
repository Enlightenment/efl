#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#define EFL_ACCESS_OBJECT_BETA
#include <Elementary.h>
#include "elm_suite.h"

EFL_START_TEST(elm_web_legacy_type_check)
{
   Evas_Object *win, *web;
   const char *type;

   win = win_add(NULL, "web", ELM_WIN_BASIC);

   web = elm_web_add(win);

   type = elm_object_widget_type_get(web);
   ck_assert(type != NULL);
   ck_assert(!strcmp(type, "Elm_Web"));

   type = evas_object_type_get(web);
   ck_assert(type != NULL);
   ck_assert(!strcmp(type, "elm_web"));

}
EFL_END_TEST

EFL_START_TEST(elm_atspi_role_get)
{
   Evas_Object *win, *web;
   Efl_Access_Role role;

   win = win_add(NULL, "web", ELM_WIN_BASIC);

   web = elm_web_add(win);
   role = efl_access_object_role_get(web);

   ck_assert(role == EFL_ACCESS_ROLE_HTML_CONTAINER);

}
EFL_END_TEST

void elm_test_web(TCase *tc)
{
   tcase_add_test(tc, elm_web_legacy_type_check);
   tcase_add_test(tc, elm_atspi_role_get);
}
