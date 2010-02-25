#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <Ecore.h>

#include "ecore_suite.h"

static int
_timer_cb(void *data)
{
   ecore_main_loop_quit();
}

START_TEST(ecore_test_ecore_init)
{
   int ret;

   ret = ecore_init();
   fail_if(ret != 1);

   ret = ecore_shutdown();
   fail_if(ret != 0);
}
END_TEST

START_TEST(ecore_test_ecore_main_loop)
{
   Ecore_Timer *timer = NULL;
   int          ret;

   ret = ecore_init();
   fail_if(ret != 1);

   timer = ecore_timer_add(0.5, _timer_cb, NULL);
   fail_if(timer == NULL);

   ecore_main_loop_begin();

   ret = ecore_shutdown();
   fail_if(ret != 0);
}
END_TEST

void ecore_test_ecore(TCase *tc)
{
   tcase_add_test(tc, ecore_test_ecore_init);
   tcase_add_test(tc, ecore_test_ecore_main_loop);
}
