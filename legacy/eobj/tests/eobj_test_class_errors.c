#include "config.h"

#include <stdio.h>

#include "eobj_suite.h"
#include "Eobj.h"

#include "class_simple.h"

START_TEST(eobj_incomplete_desc)
{
   eobj_init();

   const Eobj_Class *klass;
   static Eobj_Op TMP_BASE_ID = EOBJ_NOOP;

   enum {
        TEST_SUB_ID_FOO,
        TEST_SUB_ID_FOO2,
        TEST_SUB_ID_LAST
   };

   static const Eobj_Op_Description op_desc[] = {
        EOBJ_OP_DESCRIPTION(TEST_SUB_ID_FOO, "i", "Foo"),
        EOBJ_OP_DESCRIPTION(TEST_SUB_ID_FOO2, "i", "Foo2"),
        EOBJ_OP_DESCRIPTION_SENTINEL
   };

   static const Eobj_Op_Description op_desc_wrong[] = {
        EOBJ_OP_DESCRIPTION(TEST_SUB_ID_FOO2, "i", "Foo2"),
        EOBJ_OP_DESCRIPTION(TEST_SUB_ID_FOO, "i", "Foo"),
        EOBJ_OP_DESCRIPTION_SENTINEL
   };

   /* XXX: In real life this should be const, this is just for testing. */
   static Eobj_Class_Description class_desc = {
        "Simple",
        EOBJ_CLASS_TYPE_REGULAR,
        EOBJ_CLASS_DESCRIPTION_OPS(NULL, op_desc, 1),
        NULL,
        0,
        NULL,
        NULL,
        NULL,
        NULL
   };

   klass = eobj_class_new(&class_desc, NULL, NULL);
   fail_if(klass);

   class_desc.ops.base_op_id = &TMP_BASE_ID;
   class_desc.ops.descs = NULL;

   klass = eobj_class_new(&class_desc, NULL, NULL);
   fail_if(klass);

   class_desc.ops.descs = op_desc;
   class_desc.ops.count = TEST_SUB_ID_LAST + 1;

   klass = eobj_class_new(&class_desc, NULL, NULL);
   fail_if(klass);

   class_desc.ops.count = 0;

   klass = eobj_class_new(&class_desc, NULL, NULL);
   fail_if(klass);

   class_desc.ops.count = TEST_SUB_ID_LAST;
   class_desc.ops.descs = op_desc_wrong;

   klass = eobj_class_new(&class_desc, NULL, NULL);
   fail_if(klass);

   class_desc.ops.descs = op_desc;
   class_desc.name = NULL;

   klass = eobj_class_new(&class_desc, NULL, NULL);
   fail_if(klass);

   class_desc.name = "Simple";


   klass = eobj_class_new(NULL, NULL, NULL);
   fail_if(klass);

   /* Should create a class. */
   klass = eobj_class_new(&class_desc, NULL, NULL);
   fail_if(!klass);

   (void) klass;

   eobj_shutdown();
}
END_TEST

START_TEST(eobj_inherit_errors)
{
   eobj_init();

   const Eobj_Class *klass;
   const Eobj_Class *klass_mixin;
   const Eobj_Class *klass_simple;

   static const Eobj_Class_Description class_desc_simple = {
        "Simple",
        EOBJ_CLASS_TYPE_REGULAR,
        EOBJ_CLASS_DESCRIPTION_OPS(NULL, NULL, 0),
        NULL,
        0,
        NULL,
        NULL,
        NULL,
        NULL
   };

   static const Eobj_Class_Description class_desc_mixin = {
        "Mixin",
        EOBJ_CLASS_TYPE_MIXIN,
        EOBJ_CLASS_DESCRIPTION_OPS(NULL, NULL, 0),
        NULL,
        0,
        NULL,
        NULL,
        NULL,
        NULL
   };

   static Eobj_Class_Description class_desc = {
        "General",
        EOBJ_CLASS_TYPE_MIXIN,
        EOBJ_CLASS_DESCRIPTION_OPS(NULL, NULL, 0),
        NULL,
        0,
        NULL,
        NULL,
        NULL,
        NULL
   };

   klass_mixin = eobj_class_new(&class_desc_mixin, NULL, NULL);
   fail_if(!klass_mixin);

   klass_simple = eobj_class_new(&class_desc_simple, NULL, NULL);
   fail_if(!klass_simple);

   klass = eobj_class_new(&class_desc, klass_simple, NULL);
   fail_if(klass);

   class_desc.type = EOBJ_CLASS_TYPE_REGULAR;

   klass = eobj_class_new(&class_desc, klass_mixin, NULL);
   fail_if(klass);

   (void) klass;

   eobj_shutdown();
}
END_TEST

