#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <stdio.h>

#ifndef _WIN32
# include <signal.h>
# include <unistd.h>
#endif

#include <Eo.h>

#include "eo_internal.h"

#include "eo_suite.h"
#include "eo_test_class_simple.h"
#include "eo_test_class_singleton.h"
#include "eo_test_domain.h"

/* Loading this internal header for testing purposes. */
#include "eo_ptr_indirection.h"

EFL_START_TEST(eo_simple)
{
   Eo *obj = efl_add_ref(EO_CLASS, NULL);
   fail_if(obj);

   obj = efl_add_ref(SIMPLE_CLASS, NULL);
   fail_if(!obj);
   efl_constructor(obj);
   efl_destructor(obj);
   efl_unref(obj);

}
EFL_END_TEST

EFL_START_TEST(eo_singleton)
{

   Eo *obj = efl_add_ref(SINGLETON_CLASS, NULL);
   fail_if(!obj);

   Eo *obj2 = efl_add_ref(SINGLETON_CLASS, NULL);
   fail_if(!obj2);

   ck_assert_ptr_eq(obj, obj2);

   efl_unref(obj);
   efl_unref(obj2);

}
EFL_END_TEST

#define OVERRIDE_A_SIMPLE 100859
#define OVERRIDE_A 324000
static int
_simple_obj_override_a_get(Eo *obj, void *class_data EINA_UNUSED)
{
   return OVERRIDE_A + simple_a_get(efl_super(obj, EFL_OBJECT_OVERRIDE_CLASS));
}

static void
_simple_obj_override_a_double_set(Eo *obj, void *class_data EINA_UNUSED, int a)
{
   simple_a_set(efl_super(obj, EFL_OBJECT_OVERRIDE_CLASS), 2 * a);
}

EFL_START_TEST(efl_object_override_tests)
{

   Eo *obj = efl_add_ref(SIMPLE_CLASS, NULL);
   fail_if(!obj);

   /* First get the value before the override to make sure it works and to
    * make sure we don't cache. */
   ck_assert_int_eq(simple_a_get(obj), 0);

   /* Test override */
   EFL_OPS_DEFINE(
            overrides,
            EFL_OBJECT_OP_FUNC(simple_a_get, _simple_obj_override_a_get));
   fail_if(!efl_object_override(obj, &overrides));

   ck_assert_int_eq(simple_a_get(obj), OVERRIDE_A);

   /* Check non-overriden functions work. */
   simple_a_set(obj, OVERRIDE_A_SIMPLE);
   ck_assert_int_eq(simple_a_get(obj), OVERRIDE_A + OVERRIDE_A_SIMPLE);

   /* Override again. */
   EFL_OPS_DEFINE(
            overrides2,
            EFL_OBJECT_OP_FUNC(simple_a_set, _simple_obj_override_a_double_set));
   fail_if(!efl_object_override(obj, NULL));
   fail_if(!efl_object_override(obj, &overrides2));

   simple_a_set(obj, OVERRIDE_A_SIMPLE);
   ck_assert_int_eq(simple_a_get(obj), OVERRIDE_A_SIMPLE * 2);

   /* Try overriding again, allowed since 1.21. */
   fail_if(!efl_object_override(obj, &overrides));
   ck_assert_int_eq(simple_a_get(obj), OVERRIDE_A + OVERRIDE_A_SIMPLE * 2);

   /* Try introducing a new function */
   EFL_OPS_DEFINE(
            overrides3,
            EFL_OBJECT_OP_FUNC(simple2_class_beef_get, _simple_obj_override_a_double_set));
   fail_if(efl_object_override(obj, &overrides3));
   fail_if(!efl_object_override(obj, NULL));
   fail_if(efl_object_override(obj, &overrides3));

   /* Test override reset of all functions */
   fail_if(!efl_object_override(obj, NULL));
   simple_a_set(obj, 42 * OVERRIDE_A_SIMPLE);
   ck_assert_int_eq(simple_a_get(obj), 42 * OVERRIDE_A_SIMPLE);

   /* Test override reset of a single functions, allowed since 1.21. */
   EFL_OPS_DEFINE(
            overrides4,
            EFL_OBJECT_OP_FUNC(simple_a_get, NULL));
   fail_if(!efl_object_override(obj, NULL));
   simple_a_set(obj, 1337);
   ck_assert_int_eq(simple_a_get(obj), 1337);
   fail_if(!efl_object_override(obj, &overrides));
   fail_if(!efl_object_override(obj, &overrides2));
   ck_assert_int_eq(simple_a_get(obj), OVERRIDE_A + 1337);
   fail_if(!efl_object_override(obj, &overrides4));
   ck_assert_int_eq(simple_a_get(obj), 1337);

   efl_unref(obj);

}
EFL_END_TEST

static int _eo_signals_cb_current = 0;
static int _eo_signals_cb_flag = 0;

static void
_eo_signals_a_changed_cb(void *_data, const Efl_Event *event EINA_UNUSED)
{
   int data = (intptr_t) _data;
   _eo_signals_cb_current++;

   ck_assert_int_eq(data, _eo_signals_cb_current);
   _eo_signals_cb_flag |= 0x1;
}

static void
_eo_signals_a_changed_cb2(void *_data EINA_UNUSED, const Efl_Event *event EINA_UNUSED)
{
   _eo_signals_cb_flag |= 0x2;
   efl_event_callback_stop(event->object);
}

static void
_eo_signals_a_changed_within_cb(void *_data EINA_UNUSED, const Efl_Event *event)
{
   int a = 3;
   efl_event_callback_legacy_call(event->object, EV_A_CHANGED, &a);
   _eo_signals_cb_flag = 0x8;
}

static void
_eo_signals_a_changed_never(void *_data EINA_UNUSED, const Efl_Event *event EINA_UNUSED)
{
   /* This one should never be called. */
   fail_if(1);
}

static void
_eo_signals_efl_del_cb(void *_data EINA_UNUSED, const Efl_Event *event EINA_UNUSED)
{
   _eo_signals_cb_flag |= 0x4;
}

static int check_is_deled = 0;

void
_eo_signals_cb_added_deled(void *data EINA_UNUSED, const Efl_Event *event)
{
   const Efl_Callback_Array_Item_Full *callback_array = event->info;

   if (check_is_deled)
     fail_if(callback_array->func == _eo_signals_cb_added_deled);
   else
     fail_if(callback_array->func != _eo_signals_cb_added_deled);
}

EFL_CALLBACKS_ARRAY_DEFINE(_eo_signals_callbacks,
{ EV_A_CHANGED, _eo_signals_a_changed_cb },
{ EV_A_CHANGED, _eo_signals_a_changed_cb2 },
{ EV_A_CHANGED, _eo_signals_a_changed_never },
{ EFL_EVENT_DEL, _eo_signals_efl_del_cb });

