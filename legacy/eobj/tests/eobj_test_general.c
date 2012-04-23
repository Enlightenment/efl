#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <stdio.h>

#include "eobj_suite.h"
#include "Eobj.h"

#include "class_simple.h"

START_TEST(eobj_simple)
{
   eobj_init();
   Eobj *obj = eobj_add(EOBJ_BASE_CLASS, NULL);
   fail_if(obj);

   eobj_shutdown();
}
END_TEST

START_TEST(eobj_refs)
{
   eobj_init();
   Eobj *obj = eobj_add(SIMPLE_CLASS, NULL);
   Eobj *obj2 = eobj_add(SIMPLE_CLASS, NULL);
   Eobj *obj3 = eobj_add(SIMPLE_CLASS, NULL);

   eobj_xref(obj, obj2);
   fail_if(eobj_ref_get(obj) != 2);
   eobj_xref(obj, obj3);
   fail_if(eobj_ref_get(obj) != 3);

   eobj_xunref(obj, obj2);
   fail_if(eobj_ref_get(obj) != 2);
   eobj_xunref(obj, obj3);
   fail_if(eobj_ref_get(obj) != 1);

#ifndef NDEBUG
   eobj_xunref(obj, obj3);
   fail_if(eobj_ref_get(obj) != 1);

   eobj_xref(obj, obj2);
   fail_if(eobj_ref_get(obj) != 2);

   eobj_xunref(obj, obj3);
   fail_if(eobj_ref_get(obj) != 2);

   eobj_xunref(obj, obj2);
   fail_if(eobj_ref_get(obj) != 1);
#endif

   eobj_unref(obj);
   eobj_unref(obj2);
   eobj_unref(obj3);

   /* Just check it doesn't seg atm. */
   obj = eobj_add(SIMPLE_CLASS, NULL);
   eobj_ref(obj);
   eobj_del(obj);
   eobj_del(obj);

   eobj_shutdown();
}
END_TEST

START_TEST(eobj_weak_reference)
{
   eobj_init();

   Eobj *obj = eobj_add(SIMPLE_CLASS, NULL);
   Eobj_Weak_Ref *wref = eobj_weak_ref_new(obj);
   fail_if(!eobj_weak_ref_get(wref));

   eobj_unref(obj);
   fail_if(eobj_weak_ref_get(wref));

   eobj_weak_ref_free(wref);

   obj = eobj_add(SIMPLE_CLASS, NULL);
   wref = eobj_weak_ref_new(obj);

   eobj_ref(obj);
   fail_if(!eobj_weak_ref_get(wref));

   eobj_del(obj);
   fail_if(eobj_weak_ref_get(wref));

   eobj_unref(obj);
   fail_if(eobj_weak_ref_get(wref));

   eobj_weak_ref_free(wref);

   obj = eobj_add(SIMPLE_CLASS, NULL);

   wref = eobj_weak_ref_new(obj);
   eobj_weak_ref_free(wref);

   eobj_unref(obj);


   eobj_shutdown();
}
END_TEST

static void
_a_set(Eobj *obj EINA_UNUSED, void *class_data EINA_UNUSED, va_list *list EINA_UNUSED)
{
   fail_if(EINA_TRUE);
}

static void
_op_errors_class_constructor(Eobj_Class *klass)
{
   const Eobj_Op_Func_Description func_desc[] = {
        EOBJ_OP_FUNC(SIMPLE_ID(SIMPLE_SUB_ID_LAST), _a_set),
        EOBJ_OP_FUNC(SIMPLE_ID(SIMPLE_SUB_ID_LAST + 1), _a_set),
        EOBJ_OP_FUNC(0x0F010111, _a_set),
        EOBJ_OP_FUNC_SENTINEL
   };

   eobj_class_funcs_set(klass, func_desc);
}

