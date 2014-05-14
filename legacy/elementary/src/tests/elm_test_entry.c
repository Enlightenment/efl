#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include <Elementary.h>
#include "elm_suite.h"

START_TEST (elm_entry_del)
{
   Evas_Object *win, *entry;

   elm_init(1, NULL);
   win = elm_win_add(NULL, "check", ELM_WIN_BASIC);

   entry = elm_entry_add(win);
   elm_object_text_set(entry, "TEST");

   elm_shutdown();
}
END_TEST

void elm_test_entry(TCase *tc)
{
   tcase_add_test(tc, elm_entry_del);
}
