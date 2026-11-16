#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include <Efl_Ui.h>
#include "efl_ui_suite.h"
#include "efl_ui_homogeneous_model.eo.h"
#include "efl_ui_exact_model.eo.h"
#include "efl_ui_average_model.eo.h"

#define NUM_ITEMS 50
#define SINGLE_ADDS 2000

static Efl_Model*
_make_model(Efl_Ui_Win *win)
{
   Eina_Value vi;
   Efl_Generic_Model *model, *child;
   unsigned int i;

   model = efl_add(EFL_GENERIC_MODEL_CLASS, win);
   model = efl_add(EFL_UI_VIEW_MODEL_CLASS, model,
                   efl_ui_view_model_set(efl_added, model));
   eina_value_setup(&vi, EINA_VALUE_TYPE_INT);

   efl_ui_view_model_property_string_add(model, "relative", "Relative index ${child.index}", "WRONG", "WRONG");
   efl_ui_view_model_property_string_add(model, "title", "Initial index ${initial}", "WRONG", "WRONG");


   for (i = 0; i < (NUM_ITEMS); i++)
     {
        child = efl_model_child_add(model);
        eina_value_set(&vi, i);
        efl_model_property_set(child, "initial", &vi);
     }

   eina_value_flush(&vi);
   return model;
}

static Eina_Value
_quit(Eo *obj, void *data EINA_UNUSED, const Eina_Value v)
{
   efl_loop_quit(efl_loop_get(obj), v);
   return v;
}

static Eina_Value
_children_get(Eo *obj EINA_UNUSED, void *data EINA_UNUSED, const Eina_Value v)
{
   Efl_Model *child;
   unsigned int i, len;

   EINA_VALUE_ARRAY_FOREACH(&v, len, i, child)
     {
        Eina_Value *rel_val, *title_val;
        char *relative, *title;
        if (i != 5) continue;
        rel_val = efl_model_property_get(child, "relative");
        title_val = efl_model_property_get(child, "title");
        relative = eina_value_to_string(rel_val);
        title = eina_value_to_string(title_val);

        ck_assert(eina_streq(relative, "Relative index 5"));
        ck_assert(eina_streq(title, "Initial index 5"));
        free(relative);
        free(title);
        break;
     }
   return EINA_VALUE_EMPTY;
}

/* A collection view nobody gave a position manager lists its items. */
EFL_START_TEST(efl_ui_collection_view_bare)
{
   Efl_Ui_Win *win;
   Efl_Model *model;
   Efl_Ui_Collection_View *cv;
   Efl_Ui_Factory *fl;
   int count_realize = 0;

   win = win_add(NULL, "win", EFL_UI_WIN_TYPE_BASIC);
   efl_gfx_entity_size_set(win, EINA_SIZE2D(200, 200));

   model = _make_model(win);
   fl = efl_add(EFL_UI_LAYOUT_FACTORY_CLASS, win,
                efl_ui_property_bind(efl_added, "text", "title"),
                efl_ui_widget_factory_item_class_set(efl_added, EFL_UI_LIST_DEFAULT_ITEM_CLASS));

   cv = efl_add(EFL_UI_COLLECTION_VIEW_CLASS, win,
                efl_ui_collection_view_factory_set(efl_added, fl),
                efl_ui_view_model_set(efl_added, model),
                efl_event_callback_add(efl_added, EFL_UI_COLLECTION_VIEW_EVENT_ITEM_REALIZED,
                                       (void*)event_callback_that_increments_an_int_when_called,
                                       &count_realize));
   ck_assert_ptr_ne(cv, NULL);
   ck_assert(efl_isa(efl_ui_collection_view_position_manager_get(cv),
                     EFL_UI_POSITION_MANAGER_LIST_CLASS));
   efl_content_set(win, cv);

   get_me_to_those_events(cv);
   ck_assert_int_gt(count_realize, 0);
   efl_event_callback_del(cv, EFL_UI_COLLECTION_VIEW_EVENT_ITEM_REALIZED,
                          (void*)event_callback_that_increments_an_int_when_called,
                          &count_realize);
}
EFL_END_TEST

EFL_START_TEST(test_efl_ui_collection_view_basic)
{
   Efl_Ui_Win *win;
   Efl_Model *model;
   Efl_Ui_List_View *lv;
   Efl_Ui_Factory *fl;
   Eina_Future *f;
   int count_realize = 0;
   int count_unrealize = 0;

   win = win_add(NULL, "win", EFL_UI_WIN_TYPE_BASIC);
   efl_gfx_entity_size_set(win, EINA_SIZE2D(100, 100));

   model = _make_model(win);
   fl = efl_add(EFL_UI_LAYOUT_FACTORY_CLASS, win,
                efl_ui_property_bind(efl_added, "text", "title"),
                efl_ui_widget_factory_item_class_set(efl_added, EFL_UI_LIST_DEFAULT_ITEM_CLASS));

   lv = efl_add(EFL_UI_LIST_VIEW_CLASS, win,
                efl_ui_collection_view_factory_set(efl_added, fl),
                efl_ui_view_model_set(efl_added, model),
                efl_event_callback_add(efl_added, EFL_UI_COLLECTION_VIEW_EVENT_ITEM_REALIZED,
                  (void*)event_callback_that_increments_an_int_when_called, &count_realize),
                efl_event_callback_add(efl_added, EFL_UI_COLLECTION_VIEW_EVENT_ITEM_UNREALIZED,
                  (void*)event_callback_that_increments_an_int_when_called, &count_unrealize)
               );
   efl_content_set(win, lv);

   get_me_to_those_events(lv);
   ck_assert_int_gt(count_realize, 0);
   /* FIXME: this should eventually be eq(0) */
   ck_assert_int_gt(count_unrealize, 0);

   f = efl_model_children_slice_get(lv, 0, efl_model_children_count_get(lv));
   f = efl_future_then(lv, f, .success_type = EINA_VALUE_TYPE_ARRAY, .success = _children_get);
   ecore_main_loop_iterate();
   efl_event_callback_del(lv, EFL_UI_COLLECTION_VIEW_EVENT_ITEM_REALIZED, (void*)event_callback_that_increments_an_int_when_called, &count_realize);
   efl_event_callback_del(lv, EFL_UI_COLLECTION_VIEW_EVENT_ITEM_UNREALIZED, (void*)event_callback_that_increments_an_int_when_called, &count_unrealize);
}
EFL_END_TEST

EFL_START_TEST(test_efl_ui_collection_view_select)
{
   Efl_Ui_Win *win;
   Efl_Model *model;
   Efl_Ui_List_View *lv;
   Efl_Ui_Factory *fl;
   Eina_Value *sel_val;
   unsigned long sel = 10000;

   win = win_add(NULL, "win", EFL_UI_WIN_TYPE_BASIC);
   efl_gfx_entity_size_set(win, EINA_SIZE2D(100, 100));

   model = _make_model(win);
   model = efl_add(EFL_UI_SELECT_MODEL_CLASS, efl_main_loop_get(), efl_ui_view_model_set(efl_added, model));
   fl = efl_add(EFL_UI_LAYOUT_FACTORY_CLASS, win,
                efl_ui_property_bind(efl_added, "text", "title"),
                efl_ui_widget_factory_item_class_set(efl_added, EFL_UI_LIST_DEFAULT_ITEM_CLASS));

   lv = efl_add(EFL_UI_LIST_VIEW_CLASS, win,
                efl_ui_collection_view_factory_set(efl_added, fl),
                efl_ui_view_model_set(efl_added, model)
               );
   efl_content_set(win, lv);

   get_me_to_those_events(lv);

   /* nothing selected yet */
   sel_val = efl_model_property_get(model, "child.selected");
   ck_assert(eina_value_type_get(sel_val) == EINA_VALUE_TYPE_ERROR);

   efl_future_then(model, efl_model_property_ready_get(model, "child.selected"), .success = _quit);

   click_object_at(lv, 50, 5);
   ecore_main_loop_begin();

   sel_val = efl_model_property_get(model, "child.selected");
   ck_assert(eina_value_type_get(sel_val) == EINA_VALUE_TYPE_ULONG);
   ck_assert(eina_value_ulong_get(sel_val, &sel));
   ck_assert_int_eq(sel, 0);
}
EFL_END_TEST

static void
_count_size_disagreements(const Eina_Log_Domain *d, Eina_Log_Level level,
                          const char *file, const char *fnc, int line,
                          const char *fmt, void *data, va_list args)
{
   int *disagreements = data;

   if (eina_str_has_prefix(fmt, "Reported sizes changed"))
     (*disagreements)++;
   eina_log_print_cb_stderr(d, level, file, fnc, line, fmt, NULL, args);
}

/* The row an item shows: the item sits in a view, not in a pack, so it has
   no index of its own, but its model is the row. */
static int
_item_row(Eo *item)
{
   Efl_Model *model = NULL;

   if (efl_isa(item, EFL_UI_VIEW_INTERFACE)) model = efl_ui_view_model_get(item);
   return (model && efl_isa(model, EFL_COMPOSITE_MODEL_CLASS)) ?
     (int) efl_composite_model_index_get(model) : -1;
}

static Eina_Value
_select_child(Eo *o EINA_UNUSED, void *data EINA_UNUSED, const Eina_Value v)
{
   Efl_Model *child = NULL;
   Eina_Value set = eina_value_bool_init(EINA_TRUE);

   eina_value_array_get(&v, 0, &child);
   efl_model_property_set(child, "self.selected", &set);
   eina_value_flush(&set);
   return v;
}

/* Turns the loop until last_selected is an item showing row index, or gives
   up; returns the item. */
static Eo *
_wait_last_selected(Eo *lv, int index)
{
   int i;

   for (i = 0; i < 500; i++)
     {
        Eo *item = efl_ui_selectable_last_selected_get(lv);

        if (item && (_item_row(item) == index)) return item;
        ecore_main_loop_iterate();
     }
   return efl_ui_selectable_last_selected_get(lv);
}

/* What a view says was selected last is an item, as for any other widget,
   and it stays that item while its row is scrolled away: the view keeps it
   aside, out of sight, rather than letting it go. A row selected without
   ever being shown gets an item too. */
EFL_START_TEST(test_efl_ui_collection_view_last_selected_is_an_item)
{
   Efl_Ui_Win *win;
   Efl_Model *model;
   Efl_Ui_List_View *lv;
   Efl_Ui_Factory *fl;
   Eo *item, *later;
   int i;

   win = win_add(NULL, "win", EFL_UI_WIN_TYPE_BASIC);
   efl_gfx_entity_size_set(win, EINA_SIZE2D(100, 100));

   model = _make_model(win);
   model = efl_add(EFL_UI_SELECT_MODEL_CLASS, efl_main_loop_get(),
                   efl_ui_view_model_set(efl_added, model));
   fl = efl_add(EFL_UI_LAYOUT_FACTORY_CLASS, win,
                efl_ui_property_bind(efl_added, "text", "title"),
                efl_ui_widget_factory_item_class_set(efl_added, EFL_UI_LIST_DEFAULT_ITEM_CLASS));
   lv = efl_add(EFL_UI_LIST_VIEW_CLASS, win,
                efl_ui_collection_view_factory_set(efl_added, fl),
                efl_ui_view_model_set(efl_added, model));
   efl_content_set(win, lv);
   get_me_to_those_events(lv);

   click_object_at(lv, 50, 5);
   item = _wait_last_selected(lv, 0);
   ck_assert_ptr_nonnull(item);
   ck_assert(efl_isa(item, EFL_UI_LIST_DEFAULT_ITEM_CLASS));
   ck_assert_int_eq(_item_row(item), 0);
   ck_assert(efl_ui_selectable_selected_get(item));

   /* far away, and long enough for the view to let rows' items go */
   efl_ui_scrollable_content_pos_set(lv, EINA_POSITION2D(0, 100000));
   get_me_to_those_events(lv);
   for (i = 0; i < 50; i++)
     {
        ecore_main_loop_iterate();
     }

   ck_assert_ptr_eq(efl_ui_selectable_last_selected_get(lv), item);
   ck_assert_int_eq(_item_row(item), 0);
   ck_assert(efl_ui_selectable_selected_get(item));
   ck_assert(!efl_gfx_entity_visible_get(item));

   /* a row nobody scrolled to, chosen from code */
   efl_future_then(model, efl_model_children_slice_get(model, 25, 1),
                   .success = _select_child, .success_type = EINA_VALUE_TYPE_ARRAY);
   later = _wait_last_selected(lv, 25);
   ck_assert_ptr_nonnull(later);
   ck_assert_int_eq(_item_row(later), 25);
}
EFL_END_TEST

