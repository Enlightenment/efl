#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

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

void eobj_test_class_errors(TCase *tc)
{
   tcase_add_test(tc, eobj_incomplete_desc);
   tcase_add_test(tc, eobj_inherit_errors);
}
