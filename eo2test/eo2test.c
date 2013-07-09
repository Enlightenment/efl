#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "Eo.h"
#include "eo2test.h"

#ifndef NOMAIN

int
main()
{
   eo_init();
   int a = 45, b = 0;

   Eo *obj = eo_add(TEST_CLASS, NULL);

   eo2_do(obj,
         a = inst_func_set(eo2_o, 32, 12);
         inst_func_set(eo2_o, 10, 31);
         b = inst_func_set(eo2_o, 50, 42);
         a = inst_func_get(eo2_o);
         );

   printf("%d %d\n", a, b);

   eo_del(obj);

   eo_shutdown();
}

#endif

/* static */

typedef struct
{
   int a;
   int b;
} Private_Data;

static int
_inst_func_set(Eo *objid EINA_UNUSED, void *obj_data, int a, int b)
{
   Private_Data *data = (Private_Data *) obj_data;
   data->a = a;
   data->b = b;
   return a + b;
}

static int
_inst_func_get(Eo *objid EINA_UNUSED, void *obj_data)
{
   Private_Data *data = (Private_Data *) obj_data;
   return data->a;
}

static void
_class_constructor(Eo_Class *klass)
{
   const Eo_Op_Func_Description func_desc[] = {
        EO_OP_FUNC(TEST_ID(inst_func_set), (void *) _inst_func_set),
        EO_OP_FUNC(TEST_ID(inst_func_get), (void *) _inst_func_get),
        EO_OP_FUNC_SENTINEL
   };

   eo_class_funcs_set(klass, func_desc);
}

EAPI Eo_Op TEST_BASE_ID = 0;

static const Eo_Op_Description op_desc[] = {
     EO_OP_DESCRIPTION(TEST_SUB_ID_inst_func_set, "Set"),
     EO_OP_DESCRIPTION(TEST_SUB_ID_inst_func_get, "Get"),
     EO_OP_DESCRIPTION_SENTINEL
};

static const Eo_Class_Description class_desc = {
     EO_VERSION,
     "Test",
     EO_CLASS_TYPE_REGULAR,
     EO_CLASS_DESCRIPTION_OPS(&TEST_BASE_ID, op_desc, TEST_SUB_ID_LAST),
     NULL,
     sizeof(Private_Data),
     _class_constructor,
     NULL
};

EO_DEFINE_CLASS(test_class_get, &class_desc, EO_BASE_CLASS, NULL)

/* fct_sym, ret_type, OP_ID, fct_call, default_ret_val, arguments… */
EAPI EO_FUNC_BODYV(inst_func_set, int, TEST_ID, EO_FUNC_CALLV(a, b), 0, int a, int b)
