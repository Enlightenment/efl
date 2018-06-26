#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include <Ecore.h>
#include <Ecore_File.h>
#include <Eio.h>

#include "eio_suite.h"

/////////////////timeout function

#define TEST_TIMEOUT_SEC 0.5
#define TEST_OPERATION_DELAY 0.05

static Ecore_Timer *test_timeout_timer;

static Eina_Bool _test_timeout_cb(void *data EINA_UNUSED)
{
   ck_abort_msg("test timeout");
   ecore_main_loop_quit();
   return ECORE_CALLBACK_CANCEL;
}

static void _cancel_timeout()
{
   if (test_timeout_timer != NULL)
     {
        ecore_timer_del (test_timeout_timer);
        test_timeout_timer = NULL;
     }
}

static Eina_Bool _test_timeout_expected(void *data EINA_UNUSED)
{
   if (test_timeout_timer != NULL)
     {
        _cancel_timeout();
     }
   ecore_main_loop_quit();
   return ECORE_CALLBACK_CANCEL;
}

///////////////// file and directory operations

typedef struct {
   const char *src;
   const char *dst;
} RenameOperation;

static Eina_Bool _delete_directory(void *data)
{
   const char *dirname = (const char*)data;
   if (ecore_file_is_dir(dirname))
     {
        ecore_file_recursive_rm(dirname);
     }
   return ECORE_CALLBACK_CANCEL;
}

static Eina_Bool _create_directory(void *data)
{
   const char *dirname = (const char*)data;
   ecore_file_mkpath(dirname);
   return ECORE_CALLBACK_CANCEL;
}


static Eina_Bool _create_file(void *data)
{
   FILE *fd = fopen((const char*)data, "wb+");
   ck_assert_ptr_ne(fd, NULL);
   fprintf(fd, "test test");
   fclose(fd);
   return ECORE_CALLBACK_CANCEL;
}

static Eina_Bool _delete_file(void *data)
{
   Eina_Bool file_removed = ecore_file_remove((const char*)data);
   ck_assert(file_removed);
   return ECORE_CALLBACK_CANCEL;
}

static Eina_Bool _modify_file(void *data)
{
   FILE *fd = fopen((const char*)data, "ab");
   ck_assert_ptr_ne(fd, NULL);
   fprintf(fd, "appened");
   fclose(fd);
   return ECORE_CALLBACK_CANCEL;
}

static Eina_Bool _modify_attrib_file(void *data)
{
   int ret = chmod((const char*)data, 0666);
   ck_assert_int_eq(ret, 0);
   return ECORE_CALLBACK_CANCEL;
}

/////// helper functions

static void _check_event_path(void *data, const Efl_Event *event)
{
   Eio_Sentry_Event *event_info = event->info;
   /* ignore directory creation events */
   if (ecore_file_is_dir(event_info->trigger)) return;
   ck_assert_str_eq((const char*)data, event_info->trigger);
}

static Eina_Tmpstr *_common_init()
{
   Eina_Tmpstr *dirname;
   ecore_file_init();

   //test timeout
   test_timeout_timer = ecore_timer_add(TEST_TIMEOUT_SEC, _test_timeout_cb, NULL);

   eina_file_mkdtemp("checkFileCreationXXXXXX", &dirname);
   return dirname;
}

static void _common_shutdown(Eina_Tmpstr *dirname)
{
   _delete_directory((void*)dirname);
   ecore_file_shutdown();
   eina_tmpstr_del(dirname);
   _cancel_timeout();
}

/////// tests monitoring a directory

EFL_START_TEST(eio_test_sentry_add_and_remove)
{
   Eina_Tmpstr *dirname = _common_init();
   Eina_Stringshare *filename;
   Eo *sentry;

   filename = eina_stringshare_printf("%s/eio_test_sentry_add_and_remove", dirname);
   _create_directory((void*)filename);

   //monitor directory
   sentry = efl_add_ref(EIO_SENTRY_CLASS, NULL);
   fail_if(!eio_sentry_add(sentry, filename));

   eio_sentry_remove(sentry, filename);

   efl_unref(sentry);

   _common_shutdown(dirname);
}
EFL_END_TEST

