#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#define EFL_ACCESS_BETA
#include <Elementary.h>
#include "elm_suite.h"

START_TEST (elm_scroller_legacy_type_check)
{
   Evas_Object *win, *scroller;
   const char *type;

   elm_init(1, NULL);
   win = elm_win_add(NULL, "scroller", ELM_WIN_BASIC);

   scroller = elm_scroller_add(win);

   type = elm_object_widget_type_get(scroller);
   ck_assert(type != NULL);
   ck_assert(!strcmp(type, "Elm_Scroller"));

   type = evas_object_type_get(scroller);
   ck_assert(type != NULL);
   ck_assert(!strcmp(type, "elm_scroller"));

   elm_shutdown();
}
END_TEST

START_TEST (elm_atspi_role_get)
{
   Evas_Object *win, *scroller;
   Efl_Access_Role role;

   elm_init(1, NULL);
   win = elm_win_add(NULL, "scroller", ELM_WIN_BASIC);

   scroller = elm_scroller_add(win);
   role = efl_access_role_get(scroller);

   ck_assert(role == EFL_ACCESS_ROLE_SCROLL_PANE);

   elm_shutdown();
}
END_TEST

void elm_test_scroller(TCase *tc)
{
   tcase_add_test(tc, elm_scroller_legacy_type_check);
   tcase_add_test(tc, elm_atspi_role_get);
}
