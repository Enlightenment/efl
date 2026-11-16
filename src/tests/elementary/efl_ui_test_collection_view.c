#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include <Efl_Ui.h>
#include "efl_ui_suite.h"
#include "efl_ui_homogeneous_model.eo.h"
#include "efl_ui_exact_model.eo.h"

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
   static const Efl_Class *list_klass = NULL;
   static const Efl_Class *grid_klass = NULL;

   if (grid)
     {
        if (!grid_klass)
          grid_klass = efl_class_new(&grid_desc, EFL_UI_POSITION_MANAGER_GRID_CLASS, NULL);
        return grid_klass;
     }
   if (!list_klass)
     list_klass = efl_class_new(&list_desc, EFL_UI_POSITION_MANAGER_LIST_CLASS, NULL);
   return list_klass;
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
   tcase_add_test(tc, test_efl_ui_position_manager_grid_groups_follow);
   tcase_add_test(tc, test_efl_ui_collection_view_many_single_adds_idle);
   tcase_add_test(tc, test_efl_ui_collection_view_many_single_adds_grid);
   tcase_add_test(tc, test_efl_ui_collection_view_exact_model_renumbered);
}
