#include "config.h"

#include <stdio.h>

#include "eo_suite.h"
#include "Eo.h"

#include "class_simple.h"

START_TEST(eo_incomplete_desc)
{
   eo_init();

   const Eo_Class *klass;
   static Eo_Op TMP_BASE_ID = EO_NOOP;

   enum {
        TEST_SUB_ID_FOO,
        TEST_SUB_ID_FOO2,
        TEST_SUB_ID_LAST
   };

   static const Eo_Op_Description op_desc[] = {
        EO_OP_DESCRIPTION(TEST_SUB_ID_FOO, "Foo"),
        EO_OP_DESCRIPTION(TEST_SUB_ID_FOO2, "Foo2"),
        EO_OP_DESCRIPTION_SENTINEL
   };

   static const Eo_Op_Description op_desc_wrong[] = {
        EO_OP_DESCRIPTION(TEST_SUB_ID_FOO2, "Foo2"),
        EO_OP_DESCRIPTION(TEST_SUB_ID_FOO, "Foo"),
        EO_OP_DESCRIPTION_SENTINEL
   };

   /* XXX: In real life this should be const, this is just for testing. */
   static Eo_Class_Description class_desc = {
        EO_VERSION,
        "Simple",
        EO_CLASS_TYPE_REGULAR,
        EO_CLASS_DESCRIPTION_OPS(NULL, op_desc, 1),
        NULL,
        0,
        NULL,
        NULL
   };

   klass = eo_class_new(&class_desc, NULL, NULL);
   fail_if(klass);

   class_desc.ops.base_op_id = &TMP_BASE_ID;
   class_desc.ops.descs = NULL;

   klass = eo_class_new(&class_desc, NULL, NULL);
   fail_if(klass);

   class_desc.ops.descs = op_desc;
   class_desc.ops.count = TEST_SUB_ID_LAST + 1;

   klass = eo_class_new(&class_desc, NULL, NULL);
   fail_if(klass);

   class_desc.ops.count = 0;

   klass = eo_class_new(&class_desc, NULL, NULL);
   fail_if(klass);

   class_desc.ops.count = TEST_SUB_ID_LAST;
   class_desc.ops.descs = op_desc_wrong;

   klass = eo_class_new(&class_desc, NULL, NULL);
   fail_if(klass);

   class_desc.ops.descs = op_desc;
   class_desc.name = NULL;

   klass = eo_class_new(&class_desc, NULL, NULL);
   fail_if(klass);

   class_desc.name = "Simple";


   klass = eo_class_new(NULL, NULL, NULL);
   fail_if(klass);

   /* Should create a class. */
   klass = eo_class_new(&class_desc, EO_BASE_CLASS, NULL);
   fail_if(!klass);

   (void) klass;

   eo_shutdown();
}
END_TEST

START_TEST(eo_inherit_errors)
{
   eo_init();

   const Eo_Class *klass;
   const Eo_Class *klass_mixin;
   const Eo_Class *klass_simple;

   static const Eo_Class_Description class_desc_simple = {
        EO_VERSION,
        "Simple",
        EO_CLASS_TYPE_REGULAR,
        EO_CLASS_DESCRIPTION_OPS(NULL, NULL, 0),
        NULL,
        0,
        NULL,
        NULL
   };

   static const Eo_Class_Description class_desc_mixin = {
        EO_VERSION,
        "Mixin",
        EO_CLASS_TYPE_MIXIN,
        EO_CLASS_DESCRIPTION_OPS(NULL, NULL, 0),
        NULL,
        0,
        NULL,
        NULL
   };

   static Eo_Class_Description class_desc = {
        EO_VERSION,
        "General",
        EO_CLASS_TYPE_MIXIN,
        EO_CLASS_DESCRIPTION_OPS(NULL, NULL, 0),
        NULL,
        0,
        NULL,
        NULL
   };

   klass_mixin = eo_class_new(&class_desc_mixin, NULL, NULL);
   fail_if(!klass_mixin);

   klass_simple = eo_class_new(&class_desc_simple, EO_BASE_CLASS, NULL);
   fail_if(!klass_simple);

   klass = eo_class_new(&class_desc, klass_simple, NULL);
   fail_if(klass);

   class_desc.type = EO_CLASS_TYPE_REGULAR;

   klass = eo_class_new(&class_desc, klass_mixin, NULL);
   fail_if(klass);

   (void) klass;

   eo_shutdown();
}
END_TEST

