#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "ecordova_entry_test.h"
#include "ecordova_directoryentry_test.h"
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

START_TEST(smoke)
{
   Ecordova_Entry *entry = eo_add(ECORDOVA_ENTRY_CLASS,
                                  NULL,
                                  ecordova_entry_constructor(EINA_FALSE,
                                                             EINA_TRUE,
                                                             "",
                                                             TESTS_BUILD_DIR,
                                                             NULL,
                                                             TESTS_BUILD_DIR));
   eo_unref(entry);
}
END_TEST

static bool
_move_copy_to(Ecordova_Entry *source,
              Ecordova_Entry *parent_dest,
              const char *new_name,
              Ecordova_Entry **entry,
              const Eo_Event_Description *event_desc,
              void(*method)(Ecordova_DirectoryEntry*, const char *))
{
   *entry = NULL;
   bool error = false;
   bool timeout = false;

   Ecore_Timer *timeout_timer = eo_add(ECORE_TIMER_CLASS, NULL, ecore_obj_timer_constructor(10, _timeout_cb, &timeout));

   eo_do(source, eo_event_callback_add(event_desc, _entry_get_cb, entry),
                 eo_event_callback_add(ECORDOVA_ENTRY_EVENT_ERROR, _error_cb, &error),
                 method(parent_dest, new_name));

   ecore_main_loop_begin();

   eo_do(source, eo_event_callback_del(event_desc, _entry_get_cb, entry),
                 eo_event_callback_del(ECORDOVA_ENTRY_EVENT_ERROR, _error_cb, &error));

   eo_unref(timeout_timer);

   fail_if(timeout);

   return error;
}

static bool
_move_to(Ecordova_Entry *source,
         Ecordova_Entry *parent_dest,
         const char *new_name,
         Ecordova_Entry **moved_entry)
{
   return _move_copy_to(source,
                        parent_dest,
                        new_name,
                        moved_entry,
                        ECORDOVA_ENTRY_EVENT_MOVE_SUCCESS,
                        ecordova_entry_move);
}

static bool
_copy_to(Ecordova_Entry *source,
         Ecordova_Entry *parent_dest,
         const char *new_name,
         Ecordova_Entry **copied_entry)
{
   return _move_copy_to(source,
                        parent_dest,
                        new_name,
                        copied_entry,
                        ECORDOVA_ENTRY_EVENT_COPY_SUCCESS,
                        ecordova_entry_copy);
}

Eina_Bool
_do_cb(void *data,
       Eo *obj EINA_UNUSED,
       const Eo_Event_Description *desc EINA_UNUSED,
       void *event_info)
{
   bool *removed = data;
   fail_if(NULL != event_info);

   *removed = true;
   ecore_main_loop_quit();
   return EO_CALLBACK_CONTINUE;
}

bool
entry_do(Ecordova_Entry *entry,
         const Eo_Event_Description *event_desc,
         void(*method)())
{
   bool success = false;
   bool error = false;
   bool timeout = false;

   Ecore_Timer *timeout_timer = eo_add(ECORE_TIMER_CLASS, NULL, ecore_obj_timer_constructor(10, _timeout_cb, &timeout));

   eo_do(entry, eo_event_callback_add(event_desc, _do_cb, &success),
                eo_event_callback_add(ECORDOVA_ENTRY_EVENT_ERROR, _error_cb, &error),
                method());

   ecore_main_loop_begin();

   eo_do(entry, eo_event_callback_del(event_desc, _do_cb, &success),
                eo_event_callback_del(ECORDOVA_ENTRY_EVENT_ERROR, _error_cb, &error));

   eo_unref(timeout_timer);

   fail_if(timeout);

   return error || !success;
}

static bool
_entry_parent_get(Ecordova_Entry *obj, Ecordova_Entry **parent)
{
   *parent = NULL;
   bool error = false;
   bool timeout = false;

   Ecore_Timer *timeout_timer = eo_add(ECORE_TIMER_CLASS, NULL, ecore_obj_timer_constructor(10, _timeout_cb, &timeout));

   eo_do(obj, eo_event_callback_add(ECORDOVA_ENTRY_EVENT_PARENT_GET, _entry_get_cb, parent),
              eo_event_callback_add(ECORDOVA_ENTRY_EVENT_ERROR, _error_cb, &error),
              ecordova_entry_parent_get());

   ecore_main_loop_begin();

   eo_do(obj, eo_event_callback_del(ECORDOVA_ENTRY_EVENT_PARENT_GET, _entry_get_cb, parent),
              eo_event_callback_del(ECORDOVA_ENTRY_EVENT_ERROR, _error_cb, &error));

   eo_unref(timeout_timer);

   fail_if(timeout);

   return error;
}

