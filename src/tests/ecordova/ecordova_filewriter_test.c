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

static Ecordova_FileWriter *
_filewriter_new(Ecordova_File *file)
{
   return eo_add(ECORDOVA_FILEWRITER_CLASS, NULL, ecordova_filewriter_file_set(file));
}

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

static bool
_filewriter_write(Ecordova_File *file, const char *content, size_t length)
{
   bool error = false;
   bool timeout = false;

   Ecordova_File *filewriter = _filewriter_new(file);
   Ecore_Timer *timeout_timer = eo_add(ECORE_TIMER_CLASS, NULL, ecore_obj_timer_constructor(10, _timeout_cb, &timeout));

   eo_do(filewriter, eo_event_callback_add(ECORDOVA_FILEWRITER_EVENT_ON_WRITE_END, _main_loop_quit, NULL),
                     eo_event_callback_add(ECORDOVA_FILEWRITER_EVENT_ON_ERROR, _bool_set, &error),
                     ecordova_filewriter_write(content, length));

   ecore_main_loop_begin();

   eo_do(filewriter, eo_event_callback_del(ECORDOVA_FILEWRITER_EVENT_ON_WRITE_END, _main_loop_quit, NULL),
                     eo_event_callback_del(ECORDOVA_ENTRY_EVENT_ERROR, _bool_set, &error));

   eo_unref(timeout_timer);

   eo_unref(filewriter);

   fail_if(timeout);

   return error;
}

START_TEST(writer_all)
{
   Eina_Tmpstr *tmpdir = NULL;
   Ecordova_DirectoryEntry *directory_entry = _create_tmpdir(&tmpdir);

   const char *filename = "file.txt";
   const char *filepath = tmpdir;
   size_t len = strlen(filepath) + 1 + strlen(filename) + 1;
   char file_url[len];
   snprintf(file_url, len, "%s/%s", tmpdir, filename);

   Ecordova_FileEntry *file_entry = NULL;
   bool error = false;

   // create exclusive
   Ecordova_FileFlags flags = ECORDOVA_FILEFLAGS_CREATE | ECORDOVA_FILEFLAGS_EXCLUSIVE;
   error = fileentry_get(directory_entry, filename, flags, &file_entry);
   fail_if(error);
   fail_unless(NULL != file_entry);
   check_exists(file_url);

   Ecordova_File *file = NULL;
   fail_if(error = fileentry_file_get(file_entry, &file));
   size_t expected_size;
   unsigned char *expected_content = generate_random_buffer(&expected_size);
   fail_if(error = _filewriter_write(file, (char*)expected_content, expected_size));
   eo_unref(file);

   fail_if(error = fileentry_file_get(file_entry, &file));
   char *actual_content;
   size_t actual_size;
   fail_if(error = filereader_read(file, &actual_content, &actual_size));
   ck_assert_int_eq(expected_size, actual_size);
   ck_assert_int_eq(0, memcmp(expected_content, actual_content, expected_size));
   free(actual_content);
   eo_unref(file);

   free(expected_content);

   fail_if(error = entry_remove(file_entry));
   eo_unref(file_entry);
   fail_if(error = entry_remove(directory_entry));
   eo_unref(directory_entry);
   eina_tmpstr_del(tmpdir);
}
END_TEST

static Eina_Bool
_filewriter_abort(void *data,
                  Eo *obj EINA_UNUSED,
                  const Eo_Event_Description *desc EINA_UNUSED,
                  void *event_info EINA_UNUSED)
{
   Ecordova_File *filewriter = data;
   fail_if(NULL == filewriter);
   eo_do(filewriter, ecordova_filewriter_abort());
   return EO_CALLBACK_CONTINUE;
}

START_TEST(aborting)
{
   Eina_Tmpstr *tmpdir = NULL;
   Ecordova_DirectoryEntry *directory_entry = _create_tmpdir(&tmpdir);

   const char *filename = "file.txt";
   const char *filepath = tmpdir;
   size_t len = strlen(filepath) + 1 + strlen(filename) + 1;
   char file_url[len];
   snprintf(file_url, len, "%s/%s", tmpdir, filename);

   Ecordova_FileEntry *file_entry = NULL;
   bool error = false;
   bool timeout = false;
   bool aborted = false;

   // create exclusive
   Ecordova_FileFlags flags = ECORDOVA_FILEFLAGS_CREATE | ECORDOVA_FILEFLAGS_EXCLUSIVE;
   error = fileentry_get(directory_entry, filename, flags, &file_entry);
   fail_if(error);
   fail_unless(NULL != file_entry);
   check_exists(file_url);

   Ecordova_File *file = NULL;
   fail_if(error = fileentry_file_get(file_entry, &file));
   size_t size;
   unsigned char *content = generate_random_buffer(&size);

   Ecordova_File *filewriter = _filewriter_new(file);
   Ecore_Timer *timeout_timer = eo_add(ECORE_TIMER_CLASS, NULL, ecore_obj_timer_constructor(10, _timeout_cb, &timeout));

   eo_do(filewriter, eo_event_callback_add(ECORDOVA_FILEWRITER_EVENT_ON_WRITE_END, _main_loop_quit, NULL),
                     eo_event_callback_add(ECORDOVA_FILEWRITER_EVENT_ON_PROGRESS, _filewriter_abort, filewriter),
                     eo_event_callback_add(ECORDOVA_FILEWRITER_EVENT_ON_ABORT, _bool_set, &aborted),
                     eo_event_callback_add(ECORDOVA_FILEWRITER_EVENT_ON_ERROR, _bool_set, &error),
                     ecordova_filewriter_write((char*)content, size));

   ecore_main_loop_begin();

   eo_do(filewriter, eo_event_callback_del(ECORDOVA_FILEWRITER_EVENT_ON_WRITE_END, _main_loop_quit, NULL),
                     eo_event_callback_del(ECORDOVA_FILEWRITER_EVENT_ON_PROGRESS, _filewriter_abort, filewriter),
                     eo_event_callback_del(ECORDOVA_FILEWRITER_EVENT_ON_ABORT, _bool_set, &aborted),
                     eo_event_callback_del(ECORDOVA_FILEWRITER_EVENT_ON_ERROR, _bool_set, &error));

   eo_unref(timeout_timer);
   eo_unref(filewriter);

   fail_if(timeout);
   fail_if(error);
   fail_unless(aborted);

   eo_unref(file);
   free(content);

   fail_if(error = entry_remove(file_entry));
   eo_unref(file_entry);
   fail_if(error = entry_remove(directory_entry));
   eo_unref(directory_entry);
   eina_tmpstr_del(tmpdir);
}
END_TEST

void
ecordova_filewriter_test(TCase *tc)
{
   tcase_add_checked_fixture(tc, _setup, _teardown);
   tcase_add_test(tc, writer_all);
   tcase_add_test(tc, aborting);
}