EFL_START_TEST(eio_test_sentry_add_remove_add)
{
   Eina_Tmpstr *dirname = _common_init();
   Eina_Stringshare *filename;
   Eo* sentry;

   filename = eina_stringshare_printf("%s/eio_test_sentry_add_remove_add", dirname);
   _create_directory((void*)filename);

   //monitor directory
   sentry = efl_add_ref(EIO_SENTRY_CLASS, NULL);
   fail_if(!eio_sentry_add(sentry, filename));
   eio_sentry_remove(sentry, filename);

   fail_if(!eio_sentry_add(sentry, filename));
   eio_sentry_remove(sentry, filename);

   efl_unref(sentry);

   _common_shutdown(dirname);
}
EFL_END_TEST

EFL_START_TEST(eio_test_sentry_add_add_remove_remove)
{
   Eina_Tmpstr *dirname = _common_init();
   Eina_Stringshare *filename1;
   Eina_Stringshare *filename2;
   Eo *sentry;

   filename1 = eina_stringshare_printf("%s/eio_test_sentry_add_add_remove_remove", dirname);
   filename2 = eina_stringshare_printf("%s/eio_test_sentry_add_add_remove_remove", dirname);
   _create_directory((void*)filename1);
   _create_directory((void*)filename2);

   sentry = efl_add_ref(EIO_SENTRY_CLASS, NULL);

   //monitor directory
   fail_if(!eio_sentry_add(sentry, filename1));
   fail_if(!eio_sentry_add(sentry, filename2));

   eio_sentry_remove(sentry, filename1);
   eio_sentry_remove(sentry, filename2);

   efl_unref(sentry);

   _common_shutdown(dirname);
}
EFL_END_TEST

static void _target_notified_cb(void *data, const Efl_Event *event)
{
   _check_event_path(data, event);
   _cancel_timeout();
   ecore_main_loop_quit();
}


EFL_START_TEST(eio_test_sentry_directory_file_created_notify)
{
   Eina_Tmpstr *dirname = _common_init();
   Eina_Stringshare *filename;
   Eo* sentry;

   filename = eina_stringshare_printf("%s/eio_test_sentry_directory_file_created_notify", dirname);
   sentry = efl_add_ref(EIO_SENTRY_CLASS, NULL);

   //monitor directory
   fail_if(!eio_sentry_add(sentry, dirname));
   efl_event_callback_add(sentry, EIO_SENTRY_EVENT_FILE_CREATED, (Efl_Event_Cb)_target_notified_cb, filename);

   ecore_timer_add(TEST_OPERATION_DELAY, _create_file, filename);

   ecore_main_loop_begin();

   efl_unref(sentry);

   _common_shutdown(dirname);
}
EFL_END_TEST

EFL_START_TEST(eio_test_sentry_directory_file_deleted_notify)
{
   Eina_Tmpstr *dirname = _common_init();
   Eina_Stringshare *filename;
   Eo *sentry;

   filename = eina_stringshare_printf("%s/eio_test_sentry_directory_file_deleted_notify", dirname);
   _create_file((void*)filename);
   sentry = efl_add_ref(EIO_SENTRY_CLASS, NULL);

   //monitor directory
   fail_if(!eio_sentry_add(sentry, dirname));
   efl_event_callback_add(sentry, EIO_SENTRY_EVENT_FILE_DELETED, (Efl_Event_Cb)_target_notified_cb, filename);

   ecore_timer_add(TEST_OPERATION_DELAY, _delete_file, filename);

   ecore_main_loop_begin();

   efl_unref(sentry);

   _common_shutdown(dirname);
}
EFL_END_TEST

