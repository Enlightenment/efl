#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "ecordova_suite.h"
#include <Ecore.h>
#include <ecore_timer.eo.h>
#include <Ecordova.h>

#include <check.h>

#define CHECK(x) {int ret = x; ck_assert_int_eq(CONTACTS_ERROR_NONE, ret);}

static int _contact_id;
static const char *_contact_name = "test";

static void
_setup(void)
{
   DBG("%s", "_setup");

   int ret = ecordova_init();
   ck_assert_int_eq(ret, 1);

   DBG("%s", "_setup");

   /* CHECK(contacts_connect()); */

   // TODO: delete all record in the database to be back to square 1 and make tests deterministic
   DBG("%s", "_setup");

   /* // create contact record */
   /* contacts_record_h contact = NULL; */
   /* CHECK(contacts_record_create(_contacts_contact._uri, &contact)); */

   /* // add name */
   /* contacts_record_h name = NULL; */
   /* CHECK(contacts_record_create(_contacts_name._uri, &name)); */
   /* CHECK(contacts_record_set_str(name, _contacts_name.first, _contact_name)); */
   /* CHECK(contacts_record_add_child_record(contact, _contacts_contact.name, name)); */

   /* CHECK(contacts_db_insert_record(contact, &_contact_id)); */

   /* CHECK(contacts_record_destroy(contact, true)); */
}

static void
_teardown(void)
{
   /* CHECK(contacts_db_delete_record(_contacts_contact._uri, _contact_id)); */

   /* CHECK(contacts_disconnect()); */

   int ret = ecordova_shutdown();
   ck_assert_int_eq(ret, 0);
}

static Eina_Bool
_find_all_success_cb(void *data,
                     Eo *obj EINA_UNUSED,
                     const Eo_Event_Description *desc EINA_UNUSED,
                     void *event_info)
{
   Eina_Bool *success = data;
   Eina_Array *contacts = event_info;
   fail_if(NULL == contacts);

   size_t count = eina_array_count(contacts);
   DBG("# of contacts: %zu", count);

   *success = EINA_TRUE;
   ecore_main_loop_quit();
   return EO_CALLBACK_CONTINUE;
}

static Eina_Bool
_find_one_contact_success_cb(void *data,
                             Eo *obj EINA_UNUSED,
                             const Eo_Event_Description *desc EINA_UNUSED,
                             void *event_info)
{
   Ecordova_Contact **contact = data;
   Eina_Array *contacts = event_info;
   fail_if(NULL == contacts);

   size_t count = eina_array_count(contacts);
   DBG("# of contacts: %zu", count);

   fail_if(1 != count);
   *contact = eina_array_data_get(contacts, 0);
   eo_ref(*contact);

   ecore_main_loop_quit();
   return EO_CALLBACK_CONTINUE;
}

static Eina_Bool
_error_cb(void *data,
          Eo *obj EINA_UNUSED,
          const Eo_Event_Description *desc EINA_UNUSED,
          void *event_info)
{
   Eina_Bool *error = data;
   Ecordova_Contacts_Error *error_code = event_info;
   fail_if(NULL == error_code);

   *error = EINA_TRUE;
   ecore_main_loop_quit();
   return EO_CALLBACK_CONTINUE;
}

static Eina_Bool
_timeout_cb(void *data)
{
   Eina_Bool *timeout = data;
   *timeout = EINA_TRUE;
   ecore_main_loop_quit();
   return ECORE_CALLBACK_CANCEL;
}

START_TEST(find_all)
{
   Ecordova_Contacts *contacts = eo_add(ECORDOVA_CONTACTS_CLASS, NULL);
   fail_if(NULL == contacts);

   Eina_Bool success = EINA_FALSE;
   Eina_Bool error = EINA_FALSE;
   Eina_Bool timeout = EINA_FALSE;

   Ecore_Timer *timer = eo_add(ECORE_TIMER_CLASS, NULL, ecore_obj_timer_constructor(10, _timeout_cb, &timeout));
   eo_do(contacts, eo_event_callback_add(ECORDOVA_CONTACTS_EVENT_FIND_SUCCESS, _find_all_success_cb, &success));
   eo_do(contacts, eo_event_callback_add(ECORDOVA_CONTACTS_EVENT_ERROR, _error_cb, &error));

   Eina_List *fields = eina_list_append(NULL, "*");
   Ecordova_Contacts_FindOptions *options = NULL;
   eo_do(contacts, ecordova_contacts_find(fields, options));

   ecore_main_loop_begin();

   eina_list_free(fields);

   eo_unref(timer);

   fail_unless(success);
   fail_if(error);
   fail_if(timeout);

   eo_unref(contacts);
}
END_TEST

