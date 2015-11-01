#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <stdio.h>

#define EO_BASE_BETA

#include "Eo.h"
#include "eo_suite.h"
#include "eo_test_class_simple.h"

/* Loading this internal header for testing purposes. */
#include "eo_ptr_indirection.h"

START_TEST(eo_simple)
{
   eo_init();
   Eo *obj;
   eo_add(obj, EO_CLASS, NULL);
   fail_if(obj);

   eo_add(obj, SIMPLE_CLASS, NULL);
   fail_if(!obj);
   eo_do(obj, eo_constructor(obj));
   eo_do(obj, eo_destructor(obj));
   eo_unref(obj);

   eo_shutdown();
}
END_TEST

START_TEST(eo_stack)
{
   eo_init();
   Eo *obj;
   eo_add(obj, SIMPLE_CLASS, NULL);
   fail_if(!obj);

   eo_do(obj, simple_recursive(obj, 123));

   eo_unref(obj);

   eo_shutdown();
}
END_TEST

static int _eo_signals_cb_current = 0;
static int _eo_signals_cb_flag = 0;

static Eina_Bool
_eo_signals_a_changed_cb(void *_data, Eo *obj EINA_UNUSED, const Eo_Event_Description *desc EINA_UNUSED, void *event_info EINA_UNUSED)
{
   int data = (intptr_t) _data;
   _eo_signals_cb_current++;
   ck_assert_int_eq(data, _eo_signals_cb_current);
   _eo_signals_cb_flag |= 0x1;
   return EO_CALLBACK_CONTINUE;
}

static Eina_Bool
_eo_signals_a_changed_cb2(void *_data EINA_UNUSED, Eo *obj EINA_UNUSED, const Eo_Event_Description *desc EINA_UNUSED, void *event_info EINA_UNUSED)
{
   _eo_signals_cb_flag |= 0x2;
   return EO_CALLBACK_STOP;
}

static Eina_Bool
_eo_signals_a_changed_never(void *_data EINA_UNUSED, Eo *obj EINA_UNUSED, const Eo_Event_Description *desc EINA_UNUSED, void *event_info EINA_UNUSED)
{
   /* This one should never be called. */
   fail_if(1);
   return EO_CALLBACK_CONTINUE;
}

static Eina_Bool
_eo_signals_eo_del_cb(void *_data EINA_UNUSED, Eo *obj EINA_UNUSED, const Eo_Event_Description *desc EINA_UNUSED, void *event_info EINA_UNUSED)
{
   _eo_signals_cb_flag |= 0x4;
   return EO_CALLBACK_CONTINUE;
}

Eina_Bool
_eo_signals_cb_added_deled(void *data, Eo *obj EINA_UNUSED, const Eo_Event_Description *desc EINA_UNUSED, void *event_info)
{
   const Eo_Callback_Array_Item *callback_array = event_info;

   fail_if((callback_array != data) &&
         (callback_array->func != _eo_signals_cb_added_deled));

   return EO_CALLBACK_CONTINUE;
}

