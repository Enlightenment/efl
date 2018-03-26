#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <Eina.h>

#include "ecore_suite.h"

/* FIXME: Currently disable these tests. They are useless ATM and they just
 * make buildbot complain. Once we add useful tests here we'll also bother
 * with getting X on the server. */
#undef HAVE_ECORE_X_XLIB

#ifdef HAVE_ECORE_X_XLIB

#include <Ecore_X.h>

EFL_START_TEST(ecore_test_ecore_x_init)
{
   int ret;

   ret = ecore_x_init(NULL);
   fail_if(ret != 1);

   ret = ecore_x_shutdown();
   fail_if(ret != 0);
}
EFL_END_TEST

EFL_START_TEST(ecore_test_ecore_x_bell)
{
   int i;
   int ret;

   ret = ecore_x_init(NULL);
   fail_if(ret != 1);

   printf("You should hear 3 beeps now.\n");
   for (i = 0; i < 3; i++)
     {
	ret = ecore_x_bell(0);
	fail_if(ret != EINA_TRUE);
	ecore_x_sync();
	sleep(1);
     }

   ecore_x_shutdown();
}
EFL_END_TEST

#endif

void ecore_test_ecore_x(TCase *tc EINA_UNUSED)
{
#ifdef HAVE_ECORE_X_XLIB
   tcase_add_test(tc, ecore_test_ecore_x_init);
   tcase_add_test(tc, ecore_test_ecore_x_bell);
#endif
}
