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

static Ecordova_File *
_fileentry_new(const char *name,
               const char *path,
               Ecordova_FileSystem *file_system,
               const char *url)
{
   return eo_add(ECORDOVA_FILEENTRY_CLASS,
                 NULL,
                 ecordova_fileentry_constructor(name,
                                                path,
                                                file_system,
                                                url));
}

Ecordova_FileEntry *
create_tmpfile(Eina_Tmpstr **url, unsigned char **content, size_t *size)
{
   *size = (rand() % 1024) + 1;
   return create_tmpfile_size(url, content, *size);
}

Ecordova_FileEntry *
create_tmpfile_size(Eina_Tmpstr **url, unsigned char **content, size_t size)
{
   int fd = eina_file_mkstemp("fileentry_test_XXXXXX.bin", url);
   ck_assert_int_ne(-1, fd);
   check_exists(*url);

   const char *last_path_separator = strrchr(*url, '/');
   fail_if(NULL == last_path_separator);

   const char *name = last_path_separator + 1;
   size_t len = last_path_separator - *url;
   char path[len + 1];
   strncpy(path, *url, len);
   path[len] = '\0';

   *content = generate_random_buffer_size(size);

   write(fd, *content, size);
   close(fd);

   return _fileentry_new(name, path, NULL, *url);
}

unsigned char *
generate_random_buffer(size_t *size)
{
   *size = (rand() % 1024) + 1;
   return generate_random_buffer_size(*size);
}

unsigned char *
generate_random_buffer_size(size_t size)
{
   unsigned char *content = malloc(size);
   for (size_t i = 0; i < size; ++i)
     content[i] = (unsigned char)(rand() % 256);
   return content;
}

START_TEST(smoke)
{
   Ecordova_File *fileentry = _fileentry_new("", "", NULL, "");
   eo_unref(fileentry);
}
END_TEST

Eina_Bool
_eo_get_cb(void *data,
           Eo *obj EINA_UNUSED,
           const Eo_Event_Description *desc EINA_UNUSED,
           void *event_info)
{
   Eo **object = data;
   fail_if(NULL == event_info);

   *object = eo_ref((Eo*)event_info);
   ecore_main_loop_quit();
   return EO_CALLBACK_CONTINUE;
}

static bool
_fileentry_eo_get(Ecordova_Entry *file,
                  Eo **obj,
                  const Eo_Event_Description *desc,
                  void(*method)(void))
{
   *obj = NULL;
   bool error = false;
   bool timeout = false;

   Ecore_Timer *timeout_timer = eo_add(ECORE_TIMER_CLASS, NULL, ecore_obj_timer_constructor(10, _timeout_cb, &timeout));

   eo_do(file, eo_event_callback_add(desc, _eo_get_cb, obj),
               eo_event_callback_add(ECORDOVA_ENTRY_EVENT_ERROR, _error_cb, &error),
               method());

   ecore_main_loop_begin();

   eo_do(file, eo_event_callback_del(desc, _eo_get_cb, obj),
               eo_event_callback_del(ECORDOVA_ENTRY_EVENT_ERROR, _error_cb, &error));

   eo_unref(timeout_timer);

   fail_if(timeout);

   return error;
}

static bool
_fileentry_writer_create(Ecordova_FileEntry *file_entry,
                         Ecordova_FileWriter **filewriter)
{
   return _fileentry_eo_get(file_entry,
                            filewriter,
                            ECORDOVA_FILEENTRY_EVENT_CREATE_WRITER,
                            ecordova_fileentry_writer_create);
}

bool
fileentry_file_get(Ecordova_FileEntry *file_entry,
                   Ecordova_File **file)
{
   return _fileentry_eo_get(file_entry,
                            file,
                            ECORDOVA_FILEENTRY_EVENT_FILE,
                            ecordova_fileentry_file);
}

START_TEST(writer_create)
{
   bool error = false;
   Eina_Tmpstr *url = NULL;
   unsigned char *content = NULL;
   size_t size = 0;
   Ecordova_FileEntry *file_entry = create_tmpfile(&url, &content, &size);

   Ecordova_FileWriter *filewriter = NULL;
   fail_if(error = _fileentry_writer_create(file_entry, &filewriter));
   fail_if(NULL == filewriter);
   eo_unref(filewriter);

   fail_if(error = entry_remove(file_entry));
   eo_unref(file_entry);
   free(content);
   eina_tmpstr_del(url);
}
END_TEST

START_TEST(file_get)
{
   bool error = false;
   Eina_Tmpstr *url = NULL;
   unsigned char *content = NULL;
   size_t size = 0;
   Ecordova_FileEntry *file_entry = create_tmpfile(&url, &content, &size);

   Ecordova_File *file = NULL;
   fail_if(error = fileentry_file_get(file_entry, &file));
   fail_if(NULL == file);

   long actual_end = eo_do_ret(file, actual_end, ecordova_file_end_get());
   long expected_end = size;
   ck_assert_int_eq(expected_end, actual_end);

   eo_unref(file);

   fail_if(error = entry_remove(file_entry));
   eo_unref(file_entry);
   free(content);
   eina_tmpstr_del(url);
}
END_TEST

void
ecordova_fileentry_test(TCase *tc)
{
   tcase_add_checked_fixture(tc, _setup, _teardown);
   tcase_add_test(tc, smoke);
   tcase_add_test(tc, writer_create);
   tcase_add_test(tc, file_get);
}
