#include "Eina.h"
#include "eina_suite.h"

START_TEST(eina_simple)
{
   fail_if(!eina_init());
   fail_if(eina_shutdown() != 0);
}
END_TEST

void eina_test_main(TCase *tc)
{
   tcase_add_test(tc, eina_simple);
}