START_TEST(find_by_id)
{
   Ecordova_Contacts *contacts = eo_add(ECORDOVA_CONTACTS_CLASS, NULL);
   fail_if(NULL == contacts);

   Ecordova_Contact *contact = NULL;
   Eina_Bool error = EINA_FALSE;
   Eina_Bool timeout = EINA_FALSE;

   Ecore_Timer *timer = eo_add(ECORE_TIMER_CLASS, NULL, ecore_obj_timer_constructor(10, _timeout_cb, &timeout));
   eo_do(contacts, eo_event_callback_add(ECORDOVA_CONTACTS_EVENT_FIND_SUCCESS, _find_one_contact_success_cb, &contact));
   eo_do(contacts, eo_event_callback_add(ECORDOVA_CONTACTS_EVENT_ERROR, _error_cb, &error));


   char buf[64];
   snprintf(buf, sizeof(buf), "%d", _contact_id);

   Eina_List *fields = eina_list_append(NULL, "id");
   Ecordova_Contacts_FindOptions options = {
      .filter = buf
   };
   eo_do(contacts, ecordova_contacts_find(fields, &options));

   ecore_main_loop_begin();

   eina_list_free(fields);

   eo_unref(timer);

   fail_unless(!!contact);
   fail_if(error);
   fail_if(timeout);

   int actual_id, expected_id = _contact_id;
   eo_do(contact, actual_id = ecordova_contact_id_get());
   ck_assert_int_eq(expected_id, actual_id);

   Ecordova_ContactName *contact_name = NULL;
   eo_do(contact, contact_name = ecordova_contact_name_get());
   fail_unless(!!contact_name);

   const char *actual_given_name, *expected_given_name = _contact_name;
   eo_do(contact_name, actual_given_name = ecordova_contactname_given_name_get());
   ck_assert_str_eq(expected_given_name, actual_given_name);

   eo_unref(contact);
   eo_unref(contacts);
}
END_TEST

START_TEST(find_by_name)
{
   Ecordova_Contacts *contacts = eo_add(ECORDOVA_CONTACTS_CLASS, NULL);
   fail_if(NULL == contacts);

   Ecordova_Contact *contact = NULL;
   Eina_Bool error = EINA_FALSE;
   Eina_Bool timeout = EINA_FALSE;

   Ecore_Timer *timer = eo_add(ECORE_TIMER_CLASS, NULL, ecore_obj_timer_constructor(10, _timeout_cb, &timeout));
   eo_do(contacts, eo_event_callback_add(ECORDOVA_CONTACTS_EVENT_FIND_SUCCESS, _find_one_contact_success_cb, &contact));
   eo_do(contacts, eo_event_callback_add(ECORDOVA_CONTACTS_EVENT_ERROR, _error_cb, &error));

   Eina_List *fields = eina_list_append(NULL, "name");
   Ecordova_Contacts_FindOptions options = {
      .filter = "test",
      .multiple = EINA_FALSE
   };
   eo_do(contacts, ecordova_contacts_find(fields, &options));

   ecore_main_loop_begin();

   eina_list_free(fields);

   eo_unref(timer);

   fail_unless(!!contact);
   fail_if(error);
   fail_if(timeout);

   int actual_id, expected_id = _contact_id;
   eo_do(contact, actual_id = ecordova_contact_id_get());
   ck_assert_int_eq(expected_id, actual_id);

   Ecordova_ContactName *contact_name = NULL;
   eo_do(contact, contact_name = ecordova_contact_name_get());
   fail_unless(!!contact_name);

   const char *actual_given_name, *expected_given_name = _contact_name;
   eo_do(contact_name, actual_given_name = ecordova_contactname_given_name_get());
   ck_assert_str_eq(expected_given_name, actual_given_name);

   eo_unref(contact);
   eo_unref(contacts);
}
END_TEST

void
ecordova_contacts_test(TCase *tc)
{
   tcase_add_checked_fixture(tc, _setup, _teardown);
   tcase_add_test(tc, find_all);
   tcase_add_test(tc, find_by_id);
   tcase_add_test(tc, find_by_name);
}
/* #else */
/* START_TEST(contacts_fail_load) */
/* { */
/*    ecordova_init(); */
  
/*    fprintf(stderr, "contacts_fail_load %s %s:%d\n", __func__, __FILE__, __LINE__); fflush(stderr); */
/*    Ecordova_Contacts *contacts = eo_add(ECORDOVA_CONTACTS_CLASS, NULL); */
/*    ck_assert_ptr_eq(contacts, NULL); */

/*    fprintf(stderr, "contacts_fail_load %s %s:%d\n", __func__, __FILE__, __LINE__); fflush(stderr); */
/*    Ecordova_Contact *contact = eo_add(ECORDOVA_CONTACT_CLASS, NULL); */
/*    ck_assert_ptr_eq(contact, NULL); */

/*    fprintf(stderr, "contacts_fail_load %s %s:%d\n", __func__, __FILE__, __LINE__); fflush(stderr); */
/*    Ecordova_ContactAddress *contact_address = eo_add(ECORDOVA_CONTACTADDRESS_CLASS, NULL); */
/*    ck_assert_ptr_eq(contact_address, NULL); */

/*    fprintf(stderr, "contacts_fail_load %s %s:%d\n", __func__, __FILE__, __LINE__); fflush(stderr); */
/*    Ecordova_ContactField *contact_field = eo_add(ECORDOVA_CONTACTFIELD_CLASS, NULL); */
/*    ck_assert_ptr_eq(contact_field, NULL); */

/*    fprintf(stderr, "contacts_fail_load %s %s:%d\n", __func__, __FILE__, __LINE__); fflush(stderr); */
/*    Ecordova_ContactName *contact_name = eo_add(ECORDOVA_CONTACTNAME_CLASS, NULL); */
/*    ck_assert_ptr_eq(contact_name, NULL); */

/*    fprintf(stderr, "contacts_fail_load %s %s:%d\n", __func__, __FILE__, __LINE__); fflush(stderr); */
/*    Ecordova_ContactOrganization *contact_organization = eo_add(ECORDOVA_CONTACTORGANIZATION_CLASS, NULL); */
/*    ck_assert_ptr_eq(contact_organization, NULL); */

/*    ecordova_shutdown(); */
/* } */
/* END_TEST */

/* void */
/* ecordova_contacts_test(TCase *tc) */
/* { */
/*    tcase_add_test(tc, contacts_fail_load); */
/* } */
