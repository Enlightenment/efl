#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <stdio.h>

#include <Eo.h>

#include "eo_suite.h"

static Eina_Barrier barrier;
static Eina_Barrier barrier0;
static Eina_Spinlock locks[2];

typedef struct
{
   int v;
} Thread_Test_Public_Data;

#define THREAD_TEST_CLASS thread_test_class_get()
const Efl_Class *thread_test_class_get(void);

EFL_FUNC_BODY(thread_test_v_get, int, 0);
EFL_VOID_FUNC_BODY(thread_test_try_swap_stack);
EFL_VOID_FUNC_BODYV(thread_test_constructor, EFL_FUNC_CALL(v), int v);

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
        fail_if(EINA_LOCK_SUCCEED != eina_spinlock_release(&locks[0]));
        fail_if(EINA_LOCK_SUCCEED != eina_spinlock_take(&locks[1]));
        eina_barrier_wait(&barrier);
     }
   else if (pd->v == 1 )
     {
        eina_barrier_wait(&barrier);
        fail_if(EINA_LOCK_SUCCEED != eina_spinlock_take(&locks[1]));
     }
}

static Eo *
_constructor(Eo *obj, void *class_data EINA_UNUSED, int v)
{
   Thread_Test_Public_Data *pd = class_data;

   pd->v = v;

   return obj;
}

static Eina_Bool
_class_initializer(Efl_Class *klass)
{
   EFL_OPS_DEFINE(ops,
         EFL_OBJECT_OP_FUNC(thread_test_constructor, _constructor),
         EFL_OBJECT_OP_FUNC(thread_test_v_get, _v_get),
         EFL_OBJECT_OP_FUNC(thread_test_try_swap_stack, _try_swap_stack),
   );

   return efl_class_functions_set(klass, &ops, NULL);
}

static const Efl_Class_Description class_desc = {
     EO_VERSION,
     "Thread Test",
     EFL_CLASS_TYPE_REGULAR,
     sizeof(Thread_Test_Public_Data),
     _class_initializer,
     NULL,
     NULL
};

EFL_DEFINE_CLASS(thread_test_class_get, &class_desc, EO_CLASS, NULL)

static void *
_thread_job(void *data, Eina_Thread t EINA_UNUSED)
{
   Eo *obj;
   int v = (int) (uintptr_t) data;

   if (v == 0) {
     fail_if(EINA_LOCK_SUCCEED != eina_spinlock_take(&locks[0]));
     eina_barrier_wait(&barrier0);
   }
   else {
     eina_barrier_wait(&barrier0);
     fail_if(EINA_LOCK_SUCCEED != eina_spinlock_take(&locks[0]));
   }

   obj = efl_add_ref(THREAD_TEST_CLASS, NULL, thread_test_constructor(efl_added, v));

   thread_test_try_swap_stack(obj);
   v = thread_test_v_get(obj);

   fail_if(EINA_LOCK_SUCCEED != eina_spinlock_release(&locks[1]));

   efl_unref(obj);

   return (void *) (uintptr_t) v;
}

EFL_START_TEST(eo_threaded_calls_test)
{
   Eina_Thread threads[2];


   fail_if(!eina_spinlock_new(&locks[0]));
   fail_if(!eina_spinlock_new(&locks[1]));
   fail_if(!eina_barrier_new(&barrier, 2));
   fail_if(!eina_barrier_new(&barrier0, 2));

   fail_if(!eina_thread_create(&threads[0], EINA_THREAD_NORMAL, -1, _thread_job, (void *) (uintptr_t)0));
   fail_if(!eina_thread_create(&threads[1], EINA_THREAD_NORMAL, -1, _thread_job, (void *) (uintptr_t)1));

   fail_if(0 != (int)(uintptr_t)eina_thread_join(threads[0]));
   fail_if(1 != (int)(uintptr_t)eina_thread_join(threads[1]));

   eina_spinlock_free(&locks[0]);
   eina_spinlock_free(&locks[1]);
   eina_barrier_free(&barrier);
   eina_barrier_free(&barrier0);

}
EFL_END_TEST

void eo_test_threaded_calls(TCase *tc)
{
   tcase_add_test(tc, eo_threaded_calls_test);
}
