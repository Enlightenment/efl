#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <unistd.h>
#include <stdio.h>

#include <Eina.h>
#include <Edje.h>

#include "edje_suite.h"
#include "edje_tests_helpers.h"

START_TEST(edje_test_edje_init)
{
   int ret;

   ret = edje_init();
   fail_if(ret != 1);

   ret = edje_shutdown();
   fail_if(ret != 0);
}
END_TEST

START_TEST(edje_test_edje_load)
{
   Evas *evas = EDJE_TEST_INIT_EVAS();
   Edje_Load_Error error;
   Evas_Object *obj;

   obj = edje_object_add(evas);
   edje_object_file_set(obj, "blaoeuaeoueoaua.edj", "test");
   error = edje_object_load_error_get(obj);
   fail_if(error != EDJE_LOAD_ERROR_DOES_NOT_EXIST);

   EDJE_TEST_FREE_EVAS();
}
END_TEST

static const char *
test_layout_get(void)
{
   static int is_local = -1;
   if (is_local == -1)
     {
        struct stat st;
        is_local = (stat(PACKAGE_BUILD_DIR"/src/tests/edje/data/test_layout.edj", &st) == 0);
     }

   if (is_local)
     return PACKAGE_BUILD_DIR"/src/tests/edje/data/test_layout.edj";
   else
     return PACKAGE_DATA_DIR"/data/test_layout.edj";
}

START_TEST(edje_test_load_simple_layout)
{
   Evas *evas = EDJE_TEST_INIT_EVAS();
   Evas_Object *obj;

   obj = edje_object_add(evas);
   fail_unless(edje_object_file_set(obj, test_layout_get(), "test_group"));

   fail_if(edje_object_part_exists(obj, "unexistant_part"));
   fail_unless(edje_object_part_exists(obj, "background"));


   EDJE_TEST_FREE_EVAS();
}
END_TEST

void edje_test_edje(TCase *tc)
{    
   tcase_add_test(tc, edje_test_edje_init);
   tcase_add_test(tc,edje_test_load_simple_layout);
   tcase_add_test(tc, edje_test_edje_load);
}
