#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdio.h>
#include <unistd.h>

#include <Eina.h>
#include <Ecore.h>
#include <EDBus.h>

#include "edbus_suite.h"

static Eina_Bool
_quit_cb(void *data EINA_UNUSED)
{
   ecore_main_loop_quit();
   return EINA_FALSE;
}

START_TEST(edbus_test_edbus)
{
   int ret;

   ret = edbus_init();
   fail_if(ret != 1);

   ret = edbus_shutdown();
   fail_if(ret != 0);
}
END_TEST

START_TEST(edbus_test_edbus_main_loop)
{
   Ecore_Timer *timer;
   int ret;

   ecore_init();
   ret = edbus_init();
   fail_if(ret != 1);

   timer = ecore_timer_add(0.1, _quit_cb, NULL);
   fail_if(timer == NULL);

   ecore_main_loop_begin();

   ret = edbus_shutdown();
   fail_if(ret != 0);

   ecore_shutdown();

}
END_TEST

void edbus_test_edbus_init(TCase *tc)
{
   tcase_add_test(tc, edbus_test_edbus);
   tcase_add_test(tc, edbus_test_edbus_main_loop);
}