static void
_realized_track(void *data, const Efl_Event *ev)
{
   Eina_List **realized = data;

   *realized = eina_list_append(*realized, ev->info);
}

static void
_unrealized_track(void *data, const Efl_Event *ev)
{
   Eina_List **realized = data;

   *realized = eina_list_remove(*realized, ev->info);
}

/* The item showing row index, among those the view has realized and not let
   go of. */
static Eo *
_realized_row(Eina_List *realized, int index)
{
   Eina_List *l;
   Eo *item;

   EINA_LIST_FOREACH(realized, l, item)
     {
        if (_item_row(item) == index) return item;
     }
   return NULL;
}

/* Focus coming into a view from below lands on its last row, so the view
   keeps an item for that row, out of sight while the top is shown -- and
   for the new last row once rows are added. */
EFL_START_TEST(test_efl_ui_collection_view_last_row_kept)
{
   Eina_List *realized = NULL;
   Efl_Ui_Win *win;
   Efl_Model *model;
   Efl_Ui_List_View *lv;
   Efl_Ui_Factory *fl;
   Eo *last;
   int i;

   win = win_add(NULL, "win", EFL_UI_WIN_TYPE_BASIC);
   efl_gfx_entity_size_set(win, EINA_SIZE2D(100, 100));

   model = _make_model(win);
   fl = efl_add(EFL_UI_LAYOUT_FACTORY_CLASS, win,
                efl_ui_property_bind(efl_added, "text", "title"),
                efl_ui_widget_factory_item_class_set(efl_added, EFL_UI_LIST_DEFAULT_ITEM_CLASS));
   lv = efl_add(EFL_UI_LIST_VIEW_CLASS, win,
                efl_event_callback_add(efl_added, EFL_UI_COLLECTION_VIEW_EVENT_ITEM_REALIZED,
                                       _realized_track, &realized),
                efl_event_callback_add(efl_added, EFL_UI_COLLECTION_VIEW_EVENT_ITEM_UNREALIZED,
                                       _unrealized_track, &realized),
                efl_ui_collection_view_factory_set(efl_added, fl),
                efl_ui_view_model_set(efl_added, model));
   efl_content_set(win, lv);
   get_me_to_those_events(lv);
   for (i = 0; i < 50; i++)
     {
        ecore_main_loop_iterate();
     }

   last = _realized_row(realized, NUM_ITEMS - 1);
   ck_assert_ptr_nonnull(last);
   ck_assert(!efl_gfx_entity_visible_get(last));

   efl_model_child_add(model);
   for (i = 0; i < 50; i++)
     {
        ecore_main_loop_iterate();
     }
   ck_assert_ptr_nonnull(_realized_row(realized, NUM_ITEMS));

   efl_event_callback_del(lv, EFL_UI_COLLECTION_VIEW_EVENT_ITEM_REALIZED, _realized_track, &realized);
   efl_event_callback_del(lv, EFL_UI_COLLECTION_VIEW_EVENT_ITEM_UNREALIZED, _unrealized_track, &realized);
   eina_list_free(realized);
}
EFL_END_TEST

/* A list emptied and filled again, as a whole new set of rows replaces the
   old one, shows the new rows. The view asks for its last row each time
   the count changes, and those asked for while the old rows went away are
   answered once the new ones are in: they must not be shown as them. */
EFL_START_TEST(test_efl_ui_collection_view_refill)
{
   Eina_List *realized = NULL;
   Efl_Ui_Win *win;
   Efl_Model *model;
   Efl_Ui_List_View *lv;
   Efl_Ui_Factory *fl;
   Eina_Value v;
   Eo *item, *rows[3];
   char text[16];
   int i;

   win = win_add(NULL, "win", EFL_UI_WIN_TYPE_BASIC);
   efl_gfx_entity_size_set(win, EINA_SIZE2D(100, 300));

   model = efl_add(EFL_GENERIC_MODEL_CLASS, win);
   eina_value_setup(&v, EINA_VALUE_TYPE_STRING);
   for (i = 0; i < 3; i++)
     {
        snprintf(text, sizeof(text), "Row %d", i);
        eina_value_set(&v, text);
        rows[i] = efl_model_child_add(model);
        efl_model_property_set(rows[i], "title", &v);
     }

   fl = efl_add(EFL_UI_LAYOUT_FACTORY_CLASS, win,
                efl_ui_property_bind(efl_added, "text", "title"),
                efl_ui_widget_factory_item_class_set(efl_added, EFL_UI_LIST_DEFAULT_ITEM_CLASS));
   lv = efl_add(EFL_UI_LIST_VIEW_CLASS, win,
                efl_event_callback_add(efl_added, EFL_UI_COLLECTION_VIEW_EVENT_ITEM_REALIZED,
                                       _realized_track, &realized),
                efl_event_callback_add(efl_added, EFL_UI_COLLECTION_VIEW_EVENT_ITEM_UNREALIZED,
                                       _unrealized_track, &realized),
                efl_ui_collection_view_factory_set(efl_added, fl),
                efl_ui_view_model_set(efl_added, model));
   efl_content_set(win, lv);
   get_me_to_those_events(lv);
   for (i = 0; i < 50; i++)
     {
        ecore_main_loop_iterate();
     }

   /* Taken out from the last, then new ones appended. */
   for (i = 2; i >= 0; i--)
     {
        efl_model_child_del(model, rows[i]);
     }
   for (i = 0; i < 3; i++)
     {
        snprintf(text, sizeof(text), "Row %d", 10 + i);
        eina_value_set(&v, text);
        efl_model_property_set(efl_model_child_add(model), "title", &v);
     }
   eina_value_flush(&v);
   for (i = 0; i < 100; i++)
     {
        ecore_main_loop_iterate();
     }

   for (i = 0; i < 3; i++)
     {
        snprintf(text, sizeof(text), "Row %d", 10 + i);
        item = _realized_row(realized, i);
        ck_assert_ptr_nonnull(item);
        ck_assert_str_eq(efl_text_get(item), text);
     }

   efl_event_callback_del(lv, EFL_UI_COLLECTION_VIEW_EVENT_ITEM_REALIZED,
                          _realized_track, &realized);
   efl_event_callback_del(lv, EFL_UI_COLLECTION_VIEW_EVENT_ITEM_UNREALIZED,
                          _unrealized_track, &realized);
   eina_list_free(realized);
}
EFL_END_TEST

/* The selection mode is kept by a model of the chain, which only exists once
   a model has been set: asked for first, it must still be there. */
EFL_START_TEST(test_efl_ui_collection_view_select_mode_before_model)
{
   Efl_Ui_Win *win;
   Efl_Ui_List_View *lv;
   Efl_Ui_Factory *fl;

   win = win_add(NULL, "win", EFL_UI_WIN_TYPE_BASIC);
   fl = efl_add(EFL_UI_LAYOUT_FACTORY_CLASS, win,
                efl_ui_property_bind(efl_added, "text", "title"),
                efl_ui_widget_factory_item_class_set(efl_added, EFL_UI_LIST_DEFAULT_ITEM_CLASS));
   lv = efl_add(EFL_UI_LIST_VIEW_CLASS, win,
                efl_ui_multi_selectable_select_mode_set(efl_added, EFL_UI_SELECT_MODE_MULTI),
                efl_ui_collection_view_factory_set(efl_added, fl));
   ck_assert_int_eq(efl_ui_multi_selectable_select_mode_get(lv), EFL_UI_SELECT_MODE_MULTI);

   efl_ui_view_model_set(lv, _make_model(win));
   ck_assert_int_eq(efl_ui_multi_selectable_select_mode_get(lv), EFL_UI_SELECT_MODE_MULTI);

   efl_ui_view_model_set(lv, _make_model(win));
   ck_assert_int_eq(efl_ui_multi_selectable_select_mode_get(lv), EFL_UI_SELECT_MODE_MULTI);
}
EFL_END_TEST

static void
_count_err_cri(const Eina_Log_Domain *d, Eina_Log_Level level,
               const char *file, const char *fnc, int line,
               const char *fmt, void *data, va_list args)
{
   int *n = data;

   if (level <= EINA_LOG_LEVEL_ERR) (*n)++;
   eina_log_print_cb_stderr(d, level, file, fnc, line, fmt, NULL, args);
}

// Rows are as high as their index says, set when the factory has built the item
// and before the view has it: odd rows 40, even rows 90.
static void
_row_height_on_build(void *data EINA_UNUSED, const Efl_Event *ev)
{
   Efl_Gfx_Entity *item = ev->info;
   Efl_Model *m = efl_ui_view_model_get(item);

   if (!m) return;
   efl_gfx_hint_size_min_set(item,
                             EINA_SIZE2D(0, (efl_composite_model_index_get(m) % 2) ? 40 : 90));
}

static Eina_Value
_zero_sizes(Efl_Model *model EINA_UNUSED, void *data EINA_UNUSED, const Eina_Value v)
{
   Efl_Model *child;
   unsigned int i, len;

   EINA_VALUE_ARRAY_FOREACH(&v, len, i, child)
     {
        Eina_Value zero = eina_value_uint_init(0);

        // (a size of 0x0 that was measured, which is not a row nobody measured)
        efl_model_property_set(child, "self.width", &zero);
        efl_model_property_set(child, "self.height", &zero);
     }
   return v;
}

static Efl_Ui_List_View *
_exact_list(Efl_Ui_Win *win, Efl_Model **sizes)
{
   Efl_Model *model = _make_model(win);
   Efl_Ui_Factory *fl;
   Efl_Ui_List_View *lv;

   *sizes = efl_add(EFL_UI_EXACT_MODEL_CLASS, win, efl_ui_view_model_set(efl_added, model));
   fl = efl_add(EFL_UI_WIDGET_FACTORY_CLASS, win,
                efl_ui_widget_factory_item_class_set(efl_added, EFL_UI_LIST_PLACEHOLDER_ITEM_CLASS),
                efl_event_callback_add(efl_added, EFL_UI_FACTORY_EVENT_ITEM_BUILDING,
                                       _row_height_on_build, NULL));
   lv = efl_add(EFL_UI_LIST_VIEW_CLASS, win,
                efl_ui_collection_view_factory_set(efl_added, fl),
                efl_ui_view_model_set(efl_added, *sizes));
   efl_content_set(win, lv);
   return lv;
}

/* A list over an Exact_Model measures its rows, each its own height. An
   unmeasured row is not ready (EAGAIN), not a size of 0. */
EFL_START_TEST(test_efl_ui_collection_view_exact_model)
{
   Efl_Ui_Win *win;
   Efl_Model *sizes;
   Efl_Ui_List_View *lv;
   Eina_List *realized = NULL, *l;
   Efl_Gfx_Entity *item;
   int errors = 0, i, tall = 0, short_ = 0;

   win = win_add(NULL, "win", EFL_UI_WIN_TYPE_BASIC);
   efl_gfx_entity_size_set(win, EINA_SIZE2D(200, 100));
   eina_log_print_cb_set(_count_err_cri, &errors);

   lv = _exact_list(win, &sizes);
   efl_event_callback_add(lv, EFL_UI_COLLECTION_VIEW_EVENT_ITEM_REALIZED,
                          _realized_track, &realized);
   efl_event_callback_add(lv, EFL_UI_COLLECTION_VIEW_EVENT_ITEM_UNREALIZED,
                          _unrealized_track, &realized);
   get_me_to_those_events(lv);
   for (i = 0; i < 50; i++)
     {
        ecore_main_loop_iterate();
     }

   ck_assert_int_ge(eina_list_count(realized), 2);
   EINA_LIST_FOREACH(realized, l, item)
     {
        Eina_Rect g = efl_gfx_entity_geometry_get(item);

        // the one kept for the focus is not placed
        if (g.h == 0) continue;
        if (g.h >= 90) tall++;
        else if (g.h >= 40) short_++;
        ck_assert_int_ge(g.h, 40);
     }
   ck_assert_int_gt(tall, 0);
   ck_assert_int_gt(short_, 0);

   eina_log_print_cb_set(eina_log_print_cb_stderr, NULL);
   ck_assert_int_eq(errors, 0);
   efl_event_callback_del(lv, EFL_UI_COLLECTION_VIEW_EVENT_ITEM_REALIZED,
                          _realized_track, &realized);
   efl_event_callback_del(lv, EFL_UI_COLLECTION_VIEW_EVENT_ITEM_UNREALIZED,
                          _unrealized_track, &realized);
   eina_list_free(realized);
}
EFL_END_TEST

