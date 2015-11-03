#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <Ecordova.h>

#include "ecordova_suite.h"

static void
_setup(void)
{
   int ret = ecordova_init();
   ck_assert_int_eq(ret, 1);
}

static void
_teardown(void)
{
   int ret = ecordova_shutdown();
   ck_assert_int_eq(ret, 0);
}


static Ecordova_DirectoryReader *
_directoryreader_new(const char *path)
{
   return eo_add(ECORDOVA_DIRECTORYREADER_CLASS,
                 NULL,
                 ecordova_directoryreader_constructor(path));
}

START_TEST(smoke)
{
   Ecordova_DirectoryReader *directory_reader = _directoryreader_new(TESTS_BUILD_DIR);
   eo_unref(directory_reader);
}
END_TEST

static Eina_Bool
_timeout_cb(void *data)
{
   Eina_Bool *timeout = data;
   *timeout = EINA_TRUE;
   fprintf(stderr, "_timeout_cb\n"); fflush(stderr);
   ecore_main_loop_quit();
   return ECORE_CALLBACK_CANCEL;
}

static Eina_Bool
_success_cb(void *data,
            Eo *obj EINA_UNUSED,
            const Eo_Event_Description *desc EINA_UNUSED,
            void *event_info)
{
   fprintf(stderr, "%s:%s:%d\n", __func__, __FILE__, __LINE__); fflush(stderr);
   Eina_Bool *success = data;
   Eina_List *entries = event_info;
   fail_if(NULL == entries);

   *success = EINA_TRUE;
   fprintf(stderr, "%s:%s:%d\n", __func__, __FILE__, __LINE__); fflush(stderr);
   ecore_main_loop_quit();
   return EO_CALLBACK_CONTINUE;
}

static Eina_Bool
_error_cb(void *data,
          Eo *obj EINA_UNUSED,
          const Eo_Event_Description *desc EINA_UNUSED,
          void *event_info)
{
   fprintf(stderr, "%s:%s:%d\n", __func__, __FILE__, __LINE__); fflush(stderr);
   Eina_Bool *error = data;
   Ecordova_FileError *error_code = event_info;
   fail_if(NULL == error_code);

   *error = EINA_TRUE;
   fprintf(stderr, "%s:%s:%d\n", __func__, __FILE__, __LINE__); fflush(stderr);
   ecore_main_loop_quit();
   return EO_CALLBACK_CONTINUE;
}

START_TEST(read_entries_success)
{
   Ecordova_DirectoryReader *directory_reader = _directoryreader_new(TESTS_BUILD_DIR);

   Eina_Bool success = EINA_FALSE;
   Eina_Bool error = EINA_FALSE;
   Eina_Bool timeout = EINA_FALSE;

   /* Ecore_Timer *timer = eo_add(ECORE_TIMER_CLASS, NULL, ecore_obj_timer_constructor(2, _timeout_cb, &timeout)); */
   fprintf(stderr, "Starting test\n"); fflush(stderr);
   eo_do(directory_reader, eo_event_callback_add(ECORDOVA_DIRECTORYREADER_EVENT_SUCCESS, _success_cb, &success));
   eo_do(directory_reader, eo_event_callback_add(ECORDOVA_DIRECTORYREADER_EVENT_ERROR, _error_cb, &error));
   eo_do(directory_reader, ecordova_directoryreader_entries_read());

   fprintf(stderr, "Going to event loop\n"); fflush(stderr);
   if(!error && !timeout)
     ecore_main_loop_begin();
   fprintf(stderr, "Out of event loop\n"); fflush(stderr);

   fprintf(stderr, "%s:%s:%d\n", __func__, __FILE__, __LINE__); fflush(stderr);
   /* eo_unref(timer); */
   fprintf(stderr, "%s:%s:%d\n", __func__, __FILE__, __LINE__); fflush(stderr);
   fail_if(error);
   fprintf(stderr, "%s:%s:%d\n", __func__, __FILE__, __LINE__); fflush(stderr);
   /* fail_if(timeout); */
   fprintf(stderr, "%s:%s:%d\n", __func__, __FILE__, __LINE__); fflush(stderr);
   fail_unless(success);

   fprintf(stderr, "%s:%s:%d\n", __func__, __FILE__, __LINE__); fflush(stderr);
   eo_unref(directory_reader);
   fprintf(stderr, "Out of test\n"); fflush(stderr);
}
END_TEST

START_TEST(read_entries_error)
{
   Ecordova_DirectoryReader *directory_reader = _directoryreader_new("/**??this_directory_doesn't_exist??**");

   Eina_Bool success = EINA_FALSE;
   Eina_Bool error = EINA_FALSE;
   Eina_Bool timeout = EINA_FALSE;

   Ecore_Timer *timer = eo_add(ECORE_TIMER_CLASS, NULL, ecore_obj_timer_constructor(10, _timeout_cb, &timeout));
   eo_do(directory_reader, eo_event_callback_add(ECORDOVA_DIRECTORYREADER_EVENT_SUCCESS, _success_cb, &success));
   eo_do(directory_reader, eo_event_callback_add(ECORDOVA_DIRECTORYREADER_EVENT_ERROR, _error_cb, &error));
   eo_do(directory_reader, ecordova_directoryreader_entries_read());
   ecore_main_loop_begin();

   eo_unref(timer);
   fail_if(success);
   fail_if(timeout);
   fail_unless(error);

   eo_unref(directory_reader);
}
END_TEST

void
ecordova_directoryreader_test(TCase *tc)
{
   tcase_add_checked_fixture(tc, _setup, _teardown);
   tcase_add_test(tc, smoke);
   tcase_add_test(tc, read_entries_success);
   tcase_add_test(tc, read_entries_error);
}
