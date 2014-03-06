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
test_layout_get(const char *name)
{
   static char filename[PATH_MAX];

   snprintf(filename, PATH_MAX, TESTS_BUILD_DIR"/data/%s", name);

   static int is_local = -1;
   if (is_local == -1)
     {
        struct stat st;
        is_local = (stat(filename, &st) == 0);
     }

   if (!is_local)
     snprintf(filename, PATH_MAX, PACKAGE_DATA_DIR"/data/%s", name);

   return filename;
}

START_TEST(edje_test_load_simple_layout)
{
   Evas *evas = EDJE_TEST_INIT_EVAS();
   Evas_Object *obj;

   obj = edje_object_add(evas);
   fail_unless(edje_object_file_set(obj, test_layout_get("test_layout.edj"), "test_group"));

   fail_if(edje_object_part_exists(obj, "unexistant_part"));
   fail_unless(edje_object_part_exists(obj, "background"));


   EDJE_TEST_FREE_EVAS();
}
END_TEST

START_TEST(edje_test_simple_layout_geometry)
{
   int x, y, w, h;
   int r, g, b, a;
   Evas *evas = EDJE_TEST_INIT_EVAS();
   Evas_Object *obj;
   const Evas_Object *bg;

   obj = edje_object_add(evas);
   fail_unless(edje_object_file_set(obj, test_layout_get("test_layout.edj"), "test_group"));

   /* rel1.relative: 0.0 0.0;
    * rel2.relative: 1.0 1.0; */
   evas_object_resize(obj, 1000, 1000);
   edje_object_part_geometry_get(obj, "background", &x, &y, &w, &h);

   fail_if(x != 0 || y != 0);
   fail_if(w != 1000 || h != 1000);

   bg = edje_object_part_object_get(obj, "background");
   fail_if(!bg);

   evas_object_color_get(bg, &r, &g, &b, &a);
   fail_if(r != 255 || g != 255 || b != 255 || a != 255);

   EDJE_TEST_FREE_EVAS();
}
END_TEST

START_TEST(edje_test_complex_layout)
{
   int x, y, w, h;
   Evas *evas = EDJE_TEST_INIT_EVAS();
   Evas_Object *obj;

   obj = edje_object_add(evas);
   fail_unless(edje_object_file_set(obj, test_layout_get("complex_layout.edj"), "test_group"));
   evas_object_resize(obj, 1000, 1000);

   /* Apparently rel2 offsets are retarded and you have to introduce off-by-one
    * madness (rel2.offset: -1 -1; acts like there is zero offset...)
    * Take that into account when checking w and h */

   /* rel1.relative: 0.0 0.0; rel1.offset: 0 0;
    * rel2.relative: 1.0 1.0; rel2.offset -1 -1; */
   edje_object_part_geometry_get(obj, "background", &x, &y, &w, &h);
   fail_if(x != 0 || y != 0);
   fail_if(w != 1000-1 + 1 || h != 1000-1 + 1);

   /* rel1.relative: 0.0 0.0; rel1.offset: 1 1;
    * rel2.relative: 1.0 1.0; rel2.offset 0 0; */
   edje_object_part_geometry_get(obj, "background2", &x, &y, &w, &h);
   fail_if(x != 1 || y != 1);
   fail_if(w != 1000-1 + 1 || h != 1000-1 + 1);

   /* rel1.relative: 0.0 0.0; rel1.offset: 5 5;
    * rel2.relative: 0.5 0.5; rel2.offset -2 -2; */
   edje_object_part_geometry_get(obj, "ul", &x, &y, &w, &h);
   fail_if(x != 5 || y != 5);
   fail_if(w != 500-5-2 + 1 || h != 500-5-2 + 1);

   /* rel1.relative: 0.5 0.0; rel1.offset: 2 5;
    * rel2.relative: 1.0 0.5; rel2.offset -5 -2; */
   edje_object_part_geometry_get(obj, "ur", &x, &y, &w, &h);
   fail_if(x != 500+2 || y != 5);
   fail_if(w != 500-5-2 + 1 || h != 500-5-2 + 1);

   /* rel1.relative: 0.0 0.5; rel1.offset: 5 2;
    * rel2.relative: 1.0 1.0; rel2.offset -5 -5; */
   edje_object_part_geometry_get(obj, "l", &x, &y, &w, &h);
   fail_if(x != 5 || y != 500+2);
   fail_if(w != 1000-5-5 + 1 || h != 500-5-2 + 1);

   EDJE_TEST_FREE_EVAS();
}
END_TEST

void edje_test_edje(TCase *tc)
{    
   tcase_add_test(tc, edje_test_edje_init);
   tcase_add_test(tc,edje_test_load_simple_layout);
   tcase_add_test(tc, edje_test_edje_load);
   tcase_add_test(tc, edje_test_simple_layout_geometry);
   tcase_add_test(tc, edje_test_complex_layout);
}
