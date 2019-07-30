#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include <Efl_Ui.h>
#include "efl_ui_suite.h"
#include "efl_ui_test_collection_common.h"

static Eo* win;

static void
item_container_setup()
{
   win = win_add();
}

static void
item_container_teardown()
{
   win = NULL;
}

EFL_START_TEST(finalizer_check)
{
   Eo *grid = efl_new(EFL_UI_POSITION_MANAGER_GRID_CLASS);
   Eo *list = efl_new(EFL_UI_POSITION_MANAGER_GRID_CLASS);
   Eo *random_obj = efl_add(EFL_UI_BUTTON_CLASS, win);
   Eo *o;

   EXPECT_ERROR_START;
   ck_assert_ptr_eq(efl_add(EFL_UI_COLLECTION_CLASS, win), NULL);
   EXPECT_ERROR_END;

   EXPECT_ERROR_START;
   ck_assert_ptr_eq(efl_add(EFL_UI_COLLECTION_CLASS, win, efl_ui_collection_position_manager_set(efl_added, random_obj)), NULL);
   EXPECT_ERROR_END;

   o = efl_add(EFL_UI_COLLECTION_CLASS, win, efl_ui_collection_position_manager_set(efl_added, grid));
   ck_assert_ptr_ne(o, NULL);
   ck_assert_ptr_eq(efl_parent_get(grid), o);
   efl_del(o);
   o = efl_add(EFL_UI_COLLECTION_CLASS, win, efl_ui_collection_position_manager_set(efl_added, list));
   ck_assert_ptr_ne(o, NULL);
   ck_assert_ptr_eq(efl_parent_get(list), o);
   efl_del(o);
}
EFL_END_TEST

void efl_ui_test_item_container(TCase *tc)
{
   tcase_add_checked_fixture(tc, fail_on_errors_setup, fail_on_errors_teardown);
   tcase_add_checked_fixture(tc, item_container_setup, item_container_teardown);
   tcase_add_test(tc, finalizer_check);
}