EFL_START_TEST(eio_test_sentry_directory_file_modified_notify)
{
   Eina_Tmpstr *dirname = _common_init();
   Eina_Stringshare *filename;
   Eo *sentry;

   filename = eina_stringshare_printf("%s/eio_test_sentry_directory_file_modified_notify", dirname);
   _create_file((void*)filename);
   sentry = efl_add_ref(EIO_SENTRY_CLASS, NULL);

   //monitor directory
   fail_if(!eio_sentry_add(sentry, dirname));
   efl_event_callback_add(sentry, EIO_SENTRY_EVENT_FILE_MODIFIED, (Efl_Event_Cb)_target_notified_cb, filename);

   ecore_timer_add(TEST_OPERATION_DELAY, _modify_file, filename);

   ecore_main_loop_begin();

   efl_unref(sentry);

   //cleanup
   _common_shutdown(dirname);
}
EFL_END_TEST

EFL_START_TEST(eio_test_sentry_directory_file_closed_notify)
{
   Eina_Tmpstr *dirname = _common_init();
   Eina_Stringshare *filename;
   Eo *sentry;

   filename = eina_stringshare_printf("%s/eio_test_sentry_directory_file_closed_notify", dirname);
   _create_file((void*)filename);
   sentry = efl_add_ref(EIO_SENTRY_CLASS, NULL);

   //monitor directory
   fail_if(!eio_sentry_add(sentry, dirname));
   efl_event_callback_add(sentry, EIO_SENTRY_EVENT_FILE_CLOSED, (Efl_Event_Cb)_target_notified_cb, filename);
   ecore_timer_add(TEST_OPERATION_DELAY, _modify_file, filename);

   ecore_main_loop_begin();

   efl_unref(sentry);

   _common_shutdown(dirname);
}
EFL_END_TEST

EFL_START_TEST(eio_test_sentry_directory_directory_created_notify)
{
   Eina_Tmpstr *dirname = _common_init();
   Eina_Stringshare *filename;
   Eo *sentry;

   filename = eina_stringshare_printf("%s/eio_test_sentry_directory_directory_created_notify", dirname);
   sentry = efl_add_ref(EIO_SENTRY_CLASS, NULL);

   //monitor directory
   fail_if(!eio_sentry_add(sentry, dirname));
   efl_event_callback_add(sentry, EIO_SENTRY_EVENT_DIRECTORY_CREATED, (Efl_Event_Cb)_target_notified_cb, filename);

   ecore_timer_add(TEST_OPERATION_DELAY, _create_directory, filename);

   ecore_main_loop_begin();

   efl_unref(sentry);

   _common_shutdown(dirname);
}
EFL_END_TEST

EFL_START_TEST(eio_test_sentry_directory_directory_deleted_notify)
{
   Eina_Tmpstr *dirname = _common_init();
   Eina_Stringshare *filename;
   Eo *sentry;

   filename = eina_stringshare_printf("%s/eio_test_sentry_directory_directory_deleted_notify", dirname);
   _create_directory((void*)filename);
   sentry = efl_add_ref(EIO_SENTRY_CLASS, NULL);

   //monitor directory
   fail_if(!eio_sentry_add(sentry, dirname));
   efl_event_callback_add(sentry, EIO_SENTRY_EVENT_DIRECTORY_DELETED, (Efl_Event_Cb)_target_notified_cb, filename);

   ecore_timer_add(TEST_OPERATION_DELAY, _delete_directory, filename);

   ecore_main_loop_begin();

   efl_unref(sentry);

   _common_shutdown(dirname);
}
EFL_END_TEST

EFL_START_TEST(eio_test_sentry_directory_directory_modified_notify)
{
   Eina_Tmpstr *dirname = _common_init();
   Eina_Stringshare *filename;
   Eo *sentry;

   filename = eina_stringshare_printf("%s/eio_test_sentry_directory_directory_modified_notify", dirname);
   _create_directory((void*)filename);
   sentry = efl_add_ref(EIO_SENTRY_CLASS, NULL);

   //monitor directory
   fail_if(!eio_sentry_add(sentry, dirname));
   efl_event_callback_add(sentry, EIO_SENTRY_EVENT_DIRECTORY_MODIFIED, (Efl_Event_Cb)_target_notified_cb, filename);

   ecore_timer_add(TEST_OPERATION_DELAY, _modify_attrib_file, filename);

   ecore_main_loop_begin();

   efl_unref(sentry);

   _common_shutdown(dirname);
}
EFL_END_TEST