Eina_Bool
_metadata_get_cb(void *data,
                 Eo *obj EINA_UNUSED,
                 const Eo_Event_Description *desc EINA_UNUSED,
                 void *event_info)
{
   Ecordova_Metadata *metadata = data;
   fail_if(NULL == event_info);

   *metadata = *(Ecordova_Metadata*)event_info;
   ecore_main_loop_quit();
   return EO_CALLBACK_CONTINUE;
}

static bool
_entry_metadataget(Ecordova_Entry *obj, Ecordova_Metadata *metadata)
{
   *metadata = (Ecordova_Metadata){0};
   bool error = false;
   bool timeout = false;

   Ecore_Timer *timeout_timer = eo_add(ECORE_TIMER_CLASS, NULL, ecore_obj_timer_constructor(10, _timeout_cb, &timeout));

   eo_do(obj, eo_event_callback_add(ECORDOVA_ENTRY_EVENT_METADATA_GET, _metadata_get_cb, metadata),
              eo_event_callback_add(ECORDOVA_ENTRY_EVENT_ERROR, _error_cb, &error),
              ecordova_entry_metadata_get());

   ecore_main_loop_begin();

   eo_do(obj, eo_event_callback_del(ECORDOVA_ENTRY_EVENT_METADATA_GET, _metadata_get_cb, metadata),
              eo_event_callback_del(ECORDOVA_ENTRY_EVENT_ERROR, _error_cb, &error));

   eo_unref(timeout_timer);

   fail_if(timeout);

   return error;
}

bool
entry_remove(Ecordova_Entry *entry)
{
   return entry_do(entry,
                   ECORDOVA_ENTRY_EVENT_REMOVE_SUCCESS,
                   ecordova_entry_remove);
}

START_TEST(dir_remove)
{
   Eina_Tmpstr *tmpdir = NULL;
   Ecordova_DirectoryEntry *directory_entry = _create_tmpdir(&tmpdir);

   const char *filename = "file.txt";
   size_t len = strlen(tmpdir) + 1 + strlen(filename) + 1;
   char file_url[len];
   snprintf(file_url, len, "%s/%s", tmpdir, filename);


   bool error = false;

   Ecordova_FileEntry *file_entry = NULL;
   const Ecordova_FileFlags flags = ECORDOVA_FILEFLAGS_CREATE | ECORDOVA_FILEFLAGS_EXCLUSIVE;
   error = fileentry_get(directory_entry, file_url, flags, &file_entry);
   fail_if(error);
   fail_unless(NULL != file_entry);
   check_exists(file_url);

   // non empty directory cannot be removed, gives error
   fail_unless(error = entry_remove(directory_entry));

   // turns the directory empty
   fail_if(error = entry_remove(file_entry));

   // removes the directory
   fail_if(error = entry_remove(directory_entry));

   // non existent directory gives error
   fail_unless(error = entry_remove(directory_entry));

   eo_unref(file_entry);
   eo_unref(directory_entry);

   eina_tmpstr_del(tmpdir);
}
END_TEST

START_TEST(dir_move_to)
{
   Eina_Tmpstr *tmpdir = NULL;
   Ecordova_DirectoryEntry *directory_entry = _create_tmpdir(&tmpdir);

   const char *child_name1 = "child1";
   size_t len = strlen(tmpdir) + 1 + strlen(child_name1) + 1;
   char child_dir1[len];
   snprintf(child_dir1, len, "%s/%s", tmpdir, child_name1);
   Eina_Bool ret = ecore_file_mkdir(child_dir1);
   ck_assert_int_eq(EINA_TRUE, ret);

   const char *child_name2 = "child2";
   len = strlen(tmpdir) + 1 + strlen(child_name2) + 1;
   char child_dir2[len];
   snprintf(child_dir2, len, "%s/%s", tmpdir, child_name2);
   ret = ecore_file_mkdir(child_dir2);
   ck_assert_int_eq(EINA_TRUE, ret);

   Ecordova_DirectoryEntry *child_entry1 = NULL;
   Ecordova_DirectoryEntry *child_entry2 = NULL;
   bool error = false;

   error = directoryentry_get(directory_entry, child_dir1, 0, &child_entry1);
   fail_if(error);
   fail_unless(NULL != child_entry1);
   error = directoryentry_get(directory_entry, child_dir2, 0, &child_entry2);
   fail_if(error);
   fail_unless(NULL != child_entry2);

   Ecordova_DirectoryEntry *moved_entry = NULL;
   error = _move_to(child_entry1, child_entry2, NULL, &moved_entry);
   fail_if(error);
   fail_unless(NULL != moved_entry);

   fail_if(error = entry_remove(moved_entry));
   fail_if(error = entry_remove(child_entry2));
   fail_if(error = entry_remove(directory_entry));

   eo_unref(moved_entry);
   eo_unref(child_entry2);
   eo_unref(child_entry1);
   eo_unref(directory_entry);

   eina_tmpstr_del(tmpdir);
}
END_TEST

