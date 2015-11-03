#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "ecordova_file_tests.h"
#include "ecordova_suite.h"

#include <Eio.h>

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

Ecordova_DirectoryEntry *
directoryentry_new(const char *name,
                   const char *path,
                   const char *url)
{
   return eo_add(ECORDOVA_DIRECTORYENTRY_CLASS,
                 NULL,
                 ecordova_directoryentry_constructor(name, path, NULL, url));
}

void
check_exists(const char *url)
{
   Eina_Bool ret = ecore_file_exists(url);
   ck_assert_int_eq(EINA_TRUE, ret);
}

void
check_doesnt_exist(const char *url)
{
   Eina_Bool ret = ecore_file_exists(url);
   ck_assert_int_eq(EINA_FALSE, ret);
}

Ecordova_DirectoryEntry *
_create_tmpdir(Eina_Tmpstr **tmpdir)
{
   Eina_Bool tmpdir_created = eina_file_mkdtemp("directoryentry_test_XXXXXX", tmpdir);
   ck_assert_int_eq(EINA_TRUE, tmpdir_created);
   check_exists(*tmpdir);

   const char *last_path_separator = strrchr(*tmpdir, '/');
   fail_if(NULL == last_path_separator);

   const char *name = last_path_separator + 1;
   size_t len = last_path_separator - *tmpdir;
   char path[len + 1];
   strncpy(path, *tmpdir, len);
   path[len] = '\0';

   return directoryentry_new(name, path, *tmpdir);
}

START_TEST(smoke)
{
   Ecordova_DirectoryEntry *directory_entry =
     directoryentry_new("", TESTS_BUILD_DIR, TESTS_BUILD_DIR);
   eo_unref(directory_entry);
}
END_TEST

Eina_Bool
_timeout_cb(void *data)
{
   bool *timeout = data;
   *timeout = true;
   ecore_main_loop_quit();
   return ECORE_CALLBACK_CANCEL;
}

Eina_Bool
_entry_get_cb(void *data,
              Eo *obj EINA_UNUSED,
              const Eo_Event_Description *desc EINA_UNUSED,
              void *event_info)
{
   Ecordova_Entry **entry = data;
   fail_if(NULL == event_info);

   *entry = eo_ref((Ecordova_Entry*)event_info);
   ecore_main_loop_quit();
   return EO_CALLBACK_CONTINUE;
}

Eina_Bool
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

void
_check_entry_name(Ecordova_Entry *entry, const char *expected_name)
{
   const char *actual_name = eo_do_ret(entry,
                                       actual_name,
                                       ecordova_entry_name_get());
   fail_if(NULL == actual_name);
   ck_assert_str_eq(expected_name, actual_name);
}

void
_check_entry_path(Ecordova_Entry *entry, const char *expected_path)
{
   const char *actual_path = eo_do_ret(entry,
                                       actual_path,
                                       ecordova_entry_path_get());
   fail_if(NULL == actual_path);
   ck_assert_str_eq(expected_path, actual_path);
}

static void
_check_entry_is_file(Ecordova_Entry *entry)
{
   Eina_Bool is_file;
   Eina_Bool is_directory;
   eo_do(entry,
         is_file = ecordova_entry_file_is_get(),
         is_directory = ecordova_entry_directory_is_get());
   fail_unless(is_file);
   fail_if(is_directory);
}

static void
_check_entry_is_directory(Ecordova_Entry *entry)
{
   Eina_Bool is_directory;
   Eina_Bool is_file;
   eo_do(entry,
         is_directory = ecordova_entry_directory_is_get(),
         is_file = ecordova_entry_file_is_get());
   fail_unless(is_directory);
   fail_if(is_file);
}

static bool
_entry_get(Ecordova_Entry *obj,
           const char *url,
           Ecordova_FileFlags flags,
           Ecordova_Entry **entry,
           const Eo_Event_Description *get_event,
           void(*entry_get)(const char *, Ecordova_FileFlags))
{
   *entry = NULL;
   bool error = false;
   bool timeout = false;

   Ecore_Timer *timeout_timer = eo_add(ECORE_TIMER_CLASS, NULL, ecore_obj_timer_constructor(10, _timeout_cb, &timeout));

   eo_do(obj, eo_event_callback_add(get_event, _entry_get_cb, entry),
              eo_event_callback_add(ECORDOVA_ENTRY_EVENT_ERROR, _error_cb, &error),
              entry_get(url, flags));

   ecore_main_loop_begin();

   eo_do(obj, eo_event_callback_del(get_event, _entry_get_cb, entry),
              eo_event_callback_del(ECORDOVA_ENTRY_EVENT_ERROR, _error_cb, &error));

   eo_unref(timeout_timer);

   fail_if(timeout);

   return error;
}