EFL_START_TEST(eio_test_sentry_directory_directory_self_deleted_notify)
{
   Eina_Tmpstr *dirname = _common_init();
   Eo *sentry = efl_add_ref(EIO_SENTRY_CLASS, NULL);

   //monitor directory
   fail_if(!eio_sentry_add(sentry, dirname));
   efl_event_callback_add(sentry, EIO_SENTRY_EVENT_SELF_DELETED, (Efl_Event_Cb)_target_notified_cb, dirname);

   ecore_timer_add(TEST_OPERATION_DELAY, _delete_directory, dirname);

   ecore_main_loop_begin();

   efl_unref(sentry);

   _common_shutdown(dirname);
}
EFL_END_TEST

// //////// test monitoring a single file

EFL_START_TEST(eio_test_sentry_file_file_modified_notify)
{
   Eina_Tmpstr *dirname = _common_init();
   Eina_Stringshare *filename;
   Eo *sentry;

   filename = eina_stringshare_printf("%s/filecreated", dirname);
   _create_file((void*)filename);
   sentry = efl_add_ref(EIO_SENTRY_CLASS, NULL);

   //monitor file
   fail_if(!eio_sentry_add(sentry, filename));
   efl_event_callback_add(sentry, EIO_SENTRY_EVENT_FILE_MODIFIED, (Efl_Event_Cb)_target_notified_cb, filename);

   ecore_timer_add(TEST_OPERATION_DELAY, _modify_file, filename);

   ecore_main_loop_begin();

   efl_unref(sentry);

   //cleanup
   _common_shutdown(dirname);
}
EFL_END_TEST

EFL_START_TEST(eio_test_sentry_file_file_attrib_modified_notify)
{
   Eina_Tmpstr *dirname = _common_init();
   Eina_Stringshare *filename;
   Eo *sentry;

   filename = eina_stringshare_printf("%s/eio_test_sentry_file_file_attrib_modified_notify", dirname);
   _create_file((void*)filename);
   sentry = efl_add_ref(EIO_SENTRY_CLASS, NULL);

   //monitor file
   fail_if(!eio_sentry_add(sentry, filename));
   efl_event_callback_add(sentry, EIO_SENTRY_EVENT_FILE_MODIFIED, (Efl_Event_Cb)_target_notified_cb, filename);

   ecore_timer_add(TEST_OPERATION_DELAY, _modify_attrib_file, filename);

   ecore_main_loop_begin();

   efl_unref(sentry);

   _common_shutdown(dirname);
}
EFL_END_TEST


EFL_START_TEST(eio_test_sentry_file_file_closed_notify)
{
   Eina_Tmpstr *dirname = _common_init();
   Eina_Stringshare *filename;
   Eo* sentry;

   filename = eina_stringshare_printf("%s/eio_test_sentry_file_file_closed_notify", dirname);
   _create_file((void*)filename);
   sentry = efl_add_ref(EIO_SENTRY_CLASS, NULL);

   //monitor file
   fail_if(!eio_sentry_add(sentry, dirname));
   efl_event_callback_add(sentry, EIO_SENTRY_EVENT_FILE_CLOSED, (Efl_Event_Cb)_target_notified_cb, filename);
   ecore_timer_add(TEST_OPERATION_DELAY, _modify_file, filename);

   ecore_main_loop_begin();

   efl_unref(sentry);

   _common_shutdown(dirname);
}
EFL_END_TEST

EFL_START_TEST(eio_test_sentry_file_file_self_deleted_notify)
{
   Eina_Tmpstr *dirname = _common_init();
   Eina_Stringshare *filename;
   Eo* sentry;

   filename = eina_stringshare_printf("%s/eio_test_sentry_file_file_self_deleted_notify", dirname);
   _create_file((void*)filename);
   sentry = efl_add_ref(EIO_SENTRY_CLASS, NULL);

   //monitor file
   fail_if(!eio_sentry_add(sentry, filename));
   efl_event_callback_add(sentry, EIO_SENTRY_EVENT_SELF_DELETED, (Efl_Event_Cb)_target_notified_cb, filename);

   ecore_timer_add(TEST_OPERATION_DELAY, _delete_file, filename);

   ecore_main_loop_begin();

   efl_unref(sentry);

   _common_shutdown(dirname);
}
EFL_END_TEST

