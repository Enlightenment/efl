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

static void
_item_clicked_cb(void *data, const Efl_Event *ev)
{
   const Efl_Ui_Item_Clickable_Clicked *c = ev->info;
   int *n = data;

   ck_assert_int_eq(c->clicked.button, 1);
   ck_assert_int_eq(c->clicked.repeated, 0);
   ck_assert_ptr_ne(c->item, NULL);
   (*n)++;
}

static void
_item_pressed_cb(void *data, const Efl_Event *ev)
{
   const Efl_Ui_Item_Clickable_Pressed *p = ev->info;
   int *n = data;

   ck_assert_int_eq(p->button, 1);
   ck_assert_ptr_ne(p->item, NULL);
   (*n)++;
}

// An application may raise the click or the press of an item with nothing
// attached: the collection takes the primary button.
EFL_START_TEST(efl_ui_collection_item_events_without_info)
{
   Eo *c, *item;
   int clicked = 0, pressed = 0, any = 0;

   efl_gfx_entity_size_set(win, EINA_SIZE2D(200, 200));
   c = efl_add(EFL_UI_COLLECTION_CLASS, win);
   efl_content_set(win, c);
   item = efl_add(EFL_UI_LIST_DEFAULT_ITEM_CLASS, c);
   efl_pack_end(c, item);
   efl_event_callback_add(c, EFL_UI_EVENT_ITEM_CLICKED, _item_clicked_cb, &clicked);
   efl_event_callback_add(c, EFL_UI_EVENT_ITEM_CLICKED_ANY, _item_clicked_cb, &any);
   efl_event_callback_add(c, EFL_UI_EVENT_ITEM_PRESSED, _item_pressed_cb, &pressed);

   efl_event_callback_call(item, EFL_INPUT_EVENT_CLICKED, NULL);
   efl_event_callback_call(item, EFL_INPUT_EVENT_CLICKED_ANY, NULL);
   efl_event_callback_call(item, EFL_INPUT_EVENT_PRESSED, NULL);
   ck_assert_int_eq(clicked, 1);
   ck_assert_int_eq(any, 1);
   ck_assert_int_eq(pressed, 1);
}
EFL_END_TEST

static void
_manager_del(void *data, const Efl_Event *ev EINA_UNUSED)
{
   *(Eina_Bool *)data = EINA_TRUE;
}

// The ownership of a position manager is passed to the container: it holds the
// only ref once it is set, and ends the manager it replaces.
EFL_START_TEST(efl_ui_collection_manager_ownership)
{
   Eina_Bool gone = EINA_FALSE, gone2 = EINA_FALSE;
   Eo *c = efl_add(EFL_UI_COLLECTION_CLASS, win);
   Eo *grid = efl_new(EFL_UI_POSITION_MANAGER_GRID_CLASS);
   Eo *list = efl_new(EFL_UI_POSITION_MANAGER_LIST_CLASS);

   efl_event_callback_add(grid, EFL_EVENT_DEL, _manager_del, &gone);
   efl_event_callback_add(list, EFL_EVENT_DEL, _manager_del, &gone2);
   efl_ui_collection_position_manager_set(c, grid);
   ck_assert_ptr_eq(efl_ui_collection_position_manager_get(c), grid);
   ck_assert_int_eq(efl_ref_count(grid), 1);
   efl_ui_collection_position_manager_set(c, list);
   ck_assert(gone);
   ck_assert(!gone2);
   efl_del(c);
   ck_assert(gone2);
}
EFL_END_TEST

void efl_ui_test_item_container(TCase *tc)
{
   tcase_add_checked_fixture(tc, fail_on_errors_setup, fail_on_errors_teardown);
   tcase_add_checked_fixture(tc, item_container_setup, item_container_teardown);
   tcase_add_test(tc, finalizer_check);
   tcase_add_test(tc, efl_ui_collection_manager_twice);
   tcase_add_test(tc, efl_ui_collection_bare);
   tcase_add_test(tc, efl_ui_collection_item_events_without_info);
   tcase_add_test(tc, efl_ui_collection_manager_ownership);
}
