#include "eina_suite.h"
#include "eina_error.h"

START_TEST(eina_error_init_shutdown)
{
   eina_error_init();
    eina_error_init();
    eina_error_shutdown();
    eina_error_init();
     eina_error_init();
     eina_error_shutdown();
    eina_error_shutdown();
   eina_error_shutdown();
}
END_TEST

#define TEST_TEXT "The big test\n"

#include <stdio.h>

START_TEST(eina_error_errno)
{
   int test;

   eina_error_init();

   test = eina_error_register(TEST_TEXT);
   fail_if(!eina_error_msg_get(test));
   fail_if(strcmp(eina_error_msg_get(test), TEST_TEXT) != 0);

   eina_error_set(test);
   fail_if(eina_error_get() != test);

   eina_error_shutdown();
}
END_TEST

START_TEST(eina_error_macro)
{
   eina_error_log_level_set(EINA_ERROR_LEVEL_DBG);

   EINA_ERROR_PERR("An error\n");
   EINA_ERROR_PINFO("An info\n");
   EINA_ERROR_PWARN("A warning\n");
   EINA_ERROR_PDBG("A debug\n");
}
END_TEST

void
eina_test_error(TCase *tc)
{
   tcase_add_test(tc, eina_error_init_shutdown);
   tcase_add_test(tc, eina_error_errno);
   tcase_add_test(tc, eina_error_macro);
}
