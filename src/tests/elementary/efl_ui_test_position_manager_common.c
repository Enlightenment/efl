#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include <Efl_Ui.h>
#include "efl_ui_suite.h"
#include "efl_ui_test_collection_common.h"

Eo *position_manager;

static Eo* win;

static Eina_Array *arr_obj;
static Eina_Inarray *arr_size;

static void
item_container_setup(void)
{
   win = win_add(NULL, "win", EFL_UI_WIN_TYPE_BASIC);
}

static void
item_container_teardown(void)
{
   win = NULL;
}

static Efl_Ui_Position_Manager_Size_Batch_Result
_size_accessor_get_at(void *data EINA_UNUSED, Efl_Ui_Position_Manager_Size_Call_Config conf, Eina_Rw_Slice memory)
{
   int i;
   Efl_Ui_Position_Manager_Size_Batch_Entity *sizes = memory.mem;
   Efl_Ui_Position_Manager_Size_Batch_Result result = {0};

   for (i = conf.range.start_id; i < (int)(MIN(conf.range.end_id, eina_inarray_count(arr_size))); ++i)
     {
        Eina_Size2D *size = eina_inarray_nth(arr_size, i);

        sizes[i - conf.range.start_id].size = *size;
        sizes[i - conf.range.start_id].depth_leader = 0;
        sizes[i - conf.range.start_id].element_depth = 0;
     }
   result.filled_items = i - conf.range.start_id;
   result.parent_size = EINA_SIZE2D(0, 0);
   return result;
}

static Efl_Ui_Position_Manager_Object_Batch_Result
_obj_accessor_get_at(void *data EINA_UNUSED, Efl_Ui_Position_Manager_Request_Range range, Eina_Rw_Slice memory)
{
   int i;
   Efl_Ui_Position_Manager_Object_Batch_Entity *objs = memory.mem;
   Efl_Ui_Position_Manager_Object_Batch_Result result = {0};

   for (i = range.start_id; i < (int)(MIN(range.end_id, eina_array_count(arr_obj))); ++i)
     {
         Efl_Gfx_Entity *geom = eina_array_data_get(arr_obj, i);

         objs[i - range.start_id].entity = geom;
         objs[i - range.start_id].depth_leader = 0;
         objs[i - range.start_id].element_depth = 0;
     }
   result.filled_items = i - range.start_id;
   result.group = NULL;
   return result;
}
static void
_initial_setup(void)
{
   arr_obj = eina_array_new(10);
   arr_size = eina_inarray_new(sizeof(Eina_Size2D), 10);

   efl_ui_position_manager_data_access_v1_data_access_set(position_manager,
      win,
      NULL, _obj_accessor_get_at, NULL,
      NULL, _size_accessor_get_at, NULL,
      0);
}

static int
_add_item(Eo *obj, Eina_Size2D size)
{
   int idx = eina_array_count(arr_obj);
   EINA_SAFETY_ON_FALSE_RETURN_VAL(eina_array_count(arr_obj) == eina_inarray_count(arr_size), -1);

   eina_array_push(arr_obj, (void*)0x1); //wtf
   eina_array_data_set(arr_obj, idx, obj);
   eina_inarray_push(arr_size, &size);


   efl_ui_position_manager_entity_item_added(position_manager, idx, obj);

   return idx;
}

static void
_update_item(int index, Eo *obj, Eina_Size2D size)
{
   Eina_Size2D *s;

   eina_array_data_set(arr_obj, index, obj);
   s = eina_inarray_nth(arr_size, index);
   *s = size;
}

static void
_ticker(void *data EINA_UNUSED, const Efl_Event *ev EINA_UNUSED)
{
   efl_loop_quit(efl_main_loop_get(), EINA_VALUE_EMPTY);
}

static void
_iterate_a_few(void)
{
   efl_add(EFL_LOOP_TIMER_CLASS, efl_main_loop_get(),
      efl_event_callback_add(efl_added, EFL_LOOP_TIMER_EVENT_TIMER_TICK, _ticker, NULL),
      efl_loop_timer_interval_set(efl_added, 0.1));
   efl_loop_begin(efl_main_loop_get());
}

EFL_START_TEST(no_crash1)
{
   _initial_setup();

   //try to resize the viewport while we have no item
   efl_ui_position_manager_entity_viewport_set(position_manager, EINA_RECT(20, 20, 200, 200));
   _iterate_a_few();
   efl_ui_position_manager_entity_viewport_set(position_manager, EINA_RECT(20, 20, 0, 0));
   _iterate_a_few();
   _add_item(efl_add(EFL_UI_GRID_DEFAULT_ITEM_CLASS, win), EINA_SIZE2D(20, 20));
   _iterate_a_few();
   efl_ui_position_manager_entity_viewport_set(position_manager, EINA_RECT(20, 20, 200, 200));
   _iterate_a_few();
   efl_ui_position_manager_entity_viewport_set(position_manager, EINA_RECT(20, 20, 0, 0));
   _iterate_a_few();
}
EFL_END_TEST

