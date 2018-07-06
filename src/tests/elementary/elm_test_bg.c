#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include <Elementary.h>
#include "elm_suite.h"

EFL_START_TEST (elm_bg_legacy_type_check)
{
   Evas_Object *win, *bg;
   const char *type;

   win = win_add(NULL, "bg", ELM_WIN_BASIC);

   bg = elm_bg_add(win);

   type = elm_object_widget_type_get(bg);
   ck_assert(type != NULL);
   ck_assert(!strcmp(type, "Elm_Bg"));

   type = evas_object_type_get(bg);
   ck_assert(type != NULL);
   ck_assert(!strcmp(type, "elm_bg"));

}
EFL_END_TEST

EFL_START_TEST (elm_bg_legacy_file_set_get_check)
{
   Evas_Object *win, *bg;
   const char *file = NULL, *key = NULL;

   win = win_add(NULL, "bg", ELM_WIN_BASIC);

   bg = elm_bg_add(win);

   /* This test case will check the following things for legacy widget.
	* It is all about backward compatibility.
	* 1. Set and Get file path, key even if there is no proper image file for the given file path.
	* 2. Even if there is a proper image file and the given file path is interpreted to full file path,
	*    the Get function should give original file path. NOT interpreted. */
   elm_bg_file_set(bg, "~/test.png", "test_key");
   elm_bg_file_get(bg, &file, &key);

   ck_assert(file != NULL);
   ck_assert(!strcmp(file, "~/test.png"));
   ck_assert(key != NULL);
   ck_assert(!strcmp(key, "test_key"));

}
EFL_END_TEST

void elm_test_bg(TCase *tc EINA_UNUSED)
{
   tcase_add_test(tc, elm_bg_legacy_type_check);
   tcase_add_test(tc, elm_bg_legacy_file_set_get_check);
}