bool
fileentry_get(Ecordova_DirectoryEntry *directory_entry,
              const char *url,
              Ecordova_FileFlags flags,
              Ecordova_FileEntry **entry)
{
   DBG("Getting file entry: %s", url);
   return _entry_get(directory_entry,
                     url,
                     flags,
                     (Ecordova_Entry**)entry,
                     ECORDOVA_DIRECTORYENTRY_EVENT_FILE_GET,
                     ecordova_directoryentry_file_get);
}

bool
directoryentry_get(Ecordova_DirectoryEntry *directory_entry,
                   const char *url,
                   Ecordova_FileFlags flags,
                   Ecordova_DirectoryEntry **entry)
{
   DBG("Getting directory entry: %s", url);
   return _entry_get(directory_entry,
                     url,
                     flags,
                     (Ecordova_Entry**)entry,
                     ECORDOVA_DIRECTORYENTRY_EVENT_DIRECTORY_GET,
                     ecordova_directoryentry_directory_get);
}

START_TEST(get_file)
{
   Eina_Tmpstr *tmpdir = NULL;
   Ecordova_DirectoryEntry *directory_entry = _create_tmpdir(&tmpdir);

   const char *filename = "file.txt";
   const char *filepath = tmpdir;
   size_t len = strlen(filepath) + 1 + strlen(filename) + 1;
   char fileurl[len];
   snprintf(fileurl, len, "%s/%s", tmpdir, filename);

   Ecordova_FileEntry *file_entry = NULL;
   bool error = false;

   // create exclusive
   Ecordova_FileFlags flags = ECORDOVA_FILEFLAGS_CREATE | ECORDOVA_FILEFLAGS_EXCLUSIVE;
   error = fileentry_get(directory_entry, filename, flags, &file_entry);
   fail_if(error);
   fail_unless(NULL != file_entry);
   check_exists(fileurl);

   _check_entry_is_file(file_entry);
   _check_entry_name(file_entry, filename);
   _check_entry_path(file_entry, filepath);

   eo_unref(file_entry);


   // create exclusive on a existing file is an error
   flags = ECORDOVA_FILEFLAGS_CREATE | ECORDOVA_FILEFLAGS_EXCLUSIVE;
   error = fileentry_get(directory_entry, filename, flags, &file_entry);
   fail_if(NULL != file_entry);
   fail_unless(error);


   // just create on an existing file opens it
   flags = ECORDOVA_FILEFLAGS_CREATE;
   error = fileentry_get(directory_entry, filename, flags, &file_entry);
   fail_if(error);
   fail_unless(NULL != file_entry);

   _check_entry_is_file(file_entry);
   _check_entry_name(file_entry, filename);
   _check_entry_path(file_entry, filepath);

   eo_unref(file_entry);

   // just opens it
   flags = 0;
   error = fileentry_get(directory_entry, filename, flags, &file_entry);
   fail_if(error);
   fail_unless(NULL != file_entry);

   _check_entry_is_file(file_entry);
   _check_entry_name(file_entry, filename);
   _check_entry_path(file_entry, filepath);

   fail_if(error = entry_remove(file_entry));
   eo_unref(file_entry);

   // error on non existent file
   flags = 0;
   error = fileentry_get(directory_entry, ",**??non_existent??**,", flags, &file_entry);
   fail_if(NULL != file_entry);
   fail_unless(error);

   fail_if(error = entry_remove(directory_entry));
   eo_unref(directory_entry);

   eina_tmpstr_del(tmpdir);
}
END_TEST

