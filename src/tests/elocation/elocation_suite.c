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
EFL_START_TEST(elocation_test_init)
{
   Eina_Bool ret;

   ret = elocation_init();
   fail_if(ret != EINA_TRUE);

   elocation_shutdown();
}
EFL_END_TEST

/* Basic address object testing. Creating and freeing the object */
EFL_START_TEST(elocation_test_address_object)
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
EFL_END_TEST

/* Basic position object testing. Creating and freeing the object */
EFL_START_TEST(elocation_test_position_object)
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
EFL_END_TEST

/* Basic testing for the various functions of the GeoCode API */
static int cb_count = 0;

static Eina_Bool
event_cb(void *data EINA_UNUSED, int ev_type EINA_UNUSED, void *event)
{
   fail_if(event == NULL);

   /* We expect 3 callbacks right now */
   if (++cb_count == 3)
     ecore_main_loop_quit();

   return ECORE_CALLBACK_DONE;
}

EFL_START_TEST(elocation_test_api_geocode)
{
   Eina_Bool ret;
   Elocation_Position *position = NULL;
   Elocation_Address *address = NULL;
   Ecore_Event_Handler *handler;

   ret = elocation_init();
   fail_if(ret != EINA_TRUE);

   handler = ecore_event_handler_add(ELOCATION_EVENT_GEOCODE, event_cb, NULL);
   fail_if(handler == NULL);

   handler = ecore_event_handler_add(ELOCATION_EVENT_REVERSEGEOCODE, event_cb, NULL);
   fail_if(handler == NULL);

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

   ecore_main_loop_begin();

   elocation_position_free(position);
   elocation_address_free(address);

   elocation_shutdown();
}
EFL_END_TEST

/* Basic testing for position API */
EFL_START_TEST(elocation_test_api_position)
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
EFL_END_TEST

/* Basic testing for address API */
EFL_START_TEST(elocation_test_api_address)
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
EFL_END_TEST

/* Basic testing for status API */
EFL_START_TEST(elocation_test_api_status)
{
   Eina_Bool ret;
   int status = 0;

   ret = elocation_init();
   fail_if(ret != EINA_TRUE);

   ret = elocation_status_get(&status);
   fail_if(ret != EINA_TRUE);

   elocation_shutdown();
}
EFL_END_TEST

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
/*
   tc = tcase_create("Elocation_API_Status");
   tcase_add_test(tc, elocation_test_api_status);
   suite_add_tcase(s, tc);
*/
   return s;
}

int
main(void)
{
   Suite *s;
   SRunner *sr;
   int failed_count;

#ifdef NEED_RUN_IN_TREE
   putenv("EFL_RUN_IN_TREE=1");
#endif

   s = elocation_suite();
   sr = srunner_create(s);
   srunner_set_xml(sr, TESTS_BUILD_DIR "/check-results.xml");
   srunner_run_all(sr, CK_ENV);
   failed_count = srunner_ntests_failed(sr);
   srunner_free(sr);

   return (failed_count == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
