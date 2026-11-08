#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include <Efl_Ui.h>
#include "efl_ui_suite.h"
#include "efl_ui_homogeneous_model.eo.h"

#define NUM_ITEMS 50

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
   tcase_add_test(tc, test_efl_ui_collection_view_homogeneous_model);
   tcase_add_test(tc, test_efl_ui_collection_view_sizes_agree);
}
