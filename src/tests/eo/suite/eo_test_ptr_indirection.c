#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "Eo.h"
#include "eo_suite.h"
#include "eo_test_class_simple.h"

START_TEST(eo_ptr_ind)
{
   int i;
   int request = 100000;

   eo_init();

   Eo **objs = calloc(request, sizeof(Eo *));

   for (i = 0 ; i < request ; i++)
      objs[i] = eo_add(SIMPLE_CLASS, NULL);

   for (i = 0 ; i < request; i++)
      eo_unref(objs[i]);
   eo_unref(objs[0]);

   free(objs);

   eo_shutdown();
}
END_TEST

void eo_test_ptr_ind(TCase *tc)
{
   tcase_add_test(tc, eo_ptr_ind);
}
