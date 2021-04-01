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
   Eo * list = efl_new(EFL_UI_POSITION_MANAGER_LIST_CLASS);
   position_manager = efl_new(EFL_UI_POSITION_MANAGER_LIST_CLASS);
   win = win_add();
   item_container = efl_add(EFL_UI_COLLECTION_CLASS, win,
      efl_ui_collection_position_manager_set(efl_added, list));
   efl_content_set(win, item_container);

}

static void
item_container_teardown()
{
   item_container = NULL;
   position_manager = NULL;
   win = NULL;
}


EFL_START_TEST(placement_test_only_items)
{
   Eo *item[3];

   for (int i = 0; i < 2; ++i)
     {
        Eo *item = efl_add(EFL_UI_LIST_DEFAULT_ITEM_CLASS, item_container);
        efl_pack_end(item_container, item);
        efl_gfx_hint_size_min_set(item, EINA_SIZE2D(40, 40));
     }

   for (int i = 0; i < 3; ++i)
     {
        item[i] = efl_add(EFL_UI_LIST_DEFAULT_ITEM_CLASS, item_container);
        efl_pack_end(item_container, item[i]);
        efl_gfx_hint_size_min_set(item[i], EINA_SIZE2D(40, 40));

     }

   //now fill up to trigger the scrollbar to be visible
   for (int i = 0; i < 10; ++i)
     {
        Eo *item = efl_add(EFL_UI_LIST_DEFAULT_ITEM_CLASS, item_container);
        efl_pack_end(item_container, item);
        efl_gfx_hint_size_min_set(item, EINA_SIZE2D(40, 40));
     }

   efl_gfx_entity_geometry_set(win, EINA_RECT(0, 0, 200, 200));

   get_me_to_those_events(item_container);

   for (int i = 0; i < 3; ++i)
     {
        Eina_Rect r = efl_gfx_entity_geometry_get(item[i]);

        ck_assert_int_eq(r.x, 0);
        ck_assert_int_eq(r.y, (i+2)*40);
        ck_assert_int_eq(r.w, 200); // 200 - 2px border - X for the width of the scrollbar.
        ck_assert_int_eq(r.h, 40);
     }

   efl_ui_scrollable_content_pos_set(item_container, EINA_POSITION2D(0, 80));

   for (int i = 0; i < 3; ++i)
     {
        Eina_Rect r = efl_gfx_entity_geometry_get(item[i]);

        ck_assert_int_eq(r.x, 0);
        ck_assert_int_eq(r.y, i*40);
        ck_assert_int_eq(r.w, 200); // 200 - 2px border - X for the width of the scrollbar.
        ck_assert_int_eq(r.h, 40);
     }

}
EFL_END_TEST

