#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <stdio.h>

#include <Eo.h>

#include "eo_suite.h"
#include "eo_test_class_simple.h"

#define RETVAL ((void *)0x1234)

static const void *
func(void *data, Eina_Bool canceled EINA_UNUSED, Eina_Coro *coro EINA_UNUSED)
{
   Eo *obj = data;

   fail_unless(efl_ref(obj)); // if efl_domain_data is wrong, this fails...

   return RETVAL;
}

START_TEST(eo_coro)
{
   Eo *obj;
   Eina_Coro *coro;
   void *result;

   efl_object_init();

   obj = efl_add_ref(SIMPLE_CLASS, NULL);
   fail_unless(obj);

   coro = eina_coro_new(func, obj, EINA_CORO_STACK_SIZE_DEFAULT);
   fail_unless(coro);

   fail_if(eina_coro_run(&coro, &result, NULL)); // doesn't yield, so should finish
   ck_assert_ptr_eq(result, RETVAL);

   ck_assert_int_eq(efl_ref_count(obj), 2);
   efl_unref(obj);
   efl_unref(obj);

   efl_object_shutdown();
}
END_TEST

void eo_test_coro(TCase *tc)
{
   tcase_add_test(tc, eo_coro);
}