/* Every row measured as 0x0: no size is known to the position manager, which
   must lay out nothing (it used to crash on its missing cache), and the view
   must not complain about a size that is nothing. */
EFL_START_TEST(test_efl_ui_collection_view_exact_model_zero_size)
{
   Efl_Ui_Win *win;
   Efl_Model *sizes;
   Efl_Ui_List_View *lv;
   int errors = 0, i;

   win = win_add(NULL, "win", EFL_UI_WIN_TYPE_BASIC);
   efl_gfx_entity_size_set(win, EINA_SIZE2D(200, 300));
   eina_log_print_cb_set(_count_err_cri, &errors);

   lv = _exact_list(win, &sizes);
   efl_future_then(sizes, efl_model_children_slice_get(sizes, 0, NUM_ITEMS),
                   .success = _zero_sizes);
   for (i = 0; i < 20; i++)
     {
        ecore_main_loop_iterate();
     }
   get_me_to_those_events(lv);
   for (i = 0; i < 50; i++)
     {
        ecore_main_loop_iterate();
     }

   ck_assert(efl_alive_get(lv));

   eina_log_print_cb_set(eina_log_print_cb_stderr, NULL);
   ck_assert_int_eq(errors, 0);
}
EFL_END_TEST

/* The sizes the position manager caches and the sizes it places must agree
   while the window resizes the view. */
EFL_START_TEST(test_efl_ui_collection_view_sizes_agree)
{
   Efl_Ui_Win *win;
   Efl_Model *model;
   Efl_Ui_List_View *lv;
   Efl_Ui_Factory *fl;
   int disagreements = 0;

   win = win_add(NULL, "win", EFL_UI_WIN_TYPE_BASIC);
   efl_gfx_entity_size_set(win, EINA_SIZE2D(100, 100));

   eina_log_print_cb_set(_count_size_disagreements, &disagreements);

   model = _make_model(win);
   fl = efl_add(EFL_UI_LAYOUT_FACTORY_CLASS, win,
                efl_ui_property_bind(efl_added, "text", "title"),
                efl_ui_widget_factory_item_class_set(efl_added, EFL_UI_LIST_DEFAULT_ITEM_CLASS));
   lv = efl_add(EFL_UI_LIST_VIEW_CLASS, win,
                efl_ui_collection_view_factory_set(efl_added, fl),
                efl_ui_view_model_set(efl_added, model));
   efl_content_set(win, lv);

   get_me_to_those_events(lv);

   eina_log_print_cb_set(eina_log_print_cb_stderr, NULL);
   ck_assert_int_eq(disagreements, 0);
}
EFL_END_TEST

// A manager set again stays: it is not the one that is replaced.
EFL_START_TEST(efl_ui_collection_view_manager_twice)
{
   Efl_Ui_Win *win = win_add(NULL, "win", EFL_UI_WIN_TYPE_BASIC);
   Eo *cv = efl_add(EFL_UI_COLLECTION_VIEW_CLASS, win);
   Eo *grid = efl_new(EFL_UI_POSITION_MANAGER_GRID_CLASS);

   efl_ref(grid);
   efl_ui_collection_view_position_manager_set(cv, grid);
   efl_ui_collection_view_position_manager_set(cv, grid);
   ck_assert(!efl_invalidated_get(grid));
   ck_assert_ptr_eq(efl_ui_collection_view_position_manager_get(cv), grid);
   efl_del(cv);
   ck_assert(efl_invalidated_get(grid));
   efl_unref(grid);
}
EFL_END_TEST

/* A Homogeneous_Model is made from C and given to a view as its model. */
EFL_START_TEST(test_efl_ui_collection_view_homogeneous_model)
{
   Efl_Ui_Win *win;
   Efl_Model *model, *sizes;
   Efl_Ui_List_View *lv;
   Efl_Ui_Factory *fl;
   Eina_Iterator *it;
   Eina_Value *w = NULL;
   const char *property;
   Eina_Bool has_width = EINA_FALSE;
   int width = 0, i;

   win = win_add(NULL, "win", EFL_UI_WIN_TYPE_BASIC);
   efl_gfx_entity_size_set(win, EINA_SIZE2D(100, 100));

   model = _make_model(win);
   sizes = efl_add(EFL_UI_HOMOGENEOUS_MODEL_CLASS, win,
                   efl_ui_view_model_set(efl_added, model));
   ck_assert_ptr_ne(sizes, NULL);
   fl = efl_add(EFL_UI_LAYOUT_FACTORY_CLASS, win,
                efl_ui_property_bind(efl_added, "text", "title"),
                efl_ui_widget_factory_item_class_set(efl_added, EFL_UI_LIST_DEFAULT_ITEM_CLASS));
   lv = efl_add(EFL_UI_LIST_VIEW_CLASS, win,
                efl_ui_collection_view_factory_set(efl_added, fl),
                efl_ui_view_model_set(efl_added, sizes));
   efl_content_set(win, lv);

   get_me_to_those_events(lv);
   for (i = 0; i < 20; i++)
     {
        ecore_main_loop_iterate();
     }

   ck_assert_ptr_eq(efl_ui_view_model_get(lv), sizes);
   ck_assert_uint_eq(efl_model_children_count_get(sizes), NUM_ITEMS);

   // It says it has sizes, which the view looks for to know it need not
   // make a size model of its own over it.
   it = efl_model_properties_get(sizes);
   EINA_ITERATOR_FOREACH(it, property)
     {
        if (property && !strcmp(property, "item.width")) has_width = EINA_TRUE;
     }
   eina_iterator_free(it);
   ck_assert(has_width);

   // Nothing told it a size: the view measures a realized item and gives
   // the answer back to the model it was given.
   for (i = 0; i < 2000; i++)
     {
        w = efl_model_property_get(sizes, "item.width");
        if (w && (eina_value_type_get(w) != EINA_VALUE_TYPE_ERROR)) break;
        eina_value_free(w);
        w = NULL;
        ecore_main_loop_iterate();
     }
   ck_assert(!!w);
   ck_assert(eina_value_int_convert(w, &width));
   eina_value_free(w);
   ck_assert_int_gt(width, 0);
}
EFL_END_TEST

static void
_view_item_clicked_cb(void *data, const Efl_Event *ev)
{
   const Efl_Ui_Item_Clickable_Clicked *c = ev->info;
   int *n = data;

   ck_assert_int_eq(c->clicked.button, 1);
   ck_assert_ptr_ne(c->item, NULL);
   (*n)++;
}

static void
_view_item_pressed_cb(void *data, const Efl_Event *ev)
{
   const Efl_Ui_Item_Clickable_Pressed *p = ev->info;
   int *n = data;

   ck_assert_int_eq(p->button, 1);
   ck_assert_ptr_ne(p->item, NULL);
   (*n)++;
}

/* An application may raise the click or the press of an item with nothing
   attached: the view takes the primary button. */
EFL_START_TEST(test_efl_ui_collection_view_item_events_without_info)
{
   Eina_List *realized = NULL;
   Efl_Ui_Win *win;
   Efl_Ui_List_View *lv;
   Efl_Ui_Factory *fl;
   Eo *item;
   int clicked = 0, pressed = 0, i;

   win = win_add(NULL, "win", EFL_UI_WIN_TYPE_BASIC);
   efl_gfx_entity_size_set(win, EINA_SIZE2D(100, 100));
   fl = efl_add(EFL_UI_LAYOUT_FACTORY_CLASS, win,
                efl_ui_property_bind(efl_added, "text", "title"),
                efl_ui_widget_factory_item_class_set(efl_added, EFL_UI_LIST_DEFAULT_ITEM_CLASS));
   lv = efl_add(EFL_UI_LIST_VIEW_CLASS, win,
                efl_event_callback_add(efl_added, EFL_UI_COLLECTION_VIEW_EVENT_ITEM_REALIZED,
                                       _realized_track, &realized),
                efl_ui_collection_view_factory_set(efl_added, fl),
                efl_ui_view_model_set(efl_added, _make_model(win)));
   efl_content_set(win, lv);
   efl_event_callback_add(lv, EFL_UI_EVENT_ITEM_CLICKED, _view_item_clicked_cb, &clicked);
   efl_event_callback_add(lv, EFL_UI_EVENT_ITEM_PRESSED, _view_item_pressed_cb, &pressed);
   get_me_to_those_events(lv);
   for (i = 0; i < 50; i++)
     {
        ecore_main_loop_iterate();
     }

   item = _realized_row(realized, 0);
   ck_assert_ptr_nonnull(item);
   efl_event_callback_call(item, EFL_INPUT_EVENT_CLICKED, NULL);
   efl_event_callback_call(item, EFL_INPUT_EVENT_PRESSED, NULL);
   ck_assert_int_eq(clicked, 1);
   ck_assert_int_eq(pressed, 1);

   efl_event_callback_del(lv, EFL_UI_COLLECTION_VIEW_EVENT_ITEM_REALIZED,
                          _realized_track, &realized);
   eina_list_free(realized);
}
EFL_END_TEST

/* A list between two buttons, its rows more than it shows. */
static Eo *
_list_between(Eo **above, Eo **below)
{
   Efl_Ui_Win *win;
   Efl_Ui_Factory *fl;
   Eo *box, *lv;
   int i;

   win = win_add(NULL, "win", EFL_UI_WIN_TYPE_BASIC);
   efl_gfx_entity_size_set(win, EINA_SIZE2D(200, 300));
   box = efl_add(EFL_UI_BOX_CLASS, win);
   efl_content_set(win, box);
   *above = efl_add(EFL_UI_BUTTON_CLASS, box, efl_pack_end(box, efl_added));
   fl = efl_add(EFL_UI_LAYOUT_FACTORY_CLASS, win,
                efl_ui_property_bind(efl_added, "text", "title"),
                efl_ui_widget_factory_item_class_set(efl_added, EFL_UI_LIST_DEFAULT_ITEM_CLASS));
   lv = efl_add(EFL_UI_LIST_VIEW_CLASS, box,
                efl_ui_collection_view_factory_set(efl_added, fl),
                efl_ui_view_model_set(efl_added, _make_model(win)),
                efl_pack_end(box, efl_added));
   *below = efl_add(EFL_UI_BUTTON_CLASS, box, efl_pack_end(box, efl_added));
   get_me_to_those_events(lv);
   for (i = 0; i < 50; i++)
     {
        ecore_main_loop_iterate();
     }
   return lv;
}

/* Moves focus as a key would, then lets the view catch up, and says which
   row has focus in the view, or -1 if it is not in the view. */
static int
_tab(Eo *lv, Efl_Ui_Focus_Direction direction)
{
   Eo *win = efl_provider_find(lv, EFL_UI_WIN_CLASS);
   Eo *focus;
   int i;

   efl_ui_focus_manager_move(win, direction);
   for (i = 0; i < 10; i++)
     {
        ecore_main_loop_iterate();
     }
   focus = efl_ui_focus_manager_focus_get(win);
   if (focus != lv) return -1;
   focus = efl_ui_focus_manager_focus_get(lv);
   return efl_isa(focus, EFL_UI_ITEM_CLASS) ? _item_row(focus) : -1;
}

/* Tab enters a list on its first row and goes through its rows in order,
   past those it shows -- the list scrolls to each -- and out of it after
   the last one. Shift+Tab on the first row leaves it upwards. */
