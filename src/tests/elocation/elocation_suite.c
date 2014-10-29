#ifdef HAVE_CONFIG_H
# include <config.h>
#endif /* ifdef HAVE_CONFIG_H */

#include <stdlib.h>
#include <stdio.h>

#include <check.h>

#include <Elocation.h>

/* Test the init and shutdown pathes only. Good to do that as we don't set up
 * other things and already triggered problems with this.
 */
START_TEST(elocation_test_init)
{
   Eina_Bool ret;

   ret = elocation_init();
   fail_if(ret != EINA_TRUE);

   elocation_shutdown();
}
END_TEST

/* Basic address object testing. Creating and freeing the object */
START_TEST(elocation_test_address_object)
{
   Eina_Bool ret;
   Elocation_Address *address = NULL;

   ret = elocation_init();
   fail_if(ret != EINA_TRUE);

   address = elocation_address_new();
   fail_if(address == NULL);

   elocation_address_free(address);

   elocation_shutdown();
}
END_TEST

/* Basic position object testing. Creating and freeing the object */
START_TEST(elocation_test_position_object)
{
   Eina_Bool ret;
   Elocation_Position *position = NULL;

   ret = elocation_init();
   fail_if(ret != EINA_TRUE);

   position = elocation_position_new();
   fail_if(position == NULL);

   elocation_position_free(position);

   elocation_shutdown();
}
END_TEST

/* Basic testing for the various functions of the GeoCode API */
START_TEST(elocation_test_api_geocode)
{
   Eina_Bool ret;
   Elocation_Position *position = NULL;
   Elocation_Address *address = NULL;

   ret = elocation_init();
   fail_if(ret != EINA_TRUE);

   position = elocation_position_new();
   fail_if(position == NULL);

   address = elocation_address_new();
   fail_if(address == NULL);

   ret = elocation_freeform_address_to_position("London", position);
   fail_if(ret != EINA_TRUE);

   position->latitude = 51.7522;
   position->longitude = -1.25596;
   position->accur->level = 3;
   ret = elocation_position_to_address(position, address);
   fail_if(ret != EINA_TRUE);

   address->locality = strdup("Cambridge");
   address->countrycode = strdup("UK");
   ret = elocation_address_to_position(address, position);
   fail_if(ret != EINA_TRUE);

   elocation_position_free(position);
   elocation_address_free(address);

   elocation_shutdown();
}
END_TEST

/* Basic testing for position API */
START_TEST(elocation_test_api_position)
{
   Eina_Bool ret;
   Elocation_Position *position = NULL;

   ret = elocation_init();
   fail_if(ret != EINA_TRUE);

   position = elocation_position_new();
   fail_if(position == NULL);

   ret = elocation_position_get(position);
   fail_if(ret != EINA_TRUE);

   elocation_position_free(position);

   elocation_shutdown();
}
END_TEST

/* Basic testing for address API */
START_TEST(elocation_test_api_address)
{
   Eina_Bool ret;
   Elocation_Address *address = NULL;

   ret = elocation_init();
   fail_if(ret != EINA_TRUE);

   address = elocation_address_new();
   fail_if(address == NULL);

   ret = elocation_address_get(address);
   fail_if(ret != EINA_TRUE);

   elocation_address_free(address);

   elocation_shutdown();
}
END_TEST

/* Basic testing for status API */
START_TEST(elocation_test_api_status)
{
   Eina_Bool ret;
   int status = 0;

   ret = elocation_init();
   fail_if(ret != EINA_TRUE);

   ret = elocation_status_get(&status);
   fail_if(ret != EINA_TRUE);

   elocation_shutdown();
}
END_TEST

Suite *
elocation_suite(void)
{
   Suite *s;
   TCase *tc;

   s = suite_create("Elocation");

   tc = tcase_create("Elocation_Init");
   tcase_add_test(tc, elocation_test_init);
   suite_add_tcase(s, tc);

   tc = tcase_create("Elocation_Objects");
   tcase_add_test(tc, elocation_test_address_object);
   tcase_add_test(tc, elocation_test_position_object);
   suite_add_tcase(s, tc);

   tc = tcase_create("Elocation_API_Geocode");
   tcase_add_test(tc, elocation_test_api_geocode);
   suite_add_tcase(s, tc);

   tc = tcase_create("Elocation_API_Position");
   tcase_add_test(tc, elocation_test_api_position);
   suite_add_tcase(s, tc);

   tc = tcase_create("Elocation_API_Address");
   tcase_add_test(tc, elocation_test_api_address);
   suite_add_tcase(s, tc);

   tc = tcase_create("Elocation_API_Status");
   tcase_add_test(tc, elocation_test_api_status);
   suite_add_tcase(s, tc);

   return s;
}

int
main(void)
{
   Suite *s;
   SRunner *sr;
   int failed_count;

   s = elocation_suite();
   sr = srunner_create(s);
   srunner_run_all(sr, CK_ENV);
   failed_count = srunner_ntests_failed(sr);
   srunner_free(sr);

   return (failed_count == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
