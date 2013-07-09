
#include "Eo.h"

/********************************** EO **********************************/

EAPI Eo_Op SIMPLE_BASE_ID = 0;

enum {
     SIMPLE_SUB_ID_SET,
     SIMPLE_SUB_ID_LAST
};

#define SIMPLE_ID(sub_id) (SIMPLE_BASE_ID + sub_id)

#define simple_set(a, b) SIMPLE_ID(SIMPLE_SUB_ID_SET), EO_TYPECHECK(int, a), EO_TYPECHECK(int, b)

#define SIMPLE_CLASS simple_class_get()
const Eo_Class *simple_class_get(void);

typedef struct
{
   int a;
   int b;
} Private_Data;

#define MY_CLASS SIMPLE_CLASS

static void
_set(Eo *obj EINA_UNUSED, void *class_data, va_list *list)
{
   Private_Data *pd = class_data;
   pd->a = va_arg(*list, int);
   pd->b = va_arg(*list, int);
}

static void
_class_constructor(Eo_Class *klass)
{
   const Eo_Op_Func_Description func_desc[] = {
        EO_OP_FUNC(SIMPLE_ID(SIMPLE_SUB_ID_SET), _set),
        EO_OP_FUNC_SENTINEL
   };

   eo_class_funcs_set(klass, func_desc);
}

static const Eo_Op_Description op_desc[] = {
     EO_OP_DESCRIPTION(SIMPLE_SUB_ID_SET, "Set properties"),
     EO_OP_DESCRIPTION_SENTINEL
};

static const Eo_Class_Description class_desc = {
     EO_VERSION,
     "Simple",
     EO_CLASS_TYPE_REGULAR,
     EO_CLASS_DESCRIPTION_OPS(&SIMPLE_BASE_ID, op_desc, SIMPLE_SUB_ID_LAST),
     NULL,
     sizeof(Private_Data),
     _class_constructor,
     NULL
};

EO_DEFINE_CLASS(simple_class_get, &class_desc, EO_BASE_CLASS, NULL);

/********************************** EO2 **********************************/

EAPI Eo_Op TEST_BASE_ID = 0;
#define TEST_ID(func_name) (TEST_BASE_ID + (TEST_SUB_ID_ ## func_name))

enum {
     TEST_SUB_ID_inst_func,
     TEST_SUB_ID_LAST
};

EAPI int inst_func(eo2_a, int a, int b);
EO_FUNC_BODYV(inst_func, int, TEST_ID, EO_FUNC_CALLV(a, b), 0, int a, int b)

EAPI const Eo_Class *test_class_get(void);
#define TEST_CLASS test_class_get()

typedef struct
{
   int a;
   int b;
} Private_Data2;

static int
_inst_func(Eo *objid, void *obj_data, int a, int b)
{
   Private_Data2 *data = (Private_Data2 *) obj_data;
   data->a = a;
   data->b = b;
   return a + b;
}

static void
_class_constructor2(Eo_Class *klass)
{
   const Eo_Op_Func_Description func_desc[] = {
        EO_OP_FUNC(TEST_ID(inst_func), (void *) _inst_func),
        EO_OP_FUNC_SENTINEL
   };

   eo_class_funcs_set(klass, func_desc);
}

static const Eo_Op_Description op_desc2[] = {
     EO_OP_DESCRIPTION(TEST_SUB_ID_inst_func, "Test base id"),
     EO_OP_DESCRIPTION_SENTINEL
};

static const Eo_Class_Description class_desc2 = {
     EO_VERSION,
     "Test",
     EO_CLASS_TYPE_REGULAR,
     EO_CLASS_DESCRIPTION_OPS(&TEST_BASE_ID, op_desc2, TEST_SUB_ID_LAST),
     NULL,
     sizeof(Private_Data2),
     _class_constructor2,
     NULL
};

EO_DEFINE_CLASS(test_class_get, &class_desc2, EO_BASE_CLASS, NULL)

/********************************** MAIN **********************************/

int
main(int argc, char** argv, char** env)
{
   int i, a, b;
   Eo *obj, *obj2;
   uint64_t dt;
   struct timespec t0;
   struct timespec t1;

   eo_init();

   obj = eo_add(SIMPLE_CLASS, NULL);
   obj2 = eo_add(TEST_CLASS, NULL);

   /* EO */

   clock_gettime(CLOCK_MONOTONIC, &t0);
   for (i = 0; i < 1000000; i++)
     {
        eo_do(obj,
              simple_set(32, 12),
              simple_set(10, 14),
              simple_set(50, 24),
              simple_set(32, 12),
              simple_set(10, 14),
              simple_set(50, 24)
             );
     }
   clock_gettime(CLOCK_MONOTONIC, &t1);

   dt = ((t1.tv_sec * 1000000000) + t1.tv_nsec) -
      ((t0.tv_sec * 1000000000) + t0.tv_nsec);
   printf(" EO : time %7u [ms]\n", (unsigned int)(dt/1000000));

   /* EO2 */

   clock_gettime(CLOCK_MONOTONIC, &t0);
   for (i = 0; i < 1000000; i++)
     {
        // here we handle return values !
        eo2_do(obj2,
               a = inst_func(eo2_o, 32, 12);
               inst_func(eo2_o, 10, 14);
               b = inst_func(eo2_o, 50, 24);
               inst_func(eo2_o, 32, 12);
               a = inst_func(eo2_o, 10, 14);
               inst_func(eo2_o, 50, 24);
              );
     }
   clock_gettime(CLOCK_MONOTONIC, &t1);

   dt = ((t1.tv_sec * 1000000000) + t1.tv_nsec) -
      ((t0.tv_sec * 1000000000) + t0.tv_nsec);
   printf(" EO2: time %7u [ms]\n", (unsigned int)(dt/1000000));

   /* done */

   eo_del(obj);
   eo_del(obj2);

   eo_shutdown();

   return 0;
}