EFL_START_TEST(eo_signals)
{

   Eo *obj = efl_add_ref(SIMPLE_CLASS, NULL);
   Eina_Bool r;

   efl_event_callback_add(obj, EFL_EVENT_CALLBACK_ADD, _eo_signals_cb_added_deled, &_eo_signals_callbacks);
   r = efl_event_callback_add(obj, EFL_EVENT_CALLBACK_DEL, _eo_signals_cb_added_deled, &_eo_signals_callbacks);
   fail_if(!r);
   check_is_deled = 1;
   efl_event_callback_array_priority_add(obj, _eo_signals_callbacks(), -100, (void *) 1);
   efl_event_callback_array_add(obj, _eo_signals_callbacks(), (void *) 3);
   r = efl_event_callback_array_priority_add(obj, _eo_signals_callbacks(), -50, (void *) 2);
   fail_if(!r);
   simple_a_set(obj, 1);
   ck_assert_int_eq(_eo_signals_cb_flag, 0x3);

   efl_event_callback_array_del(obj, _eo_signals_callbacks(), (void *) 1);
   efl_event_callback_array_del(obj, _eo_signals_callbacks(), (void *) 2);
   r = efl_event_callback_array_del(obj, _eo_signals_callbacks(), (void *) 3);
   fail_if(!r);
   /* Try to delete something that doesn't exist. */
   r = efl_event_callback_array_del(obj, _eo_signals_callbacks(), (void *) 4);
   fail_if(r);
   _eo_signals_cb_flag = 0;
   simple_a_set(obj, 1);
   ck_assert_int_eq(_eo_signals_cb_flag, 0x0);

   r = efl_event_callback_array_add(obj, NULL, NULL);
   fail_if(r);

   efl_unref(obj);

   obj = efl_add_ref(SIMPLE_CLASS, NULL);
   /* Legacy support signals. */
     {
        const Efl_Event_Description *a_desc = efl_object_legacy_only_event_description_get("a,changed");
        fail_if(!a_desc);
        ck_assert_str_eq(a_desc->name, "a,changed");
        fail_if(a_desc == EV_A_CHANGED);

        /* Check that when calling again we still get the same event. */
        const Efl_Event_Description *a_desc2 = efl_object_legacy_only_event_description_get("a,changed");
        fail_if(!a_desc2);
        fail_if(a_desc2 != a_desc);

        const Efl_Event_Description *bad_desc = efl_object_legacy_only_event_description_get("bad");
        fail_if(!bad_desc);
        ck_assert_str_eq(bad_desc->name, "bad");

        /* Call Eo event with legacy and non-legacy callbacks. */
        _eo_signals_cb_current = 0;
        efl_event_callback_priority_add(obj, EV_A_CHANGED2, -1000, _eo_signals_a_changed_never, (void *) 1);
        efl_event_callback_priority_add(obj, EV_A_CHANGED2, 0, _eo_signals_a_changed_within_cb, NULL);
        efl_event_callback_priority_add(obj, EV_A_CHANGED, -100, _eo_signals_a_changed_cb, (void *) 1);
        efl_event_callback_add(obj, a_desc, _eo_signals_a_changed_cb2, NULL);
        simple_a_set(obj, 1);
        ck_assert_int_eq(_eo_signals_cb_flag, 0x3);

        /* We don't need this one anymore. */
        r = efl_event_callback_del(obj, EV_A_CHANGED2, _eo_signals_a_changed_never, (void *) 1);
        fail_if(!r);
        r = efl_event_callback_del(obj, a_desc, _eo_signals_a_changed_cb2, NULL);
        fail_if(!r);
        efl_event_callback_add(obj, EV_A_CHANGED, _eo_signals_a_changed_cb2, NULL);

        /* Call legacy event with legacy and non-legacy callbacks. */
        int a = 3;
        _eo_signals_cb_current = 0;
        _eo_signals_cb_flag = 0;
        efl_event_callback_legacy_call(obj, a_desc, &a);
        ck_assert_int_eq(_eo_signals_cb_flag, 0x3);

        /* Stop event within event. */
        _eo_signals_cb_current = 0;
        _eo_signals_cb_flag = 0;
        fail_if(!efl_event_callback_legacy_call(obj, EV_A_CHANGED2, &a));
        ck_assert_int_eq(_eo_signals_cb_flag, 0x8);
        fail_if(!r);
     }
   efl_unref(obj);

}
EFL_END_TEST

EFL_START_TEST(efl_data_fetch)
{

   /* Usually should be const, not const only for the test... */
   static Efl_Class_Description class_desc = {
        EO_VERSION,
        "Simple2",
        EFL_CLASS_TYPE_REGULAR,
        10,
        NULL,
        NULL,
        NULL
   };

   const Efl_Class *klass = efl_class_new(&class_desc, EO_CLASS, NULL);
   fail_if(!klass);

   Eo *obj = efl_add_ref(klass, NULL);
   fail_if(!obj);
#ifdef EO_DEBUG
   fail_if(efl_data_scope_get(obj, SIMPLE_CLASS));
#endif
   efl_unref(obj);

   class_desc.data_size = 0;
   klass = efl_class_new(&class_desc, EO_CLASS, NULL);
   fail_if(!klass);

   obj = efl_add_ref(klass, NULL);
   fail_if(!obj);
   fail_if(efl_data_scope_get(obj, klass));
   fail_if(!efl_data_scope_get(obj, EFL_OBJECT_CLASS));
   efl_unref(obj);

}
EFL_END_TEST

EFL_START_TEST(efl_data_safe_fetch)
{

   Eo *obj = efl_add_ref(SIMPLE2_CLASS, NULL);
   fail_if(!obj || !efl_data_scope_safe_get(obj, SIMPLE2_CLASS));
   efl_unref(obj);

   obj = efl_add_ref(SIMPLE3_CLASS, NULL);
   fail_if(!obj);
   fail_if(!efl_isa(obj, SIMPLE_CLASS));
   fail_if(!efl_isa(obj, SIMPLE2_CLASS));
   fail_if(!efl_isa(obj, SIMPLE3_CLASS));
   fail_if(!efl_data_scope_safe_get(obj, SIMPLE_CLASS));
   fail_if(!efl_data_scope_safe_get(obj, SIMPLE3_CLASS));
   fail_if(efl_data_scope_safe_get(obj, SIMPLE2_CLASS) != NULL);
   fail_if(efl_data_scope_safe_get(NULL, EFL_OBJECT_CLASS) != NULL);
   efl_unref(obj);
   fail_if(efl_data_scope_safe_get(obj, SIMPLE3_CLASS) != NULL);

}
EFL_END_TEST

EFL_START_TEST(efl_isa_tests)
{

   const Efl_Class *klass, *iface, *mixin;

     {
        /* Usually should be const, not const only for the test... */
        static Efl_Class_Description class_desc = {
             EO_VERSION,
             "Iface",
             EFL_CLASS_TYPE_INTERFACE,
             0,
             NULL,
             NULL,
             NULL
        };

        iface = efl_class_new(&class_desc, NULL, NULL);
        fail_if(!iface);
     }

     {
        /* Usually should be const, not const only for the test... */
        static Efl_Class_Description class_desc = {
             EO_VERSION,
             "Mixin",
             EFL_CLASS_TYPE_MIXIN,
             0,
             NULL,
             NULL,
             NULL
        };

        mixin = efl_class_new(&class_desc, NULL, NULL);
        fail_if(!mixin);
     }

     {
        /* Usually should be const, not const only for the test... */
        static Efl_Class_Description class_desc = {
             EO_VERSION,
             "Simple2",
             EFL_CLASS_TYPE_REGULAR,
             10,
             NULL,
             NULL,
             NULL
        };

        klass = efl_class_new(&class_desc, EO_CLASS, iface, mixin, NULL);
        fail_if(!klass);
     }

   Eo *obj = efl_add_ref(klass, NULL);
   fail_if(!obj);
   fail_if(efl_isa(obj, SIMPLE_CLASS));
   fail_if(!efl_isa(obj, iface));
   fail_if(!efl_isa(obj, mixin));
   fail_if(!efl_isa(obj, klass));
   fail_if(!efl_isa(obj, EO_CLASS));
   efl_unref(obj);

   obj = efl_add_ref(SIMPLE_CLASS, NULL);
   fail_if(!obj);
   fail_if(efl_isa(obj, klass));
   fail_if(efl_isa(obj, iface));
   fail_if(efl_isa(obj, mixin));
   fail_if(!efl_isa(obj, SIMPLE_CLASS));
   fail_if(!efl_isa(obj, EO_CLASS));
   efl_unref(obj);

}
EFL_END_TEST


