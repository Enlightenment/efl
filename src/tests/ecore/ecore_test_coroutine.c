#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdio.h>
#include <unistd.h>

#include <Eina.h>
#include <Ecore.h>

#include "ecore_suite.h"

typedef struct _Ecore_Coroutine_Test Ecore_Coroutine_Test;
struct _Ecore_Coroutine_Test
{
   int v;
   int s;
};

static Ecore_Coroutine_Test t1;

static int
_ecore_test_v1(void *data, Ecore_Coroutine *coro)
{
   int *t = data;

   ecore_coroutine_yield(coro, 7);
   t1.v = *t;
   ecore_coroutine_yield(coro, 42);
   t1.s = *t;

   return 0xDEADBEEF;
}

static int
_ecore_test_v2(void *data, Ecore_Coroutine *coro EINA_UNUSED)
{
   int *t = data;

   ecore_coroutine_yield(coro, 42);
   t1.v = *t;
   ecore_coroutine_yield(coro, 7);
   t1.s = *t;

   return 0xDEADBEEF;
}

START_TEST(ecore_test_coroutine_simple)
{
   Ecore_Coroutine *coro1;
   Ecore_Coroutine *coro2;
   int value[] = { 7, 42, 0xDEADBEEF };

   ecore_init();

   t1.v = 0xDEADBEEF; t1.s = 0xDEADBEEF;

   coro1 = ecore_coroutine_add(0, _ecore_test_v1, &value[0]);
   coro2 = ecore_coroutine_add(4*1024*1024, _ecore_test_v2, &value[1]);

   fail_if(ecore_coroutine_state_get(coro1) != ECORE_COROUTINE_NEW);
   fail_if(ecore_coroutine_resume(coro1) != 7);
   fail_if(t1.v != 0xDEADBEEF || t1.s != 0xDEADBEEF);
   fail_if(ecore_coroutine_resume(coro2) != 42);
   fail_if(t1.v != 0xDEADBEEF || t1.s != 0xDEADBEEF);
   fail_if(ecore_coroutine_resume(coro1) != 42);
   fail_if(t1.v != value[0] || t1.s != 0xDEADBEEF);
   fail_if(ecore_coroutine_resume(coro1) != 0xDEADBEEF);
   fail_if(t1.v != value[0] || t1.s != value[0]);
   fail_if(ecore_coroutine_resume(coro2) != 7);
   fail_if(t1.v != value[1] || t1.s != value[0]);
   fail_if(ecore_coroutine_resume(coro2) != 0xDEADBEEF);
   fail_if(t1.v != value[1] || t1.s != value[1]);
   fail_if(ecore_coroutine_state_get(coro2) != ECORE_COROUTINE_FINISHED);

   fail_if(ecore_coroutine_del(coro1) != &value[0]);
   fail_if(ecore_coroutine_del(coro2) != &value[1]);

   ecore_shutdown();
}
END_TEST

void ecore_test_coroutine(TCase *tc)
{
  tcase_add_test(tc, ecore_test_coroutine_simple);
}
