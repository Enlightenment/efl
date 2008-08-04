#include <stdio.h>
#include <stdlib.h>

#include "eina_suite.h"
#include "eina_lalloc.h"

typedef struct _Eina_Lazy_Allocator_Test Eina_Lazy_Allocator_Test;
struct _Eina_Lazy_Allocator_Test
{
   void *data;
   int num;
};

static Eina_Bool
_test_alloc(Eina_Lazy_Allocator_Test *elat, int num)
{
   fprintf(stderr, "%i %i\n", elat->num, num);

   if (elat->num == 10 && num == 1) return EINA_FALSE;
   if (elat->num == 122 && num == 128) return EINA_FALSE;

   elat->num += num;
   elat->data = realloc(elat->data, elat->num);

   return EINA_TRUE;
}

static void
_test_free(Eina_Lazy_Allocator_Test *elat)
{
   free(elat->data);
   elat->data = NULL;
   elat->num = 0;
}

START_TEST(eina_lalloc_simple)
{
   Eina_Lazy_Allocator_Test *elat;
   Eina_Lalloc *test;
   int i;

   elat = calloc(1, sizeof (Eina_Lazy_Allocator_Test));
   fail_if(!elat);

   test = eina_lalloc_new(elat, EINA_LALLOC_ALLOC(_test_alloc), EINA_LALLOC_FREE(_test_free), 10);
   fail_if(!test);

   for (i = 0; i < 10; ++i)
     fail_if(eina_lalloc_element_add(test) != EINA_TRUE);
   fail_if(eina_lalloc_element_add(test) != EINA_FALSE);
   fail_if(eina_lalloc_elements_add(test, 5) != EINA_TRUE);
   for (i = 0; i < 21; ++i)
     fail_if(eina_lalloc_element_add(test) != EINA_TRUE);

   fail_if(eina_lalloc_elements_add(test, 50) != EINA_FALSE);

   eina_lalloc_delete(test);
}
END_TEST

void
eina_test_lalloc(TCase *tc)
{
   tcase_add_test(tc, eina_lalloc_simple);
}
