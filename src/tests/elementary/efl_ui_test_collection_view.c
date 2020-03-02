#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include <Efl_Ui.h>
#include "efl_ui_suite.h"

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

EFL_START_TEST(test_efl_ui_collection_view_basic)
{
   Efl_Ui_Win *win;
   Efl_Model *model;
   Efl_Ui_List_View *lv;
   Efl_Ui_Factory *fl;
   Eina_Future *f;
   int count_realize = 0;
   int count_unrealize = 0;

   win = win_add();
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

   win = win_add();
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

void efl_ui_test_collection_view(TCase *tc)
{
   tcase_add_test(tc, test_efl_ui_collection_view_basic);
   tcase_add_test(tc, test_efl_ui_collection_view_select);
}
