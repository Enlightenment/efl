#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <stdio.h>

#include <Eo.h>

#include "eo_suite.h"
#include "eo_test_class_simple.h"
#include "eo_test_class_singleton.h"

/* Loading this internal header for testing purposes. */
#include "eo_ptr_indirection.h"

START_TEST(eo_simple)
{
   eo_init();
   Eo *obj = eo_add(EO_CLASS, NULL);
   fail_if(obj);

   obj = eo_add(SIMPLE_CLASS, NULL);
   fail_if(!obj);
   eo_constructor(obj);
   eo_destructor(obj);
   eo_unref(obj);

   eo_shutdown();
}
END_TEST

START_TEST(eo_singleton)
{
   eo_init();

   Eo *obj = eo_add(SINGLETON_CLASS, NULL);
   fail_if(!obj);

   Eo *obj2 = eo_add(SINGLETON_CLASS, NULL);
   fail_if(!obj2);

   ck_assert_ptr_eq(obj, obj2);

   eo_unref(obj);
   eo_unref(obj2);

   eo_shutdown();
}
END_TEST

#define OVERRIDE_A_SIMPLE 100859
#define OVERRIDE_A 324000
static int
_simple_obj_override_a_get(Eo *obj, void *class_data EINA_UNUSED)
{
   return OVERRIDE_A + simple_a_get(eo_super(obj, EO_OVERRIDE_CLASS));
}

static void
_simple_obj_override_a_double_set(Eo *obj, void *class_data EINA_UNUSED, int a)
{
   simple_a_set(eo_super(obj, EO_OVERRIDE_CLASS), 2 * a);
}

START_TEST(eo_override_tests)
{
   eo_init();

   Eo *obj = eo_add(SIMPLE_CLASS, NULL);
   fail_if(!obj);

   /* First get the value before the override to make sure it works and to
    * make sure we don't cache. */
   ck_assert_int_eq(simple_a_get(obj), 0);

   EO_OVERRIDE_OPS_DEFINE(
            overrides,
            EO_OP_FUNC_OVERRIDE(simple_a_get, _simple_obj_override_a_get));
   fail_if(!eo_override(obj, &overrides));

   ck_assert_int_eq(simple_a_get(obj), OVERRIDE_A);

   /* Check super works. */
   simple_a_set(obj, OVERRIDE_A_SIMPLE);
   ck_assert_int_eq(simple_a_get(obj), OVERRIDE_A + OVERRIDE_A_SIMPLE);

   /* Override again. */
   EO_OVERRIDE_OPS_DEFINE(
            overrides2,
            EO_OP_FUNC_OVERRIDE(simple_a_set, _simple_obj_override_a_double_set));
   fail_if(!eo_override(obj, NULL));
   fail_if(!eo_override(obj, &overrides2));

   simple_a_set(obj, OVERRIDE_A_SIMPLE);
   ck_assert_int_eq(simple_a_get(obj), OVERRIDE_A_SIMPLE * 2);

   /* Try overriding again - not allowed by policy */
   fail_if(eo_override(obj, &overrides));
   ck_assert_int_eq(simple_a_get(obj), OVERRIDE_A_SIMPLE * 2);

   /* Try introducing a new function */
   EO_OVERRIDE_OPS_DEFINE(
            overrides3,
            EO_OP_FUNC(simple2_class_beef_get, _simple_obj_override_a_double_set));
   fail_if(!eo_override(obj, NULL));
   fail_if(eo_override(obj, &overrides3));

   /* Test override reset */
   fail_if(!eo_override(obj, NULL));
   simple_a_set(obj, 42 * OVERRIDE_A_SIMPLE);
   ck_assert_int_eq(simple_a_get(obj), 42 * OVERRIDE_A_SIMPLE);

   eo_unref(obj);

   eo_shutdown();
}
END_TEST

static int _eo_signals_cb_current = 0;
static int _eo_signals_cb_flag = 0;

static void
_eo_signals_a_changed_cb(void *_data, const Eo_Event *event EINA_UNUSED)
{
   int data = (intptr_t) _data;
   _eo_signals_cb_current++;
   ck_assert_int_eq(data, _eo_signals_cb_current);
   _eo_signals_cb_flag |= 0x1;
}

static void
_eo_signals_a_changed_cb2(void *_data EINA_UNUSED, const Eo_Event *event EINA_UNUSED)
{
   _eo_signals_cb_flag |= 0x2;
   eo_event_callback_stop(event->object);
}

static void
_eo_signals_a_changed_within_cb(void *_data EINA_UNUSED, const Eo_Event *event)
{
   int a = 3;
   eo_event_callback_call(event->object, EV_A_CHANGED, &a);
   _eo_signals_cb_flag = 0x8;
}

static void
_eo_signals_a_changed_never(void *_data EINA_UNUSED, const Eo_Event *event EINA_UNUSED)
{
   /* This one should never be called. */
   fail_if(1);
}

static void
_eo_signals_eo_del_cb(void *_data EINA_UNUSED, const Eo_Event *event EINA_UNUSED)
{
   _eo_signals_cb_flag |= 0x4;
}

void
_eo_signals_cb_added_deled(void *data, const Eo_Event *event)
{
   const Eo_Callback_Array_Item *callback_array = event->info;
   const Eo_Callback_Array_Item *(*callback_data)(void) = data;

   fail_if((callback_data() != callback_array) &&
           (callback_array->func != _eo_signals_cb_added_deled));
}

EO_CALLBACKS_ARRAY_DEFINE(_eo_signals_callbacks,
{ EV_A_CHANGED, _eo_signals_a_changed_cb },
{ EV_A_CHANGED, _eo_signals_a_changed_cb2 },
{ EV_A_CHANGED, _eo_signals_a_changed_never },
{ EO_EVENT_DEL, _eo_signals_eo_del_cb });