EFL_START_TEST(no_crash2)
{
   _initial_setup();

   //try to resize the viewport while we have no item
   efl_ui_position_manager_entity_viewport_set(position_manager, EINA_RECT(20, 20, 200, 200));
   _iterate_a_few();
   efl_ui_position_manager_entity_viewport_set(position_manager, EINA_RECT(20, 20, 0, 0));
   _iterate_a_few();
   _add_item(NULL, EINA_SIZE2D(20, 20));
   _iterate_a_few();
   efl_ui_position_manager_entity_viewport_set(position_manager, EINA_RECT(20, 20, 0, 0));
   _iterate_a_few();
   efl_ui_position_manager_entity_viewport_set(position_manager, EINA_RECT(20, 20, 200, 200));
   _iterate_a_few();
   _update_item(0, efl_add(EFL_UI_GRID_DEFAULT_ITEM_CLASS, win), EINA_SIZE2D(20, 20));
   _iterate_a_few();
}
EFL_END_TEST

static void
_content_size_cb(void *data, const Efl_Event *ev)
{
   Eina_Size2D *size = data;
   *size = *((Eina_Size2D*)ev->info);
}

EFL_START_TEST(viewport_newsize_event_result)
{
   Eina_Size2D size = EINA_SIZE2D(-2, -2), min_size = EINA_SIZE2D(-2, -2);
   efl_event_callback_add(position_manager,
      EFL_UI_POSITION_MANAGER_ENTITY_EVENT_CONTENT_SIZE_CHANGED, _content_size_cb, &size);
   efl_event_callback_add(position_manager,
      EFL_UI_POSITION_MANAGER_ENTITY_EVENT_CONTENT_MIN_SIZE_CHANGED, _content_size_cb, &min_size);
   _initial_setup();
   _add_item(efl_add(EFL_UI_GRID_DEFAULT_ITEM_CLASS, win), EINA_SIZE2D(20, 20));
   _add_item(efl_add(EFL_UI_GRID_DEFAULT_ITEM_CLASS, win), EINA_SIZE2D(20, 30));

   efl_ui_position_manager_entity_viewport_set(position_manager, EINA_RECT(0, 0, 200, 200));
   _iterate_a_few();

   ck_assert_int_ne(size.w, -2);
   ck_assert_int_ne(size.h, -2);
   ck_assert_int_ne(min_size.w, -2);
   ck_assert_int_ne(min_size.h, -2);
}
EFL_END_TEST

/* Moving across lines goes to the item drawn next to the current one on
   that side, whatever the layout: down to the next row in a vertical grid,
   right to the next column in a horizontal one, the next item in a list.
   There is nothing past the last line, and moving back comes back. */
static void
_check_line_moves(Efl_Ui_Layout_Orientation dir, Efl_Ui_Focus_Direction forward,
                  Efl_Ui_Focus_Direction back)
{
   Eina_Rect from, to;
   unsigned int next = 0, prev = 0;
   const unsigned int start = 3;

   efl_ui_layout_orientation_set(position_manager, dir);
   efl_ui_position_manager_entity_viewport_set(position_manager, EINA_RECT(0, 0, 200, 200));
   _iterate_a_few();

   ck_assert(efl_ui_position_manager_entity_relative_item(position_manager, start, forward, &next));
   from = efl_ui_position_manager_entity_position_single_item(position_manager, start);
   to = efl_ui_position_manager_entity_position_single_item(position_manager, next);
   if (dir == EFL_UI_LAYOUT_ORIENTATION_VERTICAL)
     {
        ck_assert_int_gt(to.y, from.y);
        ck_assert_int_eq(to.x, from.x);
     }
   else
     {
        ck_assert_int_gt(to.x, from.x);
        ck_assert_int_eq(to.y, from.y);
     }

   ck_assert(efl_ui_position_manager_entity_relative_item(position_manager, next, back, &prev));
   ck_assert_int_eq(prev, start);

   ck_assert(!efl_ui_position_manager_entity_relative_item(position_manager, 24, forward, &next));
   ck_assert(!efl_ui_position_manager_entity_relative_item(position_manager, 0, back, &prev));
}

EFL_START_TEST(relative_item_across_lines)
{
   unsigned int i;

   _initial_setup();
   for (i = 0; i < 25; ++i)
     _add_item(efl_add(EFL_UI_GRID_DEFAULT_ITEM_CLASS, win), EINA_SIZE2D(20, 20));

   _check_line_moves(EFL_UI_LAYOUT_ORIENTATION_VERTICAL,
                     EFL_UI_FOCUS_DIRECTION_DOWN, EFL_UI_FOCUS_DIRECTION_UP);
   _check_line_moves(EFL_UI_LAYOUT_ORIENTATION_HORIZONTAL,
                     EFL_UI_FOCUS_DIRECTION_RIGHT, EFL_UI_FOCUS_DIRECTION_LEFT);
}
EFL_END_TEST

// A manager that goes with a job still pending must not be reached by it: the
// job is bound to the manager and ends with it.
EFL_START_TEST(deleted_with_job_pending)
{
   _initial_setup();

   efl_ui_position_manager_entity_viewport_set(position_manager, EINA_RECT(20, 20, 200, 200));
   _add_item(NULL, EINA_SIZE2D(20, 20));
   // (the fixture made it with efl_new: its one ref is ours)
   efl_unref(position_manager);
   position_manager = NULL;
   _iterate_a_few();
}
EFL_END_TEST

void efl_ui_test_position_manager_common_add(TCase *tc)
{
   tcase_add_test(tc, relative_item_across_lines);
   tcase_add_checked_fixture(tc, item_container_setup, item_container_teardown);
   tcase_add_test(tc, no_crash1);
   tcase_add_test(tc, no_crash2);
   tcase_add_test(tc, viewport_newsize_event_result);
   tcase_add_test(tc, deleted_with_job_pending);
}
