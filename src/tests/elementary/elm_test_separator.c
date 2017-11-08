#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#define EFL_ACCESS_BETA
#include <Elementary.h>
#include "elm_suite.h"


START_TEST (elm_atspi_role_get)
{
   Evas_Object *win, *separator;
   Efl_Access_Role role;

   elm_init(1, NULL);
   win = elm_win_add(NULL, "icon", ELM_WIN_BASIC);

   separator = elm_separator_add(win);
   role = efl_access_role_get(separator);

   ck_assert(role == EFL_ACCESS_ROLE_SEPARATOR);

   elm_shutdown();
}
END_TEST

void elm_test_separator(TCase *tc)
{
 tcase_add_test(tc, elm_atspi_role_get);
}
