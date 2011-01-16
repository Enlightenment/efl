#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <Ecore_Con.h>

#include "ecore_suite.h"


START_TEST(ecore_test_ecore_con_init)
{
   int ret;

   ret = ecore_con_init();
   fail_if(ret != 1);

   ret = ecore_con_shutdown();
   fail_if(ret != 0);
}
END_TEST

void ecore_test_ecore_con(TCase *tc)
{
   tcase_add_test(tc, ecore_test_ecore_con_init);
}