EFL_START_TEST(eio_test_sentry_two_monitors_one_event)
{
   Eina_Tmpstr *dirname = _common_init();
   Eina_Tmpstr *dirname2;
   Eo *sentry;

   Eina_Stringshare *filename;

   eina_file_mkdtemp("checkFileCreationXXXXXX", &dirname2);
   filename = eina_stringshare_printf("%s/eio_test_sentry_two_monitors_one_event", dirname);
   sentry = efl_add_ref(EIO_SENTRY_CLASS, NULL);

   //monitor directory
   fail_if(!eio_sentry_add(sentry, dirname));
   fail_if(!eio_sentry_add(sentry, dirname2));
   efl_event_callback_add(sentry, EIO_SENTRY_EVENT_FILE_CREATED, (Efl_Event_Cb)_target_notified_cb, filename);

   ecore_timer_add(TEST_OPERATION_DELAY, _create_file, filename);

   ecore_main_loop_begin();

   efl_unref(sentry);

   _delete_directory((void*)dirname2);
   _common_shutdown(dirname);
}
EFL_END_TEST


EFL_START_TEST(eio_test_sentry_two_monitors_one_removed_one_event)
{
   Eina_Tmpstr *dirname = _common_init();
   Eina_Tmpstr *dirname2;
   Eo *sentry;

   Eina_Stringshare *filename;

   eina_file_mkdtemp("checkFileCreationXXXXXX", &dirname2);
   filename = eina_stringshare_printf("%s/eio_test_sentry_two_monitors_one_removed", dirname);
   sentry = efl_add_ref(EIO_SENTRY_CLASS, NULL);

   //monitor directory
   fail_if(!eio_sentry_add(sentry, dirname2));
   fail_if(!eio_sentry_add(sentry, dirname));
   eio_sentry_remove(sentry, dirname2);
   efl_event_callback_add(sentry, EIO_SENTRY_EVENT_FILE_CREATED, (Efl_Event_Cb)_target_notified_cb, filename);

   ecore_timer_add(TEST_OPERATION_DELAY, _create_file, filename);

   ecore_main_loop_begin();

   efl_unref(sentry);

   _delete_directory((void*)dirname2);
   _common_shutdown(dirname);
}
EFL_END_TEST

static void _unexpected_event_cb(void *data EINA_UNUSED, const Efl_Event *event EINA_UNUSED)
{
   ck_abort_msg("unexpected event");
}

EFL_START_TEST(eio_test_sentry_two_monitors_one_removed_no_event)
{
   Eina_Tmpstr *dirname = _common_init();
   Eina_Tmpstr *dirname2;
   Eo *sentry;

   Eina_Stringshare *filename;

   eina_file_mkdtemp("checkFileCreationXXXXXX", &dirname2);
   filename = eina_stringshare_printf("%s/eio_test_sentry_two_monitors_one_removed", dirname);
   sentry = efl_add_ref(EIO_SENTRY_CLASS, NULL);

   //monitor directory
   fail_if(!eio_sentry_add(sentry, dirname));
   fail_if(!eio_sentry_add(sentry, dirname2));
   eio_sentry_remove(sentry, dirname);
   efl_event_callback_add(sentry, EIO_SENTRY_EVENT_FILE_CREATED, (Efl_Event_Cb)_unexpected_event_cb, filename);

   ecore_timer_add(TEST_OPERATION_DELAY, _create_file, filename);
   ecore_timer_add(0.2, _test_timeout_expected, NULL);

   ecore_main_loop_begin();

   efl_unref(sentry);

   _delete_directory((void*)dirname2);
   _common_shutdown(dirname);
}
EFL_END_TEST