START_TEST(eo_signals)
{
   eo_init();

   Eo *obj = eo_add(SIMPLE_CLASS, NULL);
   Eina_Bool r;

   eo_event_callback_add(obj, EO_EVENT_CALLBACK_ADD, _eo_signals_cb_added_deled, &_eo_signals_callbacks);
   r = eo_event_callback_add(obj, EO_EVENT_CALLBACK_DEL, _eo_signals_cb_added_deled, &_eo_signals_callbacks);
   fail_if(!r);
   eo_event_callback_array_priority_add(obj, _eo_signals_callbacks(), -100, (void *) 1);
   eo_event_callback_array_add(obj, _eo_signals_callbacks(), (void *) 3);
   r = eo_event_callback_array_priority_add(obj, _eo_signals_callbacks(), -50, (void *) 2);
   fail_if(!r);
   simple_a_set(obj, 1);
   ck_assert_int_eq(_eo_signals_cb_flag, 0x3);

   eo_event_callback_array_del(obj, _eo_signals_callbacks(), (void *) 1);
   eo_event_callback_array_del(obj, _eo_signals_callbacks(), (void *) 2);
   r = eo_event_callback_array_del(obj, _eo_signals_callbacks(), (void *) 3);
   fail_if(!r);
   /* Try to delete something that doesn't exist. */
   r = eo_event_callback_array_del(obj, _eo_signals_callbacks(), (void *) 4);
   fail_if(r);
   _eo_signals_cb_flag = 0;
   simple_a_set(obj, 1);
   ck_assert_int_eq(_eo_signals_cb_flag, 0x0);

   r = eo_event_callback_array_add(obj, NULL, NULL);
   fail_if(r);

   eo_unref(obj);

   obj = eo_add(SIMPLE_CLASS, NULL);
   /* Legacy support signals. */
     {
        const Eo_Event_Description *a_desc = eo_base_legacy_only_event_description_get("a,changed");
        fail_if(!a_desc);
        ck_assert_str_eq(a_desc->name, "a,changed");
        fail_if(a_desc == EV_A_CHANGED);

        /* Check that when calling again we still get the same event. */
        const Eo_Event_Description *a_desc2 = eo_base_legacy_only_event_description_get("a,changed");
        fail_if(!a_desc2);
        fail_if(a_desc2 != a_desc);

        const Eo_Event_Description *bad_desc = eo_base_legacy_only_event_description_get("bad");
        fail_if(!bad_desc);
        ck_assert_str_eq(bad_desc->name, "bad");

        /* Call Eo event with legacy and non-legacy callbacks. */
        _eo_signals_cb_current = 0;
        eo_event_callback_priority_add(obj, EV_A_CHANGED2, -1000, _eo_signals_a_changed_never, (void *) 1);
        eo_event_callback_priority_add(obj, EV_A_CHANGED2, 0, _eo_signals_a_changed_within_cb, NULL);
        eo_event_callback_priority_add(obj, EV_A_CHANGED, -100, _eo_signals_a_changed_cb, (void *) 1);
        eo_event_callback_add(obj, a_desc, _eo_signals_a_changed_cb2, NULL);
        simple_a_set(obj, 1);
        ck_assert_int_eq(_eo_signals_cb_flag, 0x3);

        /* We don't need this one anymore. */
        r = eo_event_callback_del(obj, EV_A_CHANGED2, _eo_signals_a_changed_never, (void *) 1);
        fail_if(!r);
        r = eo_event_callback_del(obj, a_desc, _eo_signals_a_changed_cb2, NULL);
        fail_if(!r);
        eo_event_callback_add(obj, EV_A_CHANGED, _eo_signals_a_changed_cb2, NULL);

        /* Call legacy event with legacy and non-legacy callbacks. */
        int a = 3;
        _eo_signals_cb_current = 0;
        _eo_signals_cb_flag = 0;
        eo_event_callback_call(obj, a_desc, &a);
        ck_assert_int_eq(_eo_signals_cb_flag, 0x3);

        /* Stop event within event. */
        _eo_signals_cb_current = 0;
        _eo_signals_cb_flag = 0;
        fail_if(!eo_event_callback_call(obj, EV_A_CHANGED2, &a));
        ck_assert_int_eq(_eo_signals_cb_flag, 0x8);
        fail_if(!r);
     }
   eo_unref(obj);

   eo_shutdown();
}
END_TEST

START_TEST(eo_data_fetch)
{
   eo_init();

   /* Usually should be const, not const only for the test... */
   static Eo_Class_Description class_desc = {
        EO_VERSION,
        "Simple2",
        EO_CLASS_TYPE_REGULAR,
        EO_CLASS_DESCRIPTION_NOOPS(),
        NULL,
        10,
        NULL,
        NULL
   };

   const Eo_Class *klass = eo_class_new(&class_desc, EO_CLASS, NULL);
   fail_if(!klass);

   Eo *obj = eo_add(klass, NULL);
   fail_if(!obj);
#ifdef EO_DEBUG
   fail_if(eo_data_scope_get(obj, SIMPLE_CLASS));
#endif
   eo_unref(obj);

   class_desc.data_size = 0;
   klass = eo_class_new(&class_desc, EO_CLASS, NULL);
   fail_if(!klass);

   obj = eo_add(klass, NULL);
   fail_if(!obj);
   fail_if(eo_data_scope_get(obj, klass));
   fail_if(!eo_data_scope_get(obj, EO_BASE_CLASS));
   eo_unref(obj);

   eo_shutdown();
}
END_TEST

