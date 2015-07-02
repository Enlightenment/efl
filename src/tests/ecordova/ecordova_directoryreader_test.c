#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "ecordova_directoryreader_test.h"
#include "ecordova_suite.h"

#include <stdbool.h>

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
   bool *timeout = data;
   *timeout = true;
   ecore_main_loop_quit();
   return ECORE_CALLBACK_CANCEL;
}

static Eina_Bool
_success_cb(void *data,
            Eo *obj EINA_UNUSED,
            const Eo_Event_Description *desc EINA_UNUSED,
            void *event_info)
{
   bool *success = data;
   Eina_List *entries = event_info;
   fail_if(NULL == entries);

   *success = true;
   ecore_main_loop_quit();
   return EO_CALLBACK_CONTINUE;
}

static Eina_Bool
_error_cb(void *data,
          Eo *obj EINA_UNUSED,
          const Eo_Event_Description *desc EINA_UNUSED,
          void *event_info)
{
   bool *error = data;
   Ecordova_FileError *error_code = event_info;
   fail_if(NULL == error_code);

   *error = true;
   ecore_main_loop_quit();
   return EO_CALLBACK_CONTINUE;
}

START_TEST(read_entries_success)
{
   Ecordova_DirectoryReader *directory_reader = _directoryreader_new(TESTS_BUILD_DIR);

   bool success = false;
   bool error = false;
   bool timeout = false;

   Ecore_Timer *timer = eo_add(ECORE_TIMER_CLASS, NULL, ecore_obj_timer_constructor(10, _timeout_cb, &timeout));
   eo_do(directory_reader, eo_event_callback_add(ECORDOVA_DIRECTORYREADER_EVENT_SUCCESS, _success_cb, &success));
   eo_do(directory_reader, eo_event_callback_add(ECORDOVA_DIRECTORYREADER_EVENT_ERROR, _error_cb, &error));
   eo_do(directory_reader, ecordova_directoryreader_entries_read());
   ecore_main_loop_begin();

   eo_unref(timer);
   fail_if(error);
   fail_if(timeout);
   fail_unless(success);

   eo_unref(directory_reader);
}
END_TEST

START_TEST(read_entries_error)
{
   Ecordova_DirectoryReader *directory_reader = _directoryreader_new("/**??this_directory_doesn't_exist??**");

   bool success = false;
   bool error = false;
   bool timeout = false;

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
