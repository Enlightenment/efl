#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "efl_suite.h"

#include <Efl.h>
#include <Ecore.h>

static void
_observer_update(Eo *obj EINA_UNUSED, void *pd EINA_UNUSED, Efl_Object *obs EINA_UNUSED,
                 const char *key EINA_UNUSED, void *data EINA_UNUSED)
{
}

static Eina_Bool
_observer_initializer(Efl_Class *klass)
{
   EFL_OPS_DEFINE(ops, EFL_OBJECT_OP_FUNC(efl_observer_update, _observer_update));

   return efl_class_functions_set(klass, &ops, NULL);
}

static const Efl_Class *
_observer_class_get(void)
{
   static const Efl_Class_Description desc = {
      EO_VERSION,
      "Test_Observer",
      EFL_CLASS_TYPE_REGULAR,
      0,
      _observer_initializer,
      NULL,
      NULL
   };
   static const Efl_Class *klass = NULL;

   if (!klass)
     klass = efl_class_new(&desc, EFL_OBJECT_CLASS, EFL_OBSERVER_INTERFACE, NULL);
   return klass;
}

static unsigned int
_observed_keys_count(Efl_Observable *observable)
{
   Eina_Iterator *it = efl_observable_iterator_tuple_new(observable);
   Efl_Observable_Tuple *tuple;
   unsigned int count = 0;

   EINA_ITERATOR_FOREACH(it, tuple)
     {
        count++;
     }
   eina_iterator_free(it);
   return count;
}

// A group of observers that loses its last observer is gone, the way
// observer_del has always done it: it must not be listed as being observed.
EFL_START_TEST(efl_test_observable_clean_drops_empty_groups)
{
   Efl_Observable *observable = efl_add(EFL_OBSERVABLE_CLASS, efl_main_loop_get());
   Efl_Observer *first = efl_add(_observer_class_get(), efl_main_loop_get());
   Efl_Observer *second = efl_add(_observer_class_get(), efl_main_loop_get());

   efl_observable_observer_add(observable, "a", first);
   efl_observable_observer_add(observable, "b", first);
   efl_observable_observer_add(observable, "b", second);
   ck_assert_uint_eq(_observed_keys_count(observable), 2);

   efl_observable_observer_clean(observable, first);
   // "a" had no one else, "b" still has the second one
   ck_assert_uint_eq(_observed_keys_count(observable), 1);

   efl_observable_observer_clean(observable, second);
   ck_assert_uint_eq(_observed_keys_count(observable), 0);

   efl_del(second);
   efl_del(first);
   efl_del(observable);
}
EFL_END_TEST

void
efl_test_case_observer(TCase *tc)
{
   tcase_add_test(tc, efl_test_observable_clean_drops_empty_groups);
}