START_TEST(dir_rename)
{
   Eina_Tmpstr *tmpdir = NULL;
   Ecordova_DirectoryEntry *directory_entry = _create_tmpdir(&tmpdir);

   const char *child_name = "child";
   size_t len = strlen(tmpdir) + 1 + strlen(child_name) + 1;
   char child_dir[len];
   snprintf(child_dir, len, "%s/%s", tmpdir, child_name);
   Eina_Bool ret = ecore_file_mkdir(child_dir);
   ck_assert_int_eq(EINA_TRUE, ret);

   Ecordova_DirectoryEntry *child_entry = NULL;
   bool error = false;

   error = directoryentry_get(directory_entry, child_dir, 0, &child_entry);
   fail_if(error);
   fail_unless(NULL != child_entry);

   Ecordova_DirectoryEntry *renamed_entry = NULL;
   error = _move_to(child_entry, directory_entry, "renamed", &renamed_entry);
   fail_if(error);
   fail_unless(NULL != renamed_entry);

   fail_if(error = entry_remove(renamed_entry));
   fail_if(error = entry_remove(directory_entry));

   eo_unref(renamed_entry);
   eo_unref(child_entry);
   eo_unref(directory_entry);

   eina_tmpstr_del(tmpdir);
}
END_TEST

START_TEST(dir_copy)
{
   Eina_Tmpstr *tmpdir = NULL;
   Ecordova_DirectoryEntry *directory_entry = _create_tmpdir(&tmpdir);

   const char *child_name = "child";
   size_t len = strlen(tmpdir) + 1 + strlen(child_name) + 1;
   char child_dir[len];
   snprintf(child_dir, len, "%s/%s", tmpdir, child_name);
   Eina_Bool ret = ecore_file_mkdir(child_dir);
   ck_assert_int_eq(EINA_TRUE, ret);

   const char *filename = "file.txt";
   len = strlen(child_dir) + 1 + strlen(filename) + 1;
   char file_url[len];
   snprintf(file_url, len, "%s/%s", child_dir, filename);

   bool error = false;

   Ecordova_FileEntry *file_entry = NULL;
   const Ecordova_FileFlags flags = ECORDOVA_FILEFLAGS_CREATE | ECORDOVA_FILEFLAGS_EXCLUSIVE;
   error = fileentry_get(directory_entry, file_url, flags, &file_entry);
   fail_if(error);
   fail_unless(NULL != file_entry);
   check_exists(file_url);

   Ecordova_DirectoryEntry *child_entry = NULL;

   error = directoryentry_get(directory_entry, child_dir, 0, &child_entry);
   fail_if(error);
   fail_unless(NULL != child_entry);

   const char *NEW_DIR_NAME = "copied";
   Ecordova_DirectoryEntry *copied_dir_entry = NULL;
   error = _copy_to(child_entry, directory_entry, NEW_DIR_NAME, &copied_dir_entry);
   fail_if(error);
   fail_unless(NULL != copied_dir_entry);

   len = strlen(tmpdir) + 1 + strlen(NEW_DIR_NAME) + 1 + strlen(filename) + 1;
   char copied_file_url[len];
   snprintf(copied_file_url, len, "%s/%s/%s", tmpdir, NEW_DIR_NAME, filename);

   check_exists(copied_file_url);
   Ecordova_FileEntry *copied_file_entry = NULL;
   error = fileentry_get(directory_entry, copied_file_url, 0, &copied_file_entry);
   fail_if(error);
   fail_unless(NULL != copied_file_entry);

   fail_if(error = entry_remove(file_entry));
   fail_if(error = entry_remove(child_entry));
   fail_if(error = entry_remove(copied_file_entry));
   fail_if(error = entry_remove(copied_dir_entry));
   fail_if(error = entry_remove(directory_entry));

   eo_unref(file_entry);
   eo_unref(copied_file_entry);
   eo_unref(copied_dir_entry);
   eo_unref(child_entry);
   eo_unref(directory_entry);

   eina_tmpstr_del(tmpdir);
}
END_TEST