EFL_START_TEST(test_efl_ui_collection_view_tab)
{
   Eo *win, *lv, *above, *below;
   int i;

   lv = _list_between(&above, &below);
   win = efl_provider_find(lv, EFL_UI_WIN_CLASS);
   efl_ui_focus_util_focus(above);
   ck_assert_int_eq(_tab(lv, EFL_UI_FOCUS_DIRECTION_NEXT), 0);
   ck_assert_int_eq(_tab(lv, EFL_UI_FOCUS_DIRECTION_PREVIOUS), -1);
   ck_assert_ptr_eq(efl_ui_focus_manager_focus_get(win), above);

   for (i = 0; i < NUM_ITEMS; i++)
     {
        ck_assert_int_eq(_tab(lv, EFL_UI_FOCUS_DIRECTION_NEXT), i);
        if (i == 10) ck_assert_int_gt(efl_ui_scrollable_content_pos_get(lv).y, 0);
     }
   ck_assert_int_eq(_tab(lv, EFL_UI_FOCUS_DIRECTION_NEXT), -1);
   ck_assert_ptr_eq(efl_ui_focus_manager_focus_get(win), below);
}
EFL_END_TEST

/* Shift+Tab from below enters a list on its last row, though it shows the
   top, and goes back through the rows; focus coming back to the list
   comes back to the row it left. */
EFL_START_TEST(test_efl_ui_collection_view_shift_tab)
{
   Eo *lv, *above, *below;
   int i;

   lv = _list_between(&above, &below);
   efl_ui_focus_util_focus(below);
   for (i = NUM_ITEMS - 1; i > NUM_ITEMS - 10; i--)
     {
        ck_assert_int_eq(_tab(lv, EFL_UI_FOCUS_DIRECTION_PREVIOUS), i);
     }

   efl_ui_focus_util_focus(above);
   ck_assert_int_eq(_tab(lv, EFL_UI_FOCUS_DIRECTION_NEXT), NUM_ITEMS - 9);
}
EFL_END_TEST

static void
_manager_del(void *data, const Efl_Event *ev EINA_UNUSED)
{
   *(Eina_Bool *)data = EINA_TRUE;
}

// The ownership of a position manager is passed to the view: it holds the
// only ref once it is set, and ends the manager it replaces.
EFL_START_TEST(efl_ui_collection_view_manager_ownership)
{
   Eina_Bool gone = EINA_FALSE, gone2 = EINA_FALSE;
   Efl_Ui_Win *win = win_add(NULL, "win", EFL_UI_WIN_TYPE_BASIC);
   Eo *cv = efl_add(EFL_UI_COLLECTION_VIEW_CLASS, win);
   Eo *grid = efl_new(EFL_UI_POSITION_MANAGER_GRID_CLASS);
   Eo *list = efl_new(EFL_UI_POSITION_MANAGER_LIST_CLASS);

   efl_event_callback_add(grid, EFL_EVENT_DEL, _manager_del, &gone);
   efl_event_callback_add(list, EFL_EVENT_DEL, _manager_del, &gone2);
   efl_ui_collection_view_position_manager_set(cv, grid);
   ck_assert_ptr_eq(efl_ui_collection_view_position_manager_get(cv), grid);
   ck_assert_int_eq(efl_ref_count(grid), 1);
   efl_ui_collection_view_position_manager_set(cv, list);
   ck_assert(gone);
   ck_assert(!gone2);
   efl_del(cv);
   ck_assert(gone2);
}
EFL_END_TEST

/* The last row of a model bound to a live view is removed: the view
   must not find an item or an entity left for a row that is gone. */
EFL_START_TEST(test_efl_ui_collection_view_remove_last_row)
{
   Efl_Ui_Win *win;
   Efl_Model *model, *rows[NUM_ITEMS];
   int total = 4;
   Efl_Ui_List_View *lv;
   Efl_Ui_Factory *fl;
   Eina_Value v;
   int errors = 0, i, n;

   win = win_add(NULL, "win", EFL_UI_WIN_TYPE_BASIC);
   efl_gfx_entity_size_set(win, EINA_SIZE2D(100, 100));

   model = efl_add(EFL_GENERIC_MODEL_CLASS, win);
   eina_value_setup(&v, EINA_VALUE_TYPE_STRING);
   eina_value_set(&v, "Row");
   for (i = 0; i < total; i++)
     {
        rows[i] = efl_model_child_add(model);
        efl_model_property_set(rows[i], "title", &v);
     }
   eina_value_flush(&v);
   fl = efl_add(EFL_UI_LAYOUT_FACTORY_CLASS, win,
                efl_ui_property_bind(efl_added, "text", "title"),
                efl_ui_widget_factory_item_class_set(efl_added, EFL_UI_LIST_DEFAULT_ITEM_CLASS));
   lv = efl_add(EFL_UI_LIST_VIEW_CLASS, win,
                efl_ui_collection_view_factory_set(efl_added, fl),
                efl_ui_view_model_set(efl_added, model));
   efl_content_set(win, lv);
   get_me_to_those_events(lv);
   for (i = 0; i < 50; i++)
     {
        ecore_main_loop_iterate();
     }

   eina_log_print_cb_set(_count_err_cri, &errors);
   for (n = 0; n < 3; n++)
     {
        efl_model_child_del(model, rows[total - 1 - n]);
        for (i = 0; i < 50; i++)
          {
             ecore_main_loop_iterate();
          }
     }
   eina_log_print_cb_set(eina_log_print_cb_stderr, NULL);
   ck_assert_int_eq(errors, 0);
}
EFL_END_TEST

/* A list of rows measured, jumped far away and back, beyond the rows kept
   around the visible ones: the rows let go of are still of the size they
   were measured at, and the position manager must not be told otherwise
   (it logs the sizes changing under it) nor place the rows by it. */
EFL_START_TEST(test_efl_ui_collection_view_exact_model_jump)
{
   Efl_Ui_Win *win;
   Efl_Model *sizes;
   Efl_Ui_List_View *lv;
   Eina_List *realized = NULL;
   Efl_Gfx_Entity *item;
   Eina_Rect g;
   int errors = 0, i, y = 0;
   const int target = 40;

   win = win_add(NULL, "win", EFL_UI_WIN_TYPE_BASIC);
   efl_gfx_entity_size_set(win, EINA_SIZE2D(200, 100));
   eina_log_print_cb_set(_count_err_cri, &errors);

   lv = _exact_list(win, &sizes);
   efl_event_callback_add(lv, EFL_UI_COLLECTION_VIEW_EVENT_ITEM_REALIZED,
                          _realized_track, &realized);
   efl_event_callback_add(lv, EFL_UI_COLLECTION_VIEW_EVENT_ITEM_UNREALIZED,
                          _unrealized_track, &realized);
   get_me_to_those_events(lv);
   for (i = 0; i < 50; i++)
     {
        ecore_main_loop_iterate();
     }

   for (i = 0; i < target; i++)
     {
        y += (i % 2) ? 40 : 90;
     }
   efl_ui_scrollable_content_pos_set(lv, EINA_POSITION2D(0, y));
   wait_timer(0.5);
   efl_ui_scrollable_content_pos_set(lv, EINA_POSITION2D(0, 0));
   wait_timer(0.5);
   efl_ui_scrollable_content_pos_set(lv, EINA_POSITION2D(0, y));
   wait_timer(0.5);

   eina_log_print_cb_set(eina_log_print_cb_stderr, NULL);
   ck_assert_int_eq(errors, 0);

   item = _realized_row(realized, target);
   ck_assert_ptr_nonnull(item);
   g = efl_gfx_entity_geometry_get(item);
   ck_assert_int_eq(g.h, 90);
   ck_assert_int_eq(g.y, efl_gfx_entity_position_get(lv).y);

   efl_event_callback_del(lv, EFL_UI_COLLECTION_VIEW_EVENT_ITEM_REALIZED,
                          _realized_track, &realized);
   efl_event_callback_del(lv, EFL_UI_COLLECTION_VIEW_EVENT_ITEM_UNREALIZED,
                          _unrealized_track, &realized);
   eina_list_free(realized);
}
EFL_END_TEST

/* The cache of the list position manager follows rows that are added,
   removed or measured again without being built again: each time it is
   needed, the place of every row must be where the sizes of the rows before
   say it is, whatever happened in between. */
#define CACHE_MAX_ROWS 400
static int _cache_sizes[CACHE_MAX_ROWS];
static unsigned int _cache_count = 0;
// The most rows the provider fills in a call
static unsigned int _cache_fill_cap = 0xffffffff;

static Efl_Ui_Position_Manager_Size_Batch_Result
_cache_size_cb(void *data EINA_UNUSED, Efl_Ui_Position_Manager_Size_Call_Config conf,
               Eina_Rw_Slice memory)
{
   Efl_Ui_Position_Manager_Size_Batch_Entity *sizes = memory.mem;
   Efl_Ui_Position_Manager_Size_Batch_Result result = { 0 };
   unsigned int i, limit = conf.range.end_id - conf.range.start_id;

   for (i = 0;
        (i < limit) && (i < _cache_fill_cap) && ((conf.range.start_id + i) < _cache_count);
        i++)
     {
        sizes[i].size = EINA_SIZE2D(20, _cache_sizes[conf.range.start_id + i]);
        sizes[i].element_depth = 0;
        sizes[i].depth_leader = EINA_FALSE;
     }
   result.filled_items = i;
   return result;
}

static Efl_Gfx_Entity *_cache_dummy = NULL;

// Every item is shown by the same object, which is all there is to move
static Efl_Ui_Position_Manager_Object_Batch_Result
_cache_object_cb(void *data EINA_UNUSED, Efl_Ui_Position_Manager_Request_Range range,
                 Eina_Rw_Slice memory)
{
   Efl_Ui_Position_Manager_Object_Batch_Entity *entities = memory.mem;
   Efl_Ui_Position_Manager_Object_Batch_Result result = { 0 };
   unsigned int i, limit = range.end_id - range.start_id;

   for (i = 0; (i < limit) && range.start_id + i < _cache_count; i++)
     {
        entities[i].entity = _cache_dummy;
        entities[i].depth_leader = EINA_FALSE;
     }
   result.filled_items = i;
   return result;
}

static int _cache_content_h = -1;

static void
_cache_content_size(void *data EINA_UNUSED, const Efl_Event *ev)
{
   Eina_Size2D *size = ev->info;

   _cache_content_h = size->h;
}

static void
_cache_places_check(Efl_Ui_Position_Manager_Entity *manager)
{
   unsigned int i;
   int y = 0;

   // The job that follows the changes gives the size of the content
   ecore_main_loop_iterate();

   for (i = 0; i < _cache_count; i++)
     {
        Eina_Rect g = efl_ui_position_manager_entity_position_single_item(manager, i);

        ck_assert_int_eq(g.y, y);
        ck_assert_int_eq(g.h, _cache_sizes[i]);
        y += _cache_sizes[i];
     }
   ck_assert_int_eq(_cache_content_h, y);
}

EFL_START_TEST(test_efl_ui_position_manager_list_cache_follows)
{
   Efl_Ui_Win *win;
   Efl_Ui_Position_Manager_Entity *manager;
   unsigned int seed = 7, i, step;

   win = win_add(NULL, "win", EFL_UI_WIN_TYPE_BASIC);
   _cache_dummy = efl_add(EFL_UI_BUTTON_CLASS, win);
   manager = efl_add(EFL_UI_POSITION_MANAGER_LIST_CLASS, win,
                     efl_ui_layout_orientation_set(efl_added, EFL_UI_LAYOUT_ORIENTATION_VERTICAL));
   efl_event_callback_add(manager, EFL_UI_POSITION_MANAGER_ENTITY_EVENT_CONTENT_SIZE_CHANGED,
                          _cache_content_size, NULL);
   _cache_content_h = -1;
   for (i = 0; i < 6; i++)
     {
        _cache_sizes[i] = 10 * (i + 1);
     }
   _cache_count = 6;
   efl_ui_position_manager_data_access_v1_data_access_set(manager, win, NULL, _cache_object_cb, NULL,
                                                          NULL, _cache_size_cb, NULL, _cache_count);
   efl_ui_position_manager_entity_viewport_set(manager, EINA_RECT(0, 0, 100, 1));
   _cache_places_check(manager);

   for (step = 0; step < 600; step++)
     {
        unsigned int idx;

        seed = seed * 1103515245 + 12345;
        idx = (seed >> 8) % 8;
        seed = seed * 1103515245 + 12345;
        switch ((seed >> 8) % 6)
          {
           case 0: /* added in front */
           case 1: /* added somewhere */
           case 2: /* appended */
             {
                unsigned int at;

                if (_cache_count >= CACHE_MAX_ROWS - 1) break;
                at = ((seed >> 8) % 6 == 0) ? 0 :
                  (((seed >> 8) % 6 == 2) ? _cache_count : (idx % (_cache_count + 1)));
                memmove(_cache_sizes + at + 1, _cache_sizes + at,
                        (_cache_count - at) * sizeof (int));
                _cache_sizes[at] = 5 + (seed >> 16) % 60;
                _cache_count++;
                efl_ui_position_manager_entity_item_added(manager, at, NULL);
                break;
             }
           case 3: /* removed */
             {
                unsigned int at;

                if (_cache_count < 2) break;
                at = idx % _cache_count;
                memmove(_cache_sizes + at, _cache_sizes + at + 1,
                        (_cache_count - at - 1) * sizeof (int));
                _cache_count--;
                efl_ui_position_manager_entity_item_removed(manager, at, NULL);
                break;
             }
           default: /* measured again */
             {
                unsigned int start = idx % _cache_count;
                unsigned int end = MIN(start + (seed >> 12) % 3, _cache_count - 1);
                unsigned int j;

                for (j = start; j <= end; j++)
                  {
                     _cache_sizes[j] = 5 + (seed >> (4 + j % 8)) % 60;
                  }
                efl_ui_position_manager_entity_item_size_changed(manager, start, end);
                break;
             }
          }
        // Not after every change: some are to pile up
        if (!((seed >> 20) % 3)) _cache_places_check(manager);
     }
   _cache_places_check(manager);
   efl_del(manager);
}
EFL_END_TEST