EFL_START_TEST(efl_composite_tests)
{

   Eo *obj = efl_add_ref(SIMPLE_CLASS, NULL);
   fail_if(!obj);
   Eo *obj2 = efl_add_ref(SIMPLE_CLASS, NULL);
   fail_if(!obj2);
   Eo *obj3 = efl_add_ref(SIMPLE_CLASS, NULL);
   fail_if(!obj3);

   efl_composite_attach(obj, obj2);
   fail_if(!efl_composite_part_is(obj2));

   /* Check swapping attachments works. */
   efl_composite_attach(obj3, obj2);
   fail_if(!efl_composite_part_is(obj2));

   /* Check that a deletion of a child detaches from the parent. */
   efl_unref(obj2);
   fail_if(!efl_composite_attach(obj3, obj));

   /* Check that a deletion of the parent detaches the child. */
   efl_unref(obj3);
   fail_if(efl_composite_part_is(obj));

   efl_unref(obj);

}
EFL_END_TEST

static Eina_Bool _man_should_con = EINA_TRUE;
static Eina_Bool _man_should_des = EINA_TRUE;
static const Efl_Class *cur_klass = NULL;

static Eo *
_man_con(Eo *obj, void *data EINA_UNUSED, va_list *list EINA_UNUSED)
{
   if (_man_should_con)
      efl_manual_free_set(obj, EINA_TRUE);
   return efl_constructor(efl_super(obj, cur_klass));
}

static void
_man_des(Eo *obj, void *data EINA_UNUSED, va_list *list EINA_UNUSED)
{
   efl_destructor(efl_super(obj, cur_klass));
   if (_man_should_des)
      efl_manual_free_set(obj, EINA_FALSE);
}

static Eina_Bool
_class_initializer(Efl_Class *klass)
{
   EFL_OPS_DEFINE(ops,
         EFL_OBJECT_OP_FUNC(efl_constructor, _man_con),
         EFL_OBJECT_OP_FUNC(efl_destructor, _man_des),
   );

   return efl_class_functions_set(klass, &ops, NULL);
}

EFL_START_TEST(eo_man_free)
{

   /* Usually should be const, not const only for the test... */
   static Efl_Class_Description class_desc = {
        EO_VERSION,
        "Simple2",
        EFL_CLASS_TYPE_REGULAR,
        10,
        _class_initializer,
        NULL,
        NULL
   };

   const Efl_Class *klass = efl_class_new(&class_desc, EO_CLASS, NULL);
   fail_if(!klass);
   cur_klass = klass;

   Eo *obj = efl_add_ref(klass, NULL);
   fail_if(!obj);
   efl_unref(obj);

   obj = efl_add_ref(klass, NULL);
   fail_if(!obj);
   fail_if(efl_manual_free(obj));
   efl_unref(obj);

   _man_should_des = EINA_FALSE;
   klass = efl_class_new(&class_desc, EO_CLASS, NULL);
   cur_klass = klass;
   fail_if(!klass);

   obj = efl_add_ref(klass, NULL);
   fail_if(!obj);
   fail_if(efl_manual_free(obj));
   fail_if(efl_destructed_is(obj));
   efl_unref(obj);
   fail_if(!efl_destructed_is(obj));
   fail_if(!efl_manual_free(obj));

   obj = efl_add_ref(klass, NULL);
   fail_if(!obj);
   efl_unref(obj);
   fail_if(!efl_destructed_is(obj));
   fail_if(!efl_manual_free(obj));

   _man_should_con = EINA_FALSE;
   klass = efl_class_new(&class_desc, EO_CLASS, NULL);
   cur_klass = klass;
   fail_if(!klass);

   obj = efl_add_ref(klass, NULL);
   fail_if(!obj);
   fail_if(efl_manual_free(obj));
   efl_unref(obj);

   obj = efl_add_ref(klass, NULL);
   fail_if(!obj);
   efl_manual_free_set(obj, EINA_TRUE);
   efl_unref(obj);
   efl_ref(obj);
   efl_unref(obj);
   DISABLE_ABORT_ON_CRITICAL_START;
   efl_unref(obj);
   DISABLE_ABORT_ON_CRITICAL_END;
   fail_if(!efl_manual_free(obj));

   obj = efl_add_ref(klass, NULL);
   fail_if(!obj);
   efl_manual_free_set(obj, EINA_TRUE);
   efl_unref(obj);
   efl_ref(obj);
   efl_unref(obj);
   DISABLE_ABORT_ON_CRITICAL_START;
   efl_unref(obj);
   efl_unref(obj);
   efl_unref(obj);
   DISABLE_ABORT_ON_CRITICAL_END;
   fail_if(!efl_manual_free(obj));

}
EFL_END_TEST

EFL_START_TEST(efl_refs)
{
   Eo *obj = efl_add_ref(SIMPLE_CLASS, NULL);
   Eo *obj2 = efl_add_ref(SIMPLE_CLASS, NULL);
   Eo *obj3 = efl_add_ref(SIMPLE_CLASS, NULL);

   efl_xref(obj, obj2);
   fail_if(efl_ref_count(obj) != 2);
   efl_xref(obj, obj3);
   fail_if(efl_ref_count(obj) != 3);

   efl_xunref(obj, obj2);
   fail_if(efl_ref_count(obj) != 2);
   efl_xunref(obj, obj3);
   fail_if(efl_ref_count(obj) != 1);

#ifdef EO_DEBUG
   efl_xunref(obj, obj3);
   fail_if(efl_ref_count(obj) != 1);

   efl_xref(obj, obj2);
   fail_if(efl_ref_count(obj) != 2);

   efl_xunref(obj, obj3);
   fail_if(efl_ref_count(obj) != 2);

   efl_xunref(obj, obj2);
   fail_if(efl_ref_count(obj) != 1);
#endif

   /* Check we don't seg if there's an extra xref. */
   efl_xref(obj, obj2);
   efl_unref(obj);

   efl_unref(obj);
   efl_unref(obj2);
   efl_unref(obj3);

   /* Check hierarchy */
   obj = efl_add_ref(SIMPLE_CLASS, NULL);
   obj2 = efl_ref(efl_add(SIMPLE_CLASS, obj));

   Eo *wref = NULL;
   efl_wref_add(obj2, &wref);
   fail_if(!wref);

   efl_unref(obj2);

   fail_if(!wref); /* Parent is still holding a reference. */

   efl_unref(obj);

   fail_if(wref);

   /* efl_add_ref and normal efl_add */
   obj = efl_add_ref(SIMPLE_CLASS, NULL);
   obj2 = efl_add(SIMPLE_CLASS, obj);
   obj3 = efl_add_ref(SIMPLE_CLASS, obj);

   ck_assert_int_eq(efl_ref_count(obj), 1);
   ck_assert_int_eq(efl_ref_count(obj2), 1);
   ck_assert_int_eq(efl_ref_count(obj3), 2);

   efl_unref(obj);
   efl_del(obj2);
   efl_unref(obj3);
   efl_del(obj3);

   /* Setting and removing parents. */
   obj = efl_add_ref(SIMPLE_CLASS, NULL);
   obj2 = efl_ref(efl_add(SIMPLE_CLASS, obj));
   obj3 = efl_add_ref(SIMPLE_CLASS, NULL);

   efl_parent_set(obj, obj);
   ck_assert_ptr_eq(efl_parent_get(obj), NULL);

   efl_parent_set(obj2, obj3);
   efl_parent_set(obj3, obj);
   ck_assert_int_eq(efl_ref_count(obj2), 2);
   ck_assert_int_eq(efl_ref_count(obj3), 2);

   efl_parent_set(obj2, NULL);
   efl_parent_set(obj3, NULL);
   ck_assert_int_eq(efl_ref_count(obj2), 1);
   ck_assert_int_eq(efl_ref_count(obj3), 1);

   fail_if(!efl_invalidated_get(obj2));
   fail_if(!efl_invalidated_get(obj3));

   efl_unref(obj);
   efl_unref(obj2);
   efl_unref(obj3);

   /* Setting and removing parents for add_ref */
   obj = efl_add_ref(SIMPLE_CLASS, NULL);
   obj2 = efl_add_ref(SIMPLE_CLASS, obj);
   obj3 = efl_add_ref(SIMPLE_CLASS, NULL);

   ck_assert_int_eq(efl_ref_count(obj2), 2);
   ck_assert_int_eq(efl_ref_count(obj3), 1);

   efl_parent_set(obj2, obj3);
   efl_parent_set(obj3, obj);
   ck_assert_int_eq(efl_ref_count(obj2), 2);
   ck_assert_int_eq(efl_ref_count(obj3), 2);

   efl_parent_set(obj2, NULL);
   efl_parent_set(obj3, NULL);
   ck_assert_int_eq(efl_ref_count(obj2), 1);
   ck_assert_int_eq(efl_ref_count(obj3), 1);

   fail_if(!efl_invalidated_get(obj2));
   fail_if(!efl_invalidated_get(obj3));

   {
      int ref_pre = efl_ref_count(obj2);
      efl_parent_set(obj2, obj3);
      ck_assert_int_eq(ref_pre, efl_ref_count(obj2));
   }

   efl_unref(obj);
   efl_unref(obj2);
   efl_unref(obj3);

   /* Just check it doesn't seg atm. */
   obj = efl_add_ref(SIMPLE_CLASS, NULL);
   efl_ref(obj);
   efl_unref(obj);
   efl_unref(obj);

   obj = efl_add_ref(SIMPLE_CLASS, NULL);
   obj2 = efl_add(SIMPLE_CLASS, obj);
   DISABLE_ABORT_ON_CRITICAL_START;
   efl_unref(obj2);
   DISABLE_ABORT_ON_CRITICAL_END;
   efl_ref(obj2);
   efl_del(obj2);
   efl_unref(obj);

}
EFL_END_TEST