START_TEST(eobj_inconsistent_mro)
{
   eobj_init();

   const Eobj_Class *klass;
   const Eobj_Class *klass_mixin;
   const Eobj_Class *klass_mixin2;
   const Eobj_Class *klass_mixin3;

   static const Eobj_Class_Description class_desc_simple = {
        "Simple",
        EOBJ_CLASS_TYPE_REGULAR,
        EOBJ_CLASS_DESCRIPTION_OPS(NULL, NULL, 0),
        NULL,
        0,
        NULL,
        NULL,
        NULL,
        NULL
   };

   static const Eobj_Class_Description class_desc_mixin = {
        "Mixin",
        EOBJ_CLASS_TYPE_MIXIN,
        EOBJ_CLASS_DESCRIPTION_OPS(NULL, NULL, 0),
        NULL,
        0,
        NULL,
        NULL,
        NULL,
        NULL
   };

   static const Eobj_Class_Description class_desc_mixin2 = {
        "Mixin2",
        EOBJ_CLASS_TYPE_MIXIN,
        EOBJ_CLASS_DESCRIPTION_OPS(NULL, NULL, 0),
        NULL,
        0,
        NULL,
        NULL,
        NULL,
        NULL
   };

   static const Eobj_Class_Description class_desc_mixin3 = {
        "Mixin3",
        EOBJ_CLASS_TYPE_MIXIN,
        EOBJ_CLASS_DESCRIPTION_OPS(NULL, NULL, 0),
        NULL,
        0,
        NULL,
        NULL,
        NULL,
        NULL
   };

   klass_mixin = eobj_class_new(&class_desc_mixin, NULL, NULL);
   fail_if(!klass_mixin);

   klass_mixin2 = eobj_class_new(&class_desc_mixin2, klass_mixin, NULL);
   fail_if(!klass_mixin2);

   klass_mixin3 = eobj_class_new(&class_desc_mixin3, klass_mixin, NULL);
   fail_if(!klass_mixin3);

   klass = eobj_class_new(&class_desc_simple, EOBJ_BASE_CLASS, klass_mixin, klass_mixin2, NULL);
   fail_if(klass);

   klass = eobj_class_new(&class_desc_simple, EOBJ_BASE_CLASS, klass_mixin2, klass_mixin, NULL);
   fail_if(!klass);

   klass = eobj_class_new(&class_desc_simple, EOBJ_BASE_CLASS, klass_mixin2, klass_mixin3, NULL);
   fail_if(!klass);

   eobj_shutdown();
}
END_TEST

static void _stub_constructor(Eobj *obj EINA_UNUSED, void *data EINA_UNUSED) {}
static void _stub_class_constructor(Eobj_Class *klass EINA_UNUSED) {}

START_TEST(eobj_bad_interface)
{
   eobj_init();

   const Eobj_Class *klass;

   static Eobj_Class_Description class_desc = {
        "Interface",
        EOBJ_CLASS_TYPE_INTERFACE,
        EOBJ_CLASS_DESCRIPTION_OPS(NULL, NULL, 0),
        NULL,
        10,
        NULL,
        NULL,
        NULL,
        NULL
   };

   klass = eobj_class_new(&class_desc, NULL, NULL);
   fail_if(klass);

   class_desc.data_size = 0;
   class_desc.constructor = _stub_constructor;

   klass = eobj_class_new(&class_desc, NULL, NULL);
   fail_if(klass);

   class_desc.constructor = NULL;
   class_desc.destructor = _stub_constructor;

   klass = eobj_class_new(&class_desc, NULL, NULL);
   fail_if(klass);

   class_desc.destructor = NULL;
   class_desc.class_constructor = _stub_class_constructor;

   klass = eobj_class_new(&class_desc, NULL, NULL);
   fail_if(klass);

   class_desc.class_constructor = NULL;
   class_desc.class_destructor = _stub_class_constructor;

   klass = eobj_class_new(&class_desc, NULL, NULL);
   fail_if(klass);

   class_desc.class_destructor = NULL;

   klass = eobj_class_new(&class_desc, NULL, NULL);
   fail_if(!klass);

   eobj_shutdown();
}
END_TEST

static int _const_ops_counter = 0;

static void
_const_ops_a_set(const Eobj *obj EINA_UNUSED, const void *class_data EINA_UNUSED, va_list *list)
{
   int a = va_arg(*list, int);
   (void) a;
   _const_ops_counter++;
}

static void
_const_ops_a_print(Eobj *obj EINA_UNUSED, void *class_data EINA_UNUSED, va_list *list EINA_UNUSED)
{
   _const_ops_counter++;
}

static void
_const_ops_class_constructor(Eobj_Class *klass)
{
   const Eobj_Op_Func_Description func_desc[] = {
        EOBJ_OP_FUNC_CONST(SIMPLE_ID(SIMPLE_SUB_ID_A_SET), _const_ops_a_set),
        EOBJ_OP_FUNC(SIMPLE_ID(SIMPLE_SUB_ID_A_PRINT), _const_ops_a_print),
        EOBJ_OP_FUNC_SENTINEL
   };

   eobj_class_funcs_set(klass, func_desc);
}

START_TEST(eobj_const_ops)
{
   eobj_init();

   const Eobj_Class *klass;

   static Eobj_Class_Description class_desc = {
        "Simple",
        EOBJ_CLASS_TYPE_REGULAR,
        EOBJ_CLASS_DESCRIPTION_OPS(NULL, NULL, 0),
        NULL,
        0,
        NULL,
        NULL,
        _const_ops_class_constructor,
        NULL
   };

   klass = eobj_class_new(&class_desc, SIMPLE_CLASS, NULL);
   fail_if(!klass);

   Eobj *obj = eobj_add(klass, NULL);
   eobj_do(obj, simple_a_set(7), simple_a_print());
   fail_if(_const_ops_counter != 0);

   eobj_unref(obj);

   eobj_shutdown();
}
END_TEST

void eobj_test_class_errors(TCase *tc)
{
   tcase_add_test(tc, eobj_incomplete_desc);
   tcase_add_test(tc, eobj_inherit_errors);
   tcase_add_test(tc, eobj_inconsistent_mro);
   tcase_add_test(tc, eobj_bad_interface);
   tcase_add_test(tc, eobj_const_ops);
}