START_TEST(eo_signals)
{
   eo_init();
   static const Eo_Callback_Array_Item callbacks[] = {
          { EV_A_CHANGED, _eo_signals_a_changed_cb },
          { EV_A_CHANGED, _eo_signals_a_changed_cb2 },
          { EV_A_CHANGED, _eo_signals_a_changed_never },
          { EO_EV_DEL, _eo_signals_eo_del_cb },
          { NULL, NULL }
   };
   Eo *obj;
   eo_add(obj, SIMPLE_CLASS, NULL);

   eo_do(obj, eo_event_callback_add(obj, EO_EV_CALLBACK_ADD, _eo_signals_cb_added_deled, callbacks));
   eo_do(obj, eo_event_callback_add(obj, EO_EV_CALLBACK_DEL, _eo_signals_cb_added_deled, callbacks));
   eo_do(obj, eo_event_callback_array_priority_add(obj, callbacks, -100, (void *) 1));
   eo_do(obj, eo_event_callback_array_add(obj, callbacks, (void *) 3));
   eo_do(obj, eo_event_callback_array_priority_add(obj, callbacks, -50, (void *) 2));
   eo_do(obj, simple_a_set(obj, 1));
   ck_assert_int_eq(_eo_signals_cb_flag, 0x3);

   eo_do(obj, eo_event_callback_array_del(obj, callbacks, (void *) 1));
   eo_do(obj, eo_event_callback_array_del(obj, callbacks, (void *) 2));
   eo_do(obj, eo_event_callback_array_del(obj, callbacks, (void *) 3));
   /* Try to delete something that doesn't exist. */
   eo_do(obj, eo_event_callback_array_del(obj, callbacks, (void *) 4));
   _eo_signals_cb_flag = 0;
   eo_do(obj, simple_a_set(obj, 1));
   ck_assert_int_eq(_eo_signals_cb_flag, 0x0);

   eo_unref(obj);

   eo_add(obj, SIMPLE_CLASS, NULL);
   /* Legacy support signals. */
     {
        const Eo_Event_Description *a_desc = eo_base_legacy_only_event_description_get("a,changed");
        fail_if(!a_desc);
        ck_assert_str_eq(a_desc->name, "a,changed");
        fail_if(a_desc == EV_A_CHANGED);

        const Eo_Event_Description *bad_desc = eo_base_legacy_only_event_description_get("bad");
        fail_if(!bad_desc);
        ck_assert_str_eq(bad_desc->name, "bad");

        /* Call Eo event with legacy and non-legacy callbacks. */
        _eo_signals_cb_current = 0;
        eo_do(obj, eo_event_callback_priority_add(obj, EV_A_CHANGED2, -1000, _eo_signals_a_changed_never, (void *) 1));
        eo_do(obj, eo_event_callback_priority_add(obj, EV_A_CHANGED, -100, _eo_signals_a_changed_cb, (void *) 1));
        eo_do(obj, eo_event_callback_add(obj, a_desc, _eo_signals_a_changed_cb2, NULL));
        eo_do(obj, simple_a_set(obj, 1));
        ck_assert_int_eq(_eo_signals_cb_flag, 0x3);

        /* We don't need this one anymore. */
        eo_do(obj, eo_event_callback_del(obj, EV_A_CHANGED2, _eo_signals_a_changed_never, (void *) 1));

        /* Call legacy event with legacy and non-legacy callbacks. */
        int a = 3;
        _eo_signals_cb_current = 0;
        _eo_signals_cb_flag = 0;
        eo_do(obj, eo_event_callback_call(obj, a_desc, &a));
        ck_assert_int_eq(_eo_signals_cb_flag, 0x3);
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

   Eo *obj;
   eo_add(obj, klass, NULL);
   fail_if(!obj);
#ifdef EO_DEBUG
   fail_if(eo_data_scope_get(obj, SIMPLE_CLASS));
#endif
   eo_unref(obj);

   class_desc.data_size = 0;
   klass = eo_class_new(&class_desc, EO_CLASS, NULL);
   fail_if(!klass);

   eo_add(obj, klass, NULL);
   fail_if(!obj);
   fail_if(eo_data_scope_get(obj, klass));
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

   Eo *obj;
   eo_add(obj, klass, NULL);
   fail_if(!obj);
   fail_if(eo_isa(obj, SIMPLE_CLASS));
   fail_if(!eo_isa(obj, iface));
   fail_if(!eo_isa(obj, mixin));
   fail_if(!eo_isa(obj, klass));
   fail_if(!eo_isa(obj, EO_CLASS));
   eo_unref(obj);

   eo_add(obj, SIMPLE_CLASS, NULL);
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
   Eina_Bool tmp;
   eo_init();

   Eo *obj;
   eo_add(obj, SIMPLE_CLASS, NULL);
   fail_if(!obj);
   Eo *obj2;
   eo_add(obj2, SIMPLE_CLASS, NULL);
   fail_if(!obj2);

   eo_do(obj, eo_composite_attach(obj, obj2));
   eo_do(obj2, eo_parent_set(obj2, NULL));
   fail_if(eo_do_ret(obj2, tmp, eo_composite_part_is(obj2)));

   eo_unref(obj2);
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
   fprintf(stderr, "%s %s:%d\n", __func__, __FILE__, __LINE__); fflush(stderr);
   if (_man_should_con)
      eo_manual_free_set(obj, EINA_TRUE);
   fprintf(stderr, "%s %s:%d\n", __func__, __FILE__, __LINE__); fflush(stderr);
   Eo* p = eo_super_eo_constructor(cur_klass, obj);
   fprintf(stderr, "%s %s:%d\n", __func__, __FILE__, __LINE__); fflush(stderr);
   return p;
}

static void
_man_des(Eo *obj, void *data EINA_UNUSED, va_list *list EINA_UNUSED)
{
   fprintf(stderr, "%s %s:%d\n", __func__, __FILE__, __LINE__); fflush(stderr);
   eo_super_eo_destructor(cur_klass, obj);
   fprintf(stderr, "%s %s:%d\n", __func__, __FILE__, __LINE__); fflush(stderr);
   if (_man_should_des)
      eo_manual_free_set(obj, EINA_FALSE);
   fprintf(stderr, "%s %s:%d\n", __func__, __FILE__, __LINE__); fflush(stderr);
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

   Eo *obj;
   eo_add(obj, klass, NULL);
   /* fail_if(!obj); */
   /* eo_unref(obj); */

   /* eo_add(obj, klass, NULL); */
   /* fail_if(!obj); */
   /* fail_if(eo_manual_free(obj)); */
   /* eo_unref(obj); */

   /* _man_should_des = EINA_FALSE; */
   /* klass = eo_class_new(&class_desc, EO_CLASS, NULL); */
   /* cur_klass = klass; */
   /* fail_if(!klass); */

   /* eo_add(obj, klass, NULL); */
   /* fail_if(!obj); */
   /* fail_if(eo_manual_free(obj)); */
   /* fail_if(eo_destructed_is(obj)); */
   /* eo_unref(obj); */
   /* fail_if(!eo_destructed_is(obj)); */
   /* fail_if(!eo_manual_free(obj)); */

   /* eo_add(obj, klass, NULL); */
   /* fail_if(!obj); */
   /* eo_unref(obj); */
   /* fail_if(!eo_destructed_is(obj)); */
   /* fail_if(!eo_manual_free(obj)); */

   /* _man_should_con = EINA_FALSE; */
   /* klass = eo_class_new(&class_desc, EO_CLASS, NULL); */
   /* cur_klass = klass; */
   /* fail_if(!klass); */

   /* eo_add(obj, klass, NULL); */
   /* fail_if(!obj); */
   /* fail_if(eo_manual_free(obj)); */
   /* eo_unref(obj); */

   /* eo_add(obj, klass, NULL); */
   /* fail_if(!obj); */
   /* eo_manual_free_set(obj, EINA_TRUE); */
   /* eo_unref(obj); */
   /* eo_ref(obj); */
   /* eo_unref(obj); */
   /* eo_unref(obj); */
   /* fail_if(!eo_manual_free(obj)); */

   /* eo_add(obj, klass, NULL); */
   /* fail_if(!obj); */
   /* eo_manual_free_set(obj, EINA_TRUE); */
   /* eo_unref(obj); */
   /* eo_ref(obj); */
   /* eo_unref(obj); */
   /* eo_unref(obj); */
   /* eo_unref(obj); */
   /* eo_unref(obj); */
   /* fail_if(!eo_manual_free(obj)); */

   eo_shutdown();
}
END_TEST

START_TEST(eo_refs)
{
   eo_init();
   Eo *obj;
   eo_add(obj, SIMPLE_CLASS, NULL);
   Eo *obj2;
   eo_add(obj2, SIMPLE_CLASS, NULL);
   Eo *obj3;
   eo_add(obj3, SIMPLE_CLASS, NULL);

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
   eo_add(obj, SIMPLE_CLASS, NULL);
   eo_add(obj2, SIMPLE_CLASS, obj);
   eo_ref(obj2);

   Eo *wref = NULL;
   eo_do(obj2, eo_wref_add(obj2, &wref));
   fail_if(!wref);

   eo_unref(obj2);

   fail_if(!wref); /* Parent is still holding a reference. */

   eo_unref(obj);

   fail_if(wref);

   /* eo_add_ref and normal eo_add */
   eo_add(obj, SIMPLE_CLASS, NULL);
   eo_add(obj2, SIMPLE_CLASS, obj);
   eo_add_ref(obj3, SIMPLE_CLASS, obj);

   ck_assert_int_eq(eo_ref_get(obj), 1);
   ck_assert_int_eq(eo_ref_get(obj2), 1);
   ck_assert_int_eq(eo_ref_get(obj3), 2);

   /* Setting and removing parents. */
   eo_add(obj, SIMPLE_CLASS, NULL);
   eo_add_ref(obj2, SIMPLE_CLASS, obj);
   eo_add_ref(obj3, SIMPLE_CLASS, NULL);

   eo_do(obj2, eo_parent_set(obj2, obj3));
   eo_do(obj3, eo_parent_set(obj3, obj));
   ck_assert_int_eq(eo_ref_get(obj2), 2);
   ck_assert_int_eq(eo_ref_get(obj3), 2);

   eo_do(obj2, eo_parent_set(obj2, NULL));
   eo_do(obj3, eo_parent_set(obj3, NULL));
   ck_assert_int_eq(eo_ref_get(obj2), 1);
   ck_assert_int_eq(eo_ref_get(obj3), 1);

   eo_do(obj2, eo_parent_set(obj2, obj));
   eo_do(obj3, eo_parent_set(obj3, obj));
   ck_assert_int_eq(eo_ref_get(obj2), 1);
   ck_assert_int_eq(eo_ref_get(obj3), 1);

   eo_del(obj);
   eo_del(obj2);
   eo_del(obj3);

   /* Just check it doesn't seg atm. */
   eo_add(obj, SIMPLE_CLASS, NULL);
   eo_ref(obj);
   eo_unref(obj);
   eo_unref(obj);

   eo_add(obj, SIMPLE_CLASS, NULL);
   eo_add(obj2, SIMPLE_CLASS, obj);
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

   Eo *obj;
   eo_add(obj, SIMPLE_CLASS, NULL);
   Eo *obj2;
   eo_add(obj2, SIMPLE_CLASS, NULL);
   Eo *wref = NULL, *wref2 = NULL, *wref3 = NULL;
   eo_do(obj, eo_wref_add(obj, &wref));
   fail_if(!wref);

   eo_unref(obj);
   fail_if(wref);

   eo_add(obj, SIMPLE_CLASS, NULL);
   eo_do(obj, eo_wref_add(obj, &wref));

   eo_ref(obj);
   fail_if(!wref);

   eo_unref(obj);
   fail_if(!wref);

   eo_unref(obj);
   fail_if(wref);

   eo_add(obj, SIMPLE_CLASS, NULL);

   eo_do(obj, eo_wref_add(obj, &wref));
   eo_do(obj, eo_wref_del(obj, &wref));
   fail_if(wref);

   eo_do(obj, eo_wref_add(obj, &wref));
   eo_do(obj2, eo_wref_del(obj2, &wref));
   fail_if(!wref);
   eo_wref_del_safe(&wref);
   fail_if(wref);

   wref = obj;
   eo_do(obj, eo_wref_del(obj, &wref));
   fail_if(wref);

   wref = wref2 = wref3 = NULL;
   eo_do(obj, eo_wref_add(obj, &wref), eo_wref_add(obj, &wref2), eo_wref_add(obj, &wref3));
   fail_if(!wref);
   fail_if(!wref2);
   fail_if(!wref3);
   eo_do(obj, eo_wref_del(obj, &wref), eo_wref_del(obj, &wref2), eo_wref_del(obj, &wref3));
   fail_if(wref);
   fail_if(wref2);
   fail_if(wref3);

   eo_do(obj, eo_wref_add(obj, &wref2), eo_wref_add(obj, &wref3));
   wref = obj;
   eo_do(obj, eo_wref_del(obj, &wref));
   fail_if(wref);
   eo_do(obj, eo_wref_del(obj, &wref2), eo_wref_del(obj, &wref3));

   eo_unref(obj);
   eo_unref(obj2);


   eo_shutdown();
}
END_TEST

START_TEST(eo_generic_data)
{
   eo_init();
   Eo *obj;
   eo_add(obj, SIMPLE_CLASS, NULL);
   void *data = NULL;

   eo_do(obj, eo_key_data_set(obj, "test1", (void *) 1));
   eo_do(obj, data = eo_key_data_get(obj, "test1"));
   fail_if(1 != (intptr_t) data);
   eo_do(obj, eo_key_data_del(obj, "test1"));
   eo_do(obj, data = eo_key_data_get(obj, "test1"));
   fail_if(data);

   eo_do(obj, eo_key_data_set(obj, "test1", (void *) 1));
   eo_do(obj, eo_key_data_set(obj, "test2", (void *) 2));
   eo_do(obj, data = eo_key_data_get(obj, "test1"));
   fail_if(1 != (intptr_t) data);
   eo_do(obj, data = eo_key_data_get(obj, "test2"));
   fail_if(2 != (intptr_t) data);

   eo_do(obj, data = eo_key_data_get(obj, "test2"));
   fail_if(2 != (intptr_t) data);
   eo_do(obj, eo_key_data_del(obj, "test2"));
   eo_do(obj, data = eo_key_data_get(obj, "test2"));
   fail_if(data);

   eo_do(obj, data = eo_key_data_get(obj, "test1"));
   fail_if(1 != (intptr_t) data);
   eo_do(obj, eo_key_data_del(obj, "test1"));
   eo_do(obj, data = eo_key_data_get(obj, "test1"));
   fail_if(data);

   eo_unref(obj);

   eo_shutdown();
}
END_TEST


START_TEST(eo_magic_checks)
{
   char _buf[sizeof(long)]; /* Just enough to hold eina magic + a bit more. */
   char *buf = _buf;
   eo_init();

   memset(_buf, 1, sizeof(_buf));

   Eo *obj;
   eo_add(obj, SIMPLE_CLASS, (Eo *) buf);
   fail_if(obj);

   fprintf(stderr, "%s %s:%d\n", __func__, __FILE__, __LINE__);

   while (1)
     {
        int i = 20, a = 0;
        Eo *parent = NULL;
        Eo *wref = NULL;
        Eo *obj2 = NULL;

        fprintf(stderr, "%s %s:%d\n", __func__, __FILE__, __LINE__);
        eo_add(obj, (Eo_Class *) buf, NULL);
        fprintf(stderr, "%s %s:%d\n", __func__, __FILE__, __LINE__);
        fail_if(obj);
        fprintf(stderr, "%s %s:%d\n", __func__, __FILE__, __LINE__);

        eo_add(obj, SIMPLE_CLASS, NULL);
        fprintf(stderr, "%s %s:%d\n", __func__, __FILE__, __LINE__);
        fail_if(!obj);
        fprintf(stderr, "%s %s:%d\n", __func__, __FILE__, __LINE__);

        eo_do((Eo *) buf, simple_a_set((Eo*)buf, ++i), a = simple_a_get((Eo*)buf));
        fprintf(stderr, "%s %s:%d\n", __func__, __FILE__, __LINE__);
        ck_assert_int_ne(i, a);
        fprintf(stderr, "%s %s:%d\n", __func__, __FILE__, __LINE__);
        eo_super_simple_a_set(SIMPLE_CLASS, (Eo*)buf, ++i);
        a = eo_super_simple_a_get(SIMPLE_CLASS, (Eo*)buf);
        ck_assert_int_ne(i, a);
        fprintf(stderr, "%s %s:%d\n", __func__, __FILE__, __LINE__);
        eo_super_simple_a_set((const Eo_Class *) buf, (Eo*)buf, ++i);
        a = eo_super_simple_a_get((const Eo_Class *) buf, (Eo*)buf);
        ck_assert_int_ne(i, a);
        fprintf(stderr, "%s %s:%d\n", __func__, __FILE__, __LINE__);
        fail_if(eo_class_get((Eo *) buf));
        fail_if(eo_class_name_get((Eo_Class*) buf));
        fail_if(eo_class_get(obj) != SIMPLE_CLASS);
        fail_if(eo_class_get(SIMPLE_CLASS) != EO_ABSTRACT_CLASS_CLASS);
        fprintf(stderr, "%s %s:%d\n", __func__, __FILE__, __LINE__);
        eo_do((Eo_Class *) buf,(void) NULL);
        eo_super_simple_a_set(SIMPLE_CLASS, (Eo*)buf, ++i);
        eo_super_simple_a_set((Eo_Class *) buf, (Eo*)buf, ++i);
        fprintf(stderr, "%s %s:%d\n", __func__, __FILE__, __LINE__);
        fail_if(eo_class_new(NULL, (Eo_Class *) buf), NULL);

        eo_xref(obj, (Eo *) buf);
        fprintf(stderr, "%s %s:%d\n", __func__, __FILE__, __LINE__);
        eo_xunref(obj, (Eo *) buf);
        fprintf(stderr, "%s %s:%d\n", __func__, __FILE__, __LINE__);
        eo_xref((Eo *) buf, obj);
        fprintf(stderr, "%s %s:%d\n", __func__, __FILE__, __LINE__);
        eo_xunref((Eo *) buf, obj);
        fprintf(stderr, "%s %s:%d\n", __func__, __FILE__, __LINE__);

        eo_ref((Eo *) buf);
        fprintf(stderr, "%s %s:%d\n", __func__, __FILE__, __LINE__);
        eo_unref((Eo *) buf);
        fprintf(stderr, "%s %s:%d\n", __func__, __FILE__, __LINE__);
        eo_del((Eo *) buf);
        fprintf(stderr, "%s %s:%d\n", __func__, __FILE__, __LINE__);

        eo_isa((Eo *) buf, SIMPLE_CLASS);
        fprintf(stderr, "%s %s:%d\n", __func__, __FILE__, __LINE__);
        eo_isa(obj, (Eo_Class *) buf);
        fprintf(stderr, "%s %s:%d\n", __func__, __FILE__, __LINE__);

        fail_if(0 != eo_ref_get((Eo *) buf));
        fprintf(stderr, "%s %s:%d\n", __func__, __FILE__, __LINE__);

        eo_do((Eo *) buf,
              eo_wref_add((Eo*)buf, &wref),
	      parent = eo_parent_get((Eo*)buf));
        fprintf(stderr, "%s %s:%d\n", __func__, __FILE__, __LINE__);
        fail_if(wref);
        fail_if(parent);
        fprintf(stderr, "%s %s:%d\n", __func__, __FILE__, __LINE__);

        fail_if(eo_data_scope_get((Eo *) buf, SIMPLE_CLASS));
        fprintf(stderr, "%s %s:%d\n", __func__, __FILE__, __LINE__);

        eo_do(obj, eo_composite_attach(obj, (Eo *) buf));
        eo_do(obj, eo_composite_detach(obj, (Eo *) buf));
        eo_do((Eo *) buf, eo_composite_part_is((Eo*)buf));
        fprintf(stderr, "%s %s:%d\n", __func__, __FILE__, __LINE__);

        eo_do(obj, eo_event_callback_forwarder_add(obj, NULL, (Eo *) buf));
        eo_do(obj, eo_event_callback_forwarder_del(obj, NULL, (Eo *) buf));
        fprintf(stderr, "%s %s:%d\n", __func__, __FILE__, __LINE__);

        eo_manual_free_set((Eo *) buf, EINA_TRUE);
        eo_manual_free((Eo *) buf);
        eo_destructed_is((Eo *) buf);
        fprintf(stderr, "%s %s:%d\n", __func__, __FILE__, __LINE__);

        obj2 = NULL;
        eo_do(obj, eo_parent_set(obj, (Eo *) buf));
        eo_do(obj, obj2 = eo_parent_get(obj));
        fail_if(obj2 && (obj2 == (Eo *) buf));
        fprintf(stderr, "%s %s:%d\n", __func__, __FILE__, __LINE__);

        eo_unref(obj);
        fprintf(stderr, "%s %s:%d\n", __func__, __FILE__, __LINE__);

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

Eina_Bool multi_a_print(Eo* _object)
{
   _eo_do_start(_object, NULL, EINA_FALSE, _eo_stack_get());
   typedef Eina_Bool (*_Eo_multi_a_print_func)(Eo const*, void *obj_data);

   Eina_Bool _ret;
   EO_FUNC_COMMON_OP(multi_a_print, EINA_FALSE);
   _ret = _func_(___call.obj, ___call.data);
   _eo_do_end(_eo_stack_get());
   return _ret;
}

Eina_Bool multi_class_hi_print(Eo* _object)
{
   _eo_do_start(_object, NULL, EINA_FALSE, _eo_stack_get());
   typedef Eina_Bool (*_Eo_multi_class_hi_print_func)(Eo const*, void *obj_data);

   Eina_Bool _ret;
   EO_FUNC_COMMON_OP(multi_class_hi_print, EINA_FALSE);
   _ret = _func_(___call.obj, ___call.data);
   _eo_do_end(_eo_stack_get());
   return _ret;
}


//EO_FUNC_BODY(multi_a_print, Eina_Bool, EINA_FALSE);
//EO_FUNC_BODY(multi_class_hi_print, Eina_Bool, EINA_FALSE);

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

   Eo *obj;
   eo_add(obj, klass, NULL);
   fail_if(!obj);

   Eina_Bool ca, cb, cc;

   ca = cb = cc = EINA_FALSE;
   eo_do(obj, ca = simple_a_print(obj), cb = multi_a_print(obj), cc = multi_a_print(obj));
   fail_if(!(ca && cb && cc));

   ca = cb = cc = EINA_FALSE;
   eo_do(klass, ca = simple_class_hi_print(klass), cb = multi_class_hi_print(klass), cc = multi_class_hi_print(klass));
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

   eo_add(obj, SIMPLE_CLASS, NULL, eo_constructor(obj));
   fail_if(!obj);
   eo_unref(obj);

   eo_add(obj, SIMPLE_CLASS, NULL, simple_a_set(obj, 7));
   fail_if(!obj);
   pd = eo_data_scope_get(obj, SIMPLE_CLASS);
   fail_if(pd->a != 7);
   eo_unref(obj);

   eo_add(obj, SIMPLE_CLASS, NULL, eo_constructor(obj), simple_a_set(obj, 7));
   fail_if(!obj);
   pd = eo_data_scope_get(obj, SIMPLE_CLASS);
   fail_if(pd->a != 7);
   eo_unref(obj);

   Eina_Bool finalized;
   eo_add(obj, SIMPLE_CLASS, NULL, finalized = eo_finalized_get(obj));
   fail_if(finalized);

   eo_do(obj, finalized = eo_finalized_get(obj));
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
   Eo *obj;
   eo_add(obj, klass, NULL);
   fail_if(!obj);
   fail_if(!eo_isa(obj, klass));
   obj = (Eo *)((char *)(obj) + 1);
   fail_if(eo_isa(obj, klass));
   obj = (Eo *)((char *)(obj) - 1);
   fail_if(!eo_isa(obj, klass));
   eo_unref(obj);
   fail_if(eo_isa(obj, klass));

   /* Check id invalidity after deletion */
   Eo *obj1;
   eo_add(obj1, klass, NULL);
   fail_if(!obj1);
   eo_unref(obj1);
   Eo *obj2;
   eo_add(obj2, klass, NULL);
   fail_if(!obj2);
   fail_if(!eo_isa(obj2, klass));
   fail_if(eo_isa(obj1, klass));
   eo_unref(obj2);

   /* Check id sanity checks for "close enough" ids. */
   eo_add(obj1, klass, NULL);
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
        eo_add(objs[obj_id], klass, NULL);
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
        eo_add(objs[obj_id], klass, NULL);
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

   Eo *obj;
   eo_add(obj, klass, NULL);

   fail_if(obj);

   eo_shutdown();
}
END_TEST

START_TEST(eo_parts)
{
   int a = 0;

   eo_init();

   Eo *obj;
   eo_add(obj, SIMPLE_CLASS, NULL);

   eo_do(obj, simple_a_set(obj, 3), a = simple_a_get(obj));
   ck_assert_int_eq(a, 3);

   /* eo_do_part(obj, simple_part_get(obj, "test"), */
   /*            simple_a_set(obj, 7), */
   /*       a = simple_a_get(obj) */
   /*       ); */
   Eo* p = simple_part_get(obj, "test");
   simple_a_set(p, 7);
   a = simple_a_get(p);
   ck_assert_int_eq(a, 7);

   eo_do(obj, simple_a_set(obj, 3), a = simple_a_get(obj));
   ck_assert_int_eq(a, 3);

   /* Faking a call, just asserting NULL as the part to check default values. */
   /* eo_do_part(obj, NULL, */
   /*            simple_a_set(obj, 7), */
   /*       a = simple_a_get(obj) */
   /*       ); */
   /* eo_do_part(obj, NULL, */
   simple_a_set(NULL, 7);
   a = simple_a_get(NULL);
         /* ); */
   ck_assert_int_eq(a, 0);

   eo_del(obj);

   eo_shutdown();
}
END_TEST

void eo_test_general(TCase *tc)
{
   /* tcase_add_test(tc, eo_simple); */
   /* tcase_add_test(tc, eo_stack); */
   /* tcase_add_test(tc, eo_signals); */
   /* tcase_add_test(tc, eo_data_fetch); */
   /* tcase_add_test(tc, eo_isa_tests); */
   /* tcase_add_test(tc, eo_composite_tests); */
   /* tcase_add_test(tc, eo_man_free); */
   /* tcase_add_test(tc, eo_refs); */
   /* tcase_add_test(tc, eo_weak_reference); */
   /* tcase_add_test(tc, eo_generic_data); */
   tcase_add_test(tc, eo_magic_checks);
   tcase_add_test(tc, eo_multiple_do);
   /* tcase_add_test(tc, eo_add_do_and_custom); */
   /* tcase_add_test(tc, eo_pointers_indirection); */
   /* tcase_add_test(tc, eo_add_failures); */
   /* tcase_add_test(tc, eo_parts); */
}
