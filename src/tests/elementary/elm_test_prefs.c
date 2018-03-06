#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#define EFL_ACCESS_BETA
#include <Elementary.h>
#include "elm_suite.h"

START_TEST (elm_prefs_legacy_type_check)
{
   Evas_Object *win, *prefs;
   const char *type;

   char *args[] = { "exe" };
   elm_init(1, args);
   win = elm_win_add(NULL, "prefs", ELM_WIN_BASIC);

   prefs = elm_prefs_add(win);

   if (prefs)
     {
        type = elm_object_widget_type_get(prefs);
        ck_assert(type != NULL);
        ck_assert(!strcmp(type, "Elm_Prefs"));

        type = evas_object_type_get(prefs);
        ck_assert(type != NULL);
        ck_assert(!strcmp(type, "elm_prefs"));
     }

   elm_shutdown();
}
END_TEST

START_TEST (elm_atspi_role_get)
{
#if 0
   Evas_Object *win, *prefs;
   Efl_Access_Role role;

   char *args[] = { "exe" };
   elm_init(1, args);
   win = elm_win_add(NULL, "prefs", ELM_WIN_BASIC);

   prefs = elm_prefs_add(win);
   role = efl_access_role_get(prefs);

   ck_assert(role == EFL_ACCESS_ROLE_REDUNDANT_OBJECT);

   elm_shutdown();
#endif
}
END_TEST

void elm_test_prefs(TCase *tc)
{
   tcase_add_test(tc, elm_prefs_legacy_type_check);
   tcase_add_test(tc, elm_atspi_role_get);
}