/* A provider may fill fewer rows than it was asked for: those it did not fill
   are asked for again, and none is placed by what the buffer held before. */
EFL_START_TEST(test_efl_ui_position_manager_list_partial_fill)
{
   Efl_Ui_Win *win;
   Efl_Ui_Position_Manager_Entity *manager;
   unsigned int i;

   win = win_add(NULL, "win", EFL_UI_WIN_TYPE_BASIC);
   _cache_dummy = efl_add(EFL_UI_BUTTON_CLASS, win);
   manager = efl_add(EFL_UI_POSITION_MANAGER_LIST_CLASS, win,
                     efl_ui_layout_orientation_set(efl_added, EFL_UI_LAYOUT_ORIENTATION_VERTICAL));
   efl_event_callback_add(manager, EFL_UI_POSITION_MANAGER_ENTITY_EVENT_CONTENT_SIZE_CHANGED,
                          _cache_content_size, NULL);
   _cache_content_h = -1;
   _cache_count = 250;
   for (i = 0; i < _cache_count; i++)
     {
        _cache_sizes[i] = 10 + (i * 13) % 50;
     }
   _cache_fill_cap = 7;
   efl_ui_position_manager_data_access_v1_data_access_set(manager, win, NULL, _cache_object_cb, NULL,
                                                          NULL, _cache_size_cb, NULL, _cache_count);
   efl_ui_position_manager_entity_viewport_set(manager, EINA_RECT(0, 0, 100, 1));
   _cache_places_check(manager);

   // The changes asked for are filled by as few as well
   _cache_sizes[20] = 77;
   _cache_sizes[21] = 78;
   _cache_sizes[22] = 79;
   efl_ui_position_manager_entity_item_size_changed(manager, 20, 22);
   _cache_places_check(manager);
   for (i = 0; i < 30; i++)
     {
        memmove(_cache_sizes + 5, _cache_sizes + 4, (_cache_count - 4) * sizeof (int));
        _cache_sizes[5] = 30 + i;
        _cache_count++;
        efl_ui_position_manager_entity_item_added(manager, 5, NULL);
     }
   _cache_places_check(manager);

   _cache_fill_cap = 0xffffffff;
   efl_del(manager);
}
EFL_END_TEST

/* A provider that gives one size to the rows nobody measured (the size of
   the view, then the shared size of a Homogeneous_Model once there is one)
   changes the size of rows it was not told about when one of them is
   measured: the first one reported was the last, and the others were still
   cached at what they had been. The cache must find that out, or items are
   placed by sizes that are not theirs. */
EFL_START_TEST(test_efl_ui_position_manager_list_cache_shared_size)
{
   Efl_Ui_Win *win;
   Efl_Ui_Position_Manager_Entity *manager;
   unsigned int i;

   win = win_add(NULL, "win", EFL_UI_WIN_TYPE_BASIC);
   _cache_dummy = efl_add(EFL_UI_BUTTON_CLASS, win);
   manager = efl_add(EFL_UI_POSITION_MANAGER_LIST_CLASS, win,
                     efl_ui_layout_orientation_set(efl_added, EFL_UI_LAYOUT_ORIENTATION_VERTICAL));
   efl_event_callback_add(manager, EFL_UI_POSITION_MANAGER_ENTITY_EVENT_CONTENT_SIZE_CHANGED,
                          _cache_content_size, NULL);
   _cache_content_h = -1;
   _cache_count = 3;
   for (i = 0; i < _cache_count; i++)
     {
        _cache_sizes[i] = 384;
     }
   efl_ui_position_manager_data_access_v1_data_access_set(manager, win, NULL, _cache_object_cb, NULL,
                                                          NULL, _cache_size_cb, NULL, _cache_count);
   efl_ui_position_manager_entity_viewport_set(manager, EINA_RECT(0, 0, 100, 1));
   _cache_places_check(manager);

   // Every row is now of the shared size, and the manager is told of the last
   for (i = 0; i < _cache_count; i++)
     {
        _cache_sizes[i] = 23;
     }
   efl_ui_position_manager_entity_item_size_changed(manager, 2, 2);
   _cache_places_check(manager);

   efl_del(manager);
}
EFL_END_TEST

/* The list position manager, told of every change by a provider whose answers
   are not the same from one time to the next, must place every item where a
   manager that is given the final items places it. The provider here gives the
   rows that nobody measured the average of those that were (so that measuring
   a row changes the size of all the others, which is not told), and fills only
   a few rows of what it is asked for at a time. Rows are changed anywhere,
   added, removed down to none, and measured past the end of the list. */
#define DIFF_MAX_ROWS 3000
static unsigned int _diff_measured[DIFF_MAX_ROWS]; // 0: not measured
static unsigned int _diff_count = 0;
static unsigned int _diff_cap = 7;
// The rows the provider was asked for, all told
static unsigned int _diff_asked = 0;
// Whether the rows nobody measured get the average (and are said to be an estimate), or 40
static Eina_Bool _diff_estimates = EINA_TRUE;

static Efl_Ui_Position_Manager_Size_Batch_Result
_diff_size_cb(void *data EINA_UNUSED, Efl_Ui_Position_Manager_Size_Call_Config conf,
              Eina_Rw_Slice memory)
{
   Efl_Ui_Position_Manager_Size_Batch_Entity *sizes = memory.mem;
   Efl_Ui_Position_Manager_Size_Batch_Result result = { 0 };
   unsigned int i, limit = conf.range.end_id - conf.range.start_id;
   unsigned int sum = 0, seen = 0, average;

   for (i = 0; i < _diff_count; i++)
     {
        if (!_diff_measured[i]) continue;
        sum += _diff_measured[i];
        seen++;
     }
   average = (seen && _diff_estimates) ? sum / seen : 40;

   for (i = 0; (i < limit) && i < _diff_cap && conf.range.start_id + i < _diff_count; i++)
     {
        unsigned int measured = _diff_measured[conf.range.start_id + i];

        sizes[i].size = EINA_SIZE2D(20, measured ? measured : average);
        sizes[i].element_depth = 0;
        sizes[i].depth_leader = EINA_FALSE;
        if (!measured && _diff_estimates) result.estimated = EINA_TRUE;
     }
   result.filled_items = i;
   // (the estimate is the average: it is another when that is)
   if (_diff_estimates) result.estimate_version = average;
   _diff_asked += i;
   return result;
}

static Efl_Ui_Position_Manager_Entity *
_diff_manager_add(Efl_Ui_Win *win)
{
   Efl_Ui_Position_Manager_Entity *manager;

   manager = efl_add(EFL_UI_POSITION_MANAGER_LIST_CLASS, win,
                     efl_ui_layout_orientation_set(efl_added, EFL_UI_LAYOUT_ORIENTATION_VERTICAL));
   efl_ui_position_manager_data_access_v1_data_access_set(manager, win, NULL, _cache_object_cb, NULL,
                                                          NULL, _diff_size_cb, NULL, _diff_count);
   efl_ui_position_manager_entity_viewport_set(manager, EINA_RECT(0, 0, 100, 1));
   return manager;
}

static void
_diff_check(Efl_Ui_Win *win, Efl_Ui_Position_Manager_Entity *manager, unsigned int step)
{
   Efl_Ui_Position_Manager_Entity *fresh;
   unsigned int i;

   // The job that follows the changes
   ecore_main_loop_iterate();
   fresh = _diff_manager_add(win);
   for (i = 0; i < _diff_count; i++)
     {
        Eina_Rect a = efl_ui_position_manager_entity_position_single_item(manager, i);
        Eina_Rect b = efl_ui_position_manager_entity_position_single_item(fresh, i);

        ck_assert_msg(a.y == b.y, "step %u item %u: y %d against %d", step, i, a.y, b.y);
        ck_assert_msg(a.h == b.h, "step %u item %u: h %d against %d", step, i, a.h, b.h);
     }
   efl_del(fresh);
}

static void
_diff_run(unsigned int seed, unsigned int steps, unsigned int range)
{
   Efl_Ui_Win *win;
   Efl_Ui_Position_Manager_Entity *manager;
   unsigned int i, step;

   win = win_add(NULL, "win", EFL_UI_WIN_TYPE_BASIC);
   _cache_dummy = efl_add(EFL_UI_BUTTON_CLASS, win);
   _diff_count = 40;
   memset(_diff_measured, 0, sizeof (_diff_measured));
   for (i = 0; i < _diff_count; i += 5)
     {
        _diff_measured[i] = 10 + i;
     }
   manager = _diff_manager_add(win);
   _diff_check(win, manager, 0);

   for (step = 1; step <= steps; step++)
     {
        unsigned int idx, kind;

        seed = seed * 1103515245 + 12345;
        // Anywhere in the whole list
        idx = ((seed >> 8) & 0xffff) % (_diff_count ? _diff_count : 1);
        seed = seed * 1103515245 + 12345;
        kind = (seed >> 8) % 12;
        switch (kind)
          {
           case 0:
           case 1: /* added */
             {
                unsigned int at;

                if (_diff_count >= range) break;
                at = _diff_count ? (idx % (_diff_count + 1)) : 0;
                memmove(_diff_measured + at + 1, _diff_measured + at,
                        (_diff_count - at) * sizeof (unsigned int));
                _diff_measured[at] = (seed >> 20) % 3 ? 0 : 5 + (seed >> 16) % 60;
                _diff_count++;
                efl_ui_position_manager_entity_item_added(manager, at, NULL);
                break;
             }
           case 2:
           case 3: /* removed */
             {
                if (!_diff_count) break;
                memmove(_diff_measured + idx, _diff_measured + idx + 1,
                        (_diff_count - idx - 1) * sizeof (unsigned int));
                _diff_count--;
                efl_ui_position_manager_entity_item_removed(manager, idx, NULL);
                break;
             }
           case 4: /* all removed, then some come */
             {
                if ((seed >> 20) % 8) break;
                while (_diff_count)
                  {
                     _diff_count--;
                     efl_ui_position_manager_entity_item_removed(manager, _diff_count, NULL);
                  }
                _diff_check(win, manager, step);
                for (i = 0; i < 3; i++)
                  {
                     _diff_measured[i] = 0;
                     _diff_count++;
                     efl_ui_position_manager_entity_item_added(manager, i, NULL);
                  }
                break;
             }
           case 5: /* told of a change past the end */
             efl_ui_position_manager_entity_item_size_changed(manager, _diff_count + idx % 3,
                                                              _diff_count + 2 + idx % 3);
             break;
           default: /* measured, and only that row is told, wherever it is */
             if (!_diff_count) break;
             _diff_measured[idx] = 5 + (seed >> 16) % 60;
             efl_ui_position_manager_entity_item_size_changed(manager, idx, idx);
             break;
          }
        if (!((seed >> 20) % 4)) _diff_check(win, manager, step);
     }
   _diff_check(win, manager, step);
   efl_del(manager);
}

