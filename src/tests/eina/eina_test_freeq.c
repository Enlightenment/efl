#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <Eina.h>

#include "eina_suite.h"

START_TEST(freeq_simple)
{
   Eina_FreeQ *fq, *pfq;

   eina_init();

   fail_if(eina_freeq_main_get() == NULL);
   pfq = eina_freeq_main_get();

   fq = eina_freeq_new();
   fail_if(!fq);

   eina_freeq_main_set(fq);
   fail_if(eina_freeq_main_get() != fq);

   eina_freeq_free(fq);
   fail_if(eina_freeq_main_get() != NULL);
   eina_freeq_main_set(pfq);

   eina_shutdown();
}
END_TEST

static int _n = 0;

static void freefn(void *data)
{
   free(data);
   _n--;
}

START_TEST(freeq_tune)
{
   void *p;

   eina_init();

   eina_freeq_count_max_set(eina_freeq_main_get(), 3);
   fail_if(eina_freeq_count_max_get(eina_freeq_main_get()) != 3);

   eina_freeq_mem_max_set(eina_freeq_main_get(), 20);
   fail_if(eina_freeq_mem_max_get(eina_freeq_main_get()) != 20);

   _n++;
   p = malloc(9);
   eina_freeq_ptr_main_add(p, freefn, 9);
   _n++;
   p = malloc(9);
   eina_freeq_ptr_main_add(p, freefn, 9);
   _n++;
   p = malloc(9);
   eina_freeq_ptr_main_add(p, freefn, 9);
   eina_freeq_ptr_main_add(NULL, freefn, 9);
   fail_if(_n > 2);

   eina_freeq_count_max_set(eina_freeq_main_get(), 1);
   fail_if(_n > 1);

   eina_freeq_clear(eina_freeq_main_get());
   fail_if(_n > 0);

   fail_if(eina_freeq_ptr_pending(eina_freeq_main_get()) == EINA_TRUE);

   eina_shutdown();
}
END_TEST

START_TEST(freeq_reduce)
{
   void *p;

   eina_init();

   _n++;
   p = malloc(9);
   eina_freeq_ptr_main_add(p, freefn, 9);
   _n++;
   p = malloc(9);
   eina_freeq_ptr_main_add(p, freefn, 9);
   _n++;
   p = malloc(9);
   eina_freeq_ptr_main_add(p, freefn, 9);

   while (eina_freeq_ptr_pending(eina_freeq_main_get()))
     eina_freeq_reduce(eina_freeq_main_get(), 1);
   fail_if(_n > 0);
   fail_if(eina_freeq_ptr_pending(eina_freeq_main_get()) == EINA_TRUE);

   _n++;
   p = malloc(9);
   eina_freeq_ptr_main_add(p, freefn, 9);
   _n++;
   p = malloc(9);
   eina_freeq_ptr_main_add(p, freefn, 9);
   _n++;
   p = malloc(9);
   eina_freeq_ptr_main_add(p, freefn, 9);

   while (eina_freeq_ptr_pending(eina_freeq_main_get()))
     eina_freeq_reduce(eina_freeq_main_get(), 5);
   fail_if(_n > 0);
   fail_if(eina_freeq_ptr_pending(eina_freeq_main_get()) == EINA_TRUE);

   eina_shutdown();
}
END_TEST

void
eina_test_freeq(TCase *tc)
{
   tcase_add_test(tc, freeq_simple);
   tcase_add_test(tc, freeq_tune);
   tcase_add_test(tc, freeq_reduce);
}