START_TEST(eobj_op_errors)
{
   eobj_init();

   static const Eobj_Class_Description class_desc = {
        "Simple",
        EOBJ_CLASS_TYPE_REGULAR,
        EOBJ_CLASS_DESCRIPTION_OPS(NULL, NULL, 0),
        NULL,
        0,
        NULL,
        NULL,
        _op_errors_class_constructor,
        NULL
   };

   const Eobj_Class *klass = eobj_class_new(&class_desc, SIMPLE_CLASS, NULL);
   fail_if(!klass);

   Eobj *obj = eobj_add(klass, NULL);

   /* Out of bounds op for a legal class. */
   fail_if(eobj_do(obj, 0x00010111));

   /* Ilegal class. */
   fail_if(eobj_do(obj, 0x0F010111));

   fail_if(eobj_ref_get(obj) != 1);

   eobj_ref(obj);
   fail_if(eobj_ref_get(obj) != 2);

   eobj_ref(obj);
   fail_if(eobj_ref_get(obj) != 3);

   eobj_unref(obj);
   fail_if(eobj_ref_get(obj) != 2);

   eobj_unref(obj);
   fail_if(eobj_ref_get(obj) != 1);

   eobj_unref(obj);
   eobj_shutdown();
}
END_TEST

static void
_fake_free_func(void *data)
{
   if (!data)
      return;

   int *a = data;
   ++*a;
}

START_TEST(eobj_generic_data)
{
   eobj_init();
   Eobj *obj = eobj_add(SIMPLE_CLASS, NULL);
   void *data;

   eobj_do(obj, EOBJ_BASE_DATA_SET("test1", (void *) 1, NULL));
   eobj_do(obj, EOBJ_BASE_DATA_GET("test1", &data));
   fail_if(1 != (int) data);
   eobj_do(obj, EOBJ_BASE_DATA_DEL("test1"));
   eobj_do(obj, EOBJ_BASE_DATA_GET("test1", &data));
   fail_if(data);

   eobj_do(obj, EOBJ_BASE_DATA_SET("test1", (void *) 1, NULL));
   eobj_do(obj, EOBJ_BASE_DATA_SET("test2", (void *) 2, NULL));
   eobj_do(obj, EOBJ_BASE_DATA_GET("test1", &data));
   fail_if(1 != (int) data);
   eobj_do(obj, EOBJ_BASE_DATA_GET("test2", &data));
   fail_if(2 != (int) data);

   eobj_do(obj, EOBJ_BASE_DATA_GET("test2", &data));
   fail_if(2 != (int) data);
   eobj_do(obj, EOBJ_BASE_DATA_DEL("test2"));
   eobj_do(obj, EOBJ_BASE_DATA_GET("test2", &data));
   fail_if(data);

   eobj_do(obj, EOBJ_BASE_DATA_GET("test1", &data));
   fail_if(1 != (int) data);
   eobj_do(obj, EOBJ_BASE_DATA_DEL("test1"));
   eobj_do(obj, EOBJ_BASE_DATA_GET("test1", &data));
   fail_if(data);

   int a = 0;
   eobj_do(obj, EOBJ_BASE_DATA_SET("test3", &a, _fake_free_func));
   eobj_do(obj, EOBJ_BASE_DATA_GET("test3", &data));
   fail_if(&a != data);
   eobj_do(obj, EOBJ_BASE_DATA_GET("test3", NULL));
   eobj_do(obj, EOBJ_BASE_DATA_DEL("test3"));
   fail_if(a != 1);

   a = 0;
   eobj_do(obj, EOBJ_BASE_DATA_SET("test3", &a, _fake_free_func));
   eobj_do(obj, EOBJ_BASE_DATA_SET("test3", NULL, _fake_free_func));
   fail_if(a != 1);
   a = 0;
   data = (void *) 123;
   eobj_do(obj, EOBJ_BASE_DATA_SET(NULL, &a, _fake_free_func));
   eobj_do(obj, EOBJ_BASE_DATA_GET(NULL, &data));
   fail_if(data);
   eobj_do(obj, EOBJ_BASE_DATA_DEL(NULL));

   a = 0;
   eobj_do(obj, EOBJ_BASE_DATA_SET("test3", &a, _fake_free_func));
   eobj_do(obj, EOBJ_BASE_DATA_SET("test3", NULL, NULL));
   fail_if(a != 1);
   eobj_do(obj, EOBJ_BASE_DATA_SET("test3", &a, _fake_free_func));

   eobj_unref(obj);
   fail_if(a != 2);

   eobj_shutdown();
}
END_TEST