EFL_START_TEST(efl_weak_reference)
{

   Eo *obj = efl_add_ref(SIMPLE_CLASS, NULL);
   Eo *obj2 = efl_add_ref(SIMPLE_CLASS, NULL);
   Eo *wref = NULL, *wref2 = NULL, *wref3 = NULL;
   efl_wref_add(obj, &wref);
   fail_if(!wref);

   efl_unref(obj);
   fail_if(wref);

   obj = efl_add_ref(SIMPLE_CLASS, NULL);
   efl_wref_add(obj, &wref);

   efl_ref(obj);
   fail_if(!wref);

   efl_unref(obj);
   fail_if(!wref);

   efl_unref(obj);
   fail_if(wref);

   obj = efl_add_ref(SIMPLE_CLASS, NULL);

   efl_wref_add(obj, &wref);
   efl_wref_del(obj, &wref);
   fail_if(wref);

   efl_wref_add(obj, &wref);
   efl_wref_del(obj2, &wref);
   fail_if(!wref);
   efl_wref_del_safe(&wref);
   fail_if(wref);

   wref = obj;
   efl_wref_del(obj, &wref);
   fail_if(wref);

   wref = wref2 = wref3 = NULL;
   efl_wref_add(obj, &wref);
   efl_wref_add(obj, &wref2);
   efl_wref_add(obj, &wref3);
   fail_if(!wref);
   fail_if(!wref2);
   fail_if(!wref3);
   efl_wref_del(obj, &wref);
   efl_wref_del(obj, &wref2);
   efl_wref_del(obj, &wref3);
   fail_if(wref);
   fail_if(wref2);
   fail_if(wref3);

   efl_wref_add(obj, &wref2);
   efl_wref_add(obj, &wref3);
   wref = obj;
   efl_wref_del(obj, &wref);
   fail_if(wref);
   efl_wref_del(obj, &wref2);
   efl_wref_del(obj, &wref3);

   efl_unref(obj);
   efl_unref(obj2);


}
EFL_END_TEST

EFL_START_TEST(eo_generic_data)
{
   Eo *obj = efl_add_ref(SIMPLE_CLASS, NULL);
   Eo *obj2 = efl_add_ref(SIMPLE_CLASS, NULL);
   Eo *obj3 = efl_add_ref(SIMPLE_CLASS, NULL);
   Eo *objtmp;
   void *data = NULL;
   Eina_Value *value;
   Eina_Value *value2;



   efl_key_data_set(obj, "test1", (void *) 1);
   data = efl_key_data_get(obj, "test1");
   fail_if(1 != (intptr_t) data);

   efl_key_data_set(obj, "test1", NULL);
   data = efl_key_data_get(obj, "test1");
   fail_if(data);

   efl_key_data_set(obj, "test1", (void *) 1);
   efl_key_data_set(obj, "test2", (void *) 2);
   data = efl_key_data_get(obj, "test1");
   fail_if(1 != (intptr_t) data);

   data = efl_key_data_get(obj, "test2");
   fail_if(2 != (intptr_t) data);

   data = efl_key_data_get(obj, "test2");
   fail_if(2 != (intptr_t) data);

   efl_key_data_set(obj, "test2", NULL);
   data = efl_key_data_get(obj, "test2");
   fail_if(data);

   data = efl_key_data_get(obj, "test1");
   fail_if(1 != (intptr_t) data);

   efl_key_data_set(obj, "test1", NULL);
   data = efl_key_data_get(obj, "test1");
   fail_if(data);



   efl_key_ref_set(obj, "test1", obj2);
   objtmp = efl_key_ref_get(obj, "test1");
   fail_if(obj2 != objtmp);

   efl_key_ref_set(obj, "test1", NULL);
   objtmp = efl_key_ref_get(obj, "test1");
   fail_if(objtmp);

   efl_key_ref_set(obj, "test1", obj2);
   fail_if(efl_ref_count(obj2) != 2);

   efl_key_ref_set(obj, "test2", obj3);
   fail_if(efl_ref_count(obj3) != 2);

   objtmp = efl_key_ref_get(obj, "test1");
   fail_if(obj2 != objtmp);

   objtmp = efl_key_ref_get(obj, "test2");
   fail_if(obj3 != objtmp);

   data = efl_key_ref_get(obj, "test2");
   fail_if(obj3 != objtmp);

   efl_key_ref_set(obj, "test2", NULL);
   fail_if(efl_ref_count(obj3) != 1);

   objtmp = efl_key_ref_get(obj, "test2");
   fail_if(objtmp);

   objtmp = efl_key_ref_get(obj, "test1");
   fail_if(obj2 != objtmp);

   efl_key_ref_set(obj, "test1", NULL);
   fail_if(efl_ref_count(obj2) != 1);

   objtmp = efl_key_ref_get(obj, "test1");
   fail_if(objtmp);

   efl_key_ref_set(obj, "test1", obj2);
   efl_key_ref_set(obj, "test2", obj3);
   efl_unref(obj2);
   efl_unref(obj2);
   efl_unref(obj3);
   efl_unref(obj3);
   objtmp = efl_key_ref_get(obj, "test1");
   fail_if(objtmp);

   objtmp = efl_key_ref_get(obj, "test2");
   fail_if(objtmp);



   obj2 = efl_add_ref(SIMPLE_CLASS, NULL);
   obj3 = efl_add_ref(SIMPLE_CLASS, NULL);

   efl_key_wref_set(obj, "test1", obj2);
   objtmp = efl_key_wref_get(obj, "test1");
   fail_if(obj2 != objtmp);

   efl_key_wref_set(obj, "test1", NULL);
   objtmp = efl_key_wref_get(obj, "test1");
   fail_if(objtmp);

   efl_key_wref_set(obj, "test1", obj2);
   fail_if(efl_ref_count(obj2) != 1);

   efl_key_wref_set(obj, "test2", obj3);
   fail_if(efl_ref_count(obj3) != 1);

   objtmp = efl_key_wref_get(obj, "test1");
   fail_if(obj2 != objtmp);

   objtmp = efl_key_wref_get(obj, "test2");
   fail_if(obj3 != objtmp);

   data = efl_key_wref_get(obj, "test2");
   fail_if(obj3 != objtmp);

   efl_key_wref_set(obj, "test2", NULL);
   fail_if(efl_ref_count(obj3) != 1);

   objtmp = efl_key_wref_get(obj, "test2");
   fail_if(objtmp);

   objtmp = efl_key_wref_get(obj, "test1");
   fail_if(obj2 != objtmp);

   efl_key_wref_set(obj, "test1", NULL);
   fail_if(efl_ref_count(obj2) != 1);

   objtmp = efl_key_wref_get(obj, "test1");
   fail_if(objtmp);

   efl_key_wref_set(obj, "test1", obj2);
   efl_key_wref_set(obj, "test2", obj3);
   efl_unref(obj2);
   efl_unref(obj3);
   objtmp = efl_key_wref_get(obj, "test1");
   fail_if(objtmp);

   objtmp = efl_key_wref_get(obj, "test2");
   fail_if(objtmp);



   value = eina_value_new(EINA_VALUE_TYPE_INT);
   eina_value_set(value, 1234);
   value2 = efl_key_value_get(obj, "value1");
   fail_if(value2 != NULL);

   efl_key_value_set(obj, "value1", value);
   value2 = efl_key_value_get(obj, "value1");
   fail_if(value != value2);

   efl_key_value_set(obj, "value1", NULL);
   value2 = efl_key_value_get(obj, "value1");
   fail_if(value2 != NULL);

   efl_key_value_set(obj, "value1", NULL);

   efl_unref(obj);
   efl_unref(obj2);
   efl_unref(obj3);

}
EFL_END_TEST


