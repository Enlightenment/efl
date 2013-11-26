#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <stdio.h>

#include "evas_suite.h"
#include "Evas.h"
#include "evas_tests_helpers.h"

START_TEST(evas_object_various)
{
   Evas *evas = EVAS_TEST_INIT_EVAS();

   Evas_Object *obj = evas_object_rectangle_add(evas);

   evas_object_ref(obj);
   evas_free(evas);
   evas_object_unref(obj);

   evas = EVAS_TEST_INIT_EVAS();
   eo_ref(obj);
   evas_free(evas);
   eo_unref(obj);


   /* Twice because EVAS_TEST_INIT_EVAS inits it twice. */
   evas_shutdown();
   evas_shutdown();
}
END_TEST

void evas_test_object(TCase *tc)
{
   tcase_add_test(tc, evas_object_various);
}
