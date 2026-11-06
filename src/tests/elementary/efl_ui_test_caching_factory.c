#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include <Efl_Ui.h>
#include <Elementary.h>
#include "elm_widget.h"
#include "elm_widget_layout.h"
#include "efl_ui_suite.h"

EFL_START_TEST(efl_ui_test_caching_factory_del_before_pause)
{
   Eo *win, *factory, *app = efl_app_main_get();
   unsigned int before;

   win = win_add(NULL, "factory", EFL_UI_WIN_TYPE_BASIC);
   before = efl_event_callback_count(app, EFL_APP_EVENT_PAUSE);

   /* A factory empties its cache when the application is paused, so it
      listens to the application -- which outlives it. Going away without
      letting go left the application calling into a freed factory the next
      time it paused, which closing the last window does. */
   factory = efl_add(EFL_UI_CACHING_FACTORY_CLASS, win,
                     efl_ui_widget_factory_item_class_set(efl_added, EFL_UI_LIST_DEFAULT_ITEM_CLASS));
   ck_assert_int_eq(efl_event_callback_count(app, EFL_APP_EVENT_PAUSE), before + 1);

   efl_del(factory);
   ck_assert_int_eq(efl_event_callback_count(app, EFL_APP_EVENT_PAUSE), before);
}
EFL_END_TEST

static Eina_Value
_made_cb(Eo *o EINA_UNUSED, void *data, const Eina_Value v)
{
   Eo **made = data;

   eina_value_array_get(&v, 0, made);
   efl_ref(*made);
   return v;
}

/* Asks a factory for one item of a model and turns the loop until it is
   there. */
static Eo *
_factory_item_make(Eo *factory, Efl_Model *model)
{
   Eo *made = NULL;
   Eina_Array arr;
   int i;

   eina_array_step_set(&arr, sizeof(arr), 1);
   eina_array_push(&arr, model);
   efl_future_then(factory,
                   efl_ui_view_factory_create_with_event(factory, eina_array_iterator_new(&arr)),
                   .success = _made_cb,
                   .success_type = EINA_VALUE_TYPE_ARRAY, .data = &made);
   eina_array_flush(&arr);
   for (i = 0; !made && (i < 100); i++)
     ecore_main_loop_iterate();
   ck_assert_ptr_ne(made, NULL);
   return made;
}

static void
_building_count(void *data, const Efl_Event *ev EINA_UNUSED)
{
   unsigned int *count = data;

   (*count)++;
}

/* Every item the factory hands out is announced as building, the ones it had
   kept as much as the ones it made. */
EFL_START_TEST(efl_ui_test_caching_factory_building_on_reuse)
{
   Eo *win, *factory, *model, *item, *again;
   Eina_Array arr;
   unsigned int building = 0;

   win = win_add(NULL, "factory", EFL_UI_WIN_TYPE_BASIC);
   model = efl_add(EFL_GENERIC_MODEL_CLASS, win);
   factory = efl_add(EFL_UI_CACHING_FACTORY_CLASS, win,
                     efl_ui_widget_factory_item_class_set(efl_added, EFL_UI_LIST_DEFAULT_ITEM_CLASS),
                     efl_event_callback_add(efl_added, EFL_UI_FACTORY_EVENT_ITEM_BUILDING,
                                            _building_count, &building));

   item = _factory_item_make(factory, efl_model_child_add(model));
   ck_assert_uint_eq(building, 1);

   eina_array_step_set(&arr, sizeof(arr), 1);
   eina_array_push(&arr, item);
   efl_ui_factory_release(factory, eina_array_iterator_new(&arr));
   eina_array_flush(&arr);
   efl_unref(item);

   again = _factory_item_make(factory, efl_model_child_add(model));
   ck_assert_ptr_eq(again, item);
   ck_assert_uint_eq(building, 2);
   efl_unref(again);
}
EFL_END_TEST

/* A layout factory binds each item again every time it hands one out, a
 * reused one too. Binding a part to the same property again must leave one
 * binding, or every trip through the cache would make the item update one
 * more time for each change of the model. */
EFL_START_TEST(efl_ui_test_layout_factory_reuse_binds_once)
{
   Eo *win, *factory, *model, *child, *item, *again;
   Efl_Ui_Layout_Data *pd;
   Eina_Iterator *it;
   Eina_Hash_Tuple *tuple;
   Eina_Array arr;
   unsigned int bound = 0;
   int i;

   win = win_add(NULL, "factory", EFL_UI_WIN_TYPE_BASIC);
   model = efl_add(EFL_GENERIC_MODEL_CLASS, win);
   child = efl_model_child_add(model);
   efl_model_property_set(child, "title", eina_value_string_new("first"));

   factory = efl_add(EFL_UI_LAYOUT_FACTORY_CLASS, win,
                     efl_ui_property_bind(efl_added, "efl.text", "title"));

   item = _factory_item_make(factory, child);
   for (i = 0; i < 6; i++)
     {
        eina_array_step_set(&arr, sizeof(arr), 1);
        eina_array_push(&arr, item);
        efl_ui_factory_release(factory, eina_array_iterator_new(&arr));
        eina_array_flush(&arr);
        efl_unref(item);

        again = _factory_item_make(factory, child);
        ck_assert_ptr_eq(again, item);
        item = again;
     }

   // One binding of the part to the property, however many times it came back
   pd = efl_data_scope_get(item, EFL_UI_LAYOUT_BASE_CLASS);
   it = eina_hash_iterator_tuple_new(pd->connect.properties);
   EINA_ITERATOR_FOREACH(it, tuple)
     {
        if (!strcmp(tuple->key, "title") && !strcmp(tuple->data, "efl.text")) bound++;
     }
   eina_iterator_free(it);
   ck_assert_uint_eq(bound, 1);
   ck_assert_int_eq(eina_hash_population(pd->connect.properties), 1);

   efl_unref(item);
}
EFL_END_TEST

void efl_ui_test_caching_factory(TCase *tc)
{
   tcase_add_test(tc, efl_ui_test_caching_factory_del_before_pause);
   tcase_add_test(tc, efl_ui_test_caching_factory_building_on_reuse);
   tcase_add_test(tc, efl_ui_test_layout_factory_reuse_binds_once);
}