EFL_START_TEST(eio_test_sentry_two_files_in_same_directory)
{
   Eina_Tmpstr *dirname = _common_init();
   Eina_Stringshare *filename;
   Eina_Stringshare *filename2;
   Eo *sentry;

   filename = eina_stringshare_printf("%s/eio_test_sentry_two_files_in_same_directory_1", dirname);
   filename2 = eina_stringshare_printf("%s/eio_test_sentry_two_files_in_same_directory_2", dirname);
   _create_file((void*)filename);
   _create_file((void*)filename2);
   sentry = efl_add_ref(EIO_SENTRY_CLASS, NULL);

   //monitor file
   fail_if(!eio_sentry_add(sentry,filename));
   fail_if(!eio_sentry_add(sentry,filename2));
   efl_event_callback_add(sentry, EIO_SENTRY_EVENT_FILE_MODIFIED, (Efl_Event_Cb)_target_notified_cb, filename);

   ecore_timer_add(TEST_OPERATION_DELAY, _modify_file, filename);

   ecore_main_loop_begin();

   efl_unref(sentry);

   //cleanup
   _common_shutdown(dirname);
}
EFL_END_TEST


EFL_START_TEST(eio_test_sentry_two_files_in_same_directory_one_removed)
{
   Eina_Tmpstr *dirname = _common_init();
   Eina_Stringshare *filename;
   Eina_Stringshare *filename2;
   Eo *sentry;

   filename = eina_stringshare_printf("%s/eio_test_sentry_two_files_in_same_directory_one_removed_1", dirname);
   filename2 = eina_stringshare_printf("%s/eio_test_sentry_two_files_in_same_directory_one_removed_2", dirname);
   _create_file((void*)filename);
   _create_file((void*)filename2);
   sentry = efl_add_ref(EIO_SENTRY_CLASS, NULL);

   //monitor file
   fail_if(!eio_sentry_add(sentry,filename));
   fail_if(!eio_sentry_add(sentry,filename2));
   eio_sentry_remove(sentry, filename);

   efl_event_callback_add(sentry, EIO_SENTRY_EVENT_FILE_MODIFIED, (Efl_Event_Cb)_unexpected_event_cb, filename);

   ecore_timer_add(TEST_OPERATION_DELAY, _modify_file, filename);
   ecore_timer_add(0.2, _test_timeout_expected, NULL);

   ecore_main_loop_begin();

   efl_unref(sentry);

   //cleanup
   _common_shutdown(dirname);
}
EFL_END_TEST


void eio_test_sentry(TCase *tc)
{
   tcase_add_test(tc, eio_test_sentry_add_and_remove);
   tcase_add_test(tc, eio_test_sentry_add_remove_add);
   tcase_add_test(tc, eio_test_sentry_add_add_remove_remove);

   tcase_add_test(tc, eio_test_sentry_directory_file_created_notify);
   tcase_add_test(tc, eio_test_sentry_directory_file_deleted_notify);
   tcase_add_test(tc, eio_test_sentry_directory_file_modified_notify);
#if !defined(_WIN32) && !defined(__MACH__)
   tcase_add_test(tc, eio_test_sentry_directory_file_closed_notify);
#endif
   tcase_add_test(tc, eio_test_sentry_directory_directory_created_notify);
   tcase_add_test(tc, eio_test_sentry_directory_directory_deleted_notify);
   tcase_add_test(tc, eio_test_sentry_directory_directory_modified_notify);
#ifndef __MACH__
   tcase_add_test(tc, eio_test_sentry_directory_directory_self_deleted_notify);
#endif

   tcase_add_test(tc, eio_test_sentry_file_file_modified_notify);
   tcase_add_test(tc, eio_test_sentry_file_file_attrib_modified_notify);
#if !defined(_WIN32) && !defined(__MACH__)
   tcase_add_test(tc, eio_test_sentry_file_file_closed_notify);
#endif
#ifndef __MACH__
   tcase_add_test(tc, eio_test_sentry_file_file_self_deleted_notify);
#endif

   tcase_add_test(tc, eio_test_sentry_two_monitors_one_event);
   tcase_add_test(tc, eio_test_sentry_two_monitors_one_removed_one_event);
   tcase_add_test(tc, eio_test_sentry_two_monitors_one_removed_no_event);
   tcase_add_test(tc, eio_test_sentry_two_files_in_same_directory);
   tcase_add_test(tc, eio_test_sentry_two_files_in_same_directory_one_removed);
}
