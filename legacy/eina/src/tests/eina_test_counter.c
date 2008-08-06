#include <stdlib.h>

#include "eina_counter.h"
#include "eina_suite.h"

START_TEST(eina_counter_init_shutdown)
{
   eina_counter_init();
    eina_counter_init();
    eina_counter_shutdown();
    eina_counter_init();
     eina_counter_init();
     eina_counter_shutdown();
    eina_counter_shutdown();
   eina_counter_shutdown();
}
END_TEST

START_TEST(eina_counter_simple)
{
   Eina_Counter *cnt;
   int i;

   eina_counter_init();

   cnt = eina_counter_add("eina_test");
   fail_if(!cnt);

   eina_counter_start(cnt);

   for (i = 0; i < 100000; ++i)
     malloc(sizeof(long int));

   eina_counter_stop(cnt, i);

   eina_counter_start(cnt);

   for (i = 0; i < 200000; ++i)
     malloc(sizeof(long int));

   eina_counter_stop(cnt, i);

   eina_counter_dump(cnt, stderr);

   eina_counter_delete(cnt);

   eina_counter_shutdown();
}
END_TEST

void eina_test_counter(TCase *tc)
{
   tcase_add_test(tc, eina_counter_init_shutdown);
   tcase_add_test(tc, eina_counter_simple);
}

