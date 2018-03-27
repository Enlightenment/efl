#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <Eina.h>

#include "eina_suite.h"

EFL_START_TEST(freeq_simple)
{
   Eina_FreeQ *fq;
   int *p;

   fq = eina_freeq_main_get();
   fail_if(fq == NULL);
   fail_if(eina_freeq_type_get(fq) != EINA_FREEQ_DEFAULT);

   p = malloc(sizeof(int));
   *p = 42;
   eina_freeq_ptr_main_add(p, NULL, sizeof(int));
   eina_freeq_clear(fq);
   fail_if(eina_freeq_ptr_pending(eina_freeq_main_get()));

   fq = eina_freeq_new(EINA_FREEQ_DEFAULT);
   fail_if(!fq);

   eina_freeq_free(fq);

}
EFL_END_TEST

static int _n = 0;

static void freefn(void *data)
{
   free(data);
   _n--;
}

EFL_START_TEST(freeq_tune)
{
   void *p;

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

}
EFL_END_TEST

EFL_START_TEST(freeq_reduce)
{
   void *p;

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

}
EFL_END_TEST

static void
postponed_free(void *data)
{
   int *p = data;

   // we leak here (by choice -- to inspect the memory after clear/reduce)
   *p = 0xDEADBEEF;
   _n--;
}

static inline unsigned int *
new_uint(int val)
{
   unsigned int *p;

   p = malloc(sizeof(*p));
   *p = val;
   return p;
}

EFL_START_TEST(freeq_postponed)
{
   Eina_FreeQ *fq;
   unsigned int *values[20];
   size_t k;

   _n = 0;

   fq = eina_freeq_new(EINA_FREEQ_POSTPONED);

   fail_if(!fq);
   fail_if(eina_freeq_type_get(fq) != EINA_FREEQ_POSTPONED);

   // by default: no limit
   ck_assert_int_eq(eina_freeq_count_max_get(fq), -1);
   ck_assert_int_eq(eina_freeq_mem_max_get(fq), 0);

   for (k = 0; k < EINA_C_ARRAY_LENGTH(values); k++)
     {
        _n++;
        values[k] = new_uint(k);
        eina_freeq_ptr_add(fq, values[k], postponed_free, sizeof(int));
     }
   ck_assert_int_eq(_n, EINA_C_ARRAY_LENGTH(values));

   fail_if(!eina_freeq_ptr_pending(fq));
   while (eina_freeq_ptr_pending(fq))
     eina_freeq_reduce(fq, 1);
   fail_if(eina_freeq_ptr_pending(fq));
   ck_assert_int_eq(_n, 0);

   for (k = 0; k < EINA_C_ARRAY_LENGTH(values); k++)
     ck_assert_int_eq(*(values[k]), 0xDEADBEEF);

   for (k = 0; k < EINA_C_ARRAY_LENGTH(values); k++)
     {
        _n++;
        values[k] = new_uint(k);
        eina_freeq_ptr_add(fq, values[k], postponed_free, sizeof(int));
     }
   ck_assert_int_eq(_n, EINA_C_ARRAY_LENGTH(values));

   fail_if(!eina_freeq_ptr_pending(fq));
   eina_freeq_clear(fq);
   fail_if(eina_freeq_ptr_pending(fq));
   ck_assert_int_eq(_n, 0);

   for (k = 0; k < EINA_C_ARRAY_LENGTH(values); k++)
     {
        ck_assert_int_eq(*(values[k]), 0xDEADBEEF);
        free(values[k]);
     }

   eina_freeq_free(fq);

}
EFL_END_TEST

void
eina_test_freeq(TCase *tc)
{
   tcase_add_test(tc, freeq_simple);
   tcase_add_test(tc, freeq_tune);
   tcase_add_test(tc, freeq_reduce);
   tcase_add_test(tc, freeq_postponed);
}