START_TEST(eobj_magic_checks)
{
   char buf[sizeof(long)]; /* Just enough to hold eina magic + a bit more. */
   eobj_init();

   memset(buf, 1, sizeof(buf));
   Eobj *obj = eobj_add((Eobj_Class *) buf, NULL);
   fail_if(obj);

   obj = eobj_add(SIMPLE_CLASS, (Eobj *) buf);
   fail_if(obj);

   obj = eobj_add(SIMPLE_CLASS, NULL);
   fail_if(!obj);

   fail_if(eobj_do((Eobj *) buf, EOBJ_NOOP));
   fail_if(eobj_do_super((Eobj *) buf, EOBJ_NOOP));
   fail_if(eobj_class_get((Eobj *) buf));
   fail_if(eobj_class_name_get((Eobj_Class*) buf));
   eobj_class_funcs_set((Eobj_Class *) buf, NULL);

   fail_if(eobj_class_new(NULL, (Eobj_Class *) buf), NULL);

   eobj_xref(obj, (Eobj *) buf);
   eobj_xunref(obj, (Eobj *) buf);
   eobj_xref((Eobj *) buf, obj);
   eobj_xunref((Eobj *) buf, obj);

   eobj_ref((Eobj *) buf);
   eobj_unref((Eobj *) buf);

   fail_if(0 != eobj_ref_get((Eobj *) buf));

   Eobj_Weak_Ref *wref = eobj_weak_ref_new((Eobj *) buf);
   fail_if(eobj_weak_ref_get(wref));
   eobj_weak_ref_free(wref);

   eobj_del((Eobj *) buf);

   fail_if(eobj_parent_get((Eobj *) buf));

   eobj_constructor_error_set((Eobj *) buf);
   fail_if(!eobj_constructor_error_get((Eobj *) buf));

   eobj_constructor_super((Eobj *) buf);
   eobj_destructor_super((Eobj *) buf);

   fail_if(eobj_data_get((Eobj *) buf, SIMPLE_CLASS));

   eobj_composite_object_attach((Eobj *) buf, obj);
   eobj_composite_object_attach(obj, (Eobj *) buf);
   eobj_composite_object_detach((Eobj *) buf, obj);
   eobj_composite_object_detach(obj, (Eobj *) buf);
   eobj_composite_is((Eobj *) buf);

   fail_if(eobj_event_callback_add((Eobj *) buf, NULL, NULL, NULL));
   fail_if(eobj_event_callback_del((Eobj *) buf, NULL, NULL));
   fail_if(eobj_event_callback_del_full((Eobj *) buf, NULL, NULL, NULL));
   fail_if(eobj_event_callback_call((Eobj *) buf, NULL, NULL));

   fail_if(eobj_event_callback_forwarder_add((Eobj *) buf, NULL, obj));
   fail_if(eobj_event_callback_forwarder_add(obj, NULL, (Eobj *) buf));
   fail_if(eobj_event_callback_forwarder_del((Eobj *) buf, NULL, obj));
   fail_if(eobj_event_callback_forwarder_del(obj, NULL, (Eobj *) buf));

   eobj_unref(obj);

   eobj_shutdown();
}
END_TEST

void eobj_test_general(TCase *tc)
{
   tcase_add_test(tc, eobj_generic_data);
   tcase_add_test(tc, eobj_op_errors);
   tcase_add_test(tc, eobj_simple);
   tcase_add_test(tc, eobj_weak_reference);
   tcase_add_test(tc, eobj_refs);
   tcase_add_test(tc, eobj_magic_checks);
}