EFL_START_TEST(eo_magic_checks)
{
   char _buf[sizeof(long)]; /* Just enough to hold eina magic + a bit more. */
   char *buf = _buf;

   memset(_buf, 1, sizeof(_buf));

   Eo *obj = efl_add(SIMPLE_CLASS, (Eo *) buf);
   fail_if(obj);

   while (1)
     {
        int i = 20, a = 0;
        Eo *parent = NULL;
        Eo *wref = NULL;
        Eo *obj2 = NULL;

        obj = efl_add((Efl_Class *) buf, NULL);
        fail_if(obj);

        obj = efl_add_ref(SIMPLE_CLASS, NULL);
        fail_if(!obj);

        simple_a_set((Eo *) buf, ++i);
        a = simple_a_get((Eo *) buf);
        ck_assert_int_ne(i, a);
        simple_a_set(efl_super((Eo *) buf, SIMPLE_CLASS), ++i);
        a = simple_a_get(efl_super((Eo *) buf, SIMPLE_CLASS));
        ck_assert_int_ne(i, a);
        simple_a_set(efl_super(obj, (const Efl_Class *) buf), ++i);
        a = simple_a_get(efl_super(obj, (const Efl_Class *) buf));
        ck_assert_int_ne(i, a);
        fail_if(efl_class_get((Eo *) buf));
        fail_if(efl_class_name_get((Efl_Class*) buf));
        fail_if(efl_class_get(obj) != SIMPLE_CLASS);
        fail_if(efl_class_get(SIMPLE_CLASS) != EFL_CLASS_CLASS);
        simple_a_set((Efl_Class *) buf, 1);
        simple_a_set(efl_super((Efl_Class *) buf, SIMPLE_CLASS), ++i);
        simple_a_set(efl_super(SIMPLE_CLASS, (Efl_Class *) buf), ++i);
        fail_if(efl_class_new(NULL, (Efl_Class *) buf), NULL);

        efl_xref(obj, (Eo *) buf);
        efl_xunref(obj, (Eo *) buf);
        efl_xref((Eo *) buf, obj);
        efl_xunref((Eo *) buf, obj);

        efl_ref((Eo *) buf);
        efl_unref((Eo *) buf);
        efl_del((Eo *) buf);

        efl_isa((Eo *) buf, SIMPLE_CLASS);
        efl_isa(obj, (Efl_Class *) buf);

        fail_if(0 != efl_ref_count((Eo *) buf));

        efl_wref_add((Eo *) buf, &wref);
        parent = efl_parent_get((Eo *) buf);
        fail_if(wref);
        fail_if(parent);

        fail_if(efl_data_scope_get((Eo *) buf, SIMPLE_CLASS));

        efl_composite_attach(obj, (Eo *) buf);
        efl_composite_detach(obj, (Eo *) buf);
        efl_composite_part_is((Eo *) buf);

        efl_event_callback_forwarder_add(obj, NULL, (Eo *) buf);
        efl_event_callback_forwarder_del(obj, NULL, (Eo *) buf);

        efl_manual_free_set((Eo *) buf, EINA_TRUE);
        efl_manual_free((Eo *) buf);
        efl_destructed_is((Eo *) buf);

        obj2 = NULL;
        efl_parent_set(obj, (Eo *) buf);
        obj2 = efl_parent_get(obj);
        fail_if(obj2 && (obj2 == (Eo *) buf));

        efl_unref(obj);

        if (!buf)
           break;
        else
           buf = NULL;
     }

}
EFL_END_TEST

/* resolve issues */

static Eina_Bool
_a_print(Eo *obj EINA_UNUSED, void *class_data EINA_UNUSED)
{
   printf("Hey\n");

   return EINA_TRUE;
}

EFL_FUNC_BODY(resolve_a_print, Eina_Bool, EINA_FALSE);

static Eina_Bool
_multi_class_initializer(Efl_Class *klass)
{
   EFL_OPS_DEFINE(ops,
         EFL_OBJECT_OP_FUNC(resolve_a_print, _a_print),
   );

   return efl_class_functions_set(klass, &ops, NULL);
}

EFL_START_TEST(efl_func_resolve)
{

   /* Usually should be const, not const only for the test... */
   static Efl_Class_Description class_desc = {
        EO_VERSION,
        "Inherit",
        EFL_CLASS_TYPE_REGULAR,
        0,
        _multi_class_initializer,
        NULL,
        NULL
   };

   const Efl_Class *klass = efl_class_new(&class_desc, SIMPLE_CLASS, NULL);
   fail_if(!klass);

   Eo *obj = efl_add_ref(klass, NULL);
   fail_if(!resolve_a_print(obj));
   efl_unref(obj);


   obj = efl_add_ref(SIMPLE_CLASS, NULL);
   fail_if(!obj);
   efl_manual_free_set(obj, EINA_TRUE);

   fail_if(resolve_a_print(obj));

   efl_unref(obj);

   fail_if(!efl_destructed_is(obj));
   efl_manual_free(obj);

}
EFL_END_TEST

EFL_START_TEST(efl_add_do_and_custom)
{
   Simple_Public_Data *pd = NULL;
   Eo *obj = NULL;

   obj = efl_add_ref(SIMPLE_CLASS, NULL, efl_constructor(efl_added));
   fail_if(!obj);
   efl_unref(obj);

   obj = efl_add_ref(SIMPLE_CLASS, NULL, simple_a_set(efl_added, 7));
   fail_if(!obj);
   pd = efl_data_scope_get(obj, SIMPLE_CLASS);
   fail_if(pd->a != 7);
   efl_unref(obj);

   obj = efl_add_ref(SIMPLE_CLASS, NULL, efl_constructor(efl_added), simple_a_set(efl_added, 7));
   fail_if(!obj);
   pd = efl_data_scope_get(obj, SIMPLE_CLASS);
   fail_if(pd->a != 7);
   efl_unref(obj);

   Eina_Bool finalized;
   obj = efl_add_ref(SIMPLE_CLASS, NULL, finalized = efl_finalized_get(efl_added));
   fail_if(finalized);

   finalized = efl_finalized_get(obj);
   fail_if(!finalized);
   efl_unref(obj);

}
EFL_END_TEST

