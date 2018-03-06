#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#define EFL_ACCESS_BETA
#include <Elementary.h>
#include "elm_suite.h"

START_TEST (elm_glview_legacy_type_check)
{
   Evas_Object *win, *glview;
   const char *type;

   char *args[] = { "exe" };
   elm_init(1, args);
   win = elm_win_add(NULL, "glview", ELM_WIN_BASIC);

   glview = elm_glview_add(win);

   if (glview)
     {
        type = elm_object_widget_type_get(glview);
        ck_assert(type != NULL);
        ck_assert(!strcmp(type, "Elm_Glview"));

        type = evas_object_type_get(glview);
        ck_assert(type != NULL);
        ck_assert(!strcmp(type, "elm_glview"));
     }

   elm_shutdown();
}
END_TEST

START_TEST (elm_atspi_role_get)
{
   Evas_Object *win, *glview;
   Efl_Access_Role role;

   char *args[] = { "exe" };
   elm_init(1, args);
   win = elm_win_add(NULL, "glview", ELM_WIN_BASIC);

   glview = elm_glview_add(win);

   // if no gl backend skip test
   if (glview)
     {
        role = efl_access_role_get(glview);
        ck_assert(role == EFL_ACCESS_ROLE_ANIMATION);
     }

   elm_shutdown();
}
END_TEST

void elm_test_glview(TCase *tc)
{
   tcase_add_test(tc, elm_glview_legacy_type_check);
   tcase_add_test(tc, elm_atspi_role_get);
}