START_TEST(eo_isa_tests)
{
   eo_init();

   const Eo_Class *klass, *iface, *mixin;

     {
        /* Usually should be const, not const only for the test... */
        static Eo_Class_Description class_desc = {
             EO_VERSION,
             "Iface",
             EO_CLASS_TYPE_INTERFACE,
             EO_CLASS_DESCRIPTION_NOOPS(),
             NULL,
             0,
             NULL,
             NULL
        };

        iface = eo_class_new(&class_desc, NULL, NULL);
        fail_if(!iface);
     }

     {
        /* Usually should be const, not const only for the test... */
        static Eo_Class_Description class_desc = {
             EO_VERSION,
             "Mixin",
             EO_CLASS_TYPE_MIXIN,
             EO_CLASS_DESCRIPTION_NOOPS(),
             NULL,
             0,
             NULL,
             NULL
        };

        mixin = eo_class_new(&class_desc, NULL, NULL);
        fail_if(!mixin);
     }

     {
        /* Usually should be const, not const only for the test... */
        static Eo_Class_Description class_desc = {
             EO_VERSION,
             "Simple2",
             EO_CLASS_TYPE_REGULAR,
             EO_CLASS_DESCRIPTION_NOOPS(),
             NULL,
             10,
             NULL,
             NULL
        };

        klass = eo_class_new(&class_desc, EO_CLASS, iface, mixin, NULL);
        fail_if(!klass);
     }

   Eo *obj = eo_add(klass, NULL);
   fail_if(!obj);
   fail_if(eo_isa(obj, SIMPLE_CLASS));
   fail_if(!eo_isa(obj, iface));
   fail_if(!eo_isa(obj, mixin));
   fail_if(!eo_isa(obj, klass));
   fail_if(!eo_isa(obj, EO_CLASS));
   eo_unref(obj);

   obj = eo_add(SIMPLE_CLASS, NULL);
   fail_if(!obj);
   fail_if(eo_isa(obj, klass));
   fail_if(eo_isa(obj, iface));
   fail_if(eo_isa(obj, mixin));
   fail_if(!eo_isa(obj, SIMPLE_CLASS));
   fail_if(!eo_isa(obj, EO_CLASS));
   eo_unref(obj);

   eo_shutdown();
}
END_TEST


START_TEST(eo_composite_tests)
{
   eo_init();

   Eo *obj = eo_add(SIMPLE_CLASS, NULL);
   fail_if(!obj);
   Eo *obj2 = eo_add(SIMPLE_CLASS, NULL);
   fail_if(!obj2);
   Eo *obj3 = eo_add(SIMPLE_CLASS, NULL);
   fail_if(!obj3);

   eo_composite_attach(obj, obj2);
   fail_if(!eo_composite_part_is(obj2));

   /* Check swapping attachments works. */
   eo_composite_attach(obj3, obj2);
   fail_if(!eo_composite_part_is(obj2));

   /* Check that a deletion of a child detaches from the parent. */
   eo_del(obj2);
   fail_if(!eo_composite_attach(obj3, obj));

   /* Check that a deletion of the parent detaches the child. */
   eo_del(obj3);
   fail_if(eo_composite_part_is(obj));

   eo_unref(obj);

   eo_shutdown();
}
END_TEST

static Eina_Bool _man_should_con = EINA_TRUE;
static Eina_Bool _man_should_des = EINA_TRUE;
static const Eo_Class *cur_klass = NULL;

static Eo *
_man_con(Eo *obj, void *data EINA_UNUSED, va_list *list EINA_UNUSED)
{
   if (_man_should_con)
      eo_manual_free_set(obj, EINA_TRUE);
   return eo_constructor(eo_super(obj, cur_klass));
}

static void
_man_des(Eo *obj, void *data EINA_UNUSED, va_list *list EINA_UNUSED)
{
   eo_destructor(eo_super(obj, cur_klass));
   if (_man_should_des)
      eo_manual_free_set(obj, EINA_FALSE);
}

static Eo_Op_Description op_descs[] = {
     EO_OP_FUNC_OVERRIDE(eo_constructor, _man_con),
     EO_OP_FUNC_OVERRIDE(eo_destructor, _man_des),
};

START_TEST(eo_man_free)
{
   eo_init();

   /* Usually should be const, not const only for the test... */
   static Eo_Class_Description class_desc = {
        EO_VERSION,
        "Simple2",
        EO_CLASS_TYPE_REGULAR,
        EO_CLASS_DESCRIPTION_OPS(op_descs),
        NULL,
        10,
        NULL,
        NULL
   };

   const Eo_Class *klass = eo_class_new(&class_desc, EO_CLASS, NULL);
   fail_if(!klass);
   cur_klass = klass;

   Eo *obj = eo_add(klass, NULL);
   fail_if(!obj);
   eo_unref(obj);

   obj = eo_add(klass, NULL);
   fail_if(!obj);
   fail_if(eo_manual_free(obj));
   eo_unref(obj);

   _man_should_des = EINA_FALSE;
   klass = eo_class_new(&class_desc, EO_CLASS, NULL);
   cur_klass = klass;
   fail_if(!klass);

   obj = eo_add(klass, NULL);
   fail_if(!obj);
   fail_if(eo_manual_free(obj));
   fail_if(eo_destructed_is(obj));
   eo_unref(obj);
   fail_if(!eo_destructed_is(obj));
   fail_if(!eo_manual_free(obj));

   obj = eo_add(klass, NULL);
   fail_if(!obj);
   eo_unref(obj);
   fail_if(!eo_destructed_is(obj));
   fail_if(!eo_manual_free(obj));

   _man_should_con = EINA_FALSE;
   klass = eo_class_new(&class_desc, EO_CLASS, NULL);
   cur_klass = klass;
   fail_if(!klass);

   obj = eo_add(klass, NULL);
   fail_if(!obj);
   fail_if(eo_manual_free(obj));
   eo_unref(obj);

   obj = eo_add(klass, NULL);
   fail_if(!obj);
   eo_manual_free_set(obj, EINA_TRUE);
   eo_unref(obj);
   eo_ref(obj);
   eo_unref(obj);
   eo_unref(obj);
   fail_if(!eo_manual_free(obj));

   obj = eo_add(klass, NULL);
   fail_if(!obj);
   eo_manual_free_set(obj, EINA_TRUE);
   eo_unref(obj);
   eo_ref(obj);
   eo_unref(obj);
   eo_unref(obj);
   eo_unref(obj);
   eo_unref(obj);
   fail_if(!eo_manual_free(obj));

   eo_shutdown();
}
END_TEST