START_TEST(get_dir)
{
   Eina_Tmpstr *tmpdir = NULL;
   Ecordova_DirectoryEntry *directory_entry = _create_tmpdir(&tmpdir);

   const char *dir_name = "some_path";
   const char *dir_path = tmpdir;
   size_t len = strlen(dir_path) + 1 + strlen(dir_name) + 1;
   char dir_url[len];
   snprintf(dir_url, len, "%s/%s", tmpdir, dir_name);

   Ecordova_DirectoryEntry *child_entry = NULL;
   bool error = false;

   // create exclusive
   Ecordova_FileFlags flags = ECORDOVA_FILEFLAGS_CREATE | ECORDOVA_FILEFLAGS_EXCLUSIVE;
   error = directoryentry_get(directory_entry, dir_name, flags, &child_entry);
   fail_if(error);
   fail_unless(NULL != child_entry);
   check_exists(dir_url);

   _check_entry_is_directory(child_entry);
   _check_entry_name(child_entry, dir_name);
   _check_entry_path(child_entry, dir_path);

   eo_unref(child_entry);


   // create exclusive on a existing directory is an error
   flags = ECORDOVA_FILEFLAGS_CREATE | ECORDOVA_FILEFLAGS_EXCLUSIVE;
   error = directoryentry_get(directory_entry, dir_name, flags, &child_entry);
   fail_if(NULL != child_entry);
   fail_unless(error);


   // just the create flag on an existing directory opens it
   flags = ECORDOVA_FILEFLAGS_CREATE;
   error = directoryentry_get(directory_entry, dir_name, flags, &child_entry);
   fail_if(error);
   fail_unless(NULL != child_entry);

   _check_entry_is_directory(child_entry);
   _check_entry_name(child_entry, dir_name);
   _check_entry_path(child_entry, dir_path);

   eo_unref(child_entry);

   // just opens it
   flags = 0;
   error = directoryentry_get(directory_entry, dir_name, flags, &child_entry);
   fail_if(error);
   fail_unless(NULL != child_entry);

   _check_entry_is_directory(child_entry);
   _check_entry_name(child_entry, dir_name);
   _check_entry_path(child_entry, dir_path);

   fail_if(error = entry_remove(child_entry));
   eo_unref(child_entry);


   // error on non existent directory
   flags = 0;
   error = directoryentry_get(directory_entry, ",**??non_existent??**,", flags, &child_entry);
   fail_if(NULL != child_entry);
   fail_unless(error);


   fail_if(error = entry_remove(directory_entry));
   eo_unref(directory_entry);

   eina_tmpstr_del(tmpdir);
}
END_TEST

static bool
_directoryentry_recursively_remove(Ecordova_DirectoryEntry *directory_entry)
{
   return entry_do(directory_entry,
                   ECORDOVA_DIRECTORYENTRY_EVENT_REMOVE_SUCCESS,
                   ecordova_directoryentry_recursively_remove);
}

START_TEST(remove_recursively)
{
   Eina_Tmpstr *tmpdir = NULL;
   Ecordova_DirectoryEntry *directory_entry = _create_tmpdir(&tmpdir);

   // removes an existent directory recursively
   const char *child_name = "child";
   size_t len = strlen(tmpdir) + 1 + strlen(child_name) + 1;
   char child_dir[len];
   snprintf(child_dir, len, "%s/%s", tmpdir, child_name);

   Eina_Bool ret = ecore_file_mkdir(child_dir);
   ck_assert_int_eq(EINA_TRUE, ret);

   bool error = _directoryentry_recursively_remove(directory_entry);
   fail_if(error);

   check_doesnt_exist(child_dir);
   check_doesnt_exist(tmpdir);

   // error on non existent directory
   error = _directoryentry_recursively_remove(directory_entry);
   fail_unless(error);

   eo_unref(directory_entry);

   eina_tmpstr_del(tmpdir);
}
END_TEST

START_TEST(create_reader)
{
   Eina_Tmpstr *tmpdir = NULL;
   Ecordova_DirectoryEntry *directory_entry = _create_tmpdir(&tmpdir);

   Ecordova_DirectoryReader *directory_reader =
     eo_do_ret(directory_entry,
               directory_reader,
               ecordova_directoryentry_reader_create());
   fail_if(NULL == directory_reader);
   eo_unref(directory_reader);

   bool error = false;
   fail_if(error = entry_remove(directory_entry));
   eo_unref(directory_entry);
   eina_tmpstr_del(tmpdir);
}
END_TEST

void
ecordova_directoryentry_test(TCase *tc)
{
   tcase_add_checked_fixture(tc, _setup, _teardown);
   tcase_add_test(tc, smoke);
   tcase_add_test(tc, get_file);
   tcase_add_test(tc, get_dir);
   tcase_add_test(tc, remove_recursively);
   tcase_add_test(tc, create_reader);
}