START_TEST(dir_parent)
{
   Eina_Tmpstr *tmpdir = NULL;
   Ecordova_DirectoryEntry *directory_entry = _create_tmpdir(&tmpdir);

   const char *dir_name = "child";
   size_t len = strlen(tmpdir) + 1 + strlen(dir_name) + 1;
   char dir_url[len];
   snprintf(dir_url, len, "%s/%s", tmpdir, dir_name);

   bool error = false;

   Ecordova_FileEntry *dir_entry = NULL;
   const Ecordova_FileFlags flags = ECORDOVA_FILEFLAGS_CREATE | ECORDOVA_FILEFLAGS_EXCLUSIVE;
   error = directoryentry_get(directory_entry, dir_url, flags, &dir_entry);
   fail_if(error);
   fail_unless(NULL != dir_entry);
   check_exists(dir_url);

   Ecordova_FileEntry *parent_entry = NULL;
   fail_if(error = _entry_parent_get(dir_entry, &parent_entry));

   const char *directory_name;
   const char *parent_name;
   eo_do(directory_entry,
         directory_name = ecordova_entry_name_get(),
         parent_name = ecordova_entry_name_get());
   fail_if(NULL == directory_name);
   fail_if(NULL == parent_name);
   ck_assert_str_eq(directory_name, parent_name);

   fail_if(error = entry_remove(dir_entry));
   fail_if(error = entry_remove(directory_entry));

   eo_unref(parent_entry);
   eo_unref(dir_entry);
   eo_unref(directory_entry);

   eina_tmpstr_del(tmpdir);
}
END_TEST

START_TEST(dir_root_parent)
{
   Ecordova_DirectoryEntry *directory_entry = directoryentry_new("", "/", "/");

   Ecordova_FileEntry *parent_entry = NULL;
   bool error = false;
   fail_if(error = _entry_parent_get(directory_entry, &parent_entry));

   const char *directory_name = "";
   fail_if(NULL == directory_name);
   const char *parent_name = eo_do_ret(parent_entry,
                                       parent_name,
                                       ecordova_entry_name_get());
   fail_if(NULL == parent_name);
   ck_assert_str_eq(directory_name, parent_name);

   eo_unref(parent_entry);
   eo_unref(directory_entry);
}
END_TEST

START_TEST(dir_metadata)
{
   Eina_Tmpstr *tmpdir = NULL;
   Ecordova_DirectoryEntry *directory_entry = _create_tmpdir(&tmpdir);

   const char *dir_name = "child";
   size_t len = strlen(tmpdir) + 1 + strlen(dir_name) + 1;
   char dir_url[len];
   snprintf(dir_url, len, "%s/%s", tmpdir, dir_name);

   bool error = false;

   Ecordova_FileEntry *dir_entry = NULL;
   const Ecordova_FileFlags flags = ECORDOVA_FILEFLAGS_CREATE | ECORDOVA_FILEFLAGS_EXCLUSIVE;
   error = fileentry_get(directory_entry, dir_url, flags, &dir_entry);
   fail_if(error);
   fail_unless(NULL != dir_entry);
   check_exists(dir_url);

   Ecordova_Metadata metadata = {0};
   fail_if(error = _entry_metadataget(dir_entry, &metadata));
   ck_assert_int_ne(0, metadata.modification_date);

   fail_if(error = entry_remove(dir_entry));
   fail_if(error = entry_remove(directory_entry));

   eo_unref(dir_entry);
   eo_unref(directory_entry);

   eina_tmpstr_del(tmpdir);
}
END_TEST

START_TEST(file_remove)
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

   const Ecordova_FileFlags flags = ECORDOVA_FILEFLAGS_CREATE | ECORDOVA_FILEFLAGS_EXCLUSIVE;
   error = fileentry_get(directory_entry, filename, flags, &file_entry);
   fail_if(error);
   fail_unless(NULL != file_entry);
   check_exists(fileurl);

   fail_if(error = entry_remove(file_entry));
   // non existent gives error
   fail_unless(error = entry_remove(file_entry));
   eo_unref(file_entry);

   fail_if(error = entry_remove(directory_entry));
   eo_unref(directory_entry);

   eina_tmpstr_del(tmpdir);
}
END_TEST