EFL_START_TEST(test_efl_ui_position_manager_list_differential)
{
   _diff_estimates = EINA_TRUE;
   _diff_run(3, 300, 40);
   // The long list: most of it is far from what changes
   _diff_run(17, 500, 400);
   // Answers that only change for the rows that were told of: the cache follows without asking for all
   _diff_estimates = EINA_FALSE;
   _diff_run(5, 400, 40);
   _diff_run(29, 600, 400);
}
EFL_END_TEST

/* An estimate that stays the same is not a reason to ask for every item again:
   a row that comes with the average as it was costs the rows that changed, and a
   row that is measured and changes the average makes all of them be asked
   again, and the result is what a manager that is given the final rows has. */
EFL_START_TEST(test_efl_ui_position_manager_list_estimate_stable)
{
   Efl_Ui_Win *win;
   Efl_Ui_Position_Manager_Entity *manager;
   const unsigned int rows = 600;
   unsigned int i;

   win = win_add(NULL, "win", EFL_UI_WIN_TYPE_BASIC);
   _cache_dummy = efl_add(EFL_UI_BUTTON_CLASS, win);
   _diff_estimates = EINA_TRUE;
   _diff_cap = 100;
   _diff_count = rows;
   memset(_diff_measured, 0, sizeof (_diff_measured));
   // (all as high as 20: the average is, whichever of them are measured)
   for (i = 0; i < _diff_count; i += 5)
     {
        _diff_measured[i] = 20;
     }
   manager = _diff_manager_add(win);
   _diff_check(win, manager, 0);

   // One more row, nobody measured it: the average is what it was
   _diff_asked = 0;
   memmove(_diff_measured + 301, _diff_measured + 300, (_diff_count - 300) * sizeof (unsigned int));
   _diff_measured[300] = 0;
   _diff_count++;
   efl_ui_position_manager_entity_item_added(manager, 300, NULL);
   ecore_main_loop_iterate();
   efl_ui_position_manager_entity_position_single_item(manager, 0);
   ck_assert_uint_lt(_diff_asked, rows / 4);
   _diff_check(win, manager, 1);

   // One that was measured as the average is still the average
   _diff_asked = 0;
   _diff_measured[10] = 20;
   efl_ui_position_manager_entity_item_size_changed(manager, 11, 11);
   _diff_measured[11] = 20;
   efl_ui_position_manager_entity_item_size_changed(manager, 11, 11);
   ecore_main_loop_iterate();
   efl_ui_position_manager_entity_position_single_item(manager, 0);
   ck_assert_uint_lt(_diff_asked, rows / 4);
   _diff_check(win, manager, 2);

   // One that was measured another: the average is another, and so is every row that has it
   _diff_asked = 0;
   _diff_measured[5] = 500;
   efl_ui_position_manager_entity_item_size_changed(manager, 5, 5);
   ecore_main_loop_iterate();
   efl_ui_position_manager_entity_position_single_item(manager, 0);
   ck_assert_uint_ge(_diff_asked, rows);
   _diff_check(win, manager, 3);

   // And the same for a measured one that leaves
   _diff_asked = 0;
   memmove(_diff_measured + 5, _diff_measured + 6, (_diff_count - 6) * sizeof (unsigned int));
   _diff_count--;
   efl_ui_position_manager_entity_item_removed(manager, 5, NULL);
   ecore_main_loop_iterate();
   efl_ui_position_manager_entity_position_single_item(manager, 0);
   ck_assert_uint_ge(_diff_asked, rows);
   _diff_check(win, manager, 4);

   efl_del(manager);
   _diff_cap = 7;
}
EFL_END_TEST

/* The groups of the grid position manager follow items that are added or
   removed, with the sizes that are asked for only once: a grid told of every
   change must place every item where a grid that is given the final items
   places it. Items are plain, headers of groups, or members of the group
   before them. */
static unsigned char _grid_kinds[CACHE_MAX_ROWS];

static Efl_Ui_Position_Manager_Size_Batch_Result
_grid_size_cb(void *data EINA_UNUSED, Efl_Ui_Position_Manager_Size_Call_Config conf,
              Eina_Rw_Slice memory)
{
   Efl_Ui_Position_Manager_Size_Batch_Entity *sizes = memory.mem;
   Efl_Ui_Position_Manager_Size_Batch_Result result = { 0 };
   unsigned int i, limit = conf.range.end_id - conf.range.start_id;

   for (i = 0; (i < limit) && conf.range.start_id + i < _cache_count; i++)
     {
        unsigned char kind = _grid_kinds[conf.range.start_id + i];

        sizes[i].size = kind == 1 ? EINA_SIZE2D(200, 20) : EINA_SIZE2D(40, 30);
        sizes[i].element_depth = kind == 2 ? 1 : 0;
        sizes[i].depth_leader = kind == 1;
     }
   result.filled_items = i;
   return result;
}

static Efl_Ui_Position_Manager_Entity *
_grid_manager_add(Efl_Ui_Win *win)
{
   Efl_Ui_Position_Manager_Entity *manager;

   manager = efl_add(EFL_UI_POSITION_MANAGER_GRID_CLASS, win,
                     efl_ui_layout_orientation_set(efl_added, EFL_UI_LAYOUT_ORIENTATION_VERTICAL));
   efl_ui_position_manager_data_access_v1_data_access_set(manager, win, NULL, _cache_object_cb, NULL,
                                                          NULL, _grid_size_cb, NULL, _cache_count);
   efl_ui_position_manager_entity_viewport_set(manager, EINA_RECT(0, 0, 200, 300));
   return manager;
}

EFL_START_TEST(test_efl_ui_position_manager_grid_groups_follow)
{
   Efl_Ui_Win *win;
   Efl_Ui_Position_Manager_Entity *manager, *fresh;
   Efl_Gfx_Entity *item;
   unsigned int seed = 11, i, step;

   win = win_add(NULL, "win", EFL_UI_WIN_TYPE_BASIC);
   item = _cache_dummy = efl_add(EFL_UI_BUTTON_CLASS, win);
   {
      static const unsigned char start[] = { 0, 0, 1, 2, 2, 1, 2, 2, 2, 1, 2, 2 };

      memcpy(_grid_kinds, start, sizeof (start));
      _cache_count = sizeof (start);
   }
   manager = _grid_manager_add(win);

   for (step = 0; step < 400; step++)
     {
        unsigned int idx;
        Eina_Bool check;

        seed = seed * 1103515245 + 12345;
        idx = (seed >> 8) % 400;
        seed = seed * 1103515245 + 12345;
        if (((seed >> 8) % 3) && _cache_count < CACHE_MAX_ROWS - 1)
          {
             unsigned char kind = (seed >> 12) % 4;

             idx %= _cache_count + 1;
             // Mostly members and plain items, a header makes a group. A plain
             // item is never after a group, and a member after a header or a member.
             kind = (kind == 3) ? 1 : ((kind == 2) ? 2 : 0);
             if (kind == 1 && idx < _cache_count && _grid_kinds[idx] == 0)
               kind = 0;
             if (kind != 1)
               kind = (idx > 0 && _grid_kinds[idx - 1] != 0) ? 2 : 0;
             memmove(_grid_kinds + idx + 1, _grid_kinds + idx, _cache_count - idx);
             _grid_kinds[idx] = kind;
             _cache_count++;
             efl_ui_position_manager_entity_item_added(manager, idx, item);
          }
        else if (_cache_count > 4)
          {
             idx %= _cache_count;
             memmove(_grid_kinds + idx, _grid_kinds + idx + 1, _cache_count - idx - 1);
             _cache_count--;
             efl_ui_position_manager_entity_item_removed(manager, idx, item);
          }

        check = !((seed >> 20) % 2);
        if (!check) continue;
        // The same items, given to another one at once
        fresh = _grid_manager_add(win);
        for (i = 0; i < _cache_count; i++)
          {
             Eina_Rect a, b;

             // (the manager does not place the header of a group by itself)
             if (_grid_kinds[i] == 1) continue;
             a = efl_ui_position_manager_entity_position_single_item(manager, i);
             b = efl_ui_position_manager_entity_position_single_item(fresh, i);
             ck_assert_int_eq(a.x, b.x);
             ck_assert_int_eq(a.y, b.y);
             ck_assert_int_eq(a.w, b.w);
             ck_assert_int_eq(a.h, b.h);
          }
        efl_del(fresh);
     }
   efl_del(manager);
}
EFL_END_TEST

/* A position manager that counts the rows the view is asked the size of on
   its behalf: the size callback the view gave it is wrapped by one that counts
   what goes through. */
static Efl_Ui_Position_Manager_Size_Batch_Callback _counted_size_access = NULL;
static unsigned int _counted_calls = 0;
static unsigned int _counted_rows = 0;

static Efl_Ui_Position_Manager_Size_Batch_Result
_counting_size_cb(void *data, Efl_Ui_Position_Manager_Size_Call_Config conf, Eina_Rw_Slice memory)
{
   _counted_calls++;
   _counted_rows += conf.range.end_id - conf.range.start_id;
   return _counted_size_access(data, conf, memory);
}

static void
_counting_data_access_set(Eo *obj, void *pd EINA_UNUSED, Efl_Ui_Win *canvas,
                          void *obj_access_data,
                          Efl_Ui_Position_Manager_Object_Batch_Callback obj_access,
                          Eina_Free_Cb obj_access_free_cb,
                          void *size_access_data,
                          Efl_Ui_Position_Manager_Size_Batch_Callback size_access,
                          Eina_Free_Cb size_access_free_cb, int size)
{
   _counted_size_access = size_access;
   efl_ui_position_manager_data_access_v1_data_access_set(efl_super(obj, efl_class_get(obj)),
                                                          canvas, obj_access_data, obj_access,
                                                          obj_access_free_cb, size_access_data,
                                                          size_access ? _counting_size_cb : NULL,
                                                          size_access_free_cb, size);
}

static Eina_Bool
_counting_manager_initializer(Efl_Class *klass)
{
   EFL_OPS_DEFINE(ops,
                  EFL_OBJECT_OP_FUNC(efl_ui_position_manager_data_access_v1_data_access_set,
                                     _counting_data_access_set));

   return efl_class_functions_set(klass, &ops, NULL);
}

// (made at each call, and not kept in a static: a class made with efl_class_new
// is freed with the Eo shutdown, and the tests that are not forked init and shut
// the library down for each test, so a pointer kept from one test would dangle in
// the next. It is called once per test, so there is nothing to save.)
static const Efl_Class *
_counting_manager_class_get(Eina_Bool grid)
{
   static const Efl_Class_Description list_desc = {
      EO_VERSION,
      "Test_Counting_List_Position_Manager",
      EFL_CLASS_TYPE_REGULAR,
      0,
      _counting_manager_initializer,
      NULL,
      NULL
   };
   static const Efl_Class_Description grid_desc = {
      EO_VERSION,
      "Test_Counting_Grid_Position_Manager",
      EFL_CLASS_TYPE_REGULAR,
      0,
      _counting_manager_initializer,
      NULL,
      NULL
   };

   return efl_class_new(grid ? &grid_desc : &list_desc,
                        grid ? EFL_UI_POSITION_MANAGER_GRID_CLASS : EFL_UI_POSITION_MANAGER_LIST_CLASS,
                        NULL);
}

/* Rows are added one at a time to a view that is showing an Exact_Model, as a
   listing arriving does, with the loop let to idle in between or not. The
   rows the view is asked the size of while that goes on are counted. */