EFL_START_TEST(eo_pointers_indirection)
{
   static const Efl_Class_Description class_desc = {
        EO_VERSION,
        "Simple",
        EFL_CLASS_TYPE_REGULAR,
        0,
        NULL,
        NULL,
        NULL
   };

   const Efl_Class *klass = efl_class_new(&class_desc, EO_CLASS, NULL);
   fail_if(!klass);

   /* Check simple id validity */
   Eo *obj = efl_add_ref(klass, NULL);
   fail_if(!obj);
   fail_if(!efl_isa(obj, klass));
   obj = (Eo *)((char *)(obj) + 1);
   fail_if(efl_isa(obj, klass));
   obj = (Eo *)((char *)(obj) - 1);
   fail_if(!efl_isa(obj, klass));
   efl_unref(obj);
   fail_if(efl_isa(obj, klass));

   /* Check id invalidity after deletion */
   Eo *obj1 = efl_add_ref(klass, NULL);
   fail_if(!obj1);
   efl_unref(obj1);
   Eo *obj2 = efl_add_ref(klass, NULL);
   fail_if(!obj2);
   fail_if(!efl_isa(obj2, klass));
   fail_if(efl_isa(obj1, klass));
   efl_unref(obj2);

   /* Check id sanity checks for "close enough" ids. */
   obj1 = efl_add_ref(klass, NULL);
   fail_if(!obj1);
   obj2 = (Eo *) (((Eo_Id) obj1) & ~MASK_OBJ_TAG);
   fail_if(efl_class_get(obj2));
   efl_unref(obj1);

#define NB_OBJS 10000
   unsigned int obj_id;
   Eo **objs = calloc(NB_OBJS, sizeof(Eo *));
   fail_if(!objs);
   /* Creation of the objects */
   for ( obj_id = 0; obj_id < NB_OBJS; obj_id++)
     {
        objs[obj_id] = efl_add_ref(klass, NULL);
        if(!objs[obj_id])
          fail_if(!objs[obj_id]);
        if(!efl_isa(objs[obj_id], klass))
          fail_if(!efl_isa(objs[obj_id], klass));
     }
   /* Deletion of a few objects */
   for ( obj_id = 0; obj_id < NB_OBJS; obj_id+=2000)
     {
        efl_unref(objs[obj_id]);
        if(efl_isa(objs[obj_id], klass))
          fail_if(efl_isa(objs[obj_id], klass));
     }
   /* Creation of the deleted objects */
   for ( obj_id = 0; obj_id < NB_OBJS; obj_id+=2000)
     {
        objs[obj_id] = efl_add_ref(klass, NULL);
        if(!objs[obj_id])
          fail_if(!objs[obj_id]);
        if(!efl_isa(objs[obj_id], klass))
          fail_if(!efl_isa(objs[obj_id], klass));
     }
   /* Deletion of all the objects */
   for ( obj_id = 0; obj_id < NB_OBJS; obj_id++)
      efl_unref(objs[obj_id]);
   /* Just be sure that we trigger an already freed error */
   efl_unref(objs[0]);
   free(objs);
}
EFL_END_TEST


static Eo *
_efl_add_failures_finalize(Eo *obj EINA_UNUSED, void *class_data EINA_UNUSED)
{
   return NULL;
}

static Eina_Bool
_add_failures_class_initializer(Efl_Class *klass)
{
   EFL_OPS_DEFINE(ops,
         EFL_OBJECT_OP_FUNC(efl_finalize, _efl_add_failures_finalize),
   );

   return efl_class_functions_set(klass, &ops, NULL);
}

EFL_START_TEST(efl_add_failures)
{

   static const Efl_Class_Description class_desc = {
        EO_VERSION,
        "Simple2",
        EFL_CLASS_TYPE_REGULAR,
        0,
        _add_failures_class_initializer,
        NULL,
        NULL
   };

   const Efl_Class *klass = efl_class_new(&class_desc, EO_CLASS, NULL);

   Eo *obj = efl_add_ref(klass, NULL);

   fail_if(obj);

}
EFL_END_TEST

static Eina_Bool intercepted = EINA_FALSE;

static void
_del_intercept(Eo *obj)
{
   intercepted = EINA_TRUE;
   efl_del_intercept_set(obj, NULL);
   efl_unref(obj);
}

static void
_del_intercept_reuse(Eo *obj)
{
   efl_reuse(obj);
}

EFL_START_TEST(efl_del_intercept)
{

   static const Efl_Class_Description class_desc = {
        EO_VERSION,
        "Simple",
        EFL_CLASS_TYPE_REGULAR,
        0,
        NULL,
        NULL,
        NULL
   };

   const Efl_Class *klass = efl_class_new(&class_desc, EO_CLASS, NULL);
   fail_if(!klass);

   /* Check unref interception */
   intercepted = EINA_FALSE;
   Eo *obj = efl_add_ref(klass, NULL);
   fail_if(!obj);
   fail_if(!efl_isa(obj, klass));
   efl_del_intercept_set(obj, _del_intercept);
   efl_unref(obj);
   fail_if(!intercepted);
   fail_if(efl_isa(obj, klass));

   /* Check del interception */
   intercepted = EINA_FALSE;
   obj = efl_add_ref(klass, NULL);
   fail_if(!obj);
   fail_if(!efl_isa(obj, klass));
   efl_del_intercept_set(obj, _del_intercept);
   efl_unref(obj);
   fail_if(!intercepted);
   fail_if(efl_isa(obj, klass));

   /* Check reuse works as expected. */
   Eo *parent = efl_add_ref(SIMPLE_CLASS, NULL);
   obj = efl_add_ref(klass, NULL);
   fail_if(!obj);
   ck_assert_int_eq(efl_ref_count(obj), 1);
   efl_parent_set(obj, parent);
   ck_assert_int_eq(efl_ref_count(obj), 2);
   efl_del_intercept_set(obj, _del_intercept_reuse);
   efl_del_intercept_set(obj, NULL);
   /* This essentially checks it get unsunk */
   ck_assert_int_eq(efl_ref_count(obj), 2);
   efl_parent_set(obj, parent);
   ck_assert_int_eq(efl_ref_count(obj), 2);
   efl_unref(obj);

}
EFL_END_TEST

EFL_START_TEST(efl_name)
{
   Eo *obj = efl_add_ref(SIMPLE_CLASS, NULL);
   Eo *obj2 = efl_add_ref(SIMPLE_CLASS, NULL);
   Eo *obj3 = efl_add_ref(SIMPLE_CLASS, NULL);
   Eo *objtmp;
   const char *id;

   id = efl_name_get(obj);
   fail_if(NULL != id);

   efl_name_set(obj, "Hello");
   id = efl_name_get(obj);
   fail_if(NULL == id);
   fail_if(!!strcmp(id, "Hello"));

   efl_name_set(obj, "Hello");
   efl_name_set(obj, "");
   id = efl_name_get(obj);
   fail_if(NULL != id);

   efl_name_set(obj, "Hello");
   efl_name_set(obj, NULL);
   id = efl_name_get(obj);
   fail_if(NULL != id);

   efl_name_set(obj2, "joe");
   efl_name_set(obj3, "bob");

   efl_parent_set(obj2, obj);
   efl_parent_set(obj3, obj2);

   objtmp = efl_name_find(obj, "bob");
   fail_if(objtmp != obj3);

   objtmp = efl_name_find(obj, "joe");
   fail_if(objtmp != obj2);

   objtmp = efl_name_find(obj, "bo*");
   fail_if(objtmp != obj3);

   objtmp = efl_name_find(obj, "*oe");
   fail_if(objtmp != obj2);

   objtmp = efl_name_find(obj, "Simple:*oe");
   fail_if(objtmp != obj2);

   objtmp = efl_name_find(obj, "*mple:joe");
   fail_if(objtmp != obj2);

   efl_unref(obj);

}
EFL_END_TEST

EFL_START_TEST(eo_comment)
{
   Eo *obj = efl_add_ref(SIMPLE_CLASS, NULL);
   const char *comment;

   comment = efl_comment_get(obj);
   fail_if(NULL != comment);

   efl_comment_set(obj, "Hello");
   comment = efl_comment_get(obj);
   fail_if(NULL == comment);
   fail_if(!!strcmp(comment, "Hello"));

   efl_comment_set(obj, "Hello");
   efl_comment_set(obj, "");
   comment = efl_comment_get(obj);
   fail_if(NULL != comment);

   efl_comment_set(obj, "Hello");
   efl_comment_set(obj, NULL);
   comment = efl_comment_get(obj);
   fail_if(NULL != comment);

   efl_unref(obj);

}
EFL_END_TEST