START_TEST(file_move_to)
{
   Eina_Tmpstr *tmpdir = NULL;
   Ecordova_DirectoryEntry *directory_entry = _create_tmpdir(&tmpdir);

   const char *filename = "file.txt";
   size_t len = strlen(tmpdir) + 1 + strlen(filename) + 1;
   char file_url[len];
   snprintf(file_url, len, "%s/%s", tmpdir, filename);

   const char *dirname = "dir";
   len = strlen(tmpdir) + 1 + strlen(dirname) + 1;
   char dir_url[len];
   snprintf(dir_url, len, "%s/%s", tmpdir, dirname);

   bool error = false;

   Ecordova_FileEntry *file_entry = NULL;
   const Ecordova_FileFlags flags = ECORDOVA_FILEFLAGS_CREATE | ECORDOVA_FILEFLAGS_EXCLUSIVE;
   error = fileentry_get(directory_entry, file_url, flags, &file_entry);
   fail_if(error);
   fail_unless(NULL != file_entry);
   check_exists(file_url);

   Ecordova_DirectoryEntry *dir_entry = NULL;
   error = directoryentry_get(directory_entry, dir_url, flags, &dir_entry);
   fail_if(error);
   fail_unless(NULL != dir_entry);
   check_exists(dir_url);

   Ecordova_FileEntry *moved_entry = NULL;
   error = _move_to(file_entry, dir_entry, NULL, &moved_entry);
   fail_if(error);
   fail_unless(NULL != moved_entry);
   check_doesnt_exist(file_url);

   fail_if(error = entry_remove(moved_entry));
   fail_if(error = entry_remove(dir_entry));
   fail_if(error = entry_remove(directory_entry));

   eo_unref(moved_entry);
   eo_unref(dir_entry);
   eo_unref(file_entry);
   eo_unref(directory_entry);

   eina_tmpstr_del(tmpdir);
}
END_TEST

START_TEST(file_rename)
{
   Eina_Tmpstr *tmpdir = NULL;
   Ecordova_DirectoryEntry *directory_entry = _create_tmpdir(&tmpdir);

   const char *filename = "file.txt";
   size_t len = strlen(tmpdir) + 1 + strlen(filename) + 1;
   char file_url[len];
   snprintf(file_url, len, "%s/%s", tmpdir, filename);

   bool error = false;

   Ecordova_FileEntry *file_entry = NULL;
   const Ecordova_FileFlags flags = ECORDOVA_FILEFLAGS_CREATE | ECORDOVA_FILEFLAGS_EXCLUSIVE;
   error = fileentry_get(directory_entry, file_url, flags, &file_entry);
   fail_if(error);
   fail_unless(NULL != file_entry);
   check_exists(file_url);

   Ecordova_FileEntry *renamed_entry = NULL;
   error = _move_to(file_entry, directory_entry, "renamed.txt", &renamed_entry);
   fail_if(error);
   fail_unless(NULL != renamed_entry);
   check_doesnt_exist(file_url);

   fail_if(error = entry_remove(renamed_entry));
   fail_if(error = entry_remove(directory_entry));

   eo_unref(renamed_entry);
   eo_unref(file_entry);
   eo_unref(directory_entry);

   eina_tmpstr_del(tmpdir);
}
END_TEST

START_TEST(file_copy)
{
   Eina_Tmpstr *tmpdir = NULL;
   Ecordova_DirectoryEntry *directory_entry = _create_tmpdir(&tmpdir);

   const char *filename = "file.txt";
   size_t len = strlen(tmpdir) + 1 + strlen(filename) + 1;
   char file_url[len];
   snprintf(file_url, len, "%s/%s", tmpdir, filename);

   bool error = false;

   Ecordova_FileEntry *file_entry = NULL;
   const Ecordova_FileFlags flags = ECORDOVA_FILEFLAGS_CREATE | ECORDOVA_FILEFLAGS_EXCLUSIVE;
   error = fileentry_get(directory_entry, file_url, flags, &file_entry);
   fail_if(error);
   fail_unless(NULL != file_entry);
   check_exists(file_url);


   const char *copiedfilename = "copied.txt";
   len = strlen(tmpdir) + 1 + strlen(copiedfilename) + 1;
   char copied_file_url[len];
   snprintf(copied_file_url, len, "%s/%s", tmpdir, copiedfilename);

   Ecordova_FileEntry *copied_entry = NULL;
   error = _copy_to(file_entry, directory_entry, copiedfilename, &copied_entry);
   fail_if(error);
   fail_unless(NULL != copied_entry);
   check_exists(copied_file_url);

   fail_if(error = entry_remove(file_entry));
   fail_if(error = entry_remove(copied_entry));
   fail_if(error = entry_remove(directory_entry));

   eo_unref(copied_entry);
   eo_unref(file_entry);
   eo_unref(directory_entry);

   eina_tmpstr_del(tmpdir);
}
END_TEST

