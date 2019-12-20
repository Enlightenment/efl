#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include <Efl_Ui.h>
#include "efl_ui_suite.h"
#include "efl_ui_test_collection_common.h"

static Eo *win;

static void
item_container_setup()
{
   Eo * list = efl_new(EFL_UI_POSITION_MANAGER_GRID_CLASS);
   position_manager = efl_new(EFL_UI_POSITION_MANAGER_GRID_CLASS);
   win = win_add();
   item_container = efl_add(EFL_UI_COLLECTION_CLASS, win,
      efl_ui_collection_position_manager_set(efl_added, list));

}

static void
item_container_teardown()
{
   item_container = NULL;
   position_manager = NULL;
   win = NULL;
}

void efl_ui_test_grid_container(TCase *tc)
{
   tcase_add_checked_fixture(tc, fail_on_errors_setup, fail_on_errors_teardown);
   tcase_add_checked_fixture(tc, item_container_setup, item_container_teardown);
   efl_ui_test_item_container_common_add(tc);
   efl_ui_test_position_manager_common_add(tc);
}