EFL_START_TEST(eo_rec_interface)
{
   Eo *s = efl_add_ref(SEARCHABLE_CLASS, NULL);
   Eo *obj = efl_add(SIMPLE_CLASS, s);
   Eo *obj2 = efl_add(SIMPLE_CLASS, obj);
   Eo *objtmp;

   objtmp = efl_provider_find(obj2, SEARCHABLE_CLASS);
   fail_if(objtmp != s);

   efl_del(obj);

}
EFL_END_TEST

typedef struct
{
   Eo *obj, *objs;
   Efl_Domain_Data *dat;
} Data;

static void *
thr1(void *data, Eina_Thread t EINA_UNUSED)
{
   Data *d = data;
   Efl_Id_Domain dom;
   Eo *s1, *s2;

   fail_if(efl_domain_switch(EFL_ID_DOMAIN_THREAD) != EINA_TRUE);
   fail_if(efl_domain_get() != EFL_ID_DOMAIN_THREAD);
   printf("ADD2\n");
   Eo *obj = efl_add_ref(DOMAIN_CLASS, NULL);
   printf("ADD2 DONE = %p\n", obj);

   printf("VERIFY finalized_get()\n");
   fail_if(!efl_finalized_get(d->objs));

   s2 = efl_add_ref(DOMAIN_CLASS, obj);
   ck_assert(s2);
   efl_parent_set(s2, NULL);
   efl_unref(s2);

   printf("VERIFY parent_set(invalid) -- WILL SHOW ERRORS\n");
   efl_domain_current_push(EFL_ID_DOMAIN_SHARED);
   s1 = efl_add_ref(DOMAIN_CLASS, NULL);
   efl_domain_current_pop();
   efl_unref(s1);
   efl_parent_set(d->objs, s1);
   printf("END OF ERRORS\n");

   printf("VERIFY parent_set(invalid2) -- WILL SHOW ERRORS\n");
   efl_domain_current_push(EFL_ID_DOMAIN_SHARED);
   s1 = efl_add_ref(DOMAIN_CLASS, NULL);
   s2 = efl_add_ref(DOMAIN_CLASS, s1);
   efl_domain_current_pop();
   efl_parent_set(s2, NULL);
   efl_parent_set(s1, s2);
   efl_unref(s1);
   efl_unref(s2);
   printf("END OF ERRORS\n");

   printf("VERIFY composite\n");
   efl_domain_current_push(EFL_ID_DOMAIN_SHARED);
   s1 = efl_add_ref(SIMPLE_CLASS, NULL, simple_a_set(efl_added, 7));
   s2 = efl_add_ref(SIMPLE_CLASS, NULL, simple_a_set(efl_added, 42));
   efl_domain_current_pop();

   efl_composite_attach(d->objs, s1);
   int i1 = simple_a_get(d->objs);
   int i2 = simple_a_get(s1);
   fail_if(i1 != i2);
   fail_if(efl_composite_attach(d->objs, s2));
   efl_unref(s1);
   fail_if(!efl_composite_attach(d->objs, s2));
   efl_unref(s2);

   printf("SET ON LOCAL\n");
   domain_a_set(obj, 1234);
   fail_if(domain_a_get(obj) != 1234);
   printf("SET ON LOCAL DONE\n");

   printf("SET ON SHARED\n");
   domain_a_set(d->objs, 1234);
   fail_if(domain_a_get(d->objs) != 1234);
   printf("SET ON SHARED DONE\n");

   printf("FAAAAIL check %p\n", d->obj);
   domain_a_set(d->obj, 1234);
   int v = domain_a_get(d->obj);
   printf("........ v = %i\n", v);
   fail_if(v == 1234);
   printf("FAAAAIL DONE\n");

   printf("ADOPT...\n");
   dom = efl_domain_data_adopt(d->dat);
   fail_if(dom != EFL_ID_DOMAIN_MAIN);
   printf("SUCCEED check %p\n", d->obj);
   domain_a_set(d->obj, 8910);
   v = domain_a_get(d->obj);
   printf("........ v = %i\n", v);
   fail_if(v != 8910);
   printf("SUCCEED DONE\n");

   printf("RETURN DOMAIN DATA\n");
   fail_if(efl_domain_data_return(dom) != EINA_TRUE);
   printf("RETURN DOMAIN DATA DONE\n");

   efl_unref(obj);
   return NULL;
}

#ifndef _WIN32
static void
_timeout(int val EINA_UNUSED)
{
   EINA_LOG_CRIT("TIMED OUT!");
   exit(-1);
}
#endif

EFL_START_TEST(eo_domain)
{
   Eo *obj, *objs;

   printf("@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@\n");

#ifndef _WIN32
   if (!eina_streq(getenv("CK_FORK"), "no"))
     {
        signal(SIGALRM, _timeout);
        alarm(10);
     }
#endif

   fail_if(efl_domain_get() != EFL_ID_DOMAIN_MAIN);

   fail_if(efl_domain_switch(EFL_ID_DOMAIN_THREAD) != EINA_TRUE);

   fail_if(efl_domain_get() != EFL_ID_DOMAIN_THREAD);

   fail_if(efl_domain_switch(EFL_ID_DOMAIN_MAIN) != EINA_TRUE);

   fail_if(efl_domain_get() != EFL_ID_DOMAIN_MAIN);

   fail_if(efl_domain_current_get() != EFL_ID_DOMAIN_MAIN);

   fail_if(efl_domain_current_set(EFL_ID_DOMAIN_SHARED) != EINA_TRUE);

   fail_if(efl_domain_current_get() != EFL_ID_DOMAIN_SHARED);

   fail_if(efl_domain_current_set(EFL_ID_DOMAIN_MAIN) != EINA_TRUE);

   fail_if(efl_domain_current_get() != EFL_ID_DOMAIN_MAIN);

   fail_if(efl_domain_current_push(EFL_ID_DOMAIN_SHARED) != EINA_TRUE);

   fail_if(efl_domain_current_get() != EFL_ID_DOMAIN_SHARED);

   fail_if(efl_domain_current_push(EFL_ID_DOMAIN_THREAD) != EINA_TRUE);

   fail_if(efl_domain_current_get() != EFL_ID_DOMAIN_THREAD);

   efl_domain_current_pop();

   fail_if(efl_domain_current_get() != EFL_ID_DOMAIN_SHARED);

   efl_domain_current_pop();

   fail_if(efl_domain_current_get() != EFL_ID_DOMAIN_MAIN);

   printf("ADD1\n");
   obj = efl_add_ref(DOMAIN_CLASS, NULL);
   printf("ADD1 DONE = %p\n", obj);

   efl_domain_current_push(EFL_ID_DOMAIN_SHARED);
   printf("ADDS\n");
   objs = efl_add_ref(DOMAIN_CLASS, NULL, domain_a_set(efl_added, 42));
   printf("ADDS DONE = %p\n", objs);
   efl_domain_current_pop();

   fail_if(efl_compatible(objs, obj) == EINA_TRUE);

   domain_a_set(obj, 1234);
   fail_if(domain_a_get(obj) != 1234);

   domain_a_set(objs, 1234);
   fail_if(domain_a_get(objs) != 1234);

   ck_assert(SIMPLE_CLASS);

   Eina_Thread t;
   Data data;
   data.obj = obj;
   data.objs = objs;
   printf("GET DOMAIN DATA\n");
   data.dat = efl_domain_data_get();
   fail_if(data.dat == NULL);
   printf("THR CREATE\n");
   fail_if(!eina_thread_create(&t, EINA_THREAD_NORMAL, -1, thr1, &data));
   printf("JOIN\n");
   eina_thread_join(t);
   printf("JOIN DONE\n");

   printf("DELETING OBJECTS\n");
   efl_unref(obj);
   efl_unref(objs);

   printf("@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@\n");

#ifndef _WIN32
   alarm(0);
   signal(SIGALRM, NULL);
#endif
}
EFL_END_TEST