START_TEST(eo_refs)
{
   eo_init();
   Eo *obj = eo_add(SIMPLE_CLASS, NULL);
   Eo *obj2 = eo_add(SIMPLE_CLASS, NULL);
   Eo *obj3 = eo_add(SIMPLE_CLASS, NULL);

   eo_xref(obj, obj2);
   fail_if(eo_ref_get(obj) != 2);
   eo_xref(obj, obj3);
   fail_if(eo_ref_get(obj) != 3);

   eo_xunref(obj, obj2);
   fail_if(eo_ref_get(obj) != 2);
   eo_xunref(obj, obj3);
   fail_if(eo_ref_get(obj) != 1);

#ifdef EO_DEBUG
   eo_xunref(obj, obj3);
   fail_if(eo_ref_get(obj) != 1);

   eo_xref(obj, obj2);
   fail_if(eo_ref_get(obj) != 2);

   eo_xunref(obj, obj3);
   fail_if(eo_ref_get(obj) != 2);

   eo_xunref(obj, obj2);
   fail_if(eo_ref_get(obj) != 1);
#endif

   /* Check we don't seg if there's an extra xref. */
   eo_xref(obj, obj2);
   eo_unref(obj);

   eo_unref(obj);
   eo_unref(obj2);
   eo_unref(obj3);

   /* Check hierarchy */
   obj = eo_add(SIMPLE_CLASS, NULL);
   obj2 = eo_ref(eo_add(SIMPLE_CLASS, obj));

   Eo *wref = NULL;
   eo_wref_add(obj2, &wref);
   fail_if(!wref);

   eo_unref(obj2);

   fail_if(!wref); /* Parent is still holding a reference. */

   eo_unref(obj);

   fail_if(wref);

   /* eo_add_ref and normal eo_add */
   obj = eo_add(SIMPLE_CLASS, NULL);
   obj2 = eo_add(SIMPLE_CLASS, obj);
   obj3 = eo_add_ref(SIMPLE_CLASS, obj);

   ck_assert_int_eq(eo_ref_get(obj), 1);
   ck_assert_int_eq(eo_ref_get(obj2), 1);
   ck_assert_int_eq(eo_ref_get(obj3), 2);

   /* Setting and removing parents. */
   obj = eo_add(SIMPLE_CLASS, NULL);
   obj2 = eo_ref(eo_add(SIMPLE_CLASS, obj));
   obj3 = eo_ref(eo_add(SIMPLE_CLASS, NULL));

   eo_parent_set(obj2, obj3);
   eo_parent_set(obj3, obj);
   ck_assert_int_eq(eo_ref_get(obj2), 2);
   ck_assert_int_eq(eo_ref_get(obj3), 2);

   eo_parent_set(obj2, NULL);
   eo_parent_set(obj3, NULL);
   ck_assert_int_eq(eo_ref_get(obj2), 1);
   ck_assert_int_eq(eo_ref_get(obj3), 1);

   eo_parent_set(obj2, obj);
   eo_parent_set(obj3, obj);
   ck_assert_int_eq(eo_ref_get(obj2), 1);
   ck_assert_int_eq(eo_ref_get(obj3), 1);

   eo_del(obj);
   eo_del(obj2);
   eo_del(obj3);

   /* Just check it doesn't seg atm. */
   obj = eo_add(SIMPLE_CLASS, NULL);
   eo_ref(obj);
   eo_unref(obj);
   eo_unref(obj);

   obj = eo_add(SIMPLE_CLASS, NULL);
   obj2 = eo_add(SIMPLE_CLASS, obj);
   eo_unref(obj2);
   eo_ref(obj2);
   eo_del(obj2);
   eo_unref(obj);

   eo_shutdown();
}
END_TEST

START_TEST(eo_weak_reference)
{
   eo_init();

   Eo *obj = eo_add(SIMPLE_CLASS, NULL);
   Eo *obj2 = eo_add(SIMPLE_CLASS, NULL);
   Eo *wref = NULL, *wref2 = NULL, *wref3 = NULL;
   eo_wref_add(obj, &wref);
   fail_if(!wref);

   eo_unref(obj);
   fail_if(wref);

   obj = eo_add(SIMPLE_CLASS, NULL);
   eo_wref_add(obj, &wref);

   eo_ref(obj);
   fail_if(!wref);

   eo_unref(obj);
   fail_if(!wref);

   eo_unref(obj);
   fail_if(wref);

   obj = eo_add(SIMPLE_CLASS, NULL);

   eo_wref_add(obj, &wref);
   eo_wref_del(obj, &wref);
   fail_if(wref);

   eo_wref_add(obj, &wref);
   eo_wref_del(obj2, &wref);
   fail_if(!wref);
   eo_wref_del_safe(&wref);
   fail_if(wref);

   wref = obj;
   eo_wref_del(obj, &wref);
   fail_if(wref);

   wref = wref2 = wref3 = NULL;
   eo_wref_add(obj, &wref);
   eo_wref_add(obj, &wref2);
   eo_wref_add(obj, &wref3);
   fail_if(!wref);
   fail_if(!wref2);
   fail_if(!wref3);
   eo_wref_del(obj, &wref);
   eo_wref_del(obj, &wref2);
   eo_wref_del(obj, &wref3);
   fail_if(wref);
   fail_if(wref2);
   fail_if(wref3);

   eo_wref_add(obj, &wref2);
   eo_wref_add(obj, &wref3);
   wref = obj;
   eo_wref_del(obj, &wref);
   fail_if(wref);
   eo_wref_del(obj, &wref2);
   eo_wref_del(obj, &wref3);

   eo_unref(obj);
   eo_unref(obj2);


   eo_shutdown();
}
END_TEST

