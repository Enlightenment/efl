#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "ecordova_file_tests.h"
#include "ecordova_suite.h"

#include <Eio.h>

#include <stdbool.h>
#include <stdlib.h>

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

static Ecordova_FileReader *
_filereader_new(void)
{
   return eo_add(ECORDOVA_FILEREADER_CLASS, NULL);
}

START_TEST(smoke)
{
   Ecordova_File *filereader = _filereader_new();
   eo_unref(filereader);
}
END_TEST

static Eina_Bool
_main_loop_quit(void *data EINA_UNUSED,
                Eo *obj EINA_UNUSED,
                const Eo_Event_Description *desc EINA_UNUSED,
                void *event_info EINA_UNUSED)
{
   ecore_main_loop_quit();
   return EO_CALLBACK_CONTINUE;
}

static Eina_Bool
_bool_set(void *data,
          Eo *obj EINA_UNUSED,
          const Eo_Event_Description *desc EINA_UNUSED,
          void *event_info)
{
   Ecordova_ProgressEvent *progress = event_info;
   fail_if(NULL == progress);

   bool *value = data;
   *value = true;
   ecore_main_loop_quit();
   return EO_CALLBACK_CONTINUE;
}

bool
filereader_read(Ecordova_File *file, char **content, size_t *length)
{
   *content = NULL;
   *length = 0;
   bool error = false;
   bool timeout = false;

   Ecordova_File *filereader = _filereader_new();
   Ecore_Timer *timeout_timer = eo_add(ECORE_TIMER_CLASS, NULL, ecore_obj_timer_constructor(10, _timeout_cb, &timeout));

   eo_do(filereader, eo_event_callback_add(ECORDOVA_FILEREADER_EVENT_ON_LOAD_END, _main_loop_quit, NULL),
                     eo_event_callback_add(ECORDOVA_FILEREADER_EVENT_ON_ERROR, _bool_set, &error),
                     ecordova_filereader_read(file));

   ecore_main_loop_begin();

   eo_do(filereader, eo_event_callback_del(ECORDOVA_FILEREADER_EVENT_ON_LOAD_END, _main_loop_quit, NULL),
                     eo_event_callback_del(ECORDOVA_FILEREADER_EVENT_ON_ERROR, _bool_set, &error));

   eo_unref(timeout_timer);

   if (!error && !timeout)
     {
        *length = eo_do_ret(filereader, *length, ecordova_filereader_length_get());
        if (*length)
          {
             const char *result = eo_do_ret(filereader, result, ecordova_filereader_result_get());
             *content = malloc(*length);
             memcpy(*content, result, *length);
          }
     }

   eo_unref(filereader);

   fail_if(timeout);

   return error;
}

START_TEST(read_all)
{
   bool error = false;

   Eina_Tmpstr *file_url = NULL;
   unsigned char *expected_content = NULL;
   size_t expected_size = 0;
   Ecordova_FileEntry *file_entry = create_tmpfile(&file_url,
                                                   &expected_content,
                                                   &expected_size);

   Ecordova_File *file = NULL;
   fail_if(error = fileentry_file_get(file_entry, &file));

   char *actual_content;
   size_t actual_size;
   fail_if(error = filereader_read(file, &actual_content, &actual_size));
   ck_assert_int_eq(expected_size, actual_size);
   ck_assert_int_eq(0, memcmp(expected_content, actual_content, expected_size));
   free(actual_content);

   eo_unref(file);

   fail_if(error = entry_remove(file_entry));
   eo_unref(file_entry);
   free(expected_content);
   eina_tmpstr_del(file_url);
}
END_TEST

START_TEST(read_slice)
{
   bool error = false;

   Eina_Tmpstr *file_url = NULL;
   unsigned char *file_content = NULL;
   size_t file_size = 1024;
   Ecordova_FileEntry *file_entry = create_tmpfile_size(&file_url,
                                                        &file_content,
                                                        file_size);

   Ecordova_File *file = NULL;
   fail_if(error = fileentry_file_get(file_entry, &file));

   // take the middle
   size_t start = file_size / 3;
   size_t end = file_size / 3 * 2;
   Ecordova_File *sliced_file = eo_do_ret(file,
                                          sliced_file,
                                          ecordova_file_slice(start, end));
   fail_if(NULL == sliced_file);

   size_t expected_size = end - start;
   unsigned char *expected_content = &file_content[start];

   char *actual_content;
   size_t actual_size;
   fail_if(error = filereader_read(sliced_file, &actual_content, &actual_size));
   ck_assert_int_eq(expected_size, actual_size);
   ck_assert_int_eq(0, memcmp(expected_content, actual_content, expected_size));
   free(actual_content);


   eo_unref(sliced_file);
   eo_unref(file);

   fail_if(error = entry_remove(file_entry));
   eo_unref(file_entry);
   free(file_content);
   eina_tmpstr_del(file_url);
}
END_TEST