EFL_START_TEST(placement_test_group)
{
   Eo *core_item[4];
   Eo *group;
   Eina_Rect r;

   for (int i = 0; i < 1; ++i)
     {
        Eo *item = efl_add(EFL_UI_LIST_DEFAULT_ITEM_CLASS, item_container);
        efl_pack_end(item_container, item);
        efl_gfx_hint_size_min_set(item, EINA_SIZE2D(40, 40));
     }

   core_item[0] = group = efl_add(EFL_UI_GROUP_ITEM_CLASS, item_container);
   efl_pack_end(item_container, group);
        efl_gfx_hint_size_min_set(group, EINA_SIZE2D(40, 40));

   Eo **item = core_item+1;
   for (int i = 0; i < 3; ++i)
     {
        item[i] = efl_add(EFL_UI_LIST_DEFAULT_ITEM_CLASS, item_container);
        efl_pack_end(group, item[i]);
        efl_gfx_hint_size_min_set(item[i], EINA_SIZE2D(40, 40));

     }

   //now fill up to trigger the scrollbar to be visible
   for (int i = 0; i < 10; ++i)
     {
        Eo *item = efl_add(EFL_UI_LIST_DEFAULT_ITEM_CLASS, item_container);
        efl_pack_end(group, item);
        efl_gfx_hint_size_min_set(item, EINA_SIZE2D(40, 40));
     }

   efl_gfx_entity_geometry_set(win, EINA_RECT(0, 0, 200, 200));

   get_me_to_those_events(item_container);

   for (int i = 0; i < 4; ++i)
     {
        r = efl_gfx_entity_geometry_get(core_item[i]);

        ck_assert_int_eq(r.x, 0);
        ck_assert_int_eq(r.y, 0+(i+1)*40);
        ck_assert_int_eq(r.w, 200); // 200 - 2px border - X for the width of the scrollbar.
        ck_assert_int_eq(r.h, 40);
     }

   // test when we have scrolled to the top of the group item - just testing normal scrolling
   efl_ui_scrollable_content_pos_set(item_container, EINA_POSITION2D(0, 40));

   for (int i = 0; i < 4; ++i)
     {
        r = efl_gfx_entity_geometry_get(core_item[i]);

        ck_assert_int_eq(r.x, 0);
        ck_assert_int_eq(r.y, i*40);
        ck_assert_int_eq(r.w, 200); // 200 - 2px border - X for the width of the scrollbar.
        ck_assert_int_eq(r.h, 40);
     }

   //testing the placement in the middle of the item - group must already be warped
   efl_ui_scrollable_content_pos_set(item_container, EINA_POSITION2D(0, 60));

   r = efl_gfx_entity_geometry_get(core_item[0]);

   ck_assert_int_eq(r.x, 0);
   ck_assert_int_eq(r.y, 0);
   ck_assert_int_eq(r.w, 200); // 200 - 2px border - X for the width of the scrollbar.
   ck_assert_int_eq(r.h, 40);

   for (int i = 1; i < 4; ++i)
     {
        r = efl_gfx_entity_geometry_get(core_item[i]);

        ck_assert_int_eq(r.x, 0);
        ck_assert_int_eq(r.y, 20+(i - 1)*40);
        ck_assert_int_eq(r.w, 200); // 200 - 2px border - X for the width of the scrollbar.
        ck_assert_int_eq(r.h, 40);
     }

   // testing if we have scrolled into the middle of the group the group item is not even in the calculated index anymore
   efl_ui_scrollable_content_pos_set(item_container, EINA_POSITION2D(0, 120));

   r = efl_gfx_entity_geometry_get(core_item[0]);

   ck_assert_int_eq(r.x, 0);
   ck_assert_int_eq(r.y, 0);
   ck_assert_int_eq(r.w, 200); // 200 - 2px border - X for the width of the scrollbar.
   ck_assert_int_eq(r.h, 40);

   for (int i = 2; i < 4; ++i)
     {
        r = efl_gfx_entity_geometry_get(core_item[i]);

        ck_assert_int_eq(r.x, 0);
        ck_assert_int_eq(r.y, (i-2)*40);
        ck_assert_int_eq(r.w, 200); // 200 - 2px border - X for the width of the scrollbar.
        ck_assert_int_eq(r.h, 40);
     }

}
EFL_END_TEST