static unsigned int
_single_adds_rows(int rows, Eina_Bool idle, Eina_Bool grid)
{
   Efl_Ui_Win *win;
   Efl_Model *model, *sizes;
   Efl_Ui_Factory *fl;
   Efl_Ui_List_View *lv;
   Efl_Ui_Position_Manager_Entity *manager;
   int i;

   win = win_add(NULL, "win", EFL_UI_WIN_TYPE_BASIC);
   efl_gfx_entity_size_set(win, EINA_SIZE2D(200, 300));

   model = _make_model(win);
   sizes = efl_add(EFL_UI_EXACT_MODEL_CLASS, win, efl_ui_view_model_set(efl_added, model));
   manager = efl_new(_counting_manager_class_get(grid));
   fl = efl_add(EFL_UI_WIDGET_FACTORY_CLASS, win,
                efl_ui_widget_factory_item_class_set(efl_added, EFL_UI_LIST_PLACEHOLDER_ITEM_CLASS),
                efl_event_callback_add(efl_added, EFL_UI_FACTORY_EVENT_ITEM_BUILDING,
                                       _row_height_on_build, NULL));
   lv = efl_add(grid ? EFL_UI_GRID_VIEW_CLASS : EFL_UI_LIST_VIEW_CLASS, win,
                efl_ui_collection_view_position_manager_set(efl_added, manager),
                efl_ui_collection_view_factory_set(efl_added, fl),
                efl_ui_view_model_set(efl_added, sizes));
   efl_content_set(win, lv);
   get_me_to_those_events(lv);
   for (i = 0; i < 50; i++)
     {
        ecore_main_loop_iterate();
     }

   _counted_calls = 0;
   _counted_rows = 0;
   for (i = 0; i < rows; i++)
     {
        efl_model_child_add(model);
        if (idle) wait_timer(0.0);
     }
   for (i = 0; i < 50; i++)
     {
        ecore_main_loop_iterate();
     }
   ck_assert_int_eq(efl_model_children_count_get(model), NUM_ITEMS + rows);
   return _counted_rows;
}

/* Each addition must not cost the work of all the rows already there: the
   number of rows the view is asked the size of grows with the rows added, and
   not with the rows added times the rows there are (2000 added one by one
   used to make it two million). A few rows are asked about for each, to
   be sure what is not asked again is the same. */
EFL_START_TEST(test_efl_ui_collection_view_many_single_adds)
{
   const int rows = SINGLE_ADDS;
   unsigned int counted;

   counted = _single_adds_rows(rows, EINA_FALSE, EINA_FALSE);
   ck_assert_uint_lt(counted, rows * 20);
}
EFL_END_TEST

// The same with the loop let to idle after each row, as a listing that
// arrives slowly does.
EFL_START_TEST(test_efl_ui_collection_view_many_single_adds_idle)
{
   const int rows = SINGLE_ADDS;
   unsigned int counted;

   counted = _single_adds_rows(rows, EINA_TRUE, EINA_FALSE);
   ck_assert_uint_lt(counted, rows * 20);
}
EFL_END_TEST

// And a grid, which keeps its items in groups and asks for the size of all of
// them to build those.
EFL_START_TEST(test_efl_ui_collection_view_many_single_adds_grid)
{
   const int rows = SINGLE_ADDS;
   unsigned int counted;

   counted = _single_adds_rows(rows, EINA_TRUE, EINA_TRUE);
   ck_assert_uint_lt(counted, rows * 20);
}
EFL_END_TEST

static Eina_Future *
_skip_first_row(void *data EINA_UNUSED, Efl_Filter_Model *parent, Efl_Model *child)
{
   Eina_Value *initial = efl_model_property_get(child, "initial");
   int row = 0;

   eina_value_int_get(initial, &row);
   eina_value_free(initial);
   return efl_loop_future_resolved(parent, eina_value_bool_init(row != 0));
}

/* A view over a model that renumbers the rows of an Exact_Model (a
   Filter_Model that does not keep the first one) must not read a size by the
   index the row has in the view: the Exact_Model keeps it under the index of
   the row in the source. Each row has the height its own index in the view
   says. */
EFL_START_TEST(test_efl_ui_collection_view_exact_model_renumbered)
{
   Efl_Ui_Win *win;
   Efl_Model *model, *sizes, *filtered;
   Efl_Ui_Factory *fl;
   Efl_Ui_List_View *lv;
   Eina_List *realized = NULL, *l;
   Efl_Gfx_Entity *item;
   int errors = 0, i, checked = 0;

   win = win_add(NULL, "win", EFL_UI_WIN_TYPE_BASIC);
   efl_gfx_entity_size_set(win, EINA_SIZE2D(200, 100));
   eina_log_print_cb_set(_count_err_cri, &errors);

   model = _make_model(win);
   sizes = efl_add(EFL_UI_EXACT_MODEL_CLASS, win, efl_ui_view_model_set(efl_added, model));
   filtered = efl_add(EFL_FILTER_MODEL_CLASS, win,
                      efl_ui_view_model_set(efl_added, sizes),
                      efl_filter_model_filter_set(efl_added, NULL, _skip_first_row, NULL));
   fl = efl_add(EFL_UI_WIDGET_FACTORY_CLASS, win,
                efl_ui_widget_factory_item_class_set(efl_added, EFL_UI_LIST_PLACEHOLDER_ITEM_CLASS),
                efl_event_callback_add(efl_added, EFL_UI_FACTORY_EVENT_ITEM_BUILDING,
                                       _row_height_on_build, NULL));
   lv = efl_add(EFL_UI_LIST_VIEW_CLASS, win,
                efl_ui_collection_view_factory_set(efl_added, fl),
                efl_ui_view_model_set(efl_added, filtered));
   efl_content_set(win, lv);
   efl_event_callback_add(lv, EFL_UI_COLLECTION_VIEW_EVENT_ITEM_REALIZED,
                          _realized_track, &realized);
   efl_event_callback_add(lv, EFL_UI_COLLECTION_VIEW_EVENT_ITEM_UNREALIZED,
                          _unrealized_track, &realized);
   get_me_to_those_events(lv);
   for (i = 0; i < 50; i++)
     {
        ecore_main_loop_iterate();
     }

   EINA_LIST_FOREACH(realized, l, item)
     {
        Eina_Rect g = efl_gfx_entity_geometry_get(item);

        if (g.h == 0) continue;
        ck_assert_int_eq(g.h, (efl_composite_model_index_get(efl_ui_view_model_get(item)) % 2) ?
                         40 : 90);
        checked++;
     }
   ck_assert_int_ge(checked, 2);

   eina_log_print_cb_set(eina_log_print_cb_stderr, NULL);
   ck_assert_int_eq(errors, 0);
   efl_event_callback_del(lv, EFL_UI_COLLECTION_VIEW_EVENT_ITEM_REALIZED,
                          _realized_track, &realized);
   efl_event_callback_del(lv, EFL_UI_COLLECTION_VIEW_EVENT_ITEM_UNREALIZED,
                          _unrealized_track, &realized);
   eina_list_free(realized);
}
EFL_END_TEST

/* The grid has the size of its biggest item for all of them, and an item that
   goes may have been it: what it gives once it is gone is the biggest of those
   that are left. */
static Efl_Ui_Position_Manager_Size_Batch_Result
_gridw_size_cb(void *data EINA_UNUSED, Efl_Ui_Position_Manager_Size_Call_Config conf,
               Eina_Rw_Slice memory)
{
   Efl_Ui_Position_Manager_Size_Batch_Entity *sizes = memory.mem;
   Efl_Ui_Position_Manager_Size_Batch_Result result = { 0 };
   unsigned int i, limit = conf.range.end_id - conf.range.start_id;

   for (i = 0; (i < limit) && conf.range.start_id + i < _cache_count; i++)
     {
        sizes[i].size = EINA_SIZE2D(_cache_sizes[conf.range.start_id + i], 30);
        sizes[i].element_depth = 0;
        sizes[i].depth_leader = EINA_FALSE;
     }
   result.filled_items = i;
   return result;
}

static int _gridw_min_w = -1;

static void
_gridw_min_size(void *data EINA_UNUSED, const Efl_Event *ev)
{
   Eina_Size2D *size = ev->info;

   _gridw_min_w = size->w;
}

EFL_START_TEST(test_efl_ui_position_manager_grid_min_size_goes_down)
{
   Efl_Ui_Win *win;
   Efl_Ui_Position_Manager_Entity *manager;
   static const int widths[] = { 40, 90, 60, 50, 90 };
   unsigned int i;

   win = win_add(NULL, "win", EFL_UI_WIN_TYPE_BASIC);
   _cache_dummy = efl_add(EFL_UI_BUTTON_CLASS, win);
   _cache_count = EINA_C_ARRAY_LENGTH(widths);
   for (i = 0; i < _cache_count; i++)
     {
        _cache_sizes[i] = widths[i];
     }
   manager = efl_add(EFL_UI_POSITION_MANAGER_GRID_CLASS, win,
                     efl_ui_layout_orientation_set(efl_added, EFL_UI_LAYOUT_ORIENTATION_VERTICAL));
   efl_event_callback_add(manager, EFL_UI_POSITION_MANAGER_ENTITY_EVENT_CONTENT_MIN_SIZE_CHANGED,
                          _gridw_min_size, NULL);
   _gridw_min_w = -1;
   efl_ui_position_manager_data_access_v1_data_access_set(manager, win, NULL, _cache_object_cb, NULL,
                                                          NULL, _gridw_size_cb, NULL, _cache_count);
   efl_ui_position_manager_entity_viewport_set(manager, EINA_RECT(0, 0, 200, 300));
   ecore_main_loop_iterate();
   ck_assert_int_eq(efl_ui_position_manager_entity_position_single_item(manager, 0).w, 90);

   // One of the two biggest goes: nothing changes
   memmove(_cache_sizes + 1, _cache_sizes + 2, 3 * sizeof (int));
   _cache_count--;
   efl_ui_position_manager_entity_item_removed(manager, 1, _cache_dummy);
   ecore_main_loop_iterate();
   ck_assert_int_eq(efl_ui_position_manager_entity_position_single_item(manager, 0).w, 90);

   // The last of them
   _cache_count--;
   efl_ui_position_manager_entity_item_removed(manager, _cache_count, _cache_dummy);
   ecore_main_loop_iterate();
   ck_assert_int_eq(efl_ui_position_manager_entity_position_single_item(manager, 0).w, 60);
   ck_assert_int_eq(_gridw_min_w, 60);

   efl_del(manager);
}
EFL_END_TEST

/* What the grid asks again when an item goes is only what is known: a provider
   that does not know a size says so (0x0) when it is told not to fetch, and
   is asked nothing else. */
static unsigned int _known_calls = 0;
static unsigned int _full_calls = 0;

static Efl_Ui_Position_Manager_Size_Batch_Result
_known_size_cb(void *data EINA_UNUSED, Efl_Ui_Position_Manager_Size_Call_Config conf,
               Eina_Rw_Slice memory)
{
   Efl_Ui_Position_Manager_Size_Batch_Entity *sizes = memory.mem;
   Efl_Ui_Position_Manager_Size_Batch_Result result = { 0 };
   unsigned int i, limit = conf.range.end_id - conf.range.start_id;

   if (conf.no_fetch)
     {
        _known_calls++;
     }
   else if (conf.cache_request)
     {
        _full_calls++;
     }
   for (i = 0; (i < limit) && conf.range.start_id + i < _cache_count; i++)
     {
        unsigned int id = conf.range.start_id + i;

        // (only the first two items are known, when that is all that is wanted)
        sizes[i].size = (conf.no_fetch && id >= 2) ? EINA_SIZE2D(0, 0) :
          EINA_SIZE2D(_cache_sizes[id], 30);
        sizes[i].element_depth = 0;
        sizes[i].depth_leader = EINA_FALSE;
     }
   result.filled_items = i;
   return result;
}

EFL_START_TEST(test_efl_ui_position_manager_grid_min_size_known_only)
{
   Efl_Ui_Win *win;
   Efl_Ui_Position_Manager_Entity *manager;
   static const int widths[] = { 40, 90, 60, 50, 95 };
   unsigned int i;

   win = win_add(NULL, "win", EFL_UI_WIN_TYPE_BASIC);
   _cache_dummy = efl_add(EFL_UI_BUTTON_CLASS, win);
   _cache_count = EINA_C_ARRAY_LENGTH(widths);
   for (i = 0; i < _cache_count; i++)
     {
        _cache_sizes[i] = widths[i];
     }
   manager = efl_add(EFL_UI_POSITION_MANAGER_GRID_CLASS, win,
                     efl_ui_layout_orientation_set(efl_added, EFL_UI_LAYOUT_ORIENTATION_VERTICAL));
   efl_ui_position_manager_data_access_v1_data_access_set(manager, win, NULL, _cache_object_cb, NULL,
                                                          NULL, _known_size_cb, NULL, _cache_count);
   efl_ui_position_manager_entity_viewport_set(manager, EINA_RECT(0, 0, 200, 300));
   ecore_main_loop_iterate();
   ck_assert_int_eq(efl_ui_position_manager_entity_position_single_item(manager, 0).w, 95);

   // The item that gave the biggest one goes: the others that are known give theirs
   _known_calls = _full_calls = 0;
   _cache_count--;
   efl_ui_position_manager_entity_item_removed(manager, _cache_count, _cache_dummy);
   ecore_main_loop_iterate();
   ck_assert_uint_gt(_known_calls, 0);
   ck_assert_uint_eq(_full_calls, 0);
   ck_assert_int_eq(efl_ui_position_manager_entity_position_single_item(manager, 0).w, 90);

   efl_del(manager);
}
EFL_END_TEST

