#define EFL_NOLEGACY_API_SUPPORT
#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif
#define EFL_LAYOUT_CALC_PROTECTED
#include <Efl_Ui.h>
#include "efl_ui_suite.h"

static Eo *win;
static Eo *grid;

static void
create_ui(void)
{
   win = win_add();
   grid = efl_add(EFL_UI_GRID_CLASS, win);

}

static void
_ordering_equals(Efl_Ui_Widget **wid, unsigned int len)
{
   for (unsigned int i = 0; i < len; ++i)
     {
        ck_assert_ptr_eq(efl_pack_content_get(grid, i), wid[i]);
     }
   ck_assert_int_eq(efl_content_count(grid), len);
}

EFL_START_TEST(non_linear_insertion_pack_at)
{
   Eo *group[3];
   Eo *order[12];

   for (int i = 0; i < 3; ++i)
     {
        order[i*4] = group[i] = efl_add(EFL_UI_GROUP_ITEM_CLASS, grid);
        efl_pack_at(grid, group[i], i);
     }

   for (int i = 0; i < 3; ++i)
     {
        for (int j = 0; j < 3; ++j)
          {
             Eo *item = order[4*i+(1+j)] = efl_add(EFL_UI_GRID_DEFAULT_ITEM_CLASS, grid);
             efl_pack_at(group[i], item, j);
          }
     }
   _ordering_equals(order, 12);
}
EFL_END_TEST

EFL_START_TEST(non_linear_insertion_pack_end)
{
   Eo *group[3];
   Eo *order[12];

   for (int i = 0; i < 3; ++i)
     {
        order[i*4] = group[i] = efl_add(EFL_UI_GROUP_ITEM_CLASS, grid);
        efl_pack_end(grid, group[i]);
     }

   for (int i = 0; i < 3; ++i)
     {
        for (int j = 0; j < 3; ++j)
          {
             Eo *item = order[4*i+(1+j)] = efl_add(EFL_UI_GRID_DEFAULT_ITEM_CLASS, grid);
             efl_pack_end(group[i], item);
          }
     }
   _ordering_equals(order, 12);
}
EFL_END_TEST

void efl_ui_test_group_item(TCase *tc)
{
   tcase_add_checked_fixture(tc, fail_on_errors_setup, fail_on_errors_teardown);
   tcase_add_checked_fixture(tc, create_ui, NULL);
   tcase_add_test(tc, non_linear_insertion_pack_at);
   tcase_add_test(tc, non_linear_insertion_pack_end);
}