START_TEST(file_parent)
{
   Eina_Tmpstr *tmpdir = NULL;
   Ecordova_DirectoryEntry *directory_entry = _create_tmpdir(&tmpdir);

   const char *filename = "file.txt";
   size_t len = strlen(tmpdir) + 1 + strlen(filename) + 1;
   char file_url[len];
   snprintf(file_url, len, "%s/%s", tmpdir, filename);

   bool error = false;

   Ecordova_FileEntry *file_entry = NULL;
   const Ecordova_FileFlags flags = ECORDOVA_FILEFLAGS_CREATE | ECORDOVA_FILEFLAGS_EXCLUSIVE;
   error = fileentry_get(directory_entry, file_url, flags, &file_entry);
   fail_if(error);
   fail_unless(NULL != file_entry);
   check_exists(file_url);

   Ecordova_FileEntry *parent_entry = NULL;
   fail_if(error = _entry_parent_get(file_entry, &parent_entry));

   const char *directory_name = eo_do_ret(directory_entry,
                                          directory_name,
                                          ecordova_entry_name_get());
   const char *parent_name = eo_do_ret(parent_entry,
                                       parent_name,
                                       ecordova_entry_name_get());
   fail_if(NULL == directory_name);
   fail_if(NULL == parent_name);
   ck_assert_str_eq(directory_name, parent_name);

   fail_if(error = entry_remove(file_entry));
   fail_if(error = entry_remove(directory_entry));

   eo_unref(parent_entry);
   eo_unref(file_entry);
   eo_unref(directory_entry);

   eina_tmpstr_del(tmpdir);
}
END_TEST

START_TEST(file_metadata)
{
   Eina_Tmpstr *tmpdir = NULL;
   Ecordova_DirectoryEntry *directory_entry = _create_tmpdir(&tmpdir);

   const char *filename = "file.txt";
   size_t len = strlen(tmpdir) + 1 + strlen(filename) + 1;
   char file_url[len];
   snprintf(file_url, len, "%s/%s", tmpdir, filename);

   bool error = false;

   Ecordova_FileEntry *file_entry = NULL;
   const Ecordova_FileFlags flags = ECORDOVA_FILEFLAGS_CREATE | ECORDOVA_FILEFLAGS_EXCLUSIVE;
   error = fileentry_get(directory_entry, file_url, flags, &file_entry);
   fail_if(error);
   fail_unless(NULL != file_entry);
   check_exists(file_url);

   Ecordova_Metadata metadata = {0};
   fail_if(error = _entry_metadataget(file_entry, &metadata));
   ck_assert_int_ne(0, metadata.modification_date);

   fail_if(error = entry_remove(file_entry));
   fail_if(error = entry_remove(directory_entry));

   eo_unref(file_entry);
   eo_unref(directory_entry);

   eina_tmpstr_del(tmpdir);
}
END_TEST

void
ecordova_entry_test(TCase *tc)
{
   tcase_add_checked_fixture(tc, _setup, _teardown);
   tcase_add_test(tc, smoke);
   tcase_add_test(tc, dir_remove);
   tcase_add_test(tc, dir_move_to);
   tcase_add_test(tc, dir_rename);
   tcase_add_test(tc, dir_copy);
   tcase_add_test(tc, dir_parent);
   tcase_add_test(tc, dir_root_parent);
   tcase_add_test(tc, dir_metadata);
   tcase_add_test(tc, file_remove);
   tcase_add_test(tc, file_move_to);
   tcase_add_test(tc, file_rename);
   tcase_add_test(tc, file_copy);
   tcase_add_test(tc, file_parent);
   tcase_add_test(tc, file_metadata);
}