/* An Exact_Model that counts the rows that are fetched from it. */
static unsigned int _fetched_rows = 0;

static Eina_Future *
_fetch_counting_slice_get(Eo *obj, void *pd EINA_UNUSED, unsigned int start, unsigned int count)
{
   _fetched_rows += count;
   return efl_model_children_slice_get(efl_super(obj, efl_class_get(obj)), start, count);
}

static Eina_Bool
_fetch_counting_initializer(Efl_Class *klass)
{
   EFL_OPS_DEFINE(ops,
                  EFL_OBJECT_OP_FUNC(efl_model_children_slice_get, _fetch_counting_slice_get));

   return efl_class_functions_set(klass, &ops, NULL);
}

static const Efl_Class *
_fetch_counting_class_get(const Efl_Class *parent)
{
   static const Efl_Class_Description desc = {
      EO_VERSION,
      "Test_Fetch_Counting_Model",
      EFL_CLASS_TYPE_REGULAR,
      0,
      _fetch_counting_initializer,
      NULL,
      NULL
   };

   // (not kept, see _counting_manager_class_get)
   return efl_class_new(&desc, parent, NULL);
}

/* The biggest minimum size of a grid is asked again when an item goes, and is
   made of what the model already knows: the rows nobody built are not fetched
   to be asked about (that would be every row of a list of a hundred thousand
   when one is removed). */
EFL_START_TEST(test_efl_ui_collection_view_grid_remove_fetches_nothing)
{
   Efl_Ui_Win *win;
   Efl_Model *model, *sizes, *last = NULL;
   Efl_Ui_Factory *fl;
   Efl_Ui_Grid_View *gv;
   Efl_Ui_Position_Manager_Entity *manager;
   int i;

   win = win_add(NULL, "win", EFL_UI_WIN_TYPE_BASIC);
   efl_gfx_entity_size_set(win, EINA_SIZE2D(200, 300));

   model = _make_model(win);
   for (i = 0; i < 700; i++)
     {
        last = efl_model_child_add(model);
     }
   sizes = efl_add(_fetch_counting_class_get(EFL_UI_EXACT_MODEL_CLASS), win,
                   efl_ui_view_model_set(efl_added, model));
   manager = efl_add(EFL_UI_POSITION_MANAGER_GRID_CLASS, win);
   fl = efl_add(EFL_UI_WIDGET_FACTORY_CLASS, win,
                efl_ui_widget_factory_item_class_set(efl_added, EFL_UI_LIST_PLACEHOLDER_ITEM_CLASS),
                efl_event_callback_add(efl_added, EFL_UI_FACTORY_EVENT_ITEM_BUILDING,
                                       _row_height_on_build, NULL));
   gv = efl_add(EFL_UI_GRID_VIEW_CLASS, win,
                efl_ui_collection_view_position_manager_set(efl_added, manager),
                efl_ui_collection_view_factory_set(efl_added, fl),
                efl_ui_view_model_set(efl_added, sizes));
   efl_content_set(win, gv);
   get_me_to_those_events(gv);
   for (i = 0; i < 50; i++)
     {
        ecore_main_loop_iterate();
     }

   // The last one, which nobody built
   _fetched_rows = 0;
   efl_model_child_del(efl_ui_view_model_get(model), last);
   for (i = 0; i < 50; i++)
     {
        ecore_main_loop_iterate();
     }
   ck_assert_uint_eq(efl_model_children_count_get(sizes), 749);
   ck_assert_uint_lt(_fetched_rows, 100);
}
EFL_END_TEST

/* A list over an Average_Model: the rows nobody measured are as high as the
   average of those that were. */
#define AVG_ROWS 300

static unsigned int _avg_row_height = 40;

static void
_row_height_avg(void *data EINA_UNUSED, const Efl_Event *ev)
{
   Efl_Gfx_Entity *item = ev->info;

   efl_gfx_hint_size_min_set(item, EINA_SIZE2D(0, _avg_row_height));
}

static Eina_Value
_avg_rows_then(void *data, const Eina_Value v, const Eina_Future *dead EINA_UNUSED)
{
   Eo **rows = data;
   unsigned int i, len;
   Eo *c;

   EINA_VALUE_ARRAY_FOREACH(&v, len, i, c)
     {
        rows[i] = efl_ref(c);
     }
   return v;
}

/* Every item is where the heights the model has for the rows say, and the rows
   nobody measured are the average of those that were. */
static void
_avg_positions_check(Efl_Ui_Position_Manager_Entity *manager, Efl_Model *sizes, unsigned int n)
{
   Eo *rows[AVG_ROWS + 8] = { NULL };
   unsigned int heights[AVG_ROWS + 8];
   unsigned int i, sum = 0, seen = 0, average, y = 0;
   int loops;

   eina_future_then(efl_model_children_slice_get(sizes, 0, n), _avg_rows_then, rows, NULL);
   for (loops = 0; (loops < 20) && !rows[n - 1]; loops++)
     {
        ecore_main_loop_iterate();
     }
   ck_assert_ptr_ne(rows[n - 1], NULL);
   for (i = 0; i < n; i++)
     {
        Eina_Value *v = efl_model_property_get(rows[i], "self.height");
        unsigned int h = 0;

        heights[i] = 0;
        if (eina_value_type_get(v) != EINA_VALUE_TYPE_ERROR && eina_value_uint_convert(v, &h))
          {
             heights[i] = h;
             sum += h;
             seen++;
          }
        eina_value_free(v);
     }
   average = seen ? sum / seen : 0;
   for (i = 0; i < n; i++)
     {
        Eina_Rect r = efl_ui_position_manager_entity_position_single_item(manager, i);
        unsigned int expected = heights[i] ? heights[i] : average;

        ck_assert_int_eq(r.y, y);
        ck_assert_int_eq(r.h, expected);
        y += expected;
        efl_unref(rows[i]);
     }
}

EFL_START_TEST(test_efl_ui_collection_view_average_model_estimates)
{
   Efl_Ui_Win *win;
   Efl_Model *model, *sizes;
   Efl_Ui_Factory *fl;
   Efl_Ui_List_View *lv;
   Efl_Ui_Position_Manager_Entity *manager;
   Efl_Model *base;
   unsigned int count;
   int i;

   _avg_row_height = 40;
   win = win_add(NULL, "win", EFL_UI_WIN_TYPE_BASIC);
   efl_gfx_entity_size_set(win, EINA_SIZE2D(200, 300));

   model = _make_model(win);
   for (i = NUM_ITEMS; i < AVG_ROWS - 1; i++)
     {
        efl_model_child_add(model);
     }
   base = efl_ui_view_model_get(model);
   sizes = efl_add(EFL_UI_AVERAGE_MODEL_CLASS, win, efl_ui_view_model_set(efl_added, model));
   manager = efl_new(_counting_manager_class_get(EINA_FALSE));
   fl = efl_add(EFL_UI_WIDGET_FACTORY_CLASS, win,
                efl_ui_widget_factory_item_class_set(efl_added, EFL_UI_LIST_PLACEHOLDER_ITEM_CLASS),
                efl_event_callback_add(efl_added, EFL_UI_FACTORY_EVENT_ITEM_BUILDING,
                                       _row_height_avg, NULL));
   lv = efl_add(EFL_UI_LIST_VIEW_CLASS, win,
                efl_ui_collection_view_position_manager_set(efl_added, manager),
                efl_ui_collection_view_factory_set(efl_added, fl),
                efl_ui_view_model_set(efl_added, sizes));
   efl_content_set(win, lv);
   get_me_to_those_events(lv);
   for (i = 0; i < 50; i++)
     {
        ecore_main_loop_iterate();
     }
   count = efl_model_children_count_get(sizes);
   ck_assert_uint_eq(count, AVG_ROWS - 1);
   _avg_positions_check(manager, sizes, count);

   // A row comes at the end, and is measured the same as the others: the average is what it was
   _counted_rows = 0;
   efl_model_child_add(base);
   for (i = 0; i < 20; i++)
     {
        ecore_main_loop_iterate();
     }
   count = efl_model_children_count_get(sizes);
   ck_assert_uint_eq(count, AVG_ROWS);
   ck_assert_uint_lt(_counted_rows, AVG_ROWS / 3);
   _avg_positions_check(manager, sizes, count);

   // The ones that come are higher: the average is another, and every row has it
   _avg_row_height = 200;
   _counted_rows = 0;
   for (i = 0; i < 3; i++)
     {
        efl_model_child_add(base);
     }
   for (i = 0; i < 20; i++)
     {
        ecore_main_loop_iterate();
     }
   count = efl_model_children_count_get(sizes);
   ck_assert_uint_eq(count, AVG_ROWS + 3);
   ck_assert_uint_ge(_counted_rows, count);
   _avg_positions_check(manager, sizes, count);
}
EFL_END_TEST

void efl_ui_test_collection_view(TCase *tc)
{
   tcase_add_test(tc, test_efl_ui_collection_view_basic);
   tcase_add_test(tc, efl_ui_collection_view_manager_twice);
   tcase_add_test(tc, efl_ui_collection_view_bare);
   tcase_add_test(tc, efl_ui_collection_view_manager_ownership);
   tcase_add_test(tc, test_efl_ui_collection_view_select);
   tcase_add_test(tc, test_efl_ui_collection_view_last_selected_is_an_item);
   tcase_add_test(tc, test_efl_ui_collection_view_select_mode_before_model);
   tcase_add_test(tc, test_efl_ui_collection_view_last_row_kept);
   tcase_add_test(tc, test_efl_ui_collection_view_refill);
   tcase_add_test(tc, test_efl_ui_collection_view_tab);
   tcase_add_test(tc, test_efl_ui_collection_view_shift_tab);
   tcase_add_test(tc, test_efl_ui_collection_view_exact_model);
   tcase_add_test(tc, test_efl_ui_collection_view_exact_model_zero_size);
   tcase_add_test(tc, test_efl_ui_collection_view_homogeneous_model);
   tcase_add_test(tc, test_efl_ui_collection_view_item_events_without_info);
   tcase_add_test(tc, test_efl_ui_collection_view_sizes_agree);
   tcase_add_test(tc, test_efl_ui_collection_view_remove_last_row);
   tcase_add_test(tc, test_efl_ui_collection_view_exact_model_jump);
   tcase_add_test(tc, test_efl_ui_position_manager_list_cache_follows);
   tcase_add_test(tc, test_efl_ui_collection_view_many_single_adds);
   tcase_add_test(tc, test_efl_ui_position_manager_list_cache_shared_size);
   tcase_add_test(tc, test_efl_ui_position_manager_list_partial_fill);
   tcase_add_test(tc, test_efl_ui_position_manager_list_differential);
   tcase_add_test(tc, test_efl_ui_position_manager_list_estimate_stable);
   tcase_add_test(tc, test_efl_ui_position_manager_grid_groups_follow);
   tcase_add_test(tc, test_efl_ui_collection_view_many_single_adds_idle);
   tcase_add_test(tc, test_efl_ui_collection_view_many_single_adds_grid);
   tcase_add_test(tc, test_efl_ui_collection_view_exact_model_renumbered);
   tcase_add_test(tc, test_efl_ui_position_manager_grid_min_size_goes_down);
   tcase_add_test(tc, test_efl_ui_position_manager_grid_min_size_known_only);
   tcase_add_test(tc, test_efl_ui_collection_view_grid_remove_fetches_nothing);
   tcase_add_test(tc, test_efl_ui_collection_view_average_model_estimates);
}
