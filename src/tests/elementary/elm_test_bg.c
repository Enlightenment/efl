#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include <Elementary.h>
#include "elm_suite.h"

START_TEST (elm_bg_legacy_type_check)
{
   Evas_Object *win, *bg;
   const char *type;

   elm_init(1, NULL);
   win = elm_win_add(NULL, "bg", ELM_WIN_BASIC);

   bg = elm_bg_add(win);

   type = elm_object_widget_type_get(bg);
   ck_assert(type != NULL);
   ck_assert(!strcmp(type, "Elm_Bg"));

   type = evas_object_type_get(bg);
   ck_assert(type != NULL);
   ck_assert(!strcmp(type, "elm_bg"));

   elm_shutdown();
}
END_TEST

void elm_test_bg(TCase *tc EINA_UNUSED)
{
   tcase_add_test(tc, elm_bg_legacy_type_check);
}