START_TEST(eo_generic_data)
{
   eo_init();
   Eo *obj = eo_add(SIMPLE_CLASS, NULL);
   Eo *obj2 = eo_add(SIMPLE_CLASS, NULL);
   Eo *obj3 = eo_add(SIMPLE_CLASS, NULL);
   Eo *objtmp;
   void *data = NULL;
   Eina_Value *value;
   Eina_Value *value2;



   eo_key_data_set(obj, "test1", (void *) 1);
   data = eo_key_data_get(obj, "test1");
   fail_if(1 != (intptr_t) data);

   eo_key_data_set(obj, "test1", NULL);
   data = eo_key_data_get(obj, "test1");
   fail_if(data);

   eo_key_data_set(obj, "test1", (void *) 1);
   eo_key_data_set(obj, "test2", (void *) 2);
   data = eo_key_data_get(obj, "test1");
   fail_if(1 != (intptr_t) data);

   data = eo_key_data_get(obj, "test2");
   fail_if(2 != (intptr_t) data);

   data = eo_key_data_get(obj, "test2");
   fail_if(2 != (intptr_t) data);

   eo_key_data_set(obj, "test2", NULL);
   data = eo_key_data_get(obj, "test2");
   fail_if(data);

   data = eo_key_data_get(obj, "test1");
   fail_if(1 != (intptr_t) data);

   eo_key_data_set(obj, "test1", NULL);
   data = eo_key_data_get(obj, "test1");
   fail_if(data);



   eo_key_ref_set(obj, "test1", obj2);
   objtmp = eo_key_ref_get(obj, "test1");
   fail_if(obj2 != objtmp);

   eo_key_ref_set(obj, "test1", NULL);
   objtmp = eo_key_ref_get(obj, "test1");
   fail_if(objtmp);

   eo_key_ref_set(obj, "test1", obj2);
   fail_if(eo_ref_get(obj2) != 2);

   eo_key_ref_set(obj, "test2", obj3);
   fail_if(eo_ref_get(obj3) != 2);

   objtmp = eo_key_ref_get(obj, "test1");
   fail_if(obj2 != objtmp);

   objtmp = eo_key_ref_get(obj, "test2");
   fail_if(obj3 != objtmp);

   data = eo_key_ref_get(obj, "test2");
   fail_if(obj3 != objtmp);

   eo_key_ref_set(obj, "test2", NULL);
   fail_if(eo_ref_get(obj3) != 1);

   objtmp = eo_key_ref_get(obj, "test2");
   fail_if(objtmp);

   objtmp = eo_key_ref_get(obj, "test1");
   fail_if(obj2 != objtmp);

   eo_key_ref_set(obj, "test1", NULL);
   fail_if(eo_ref_get(obj2) != 1);

   objtmp = eo_key_ref_get(obj, "test1");
   fail_if(objtmp);

   eo_key_ref_set(obj, "test1", obj2);
   eo_key_ref_set(obj, "test2", obj3);
   eo_del(obj2);
   eo_del(obj2);
   eo_del(obj3);
   eo_del(obj3);
   objtmp = eo_key_ref_get(obj, "test1");
   fail_if(objtmp);

   objtmp = eo_key_ref_get(obj, "test2");
   fail_if(objtmp);



   obj2 = eo_add(SIMPLE_CLASS, NULL);
   obj3 = eo_add(SIMPLE_CLASS, NULL);

   eo_key_wref_set(obj, "test1", obj2);
   objtmp = eo_key_wref_get(obj, "test1");
   fail_if(obj2 != objtmp);

   eo_key_wref_set(obj, "test1", NULL);
   objtmp = eo_key_wref_get(obj, "test1");
   fail_if(objtmp);

   eo_key_wref_set(obj, "test1", obj2);
   fail_if(eo_ref_get(obj2) != 1);

   eo_key_wref_set(obj, "test2", obj3);
   fail_if(eo_ref_get(obj3) != 1);

   objtmp = eo_key_wref_get(obj, "test1");
   fail_if(obj2 != objtmp);

   objtmp = eo_key_wref_get(obj, "test2");
   fail_if(obj3 != objtmp);

   data = eo_key_wref_get(obj, "test2");
   fail_if(obj3 != objtmp);

   eo_key_wref_set(obj, "test2", NULL);
   fail_if(eo_ref_get(obj3) != 1);

   objtmp = eo_key_wref_get(obj, "test2");
   fail_if(objtmp);

   objtmp = eo_key_wref_get(obj, "test1");
   fail_if(obj2 != objtmp);

   eo_key_wref_set(obj, "test1", NULL);
   fail_if(eo_ref_get(obj2) != 1);

   objtmp = eo_key_wref_get(obj, "test1");
   fail_if(objtmp);

   eo_key_wref_set(obj, "test1", obj2);
   eo_key_wref_set(obj, "test2", obj3);
   eo_del(obj2);
   eo_del(obj3);
   objtmp = eo_key_wref_get(obj, "test1");
   fail_if(objtmp);

   objtmp = eo_key_wref_get(obj, "test2");
   fail_if(objtmp);



   value = eina_value_new(EINA_VALUE_TYPE_INT);
   eina_value_set(value, 1234);
   value2 = eo_key_value_get(obj, "value1");
   fail_if(value2 != NULL);

   eo_key_value_set(obj, "value1", value);
   value2 = eo_key_value_get(obj, "value1");
   fail_if(value != value2);

   eo_key_value_set(obj, "value1", NULL);
   value2 = eo_key_value_get(obj, "value1");
   fail_if(value2 != NULL);

   eo_key_value_set(obj, "value1", NULL);

   eo_unref(obj);
   eo_unref(obj2);
   eo_unref(obj3);

   eo_shutdown();
}
END_TEST


