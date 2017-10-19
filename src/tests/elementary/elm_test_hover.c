#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#define EFL_ACCESS_PROTECTED
#include <Elementary.h>
#include "elm_suite.h"


START_TEST (elm_atspi_role_get)
{
   Evas_Object *win, *hover;
   Efl_Access_Role role;

   elm_init(1, NULL);
   win = elm_win_add(NULL, "hover", ELM_WIN_BASIC);

   hover = elm_hover_add(win);
   role = efl_access_role_get(hover);

   ck_assert(role == EFL_ACCESS_ROLE_POPUP_MENU);

   elm_shutdown();
}
END_TEST

void elm_test_hover(TCase *tc)
{
 tcase_add_test(tc, elm_atspi_role_get);
}
