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
   win = win_add(NULL, "win", EFL_UI_WIN_TYPE_BASIC);
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

   // Something that is not a position manager is refused, and the list default stands in
   EXPECT_ERROR_START;
   o = efl_add(EFL_UI_COLLECTION_CLASS, win, efl_ui_collection_position_manager_set(efl_added, random_obj));
   EXPECT_ERROR_END;
   ck_assert_ptr_ne(o, NULL);
   ck_assert(efl_isa(efl_ui_collection_position_manager_get(o), EFL_UI_POSITION_MANAGER_LIST_CLASS));
   efl_del(o);

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

// A manager set again stays: it is not the one that is replaced.
EFL_START_TEST(efl_ui_collection_manager_twice)
{
   Eo *grid = efl_new(EFL_UI_POSITION_MANAGER_GRID_CLASS);
   Eo *c = efl_add(EFL_UI_COLLECTION_CLASS, win,
                   efl_ui_collection_position_manager_set(efl_added, grid));

   ck_assert_ptr_ne(c, NULL);
   efl_ref(grid);
   efl_ui_collection_position_manager_set(c, grid);
   ck_assert(!efl_invalidated_get(grid));
   ck_assert_ptr_eq(efl_ui_collection_position_manager_get(c), grid);
   efl_del(c);
   ck_assert(efl_invalidated_get(grid));
   efl_unref(grid);
}
EFL_END_TEST

EFL_START_TEST(efl_ui_collection_bare)
{
   Eo *c, *items[3];
   int i;

   efl_gfx_entity_size_set(win, EINA_SIZE2D(200, 200));
   c = efl_add(EFL_UI_COLLECTION_CLASS, win);
   ck_assert_ptr_ne(c, NULL);
   ck_assert(efl_isa(efl_ui_collection_position_manager_get(c), EFL_UI_POSITION_MANAGER_LIST_CLASS));
   efl_content_set(win, c);

   for (i = 0; i < 3; i++)
     {
        items[i] = efl_add(EFL_UI_LIST_DEFAULT_ITEM_CLASS, c);
        efl_text_set(items[i], "item");
        efl_pack_end(c, items[i]);
     }
   get_me_to_those_events(c);

   for (i = 0; i < 3; i++)
     {
        Eina_Size2D sz = efl_gfx_entity_size_get(items[i]);

        ck_assert_int_gt(sz.w, 0);
        ck_assert_int_gt(sz.h, 0);
     }
}
EFL_END_TEST

void efl_ui_test_item_container(TCase *tc)
{
   tcase_add_checked_fixture(tc, fail_on_errors_setup, fail_on_errors_teardown);
   tcase_add_checked_fixture(tc, item_container_setup, item_container_teardown);
   tcase_add_test(tc, finalizer_check);
   tcase_add_test(tc, efl_ui_collection_manager_twice);
   tcase_add_test(tc, efl_ui_collection_bare);
}
