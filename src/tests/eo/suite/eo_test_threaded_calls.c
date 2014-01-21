#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <stdio.h>

#include "Eo.h"
#include "eo_suite.h"

static Eina_Barrier barrier;
static Eina_Spinlock locks[2];

typedef struct
{
   int v;
} Thread_Test_Public_Data;

#define THREAD_TEST_CLASS thread_test_class_get()
const Eo_Class *thread_test_class_get(void);

EO2_FUNC_BODY(thread_test_v_get, int, 0);
EO2_VOID_FUNC_BODY(thread_test_try_swap_stack);
EO2_VOID_FUNC_BODYV(thread_test_constructor, EO2_FUNC_CALL(v), int v);

static int
_v_get(Eo *obj EINA_UNUSED, void *class_data)
{
   Thread_Test_Public_Data *pd = class_data;

   return pd->v;
}

static void
_try_swap_stack(Eo *obj EINA_UNUSED, void *class_data)
{
   Thread_Test_Public_Data *pd = class_data;

   if (pd->v == 0 )
     {
        eina_spinlock_release(&locks[0]);
        eina_spinlock_take(&locks[1]);
        eina_barrier_wait(&barrier);
     }
   else if (pd->v == 1 )
     {
        eina_barrier_wait(&barrier);
        eina_spinlock_take(&locks[1]);
     }
}

static void
_constructor(Eo *obj, void *class_data EINA_UNUSED, int v)
{
   Thread_Test_Public_Data *pd = class_data;

   eo2_do_super(obj, THREAD_TEST_CLASS, eo2_constructor());

   pd->v = v;
}

static Eo2_Op_Description op_descs[] = {
     EO2_OP_FUNC(thread_test_constructor, _constructor, "Constructor."),
     EO2_OP_FUNC(thread_test_v_get, _v_get, "Get property v."),
     EO2_OP_FUNC(thread_test_try_swap_stack, _try_swap_stack, "Swap call stack frames if it is not thread safe."),
     EO2_OP_SENTINEL
};

static const Eo_Class_Description class_desc = {
     EO2_VERSION,
     "Thread Test",
     EO_CLASS_TYPE_REGULAR,
     EO2_CLASS_DESCRIPTION_OPS(op_descs),
     NULL,
     sizeof(Thread_Test_Public_Data),
     NULL,
     NULL
};

EO_DEFINE_CLASS(thread_test_class_get, &class_desc, EO2_BASE_CLASS, NULL)

static void *
_thread_job(void *data, Eina_Thread t EINA_UNUSED)
{
   Eo *obj;
   int v = (int) (uintptr_t) data;

   if (v == 1)
     eina_spinlock_take(&locks[0]);

   obj = eo2_add_custom(THREAD_TEST_CLASS, NULL, thread_test_constructor(v));

   eo2_do(obj, thread_test_try_swap_stack(), v = thread_test_v_get());

   eina_spinlock_release(&locks[1]);

   eo_unref(obj);

   return (void *) (uintptr_t) v;
}

START_TEST(eo_threaded_calls_test)
{
   Eina_Thread threads[2];

   eo_init();

   fail_if(!eina_spinlock_new(&locks[0]));
   fail_if(!eina_spinlock_new(&locks[1]));
   fail_if(!eina_barrier_new(&barrier, 2));

   eina_spinlock_take(&locks[0]);

   fail_if(!eina_thread_create(&threads[0], EINA_THREAD_NORMAL, 0, _thread_job, (void *) (uintptr_t)0));
   fail_if(!eina_thread_create(&threads[1], EINA_THREAD_NORMAL, 0, _thread_job, (void *) (uintptr_t)1));

   fail_if(0 != (int)(uintptr_t)eina_thread_join(threads[0]));
   fail_if(1 != (int)(uintptr_t)eina_thread_join(threads[1]));

   eina_spinlock_free(&locks[0]);
   eina_spinlock_free(&locks[1]);
   eina_barrier_free(&barrier);

   eo_shutdown();
}
END_TEST

void eo_test_threaded_calls(TCase *tc)
{
   tcase_add_test(tc, eo_threaded_calls_test);
}