START_TEST(eo_magic_checks)
{
   char _buf[sizeof(long)]; /* Just enough to hold eina magic + a bit more. */
   char *buf = _buf;
   eo_init();

   memset(_buf, 1, sizeof(_buf));

   Eo *obj = eo_add(SIMPLE_CLASS, (Eo *) buf);
   fail_if(obj);

   while (1)
     {
        int i = 20, a = 0;
        Eo *parent = NULL;
        Eo *wref = NULL;
        Eo *obj2 = NULL;

        obj = eo_add((Eo_Class *) buf, NULL);
        fail_if(obj);

        obj = eo_add(SIMPLE_CLASS, NULL);
        fail_if(!obj);

        simple_a_set((Eo *) buf, ++i);
        a = simple_a_get((Eo *) buf);
        ck_assert_int_ne(i, a);
        simple_a_set(eo_super((Eo *) buf, SIMPLE_CLASS), ++i);
        a = simple_a_get(eo_super((Eo *) buf, SIMPLE_CLASS));
        ck_assert_int_ne(i, a);
        simple_a_set(eo_super(obj, (const Eo_Class *) buf), ++i);
        a = simple_a_get(eo_super(obj, (const Eo_Class *) buf));
        ck_assert_int_ne(i, a);
        fail_if(eo_class_get((Eo *) buf));
        fail_if(eo_class_name_get((Eo_Class*) buf));
        fail_if(eo_class_get(obj) != SIMPLE_CLASS);
        fail_if(eo_class_get(SIMPLE_CLASS) != EO_CLASS_CLASS);
        simple_a_set((Eo_Class *) buf, 1);
        simple_a_set(eo_super((Eo_Class *) buf, SIMPLE_CLASS), ++i);
        simple_a_set(eo_super(SIMPLE_CLASS, (Eo_Class *) buf), ++i);
        fail_if(eo_class_new(NULL, (Eo_Class *) buf), NULL);

        eo_xref(obj, (Eo *) buf);
        eo_xunref(obj, (Eo *) buf);
        eo_xref((Eo *) buf, obj);
        eo_xunref((Eo *) buf, obj);

        eo_ref((Eo *) buf);
        eo_unref((Eo *) buf);
        eo_del((Eo *) buf);

        eo_isa((Eo *) buf, SIMPLE_CLASS);
        eo_isa(obj, (Eo_Class *) buf);

        fail_if(0 != eo_ref_get((Eo *) buf));

        eo_wref_add((Eo *) buf, &wref);
        parent = eo_parent_get((Eo *) buf);
        fail_if(wref);
        fail_if(parent);

        fail_if(eo_data_scope_get((Eo *) buf, SIMPLE_CLASS));

        eo_composite_attach(obj, (Eo *) buf);
        eo_composite_detach(obj, (Eo *) buf);
        eo_composite_part_is((Eo *) buf);

        eo_event_callback_forwarder_add(obj, NULL, (Eo *) buf);
        eo_event_callback_forwarder_del(obj, NULL, (Eo *) buf);

        eo_manual_free_set((Eo *) buf, EINA_TRUE);
        eo_manual_free((Eo *) buf);
        eo_destructed_is((Eo *) buf);

        obj2 = NULL;
        eo_parent_set(obj, (Eo *) buf);
        obj2 = eo_parent_get(obj);
        fail_if(obj2 && (obj2 == (Eo *) buf));

        eo_unref(obj);

        if (!buf)
           break;
        else
           buf = NULL;
     }

   eo_shutdown();
}
END_TEST

/* MULTI */

static Eina_Bool
_a_print(Eo *obj EINA_UNUSED, void *class_data EINA_UNUSED)
{
   printf("Hey\n");

   return EINA_TRUE;
}

static Eina_Bool
_class_hi_print(Eo_Class *klass EINA_UNUSED, void *class_data EINA_UNUSED)
{
   printf("Hi\n");

   return EINA_TRUE;
}

EO_FUNC_BODY(multi_a_print, Eina_Bool, EINA_FALSE);
EO_FUNC_BODY_CONST(multi_class_hi_print, Eina_Bool, EINA_FALSE);

static Eo_Op_Description _multi_do_op_descs[] = {
     EO_OP_FUNC(multi_a_print, _a_print),
     EO_OP_FUNC(multi_class_hi_print, _class_hi_print),
};

START_TEST(eo_multiple_do)
{
   eo_init();

   /* Usually should be const, not const only for the test... */
   static Eo_Class_Description class_desc = {
        EO_VERSION,
        "Inherit",
        EO_CLASS_TYPE_REGULAR,
        EO_CLASS_DESCRIPTION_OPS(_multi_do_op_descs),
        NULL,
        0,
        NULL,
        NULL
   };

   const Eo_Class *klass = eo_class_new(&class_desc, SIMPLE_CLASS, NULL);
   fail_if(!klass);

   Eo *obj = eo_add(klass, NULL);
   fail_if(!obj);

   Eina_Bool ca, cb, cc;

   ca = cb = cc = EINA_FALSE;
   ca = simple_a_print(obj);
   cb = multi_a_print(obj);
   cc = multi_a_print(obj);
   fail_if(!(ca && cb && cc));

   ca = cb = cc = EINA_FALSE;
   ca = simple_class_hi_print(klass);
   cb = multi_class_hi_print(klass);
   cc = multi_class_hi_print(klass);
   fail_if(!(ca && cb && cc));

   eo_unref(obj);

   eo_shutdown();
}
END_TEST

