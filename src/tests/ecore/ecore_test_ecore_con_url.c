#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "ecore_suite.h"

#include <stdio.h>
#include <Ecore_Con.h>


START_TEST(ecore_test_ecore_con_url_create)
{
   Ecore_Con_Url *url;
   int ret;

   ret = eina_init();
   fail_if(ret != 1);
   ret = ecore_con_url_init();
   fail_if(ret != 1);

   url = ecore_con_url_new("http://google.com");
   fail_if(!url);

   ecore_con_url_free(url);

   ret = ecore_con_url_shutdown();
   fail_if(ret != 0);
   ret = eina_shutdown();
}
END_TEST

START_TEST(ecore_test_ecore_con_url_init)
{
   int ret;

   ret = ecore_con_url_init();
   fail_if(ret != 1);

   ret = ecore_con_url_shutdown();
   fail_if(ret != 0);
}
END_TEST

void ecore_test_ecore_con_url(TCase *tc)
{
   tcase_add_test(tc, ecore_test_ecore_con_url_init);
   tcase_add_test(tc, ecore_test_ecore_con_url_create);
}