START_TEST(eo_inconsistent_mro)
{
   eo_init();

   const Eo_Class *klass;
   const Eo_Class *klass_mixin;
   const Eo_Class *klass_mixin2;
   const Eo_Class *klass_mixin3;

   static const Eo_Class_Description class_desc_simple = {
        EO_VERSION,
        "Simple",
        EO_CLASS_TYPE_REGULAR,
        EO_CLASS_DESCRIPTION_OPS(NULL, NULL, 0),
        NULL,
        0,
        NULL,
        NULL
   };

   static const Eo_Class_Description class_desc_mixin = {
        EO_VERSION,
        "Mixin",
        EO_CLASS_TYPE_MIXIN,
        EO_CLASS_DESCRIPTION_OPS(NULL, NULL, 0),
        NULL,
        0,
        NULL,
        NULL
   };

   static const Eo_Class_Description class_desc_mixin2 = {
        EO_VERSION,
        "Mixin2",
        EO_CLASS_TYPE_MIXIN,
        EO_CLASS_DESCRIPTION_OPS(NULL, NULL, 0),
        NULL,
        0,
        NULL,
        NULL
   };

   static const Eo_Class_Description class_desc_mixin3 = {
        EO_VERSION,
        "Mixin3",
        EO_CLASS_TYPE_MIXIN,
        EO_CLASS_DESCRIPTION_OPS(NULL, NULL, 0),
        NULL,
        0,
        NULL,
        NULL
   };

   klass_mixin = eo_class_new(&class_desc_mixin, NULL, NULL);
   fail_if(!klass_mixin);

   klass_mixin2 = eo_class_new(&class_desc_mixin2, klass_mixin, NULL);
   fail_if(!klass_mixin2);

   klass_mixin3 = eo_class_new(&class_desc_mixin3, klass_mixin, NULL);
   fail_if(!klass_mixin3);

   klass = eo_class_new(&class_desc_simple, EO_BASE_CLASS, klass_mixin, klass_mixin2, NULL);
   fail_if(klass);

   klass = eo_class_new(&class_desc_simple, EO_BASE_CLASS, klass_mixin2, klass_mixin, NULL);
   fail_if(!klass);

   klass = eo_class_new(&class_desc_simple, EO_BASE_CLASS, klass_mixin2, klass_mixin3, NULL);
   fail_if(!klass);

   eo_shutdown();
}
END_TEST

static void _stub_class_constructor(Eo_Class *klass EINA_UNUSED) {}

START_TEST(eo_bad_interface)
{
   eo_init();

   const Eo_Class *klass;

   static Eo_Class_Description class_desc = {
        EO_VERSION,
        "Interface",
        EO_CLASS_TYPE_INTERFACE,
        EO_CLASS_DESCRIPTION_OPS(NULL, NULL, 0),
        NULL,
        10,
        NULL,
        NULL
   };

   klass = eo_class_new(&class_desc, NULL, NULL);
   fail_if(klass);

   class_desc.data_size = 0;
   class_desc.class_constructor = _stub_class_constructor;

   klass = eo_class_new(&class_desc, NULL, NULL);
   fail_if(!klass);

   class_desc.class_constructor = NULL;
   class_desc.class_destructor = _stub_class_constructor;

   klass = eo_class_new(&class_desc, NULL, NULL);
   fail_if(!klass);

   class_desc.class_destructor = NULL;

   klass = eo_class_new(&class_desc, NULL, NULL);
   fail_if(!klass);

   eo_shutdown();
}
END_TEST

static int _const_ops_counter = 0;

static void
_const_ops_a_set(Eo *obj EINA_UNUSED, void *class_data EINA_UNUSED, va_list *list)
{
   int a = va_arg(*list, int);
   (void) a;
   _const_ops_counter++;
}

static void
_const_ops_a_print(Eo *obj EINA_UNUSED, void *class_data EINA_UNUSED, va_list *list EINA_UNUSED)
{
   _const_ops_counter++;
}

static void
_const_ops_class_hi_print(Eo *obj EINA_UNUSED, void *class_data EINA_UNUSED, va_list *list EINA_UNUSED)
{
   _const_ops_counter++;
}

static void
_const_ops_class_constructor(Eo_Class *klass)
{
   const Eo_Op_Func_Description func_desc[] = {
        EO_OP_FUNC(SIMPLE_ID(SIMPLE_SUB_ID_A_SET), (eo_op_func_type) _const_ops_a_set),
        EO_OP_FUNC(SIMPLE_ID(SIMPLE_SUB_ID_A_PRINT), (eo_op_func_type) _const_ops_a_print),
        EO_OP_FUNC_CLASS(SIMPLE_ID(SIMPLE_SUB_ID_A_SET), (eo_op_func_type_class) _const_ops_a_set),
        EO_OP_FUNC_CLASS(SIMPLE_ID(SIMPLE_SUB_ID_A_PRINT), (eo_op_func_type_class) _const_ops_a_print),
        EO_OP_FUNC(SIMPLE_ID(SIMPLE_SUB_ID_CLASS_HI_PRINT), (eo_op_func_type) _const_ops_class_hi_print),
        EO_OP_FUNC(SIMPLE_ID(SIMPLE_SUB_ID_CLASS_HI_PRINT), (eo_op_func_type) _const_ops_class_hi_print),
        EO_OP_FUNC_SENTINEL
   };

   eo_class_funcs_set(klass, func_desc);
}

START_TEST(eo_op_types)
{
   eo_init();

   const Eo_Class *klass;

   static Eo_Class_Description class_desc = {
        EO_VERSION,
        "Simple",
        EO_CLASS_TYPE_REGULAR,
        EO_CLASS_DESCRIPTION_OPS(NULL, NULL, 0),
        NULL,
        0,
        _const_ops_class_constructor,
        NULL
   };

   klass = eo_class_new(&class_desc, SIMPLE_CLASS, NULL);
   fail_if(!klass);

   /* Add class checks here... */
   Eo *obj = eo_add(klass, NULL);
   eo_do(obj, simple_a_set(7), simple_a_print(), simple_class_hi_print());

   eo_unref(obj);

   eo_shutdown();
}
END_TEST

void eo_test_class_errors(TCase *tc)
{
   tcase_add_test(tc, eo_incomplete_desc);
   tcase_add_test(tc, eo_inherit_errors);
   tcase_add_test(tc, eo_inconsistent_mro);
   tcase_add_test(tc, eo_bad_interface);
   tcase_add_test(tc, eo_op_types);
}