START_TEST(eo_add_do_and_custom)
{
   Simple_Public_Data *pd = NULL;
   Eo *obj = NULL;
   eo_init();

   obj = eo_add(SIMPLE_CLASS, NULL, eo_constructor(eo_self));
   fail_if(!obj);
   eo_unref(obj);

   obj = eo_add(SIMPLE_CLASS, NULL, simple_a_set(eo_self, 7));
   fail_if(!obj);
   pd = eo_data_scope_get(obj, SIMPLE_CLASS);
   fail_if(pd->a != 7);
   eo_unref(obj);

   obj = eo_add(SIMPLE_CLASS, NULL, eo_constructor(eo_self), simple_a_set(eo_self, 7));
   fail_if(!obj);
   pd = eo_data_scope_get(obj, SIMPLE_CLASS);
   fail_if(pd->a != 7);
   eo_unref(obj);

   Eina_Bool finalized;
   obj = eo_add(SIMPLE_CLASS, NULL, finalized = eo_finalized_get(eo_self));
   fail_if(finalized);

   finalized = eo_finalized_get(obj);
   fail_if(!finalized);
   eo_unref(obj);

   eo_shutdown();
}
END_TEST

START_TEST(eo_pointers_indirection)
{
#ifdef HAVE_EO_ID
   eo_init();

   static const Eo_Class_Description class_desc = {
        EO_VERSION,
        "Simple",
        EO_CLASS_TYPE_REGULAR,
        EO_CLASS_DESCRIPTION_NOOPS(),
        NULL,
        0,
        NULL,
        NULL
   };

   const Eo_Class *klass = eo_class_new(&class_desc, EO_CLASS, NULL);
   fail_if(!klass);

   /* Check simple id validity */
   Eo *obj = eo_add(klass, NULL);
   fail_if(!obj);
   fail_if(!eo_isa(obj, klass));
   obj = (Eo *)((char *)(obj) + 1);
   fail_if(eo_isa(obj, klass));
   obj = (Eo *)((char *)(obj) - 1);
   fail_if(!eo_isa(obj, klass));
   eo_unref(obj);
   fail_if(eo_isa(obj, klass));

   /* Check id invalidity after deletion */
   Eo *obj1 = eo_add(klass, NULL);
   fail_if(!obj1);
   eo_unref(obj1);
   Eo *obj2 = eo_add(klass, NULL);
   fail_if(!obj2);
   fail_if(!eo_isa(obj2, klass));
   fail_if(eo_isa(obj1, klass));
   eo_unref(obj2);

   /* Check id sanity checks for "close enough" ids. */
   obj1 = eo_add(klass, NULL);
   fail_if(!obj1);
   obj2 = (Eo *) (((Eo_Id) obj1) & ~MASK_OBJ_TAG);
   fail_if(eo_class_get(obj2));
   eo_unref(obj1);

#define NB_OBJS 10000
   unsigned int obj_id;
   Eo **objs = calloc(NB_OBJS, sizeof(Eo *));
   fail_if(!objs);
   /* Creation of the objects */
   for ( obj_id = 0; obj_id < NB_OBJS; obj_id++)
     {
        objs[obj_id] = eo_add(klass, NULL);
        if(!objs[obj_id])
          fail_if(!objs[obj_id]);
        if(!eo_isa(objs[obj_id], klass))
          fail_if(!eo_isa(objs[obj_id], klass));
     }
   /* Deletion of a few objects */
   for ( obj_id = 0; obj_id < NB_OBJS; obj_id+=2000)
     {
        eo_unref(objs[obj_id]);
        if(eo_isa(objs[obj_id], klass))
          fail_if(eo_isa(objs[obj_id], klass));
     }
   /* Creation of the deleted objects */
   for ( obj_id = 0; obj_id < NB_OBJS; obj_id+=2000)
     {
        objs[obj_id] = eo_add(klass, NULL);
        if(!objs[obj_id])
          fail_if(!objs[obj_id]);
        if(!eo_isa(objs[obj_id], klass))
          fail_if(!eo_isa(objs[obj_id], klass));
     }
   /* Deletion of all the objects */
   for ( obj_id = 0; obj_id < NB_OBJS; obj_id++)
      eo_unref(objs[obj_id]);
   /* Just be sure that we trigger an already freed error */
   eo_unref(objs[0]);
   free(objs);

   eo_shutdown();
#endif
}
END_TEST


static Eo *
_eo_add_failures_finalize(Eo *obj EINA_UNUSED, void *class_data EINA_UNUSED)
{
   return NULL;
}

static Eo_Op_Description _eo_add_failures_op_descs[] = {
     EO_OP_FUNC_OVERRIDE(eo_finalize, _eo_add_failures_finalize),
};

START_TEST(eo_add_failures)
{
   eo_init();

   static const Eo_Class_Description class_desc = {
        EO_VERSION,
        "Simple2",
        EO_CLASS_TYPE_REGULAR,
        EO_CLASS_DESCRIPTION_OPS(_eo_add_failures_op_descs),
        NULL,
        0,
        NULL,
        NULL
   };

   const Eo_Class *klass = eo_class_new(&class_desc, EO_CLASS, NULL);

   Eo *obj = eo_add(klass, NULL);

   fail_if(obj);

   eo_shutdown();
}
END_TEST

