#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#define EFL_ACCESS_PROTECTED
#include <Elementary.h>
#include "elm_suite.h"


START_TEST (elm_atspi_role_get)
{
   Evas_Object *win, *menu;
   Efl_Access_Role role;

   elm_init(1, NULL);
   win = elm_win_add(NULL, "menu", ELM_WIN_BASIC);

   menu = elm_menu_add(win);
   role = efl_access_role_get(menu);

   ck_assert(role == EFL_ACCESS_ROLE_MENU);

   elm_shutdown();
}
END_TEST

void elm_test_menu(TCase *tc)
{
 tcase_add_test(tc, elm_atspi_role_get);
}
