#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#define EFL_ACCESS_BETA
#include <Elementary.h>
#include "elm_suite.h"

EFL_START_TEST (elm_hover_legacy_type_check)
{
   Evas_Object *win, *hover;
   const char *type;

   char *args[] = { "exe" };
   elm_init(1, args);
   win = elm_win_add(NULL, "hover", ELM_WIN_BASIC);

   hover = elm_hover_add(win);

   type = elm_object_widget_type_get(hover);
   ck_assert(type != NULL);
   ck_assert(!strcmp(type, "Elm_Hover"));

   type = evas_object_type_get(hover);
   ck_assert(type != NULL);
   ck_assert(!strcmp(type, "elm_hover"));

   elm_shutdown();
}
EFL_END_TEST

EFL_START_TEST (elm_atspi_role_get)
{
   Evas_Object *win, *hover;
   Efl_Access_Role role;

   char *args[] = { "exe" };
   elm_init(1, args);
   win = elm_win_add(NULL, "hover", ELM_WIN_BASIC);

   hover = elm_hover_add(win);
   role = efl_access_role_get(hover);

   ck_assert(role == EFL_ACCESS_ROLE_POPUP_MENU);

   elm_shutdown();
}
EFL_END_TEST

void elm_test_hover(TCase *tc)
{
   tcase_add_test(tc, elm_hover_legacy_type_check);
   tcase_add_test(tc, elm_atspi_role_get);
}