#ifdef HAVE_EO_ID
static Eina_Bool intercepted = EINA_FALSE;

static void
_del_intercept(Eo *obj)
{
   intercepted = EINA_TRUE;
   eo_del_intercept_set(obj, NULL);
   eo_unref(obj);
}
#endif

START_TEST(eo_del_intercept)
{
#ifdef HAVE_EO_ID
   eo_init();

   static const Eo_Class_Description class_desc = {
        EO_VERSION,
        "Simple",
        EO_CLASS_TYPE_REGULAR,
        EO_CLASS_DESCRIPTION_NOOPS(),
        NULL,
        0,
        NULL,
        NULL
   };

   const Eo_Class *klass = eo_class_new(&class_desc, EO_CLASS, NULL);
   fail_if(!klass);

   /* Check unref interception */
   intercepted = EINA_FALSE;
   Eo *obj = eo_add(klass, NULL);
   fail_if(!obj);
   fail_if(!eo_isa(obj, klass));
   eo_del_intercept_set(obj, _del_intercept);
   eo_unref(obj);
   fail_if(!intercepted);
   fail_if(eo_isa(obj, klass));

   /* Check del interception */
   intercepted = EINA_FALSE;
   obj = eo_add(klass, NULL);
   fail_if(!obj);
   fail_if(!eo_isa(obj, klass));
   eo_del_intercept_set(obj, _del_intercept);
   eo_del(obj);
   fail_if(!intercepted);
   fail_if(eo_isa(obj, klass));

   eo_shutdown();
#endif
}
END_TEST

START_TEST(eo_name)
{
   eo_init();
   Eo *obj = eo_add(SIMPLE_CLASS, NULL);
   Eo *obj2 = eo_add(SIMPLE_CLASS, NULL);
   Eo *obj3 = eo_add(SIMPLE_CLASS, NULL);
   Eo *objtmp;
   const char *id;

   id = eo_name_get(obj);
   fail_if(NULL != id);

   eo_name_set(obj, "Hello");
   id = eo_name_get(obj);
   fail_if(NULL == id);
   fail_if(!!strcmp(id, "Hello"));

   eo_name_set(obj, "Hello");
   eo_name_set(obj, "");
   id = eo_name_get(obj);
   fail_if(NULL != id);

   eo_name_set(obj, "Hello");
   eo_name_set(obj, NULL);
   id = eo_name_get(obj);
   fail_if(NULL != id);

   eo_name_set(obj2, "joe");
   eo_name_set(obj3, "bob");

   eo_parent_set(obj2, obj);
   eo_parent_set(obj3, obj2);

   objtmp = eo_name_find(obj, "bob");
   fail_if(objtmp != obj3);

   objtmp = eo_name_find(obj, "joe");
   fail_if(objtmp != obj2);

   objtmp = eo_name_find(obj, "bo*");
   fail_if(objtmp != obj3);

   objtmp = eo_name_find(obj, "*oe");
   fail_if(objtmp != obj2);

   objtmp = eo_name_find(obj, "Simple:*oe");
   fail_if(objtmp != obj2);

   objtmp = eo_name_find(obj, "*mple:joe");
   fail_if(objtmp != obj2);

   eo_del(obj);

   eo_shutdown();
}
END_TEST

START_TEST(eo_comment)
{
   eo_init();
   Eo *obj = eo_add(SIMPLE_CLASS, NULL);
   const char *comment;

   comment = eo_comment_get(obj);
   fail_if(NULL != comment);

   eo_comment_set(obj, "Hello");
   comment = eo_comment_get(obj);
   fail_if(NULL == comment);
   fail_if(!!strcmp(comment, "Hello"));

   eo_comment_set(obj, "Hello");
   eo_comment_set(obj, "");
   comment = eo_comment_get(obj);
   fail_if(NULL != comment);

   eo_comment_set(obj, "Hello");
   eo_comment_set(obj, NULL);
   comment = eo_comment_get(obj);
   fail_if(NULL != comment);

   eo_del(obj);

   eo_shutdown();
}
END_TEST

START_TEST(eo_rec_interface)
{
   eo_init();
   Eo *s = eo_add(SEARCHABLE_CLASS, NULL);
   Eo *obj = eo_add(SIMPLE_CLASS, s);
   Eo *obj2 = eo_add(SIMPLE_CLASS, obj);
   Eo *objtmp;

   objtmp = eo_provider_find(obj2, SEARCHABLE_CLASS);
   fail_if(objtmp != s);

   eo_del(obj);

   eo_shutdown();
}
END_TEST

void eo_test_general(TCase *tc)
{
   tcase_add_test(tc, eo_simple);
   tcase_add_test(tc, eo_singleton);
   tcase_add_test(tc, eo_override_tests);
   tcase_add_test(tc, eo_signals);
   tcase_add_test(tc, eo_data_fetch);
   tcase_add_test(tc, eo_isa_tests);
   tcase_add_test(tc, eo_composite_tests);
   tcase_add_test(tc, eo_man_free);
   tcase_add_test(tc, eo_refs);
   tcase_add_test(tc, eo_weak_reference);
   tcase_add_test(tc, eo_generic_data);
   tcase_add_test(tc, eo_magic_checks);
   tcase_add_test(tc, eo_multiple_do);
   tcase_add_test(tc, eo_add_do_and_custom);
   tcase_add_test(tc, eo_pointers_indirection);
   tcase_add_test(tc, eo_add_failures);
   tcase_add_test(tc, eo_del_intercept);
   tcase_add_test(tc, eo_name);
   tcase_add_test(tc, eo_comment);
   tcase_add_test(tc, eo_rec_interface);
}
