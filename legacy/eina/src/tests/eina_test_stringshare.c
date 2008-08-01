#include <stdlib.h>
#include <time.h>
#include <stdio.h>

#include "eina_suite.h"
#include "eina_stringshare.h"
#include "eina_array.h"

START_TEST(eina_stringshare_init_shutdown)
{
   eina_stringshare_init();
    eina_stringshare_init();
    eina_stringshare_shutdown();
    eina_stringshare_init();
     eina_stringshare_init();
     eina_stringshare_shutdown();
    eina_stringshare_shutdown();
   eina_stringshare_shutdown();
}
END_TEST

#define TEST0 "test/0"
#define TEST1 "test/1"

START_TEST(eina_stringshare_simple)
{
   const char *t0;
   const char *t1;

   eina_stringshare_init();

   t0 = eina_stringshare_add(TEST0);
   t1 = eina_stringshare_add(TEST1);

   fail_if(t0 == NULL);
   fail_if(t1 == NULL);
   fail_if(strcmp(t0, TEST0) != 0);
   fail_if(strcmp(t1, TEST1) != 0);

   eina_stringshare_del(t0);
   eina_stringshare_del(t1);

   eina_stringshare_shutdown();
}
END_TEST

START_TEST(eina_stringshare_test_share)
{
   const char *t0;
   const char *t1;

   eina_stringshare_init();

   t0 = eina_stringshare_add(TEST0);
   t1 = eina_stringshare_add(TEST0);

   fail_if(t0 == NULL);
   fail_if(t1 == NULL);
   fail_if(strcmp(t0, TEST0) != 0);
   fail_if(strcmp(t1, TEST0) != 0);
   fail_if(t0 != t1);

   eina_stringshare_del(t0);
   eina_stringshare_del(t1);

   eina_stringshare_shutdown();
}
END_TEST

START_TEST(eina_stringshare_collision)
{
   Eina_Array *ea;
   char buffer[50];
   int i;

   srand(time(NULL));

   eina_stringshare_init();

   ea = eina_array_new(256);
   fail_if(!ea);

   for (i = 0; i < 10000; ++i)
     {
	snprintf(buffer, 1024, "%i", rand());
	eina_array_append(ea, (void*) eina_stringshare_add(buffer));
	if (rand() > RAND_MAX / 2) eina_stringshare_add(buffer);
     }

   for (i = 0; i < 10000; ++i)
     {
	snprintf(buffer, 1024, "%i", 60000 - i);
	eina_array_append(ea, (void*) eina_stringshare_add(buffer));
	eina_stringshare_add(buffer);
     }

   for (i = 0; i < 200; ++i)
     eina_stringshare_del(eina_array_get(ea, i));

   eina_stringshare_shutdown();

   eina_array_free(ea);
}
END_TEST

START_TEST(eina_stringshare_not_owned)
{
   eina_stringshare_init();

   eina_stringshare_add(TEST0);
   eina_stringshare_del(TEST0);

   eina_stringshare_shutdown();
}
END_TEST

void
eina_test_stringshare(TCase *tc)
{
   tcase_add_test(tc, eina_stringshare_init_shutdown);
   tcase_add_test(tc, eina_stringshare_simple);
   tcase_add_test(tc, eina_stringshare_test_share);
   tcase_add_test(tc, eina_stringshare_collision);
   tcase_add_test(tc, eina_stringshare_not_owned);
}
