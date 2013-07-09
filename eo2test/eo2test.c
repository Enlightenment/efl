#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "Eo.h"
#include "eo2test.h"

int
main()
{
   eo_init();
   int a = 45, b = 0;

   Eo *obj = eo_add(TEST_CLASS, NULL);

   eo2_do(obj,
         a = inst_func(eo2_o, 32);
         inst_func(eo2_o, 10);
         b = inst_func(eo2_o, 50);
         );

   printf("%d %d\n", a, b);

   eo_del(obj);

   eo_shutdown();
}

static int
_inst_func(Eo *obj EINA_UNUSED, int a)
{
   static int c = 0;
   int ret = a + c;
   c += a;
   return ret;
}

static void
_class_constructor(Eo_Class *klass)
{
   const Eo_Op_Func_Description func_desc[] = {
        EO_OP_FUNC(TEST_ID(inst_func), (void *) _inst_func),
        EO_OP_FUNC_SENTINEL
   };

   eo_class_funcs_set(klass, func_desc);
}

EAPI Eo_Op TEST_BASE_ID = 0;

static const Eo_Op_Description op_desc[] = {
     EO_OP_DESCRIPTION(TEST_SUB_ID_inst_func, "Test base id"),
     EO_OP_DESCRIPTION_SENTINEL
};

static const Eo_Class_Description class_desc = {
     EO_VERSION,
     "Test",
     EO_CLASS_TYPE_REGULAR,
     EO_CLASS_DESCRIPTION_OPS(&TEST_BASE_ID, op_desc, TEST_SUB_ID_LAST),
     NULL,
     0,
     _class_constructor,
     NULL
};

EO_DEFINE_CLASS(test_class_get, &class_desc, EO_BASE_CLASS, NULL)