START_TEST(read_slice_page_size)
{
   bool error = false;

   Eina_Tmpstr *file_url = NULL;
   unsigned char *file_content = NULL;
   const size_t file_size = eina_cpu_page_size() * 5;
   Ecordova_FileEntry *file_entry = create_tmpfile_size(&file_url,
                                                        &file_content,
                                                        file_size);

   Ecordova_File *file = NULL;
   fail_if(error = fileentry_file_get(file_entry, &file));

   // take the middle
   size_t start = file_size / 3;
   size_t end = file_size / 3 * 2;
   Ecordova_File *sliced_file = eo_do_ret(file,
                                          sliced_file,
                                          ecordova_file_slice(start, end));
   fail_if(NULL == sliced_file);

   size_t expected_size = end - start;
   unsigned char *expected_content = &file_content[start];

   char *actual_content;
   size_t actual_size;
   fail_if(error = filereader_read(sliced_file, &actual_content, &actual_size));
   ck_assert_int_eq(expected_size, actual_size);
   ck_assert_int_eq(0, memcmp(expected_content, actual_content, expected_size));
   free(actual_content);

   eo_unref(sliced_file);
   eo_unref(file);

   fail_if(error = entry_remove(file_entry));
   eo_unref(file_entry);
   free(file_content);
   eina_tmpstr_del(file_url);
}
END_TEST

static Eina_Bool
_filereader_abort(void *data,
                  Eo *obj EINA_UNUSED,
                  const Eo_Event_Description *desc EINA_UNUSED,
                  void *event_info EINA_UNUSED)
{
   Ecordova_File *filereader = data;
   fail_if(NULL == filereader);
   eo_do(filereader, ecordova_filereader_abort());
   return EO_CALLBACK_CONTINUE;
}

START_TEST(aborting)
{
   Eina_Tmpstr *file_url = NULL;
   unsigned char *file_content = NULL;
   size_t file_size = 0;
   Ecordova_FileEntry *file_entry = create_tmpfile(&file_url,
                                                   &file_content,
                                                   &file_size);

   bool error = false;
   bool timeout = false;
   bool aborted = false;

   Ecordova_File *file = NULL;
   fail_if(error = fileentry_file_get(file_entry, &file));

   Ecordova_File *filereader = _filereader_new();
   Ecore_Timer *timeout_timer = eo_add(ECORE_TIMER_CLASS, NULL, ecore_obj_timer_constructor(10, _timeout_cb, &timeout));

   eo_do(filereader, eo_event_callback_add(ECORDOVA_FILEREADER_EVENT_ON_LOAD_END, _main_loop_quit, NULL),
                     eo_event_callback_add(ECORDOVA_FILEREADER_EVENT_ON_PROGRESS, _filereader_abort, filereader),
                     eo_event_callback_add(ECORDOVA_FILEREADER_EVENT_ON_ABORT, _bool_set, &aborted),
                     eo_event_callback_add(ECORDOVA_FILEREADER_EVENT_ON_ERROR, _bool_set, &error),
                     ecordova_filereader_read(file));

   ecore_main_loop_begin();

   eo_do(filereader, eo_event_callback_del(ECORDOVA_FILEREADER_EVENT_ON_LOAD_END, _main_loop_quit, NULL),
                     eo_event_callback_del(ECORDOVA_FILEREADER_EVENT_ON_PROGRESS, _filereader_abort, filereader),
                     eo_event_callback_del(ECORDOVA_FILEREADER_EVENT_ON_ABORT, _bool_set, &aborted),
                     eo_event_callback_del(ECORDOVA_FILEREADER_EVENT_ON_ERROR, _bool_set, &error));

   eo_unref(timeout_timer);
   eo_unref(filereader);

   fail_if(timeout);
   fail_if(error);
   fail_unless(aborted);

   eo_unref(file);

   fail_if(error = entry_remove(file_entry));
   eo_unref(file_entry);
   free(file_content);
   eina_tmpstr_del(file_url);
}
END_TEST

void
ecordova_filereader_test(TCase *tc)
{
   tcase_add_checked_fixture(tc, _setup, _teardown);
   tcase_add_test(tc, smoke);
   tcase_add_test(tc, read_all);
   tcase_add_test(tc, read_slice);
   tcase_add_test(tc, read_slice_page_size);
   tcase_add_test(tc, aborting);
}