EFL_START_TEST(placement_test_group_crazy)
{
   Eo *core_item[4];
   Eo *group;
   Eina_Rect r;

   for (int i = 0; i < 1; ++i)
     {
        Eo *item = efl_add(EFL_UI_LIST_DEFAULT_ITEM_CLASS, item_container);
        efl_pack_end(item_container, item);
        efl_gfx_hint_size_min_set(item, EINA_SIZE2D(40, 40));
     }

   core_item[0] = group = efl_add(EFL_UI_GROUP_ITEM_CLASS, item_container);
   efl_pack_end(item_container, group);
        efl_gfx_hint_size_min_set(group, EINA_SIZE2D(40, 40));

   Eo **item = core_item+1;
   for (int i = 0; i < 3; ++i)
     {
        item[i] = efl_add(EFL_UI_LIST_DEFAULT_ITEM_CLASS, item_container);
        efl_pack_end(group, item[i]);
        efl_gfx_hint_size_min_set(item[i], EINA_SIZE2D(40, 40));

     }

   //now fill up to trigger the scrollbar to be visible
   for (int i = 0; i < 10; ++i)
     {
        Eo *group2 = efl_add(EFL_UI_GROUP_ITEM_CLASS, item_container);
        efl_pack_end(item_container, group2);
        efl_gfx_hint_size_min_set(group2, EINA_SIZE2D(40, 40));
        for (int i = 0; i < 3; ++i)
          {
             Eo *item = efl_add(EFL_UI_LIST_DEFAULT_ITEM_CLASS, item_container);
             efl_pack_end(group2, item);
             efl_gfx_hint_size_min_set(item, EINA_SIZE2D(40, 40));
          }
     }

   efl_gfx_entity_geometry_set(win, EINA_RECT(0, 0, 200, 800));

   get_me_to_those_events(item_container);

   for (int i = 0; i < 4; ++i)
     {
        r = efl_gfx_entity_geometry_get(core_item[i]);

        ck_assert_int_eq(r.x, 0);
        ck_assert_int_eq(r.y, (i+1)*40);
        ck_assert_int_eq(r.w, 200); // 200 - 2px border - X for the width of the scrollbar.
        ck_assert_int_eq(r.h, 40);
        ck_assert_int_eq(efl_gfx_entity_visible_get(core_item[i]), EINA_TRUE);
     }

   // test when we have scrolled to the top of the group item - just testing normal scrolling
   efl_ui_scrollable_content_pos_set(item_container, EINA_POSITION2D(0, 40));

   for (int i = 0; i < 4; ++i)
     {
        r = efl_gfx_entity_geometry_get(core_item[i]);

        ck_assert_int_eq(r.x, 0);
        ck_assert_int_eq(r.y, i*40);
        ck_assert_int_eq(r.w, 200); // 200 - 2px border - X for the width of the scrollbar.
        ck_assert_int_eq(r.h, 40);
        ck_assert_int_eq(efl_gfx_entity_visible_get(core_item[i]), EINA_TRUE);
     }

   //testing the placement in the middle of the item - group must already be warped
   efl_ui_scrollable_content_pos_set(item_container, EINA_POSITION2D(0, 60));

   r = efl_gfx_entity_geometry_get(core_item[0]);

   ck_assert_int_eq(r.x, 0);
   ck_assert_int_eq(r.y, 0);
   ck_assert_int_eq(r.w, 200); // 200 - 2px border - X for the width of the scrollbar.
   ck_assert_int_eq(r.h, 40);
   ck_assert_int_eq(efl_gfx_entity_visible_get(core_item[0]), EINA_TRUE);

   for (int i = 1; i < 4; ++i)
     {
        r = efl_gfx_entity_geometry_get(core_item[i]);

        ck_assert_int_eq(r.x, 0);
        ck_assert_int_eq(r.y, 20+(i - 1)*40);
        ck_assert_int_eq(r.w, 200); // 200 - 2px border - X for the width of the scrollbar.
        ck_assert_int_eq(r.h, 40);
        ck_assert_int_eq(efl_gfx_entity_visible_get(core_item[i]), EINA_TRUE);
     }

   // testing if we have scrolled into the middle of the group the group item is not even in the calculated index anymore
   efl_ui_scrollable_content_pos_set(item_container, EINA_POSITION2D(0, 120));

   r = efl_gfx_entity_geometry_get(core_item[0]);

   ck_assert_int_eq(r.x, 0);
   ck_assert_int_eq(r.y, 0);
   ck_assert_int_eq(r.w, 200); // 200 - 2px border - X for the width of the scrollbar.
   ck_assert_int_eq(r.h, 40);
   ck_assert_int_eq(efl_gfx_entity_visible_get(core_item[0]), EINA_TRUE);

   for (int i = 2; i < 4; ++i)
     {
        r = efl_gfx_entity_geometry_get(core_item[i]);

        ck_assert_int_eq(r.x, 0);
        ck_assert_int_eq(r.y, (i-2)*40);
        ck_assert_int_eq(r.w, 200); // 200 - 2px border - X for the width of the scrollbar.
        ck_assert_int_eq(r.h, 40);
        ck_assert_int_eq(efl_gfx_entity_visible_get(core_item[i]), EINA_TRUE);
     }

}
EFL_END_TEST

void efl_ui_test_list_container(TCase *tc)
{
   tcase_add_checked_fixture(tc, fail_on_errors_setup, fail_on_errors_teardown);
   tcase_add_checked_fixture(tc, item_container_setup, item_container_teardown);
   efl_ui_test_item_container_common_add(tc);
   efl_ui_test_position_manager_common_add(tc);
   tcase_add_test(tc, placement_test_only_items);
   tcase_add_test(tc, placement_test_group);
   tcase_add_test(tc, placement_test_group_crazy);
}
