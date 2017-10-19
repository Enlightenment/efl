#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#define EFL_ACCESS_PROTECTED
#include <Elementary.h>
#include "elm_suite.h"

START_TEST (elm_atspi_role_get)
{
   Evas_Object *win, *fs_button;
   Efl_Access_Role role;

   elm_init(1, NULL);
   win = elm_win_add(NULL, "fileselector_button", ELM_WIN_BASIC);

   fs_button = elm_fileselector_button_add(win);
   role = efl_access_role_get(fs_button);

   ck_assert(role == EFL_ACCESS_ROLE_PUSH_BUTTON);

   elm_shutdown();
}
END_TEST

void elm_test_fileselector_button(TCase *tc)
{
 tcase_add_test(tc, elm_atspi_role_get);
}