static int
_inherit_value_1(Eo *obj EINA_UNUSED, void *pd EINA_UNUSED)
{
   return 1;
}

static int
_inherit_value_2(Eo *obj EINA_UNUSED, void *pd EINA_UNUSED)
{
   return 2;
}

EFL_FUNC_BODY(inherit_value, int, 0);

static Eina_Bool
_cast_inherit_class_initializer_1(Efl_Class *klass)
{
   EFL_OPS_DEFINE(ops, EFL_OBJECT_OP_FUNC(inherit_value, _inherit_value_1), );
   return efl_class_functions_set(klass, &ops, NULL);
}

static Eina_Bool
_cast_inherit_class_initializer_2(Efl_Class *klass)
{
   EFL_OPS_DEFINE(ops, EFL_OBJECT_OP_FUNC(inherit_value, _inherit_value_2), );
   return efl_class_functions_set(klass, &ops, NULL);
}

EFL_START_TEST(efl_cast_test)
{

   static const Efl_Class_Description class_desc_1 = {
        EO_VERSION,
        "FirstInherit",
        EFL_CLASS_TYPE_REGULAR,
        0,
        _cast_inherit_class_initializer_1,
        NULL,
        NULL
   };

   static const Efl_Class_Description class_desc_2 = {
        EO_VERSION,
        "SecondInherit",
        EFL_CLASS_TYPE_REGULAR,
        0,
        _cast_inherit_class_initializer_2,
        NULL,
        NULL
   };

   const Efl_Class *klass1 = efl_class_new(&class_desc_1, SIMPLE_CLASS, NULL);
   fail_if(!klass1);

   const Efl_Class *klass2 = efl_class_new(&class_desc_2, klass1, NULL);
   fail_if(!klass2);

   Eo *obj;

   // Testing normal calls
   obj = efl_add_ref(SIMPLE_CLASS, NULL);
   fail_if(!obj);
   ck_assert_int_eq(inherit_value(obj), 0);
   efl_unref(obj);

   obj = efl_add_ref(klass1, NULL);
   fail_if(!obj);
   ck_assert_int_eq(inherit_value(obj), 1);
   efl_unref(obj);

   obj = efl_add_ref(klass2, NULL);
   fail_if(!obj);
   ck_assert_int_eq(inherit_value(obj), 2);
   efl_unref(obj);

   // Testing efl_super
   obj = efl_add_ref(klass2, NULL);
   fail_if(!obj);
   ck_assert_int_eq(inherit_value(efl_super(obj, klass2)), 1);
   ck_assert_int_eq(inherit_value(efl_super(obj, klass1)), 0);
   efl_unref(obj);

   // Testing efl_cast
   obj = efl_add_ref(klass2, NULL);
   fail_if(!obj);
   ck_assert_int_eq(inherit_value(efl_cast(obj, klass2)), 2);
   ck_assert_int_eq(inherit_value(efl_cast(obj, klass1)), 1);
   ck_assert_int_eq(inherit_value(efl_cast(obj, SIMPLE_CLASS)), 0);
   efl_unref(obj);

}
EFL_END_TEST

static void _destruct_test_del_cb(void *data, const Efl_Event *ev EINA_UNUSED)
{
   int *var = data;
   *var = 1;
}

static void _destruct_test_destruct_cb(void *data, const Efl_Event *ev)
{
   int *var = data;
   *var *= 2;

   ck_assert_int_eq(efl_ref_count(ev->object), 0);

   // test disabled: object isn't yet marked as destructed (we're inside the
   // base class destructor here).
   //ck_assert_int_ne(efl_destructed_is(ev->object), 0);
}

EFL_START_TEST(efl_object_destruct_test)
{
   int var = 0;
   Eo *obj;


   obj = efl_add_ref(SIMPLE_CLASS, NULL);
   fail_if(efl_ref_count(obj) != 1);
   efl_event_callback_add(obj, EFL_EVENT_DEL, _destruct_test_del_cb, &var);
   efl_event_callback_add(obj, EFL_EVENT_DESTRUCT, _destruct_test_destruct_cb, &var);
   efl_unref(obj);

   // var should be 2 if del then destruct, 0 otherwise
   ck_assert_int_eq(var, 2);

}
EFL_END_TEST

static void
_auto_unref_del_cb(void *data, const Efl_Event *ev EINA_UNUSED)
{
   *((int *) data) = 1;
}

EFL_START_TEST(efl_object_auto_unref_test)
{
   int _auto_unref_del;
   Eo *obj, *parent;


   // Test unref after valid call
   _auto_unref_del = 0;
   obj = efl_add_ref(SIMPLE_CLASS, NULL);
   fail_if(efl_ref_count(obj) != 1);
   efl_event_callback_add(obj, EFL_EVENT_DEL, _auto_unref_del_cb, &_auto_unref_del);
   ___efl_auto_unref_set(obj, 1);
   fail_if(_auto_unref_del);
   fail_if(efl_ref_count(obj) != 1);
   efl_name_set(obj, "name");
   fail_if(!_auto_unref_del);

   // Test unref after invalid call
   _auto_unref_del = 0;
   obj = efl_add_ref(SIMPLE_CLASS, NULL);
   fail_if(efl_ref_count(obj) != 1);
   efl_event_callback_add(obj, EFL_EVENT_DEL, _auto_unref_del_cb, &_auto_unref_del);
   ___efl_auto_unref_set(obj, 1);
   fail_if(_auto_unref_del);
   fail_if(efl_ref_count(obj) != 1);
   simple_no_implementation(obj);
   fail_if(!_auto_unref_del);

   // Same with a parent
   _auto_unref_del = 0;
   parent = efl_add_ref(SIMPLE_CLASS, NULL);
   obj = efl_add(SIMPLE_CLASS, parent);
   fail_if(efl_ref_count(obj) != 1);
   efl_allow_parent_unref_set(obj, 1);
   efl_event_callback_add(obj, EFL_EVENT_DEL, _auto_unref_del_cb, &_auto_unref_del);
   ___efl_auto_unref_set(obj, 1);
   fail_if(_auto_unref_del);
   fail_if(efl_ref_count(obj) != 1);
   efl_name_set(obj, "name");
   fail_if(!_auto_unref_del);
   efl_unref(parent);

}
EFL_END_TEST

EFL_START_TEST(efl_object_size)
{
   // This test is checking that we are not increasing the size of our object over time
   // Update this number only if you modified the class size on purpose
   ck_assert_int_le(efl_class_memory_size_get(SIMPLE_CLASS), 148);
}
EFL_END_TEST

void eo_test_general(TCase *tc)
{
   tcase_add_test(tc, eo_simple);
   tcase_add_test(tc, eo_singleton);
   tcase_add_test(tc, efl_object_override_tests);
   tcase_add_test(tc, eo_signals);
   tcase_add_test(tc, efl_data_fetch);
   tcase_add_test(tc, efl_data_safe_fetch);
   tcase_add_test(tc, efl_isa_tests);
   tcase_add_test(tc, efl_composite_tests);
   tcase_add_test(tc, eo_man_free);
   tcase_add_test(tc, efl_refs);
   tcase_add_test(tc, efl_weak_reference);
   tcase_add_test(tc, eo_generic_data);
   tcase_add_test(tc, eo_magic_checks);
   tcase_add_test(tc, efl_func_resolve);
   tcase_add_test(tc, efl_add_do_and_custom);
   tcase_add_test(tc, eo_pointers_indirection);
   tcase_add_test(tc, efl_add_failures);
   tcase_add_test(tc, efl_del_intercept);
   tcase_add_test(tc, efl_name);
   tcase_add_test(tc, eo_comment);
   tcase_add_test(tc, eo_rec_interface);
   tcase_add_test(tc, eo_domain);
   tcase_add_test(tc, efl_cast_test);
   tcase_add_test(tc, efl_object_destruct_test);
   tcase_add_test(tc, efl_object_auto_unref_test);
   tcase_add_test(tc, efl_object_size);
}
