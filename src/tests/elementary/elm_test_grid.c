#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#define EFL_ACCESS_PROTECTED
#include <Elementary.h>
#include "elm_suite.h"


START_TEST (elm_atspi_role_get)
{
   Evas_Object *win, *grid;
   Efl_Access_Role role;

   elm_init(1, NULL);
   win = elm_win_add(NULL, "grid", ELM_WIN_BASIC);

   grid = elm_grid_add(win);
   role = efl_access_role_get(grid);

   ck_assert(role == EFL_ACCESS_ROLE_FILLER);

   elm_shutdown();
}
END_TEST

void elm_test_grid(TCase *tc)
{
 tcase_add_test(tc, elm_atspi_role_get);
}
