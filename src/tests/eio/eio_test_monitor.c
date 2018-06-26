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

static Eina_Bool _check_event_path(void *data, void *event)
{
   const char *f = ((Eio_Monitor_Event*)event)->filename;
    /* ignore directory creation events */
   if (ecore_file_is_dir(f)) return ECORE_CALLBACK_RENEW;
   ck_assert_str_eq((const char*)data, f);
   return EINA_TRUE;
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

EFL_START_TEST(eio_test_monitor_add_and_remove)
{
   Eina_Tmpstr *dirname = _common_init();
   Eina_Stringshare *filename;
   Eio_Monitor *monitor;

   filename = eina_stringshare_printf("%s/eio_test_monitor_add_and_remove", dirname);
   _create_directory((void*)filename);

   //monitor directory
   monitor = eio_monitor_add(filename);

   eio_monitor_del(monitor);

   _common_shutdown(dirname);
}
EFL_END_TEST

EFL_START_TEST(eio_test_monitor_add_remove_add)
{
   Eina_Tmpstr *dirname = _common_init();
   Eina_Stringshare *filename;
   Eio_Monitor *monitor1;
   Eio_Monitor *monitor2;

   filename = eina_stringshare_printf("%s/eio_test_monitor_add_remove_add", dirname);
   _create_directory((void*)filename);

   //monitor directory
   monitor1 = eio_monitor_add(filename);
   eio_monitor_del(monitor1);

   monitor2 = eio_monitor_add(filename);
   eio_monitor_del(monitor2);

   _common_shutdown(dirname);
}
EFL_END_TEST

EFL_START_TEST(eio_test_monitor_add_add_remove_remove)
{
   Eina_Tmpstr *dirname = _common_init();
   Eina_Stringshare *filename1;
   Eina_Stringshare *filename2;
   Eio_Monitor *monitor1;
   Eio_Monitor *monitor2;

   filename1 = eina_stringshare_printf("%s/eio_test_monitor_add_add_remove_remove", dirname);
   filename2 = eina_stringshare_printf("%s/eio_test_monitor_add_add_remove_remove", dirname);
   _create_directory((void*)filename1);
   _create_directory((void*)filename2);

   //monitor directory
   monitor1 = eio_monitor_add(filename1);
   monitor2 = eio_monitor_add(filename2);

   eio_monitor_del(monitor2);
   eio_monitor_del(monitor1);

   _common_shutdown(dirname);
}
EFL_END_TEST

static void _file_created_cb(void *data, int type, void *event)
{
   ck_assert_int_eq(type, (int)EIO_MONITOR_FILE_CREATED);
   if (_check_event_path(data, event))
     {
        _cancel_timeout();
        ecore_main_loop_quit();
     }
}


EFL_START_TEST(eio_test_monitor_directory_file_created_notify)
{
   Eina_Tmpstr *dirname = _common_init();
   Eina_Stringshare *filename;

   filename = eina_stringshare_printf("%s/eio_test_monitor_directory_file_created_notify", dirname);

   //monitor directory
   eio_monitor_add(dirname);
   ecore_event_handler_add(EIO_MONITOR_FILE_CREATED, (Ecore_Event_Handler_Cb)_file_created_cb, filename);

   ecore_timer_add(TEST_OPERATION_DELAY, _create_file, filename);

   ecore_main_loop_begin();

   _common_shutdown(dirname);
}
EFL_END_TEST

struct {
   Eina_Stringshare *s;
   Eina_Bool checked;
} multi_files[] = {
  { NULL, EINA_FALSE },
  { NULL, EINA_FALSE },
  { NULL, EINA_FALSE },
  { NULL, EINA_FALSE }
};

static void
_multi_file_created_cb(void *data EINA_UNUSED, int type, void *event)
{
   Eio_Monitor_Event *ev = event;
   Eina_Bool checked = EINA_TRUE;
   int i;

   ck_assert_int_eq(type, (int)EIO_MONITOR_FILE_CREATED);

   for (i = 0; multi_files[i].s && strcmp(multi_files[i].s, ev->filename); i++)
     ;

   if (!multi_files[i].s) return ;
   multi_files[i].checked = EINA_TRUE;

   for (i = 0; multi_files[i].s; i++)
     checked &= multi_files[i].checked;

   if (!checked) return ;

   _cancel_timeout();
   ecore_main_loop_quit();
}

EFL_START_TEST(eio_test_monitor_directory_multi_file_created_notify)
{
   Eina_Tmpstr *dirname = _common_init();

   multi_files[0].s = eina_stringshare_printf("%s/eio_test_monitor_directory_m1file_created_notify", dirname);
   multi_files[1].s = eina_stringshare_printf("%s/eio_test_monitor_directory_m2file_created_notify", dirname);
   multi_files[2].s = eina_stringshare_printf("%s/eio_test_monitor_directory_m3file_created_notify", dirname);

   //monitor directory
   eio_monitor_add(dirname);
   ecore_event_handler_add(EIO_MONITOR_FILE_CREATED, (Ecore_Event_Handler_Cb)_multi_file_created_cb, NULL);

   ecore_timer_add(TEST_OPERATION_DELAY, _create_file, multi_files[0].s);
   ecore_timer_add(TEST_OPERATION_DELAY, _create_file, multi_files[1].s);
   ecore_timer_add(TEST_OPERATION_DELAY + TEST_OPERATION_DELAY, _create_file, multi_files[2].s);

   ecore_main_loop_begin();

   _common_shutdown(dirname);
}
EFL_END_TEST

static void _file_deleted_cb(void *data, int type, void *event)
{
   ck_assert_int_eq(type, (int)EIO_MONITOR_FILE_DELETED);
   if (_check_event_path(data, event))
     {
        _cancel_timeout();
        ecore_main_loop_quit();
     }
}

EFL_START_TEST(eio_test_monitor_directory_file_deleted_notify)
{
   Eina_Tmpstr *dirname = _common_init();
   Eina_Stringshare *filename;

   filename = eina_stringshare_printf("%s/eio_test_monitor_directory_file_deleted_notify", dirname);
   _create_file((void*)filename);

   //monitor directory
   eio_monitor_add(dirname);
   ecore_event_handler_add(EIO_MONITOR_FILE_DELETED, (Ecore_Event_Handler_Cb)_file_deleted_cb, filename);

   ecore_timer_add(TEST_OPERATION_DELAY, _delete_file, filename);

   ecore_main_loop_begin();

   _common_shutdown(dirname);
}
EFL_END_TEST

static void _file_modified_cb(void *data, int type, void *event)
{
   ck_assert_int_eq(type, (int)EIO_MONITOR_FILE_MODIFIED);
   if(_check_event_path(data, event))
     {
        _cancel_timeout();
        ecore_main_loop_quit();
     }
}

EFL_START_TEST(eio_test_monitor_directory_file_modified_notify)
{
   Eina_Tmpstr *dirname = _common_init();
   Eina_Stringshare *filename;

   filename = eina_stringshare_printf("%s/eio_test_monitor_directory_file_modified_notify", dirname);
   _create_file((void*)filename);

   //monitor directory
   eio_monitor_add(dirname);
   ecore_event_handler_add(EIO_MONITOR_FILE_MODIFIED, (Ecore_Event_Handler_Cb)_file_modified_cb, filename);

   ecore_timer_add(TEST_OPERATION_DELAY, _modify_file, filename);

   ecore_main_loop_begin();

   //cleanup
   _common_shutdown(dirname);
}
EFL_END_TEST

static void _file_closed_cb(void *data, int type, void *event)
{
   ck_assert_int_eq(type, (int)EIO_MONITOR_FILE_CLOSED);
   if (_check_event_path(data, event))
     {
        _cancel_timeout();
        ecore_main_loop_quit();
     }
}

EFL_START_TEST(eio_test_monitor_directory_file_closed_notify)
{
   Eina_Tmpstr *dirname = _common_init();
   Eina_Stringshare *filename;

   filename = eina_stringshare_printf("%s/eio_test_monitor_directory_file_closed_notify", dirname);
   _create_file((void*)filename);

   //monitor directory
   eio_monitor_add(dirname);
   ecore_event_handler_add(EIO_MONITOR_FILE_CLOSED, (Ecore_Event_Handler_Cb)_file_closed_cb, filename);
   ecore_timer_add(TEST_OPERATION_DELAY, _modify_file, filename);

   ecore_main_loop_begin();

   _common_shutdown(dirname);
}
EFL_END_TEST

static void _directory_created_cb(void *data, int type, void *event)
{
   ck_assert_int_eq(type, (int)EIO_MONITOR_DIRECTORY_CREATED);
   if (_check_event_path(data, event))
     {
        _cancel_timeout();
        ecore_main_loop_quit();
     }
}

EFL_START_TEST(eio_test_monitor_directory_directory_created_notify)
{
   Eina_Tmpstr *dirname = _common_init();
   Eina_Stringshare *filename;

   filename = eina_stringshare_printf("%s/eio_test_monitor_directory_directory_created_notify", dirname);

   //monitor directory
   eio_monitor_add(dirname);
   ecore_event_handler_add(EIO_MONITOR_DIRECTORY_CREATED, (Ecore_Event_Handler_Cb)_directory_created_cb, filename);

   ecore_timer_add(TEST_OPERATION_DELAY, _create_directory, filename);

   ecore_main_loop_begin();

   _common_shutdown(dirname);
}
EFL_END_TEST

static void _directory_deleted_cb(void *data, int type, void *event)
{
   ck_assert_int_eq(type, (int)EIO_MONITOR_DIRECTORY_DELETED);
   if (_check_event_path(data, event))
     {
        _cancel_timeout();
        ecore_main_loop_quit();
     }
}

EFL_START_TEST(eio_test_monitor_directory_directory_deleted_notify)
{
   Eina_Tmpstr *dirname = _common_init();
   Eina_Stringshare *filename;

   filename = eina_stringshare_printf("%s/eio_test_monitor_directory_directory_deleted_notify", dirname);
   _create_directory((void*)filename);

   //monitor directory
   eio_monitor_add(dirname);
   ecore_event_handler_add(EIO_MONITOR_DIRECTORY_DELETED, (Ecore_Event_Handler_Cb)_directory_deleted_cb, filename);

   ecore_timer_add(TEST_OPERATION_DELAY, _delete_directory, filename);

   ecore_main_loop_begin();

   _common_shutdown(dirname);
}
EFL_END_TEST

static void _directory_modified_cb(void *data, int type, void *event)
{
   ck_assert_int_eq(type, (int)EIO_MONITOR_DIRECTORY_MODIFIED);
   if (_check_event_path(data, event))
     {
        _cancel_timeout();
        ecore_main_loop_quit();
     }
}


EFL_START_TEST(eio_test_monitor_directory_directory_modified_notify)
{
   Eina_Tmpstr *dirname = _common_init();
   Eina_Stringshare *filename;

   filename = eina_stringshare_printf("%s/eio_test_monitor_directory_directory_modified_notify", dirname);
   _create_directory((void*)filename);

   //monitor directory
   eio_monitor_add(dirname);
   ecore_event_handler_add(EIO_MONITOR_DIRECTORY_MODIFIED, (Ecore_Event_Handler_Cb)_directory_modified_cb, filename);

   ecore_timer_add(TEST_OPERATION_DELAY, _modify_attrib_file, filename);

   ecore_main_loop_begin();

   _common_shutdown(dirname);
}
EFL_END_TEST


static void _directory_self_deleted_cb(void *data, int type, void *event)
{
   ck_assert_int_eq(type, (int)EIO_MONITOR_SELF_DELETED);
   if (_check_event_path(data, event))
     {
        _cancel_timeout();
        ecore_main_loop_quit();
     }
}


EFL_START_TEST(eio_test_monitor_directory_directory_self_deleted_notify)
{
   Eina_Tmpstr *dirname = _common_init();

   //monitor directory
   eio_monitor_add(dirname);
   ecore_event_handler_add(EIO_MONITOR_SELF_DELETED, (Ecore_Event_Handler_Cb)_directory_self_deleted_cb, dirname);

   ecore_timer_add(TEST_OPERATION_DELAY, _delete_directory, dirname);

   ecore_main_loop_begin();

   _common_shutdown(dirname);
}
EFL_END_TEST

//////// test monitoring a single file

EFL_START_TEST(eio_test_monitor_file_file_modified_notify)
{
   Eina_Tmpstr *dirname = _common_init();
   Eina_Stringshare *filename;

   filename = eina_stringshare_printf("%s/filecreated", dirname);
   _create_file((void*)filename);

   //monitor file
   eio_monitor_add(filename);
   ecore_event_handler_add(EIO_MONITOR_FILE_MODIFIED, (Ecore_Event_Handler_Cb)_file_modified_cb, filename);

   ecore_timer_add(TEST_OPERATION_DELAY, _modify_file, filename);

   ecore_main_loop_begin();

   //cleanup
   _common_shutdown(dirname);
}
EFL_END_TEST

EFL_START_TEST(eio_test_monitor_file_file_attrib_modified_notify)
{
   Eina_Tmpstr *dirname = _common_init();
   Eina_Stringshare *filename;

   filename = eina_stringshare_printf("%s/eio_test_monitor_file_file_attrib_modified_notify", dirname);
   _create_file((void*)filename);

   //monitor file
   eio_monitor_add(filename);
   ecore_event_handler_add(EIO_MONITOR_FILE_MODIFIED, (Ecore_Event_Handler_Cb)_file_modified_cb, filename);

   ecore_timer_add(TEST_OPERATION_DELAY, _modify_attrib_file, filename);

   ecore_main_loop_begin();

   _common_shutdown(dirname);
}
EFL_END_TEST


EFL_START_TEST(eio_test_monitor_file_file_closed_notify)
{
   Eina_Tmpstr *dirname = _common_init();
   Eina_Stringshare *filename;

   filename = eina_stringshare_printf("%s/eio_test_monitor_file_file_closed_notify", dirname);
   _create_file((void*)filename);

   //monitor file
   eio_monitor_add(dirname);
   ecore_event_handler_add(EIO_MONITOR_FILE_CLOSED, (Ecore_Event_Handler_Cb)_file_closed_cb, filename);
   ecore_timer_add(TEST_OPERATION_DELAY, _modify_file, filename);

   ecore_main_loop_begin();

   _common_shutdown(dirname);
}
EFL_END_TEST

EFL_START_TEST(eio_test_monitor_file_file_self_deleted_notify)
{
   Eina_Tmpstr *dirname = _common_init();
   Eina_Stringshare *filename;

   filename = eina_stringshare_printf("%s/eio_test_monitor_file_file_self_deleted_notify", dirname);
   _create_file((void*)filename);

   //monitor file
   eio_monitor_add(filename);
   ecore_event_handler_add(EIO_MONITOR_SELF_DELETED, (Ecore_Event_Handler_Cb)_directory_self_deleted_cb, filename);

   ecore_timer_add(TEST_OPERATION_DELAY, _delete_file, filename);

   ecore_main_loop_begin();

   _common_shutdown(dirname);
}
EFL_END_TEST

EFL_START_TEST(eio_test_monitor_two_monitors_one_event)
{
   Eina_Tmpstr *dirname = _common_init();
   Eina_Tmpstr *dirname2;

   Eina_Stringshare *filename;

   eina_file_mkdtemp("checkFileCreationXXXXXX", &dirname2);
   filename = eina_stringshare_printf("%s/eio_test_monitor_two_monitors_one_event", dirname);

   //monitor directory
   eio_monitor_add(dirname);
   eio_monitor_add(dirname2);
   ecore_event_handler_add(EIO_MONITOR_FILE_CREATED, (Ecore_Event_Handler_Cb)_file_created_cb, filename);

   ecore_timer_add(TEST_OPERATION_DELAY, _create_file, filename);

   ecore_main_loop_begin();

   _delete_directory((void*)dirname2);
   _common_shutdown(dirname);
}
EFL_END_TEST


EFL_START_TEST(eio_test_monitor_two_monitors_one_removed_one_event)
{
   Eina_Tmpstr *dirname = _common_init();
   Eina_Tmpstr *dirname2;

   Eina_Stringshare *filename;

   Eio_Monitor *monitor;

   eina_file_mkdtemp("checkFileCreationXXXXXX", &dirname2);
   filename = eina_stringshare_printf("%s/eio_test_monitor_two_monitors_one_removed", dirname);

   //monitor directory
   monitor = eio_monitor_add(dirname2);
   eio_monitor_add(dirname);
   eio_monitor_del(monitor);
   ecore_event_handler_add(EIO_MONITOR_FILE_CREATED, (Ecore_Event_Handler_Cb)_file_created_cb, filename);

   ecore_timer_add(TEST_OPERATION_DELAY, _create_file, filename);

   ecore_main_loop_begin();

   _delete_directory((void*)dirname2);
   _common_shutdown(dirname);
}
EFL_END_TEST

static void _unexpected_event_cb(void *data EINA_UNUSED, int type EINA_UNUSED, void *event EINA_UNUSED)
{
   ck_abort_msg("unexpected event");
}

EFL_START_TEST(eio_test_monitor_two_monitors_one_removed_no_event)
{
   Eina_Tmpstr *dirname = _common_init();
   Eina_Tmpstr *dirname2;

   Eina_Stringshare *filename;

   Eio_Monitor *monitor;

   eina_file_mkdtemp("checkFileCreationXXXXXX", &dirname2);
   filename = eina_stringshare_printf("%s/eio_test_monitor_two_monitors_one_removed", dirname);

   //monitor directory
   monitor = eio_monitor_add(dirname);
   eio_monitor_add(dirname2);
   eio_monitor_del(monitor);
   ecore_event_handler_add(EIO_MONITOR_FILE_CREATED, (Ecore_Event_Handler_Cb)_unexpected_event_cb, filename);

   ecore_timer_add(TEST_OPERATION_DELAY, _create_file, filename);
   ecore_timer_add(0.2, _test_timeout_expected, NULL);

   ecore_main_loop_begin();

   _delete_directory((void*)dirname2);
   _common_shutdown(dirname);
}
EFL_END_TEST

EFL_START_TEST(eio_test_monitor_two_files_in_same_directory)
{
   Eina_Tmpstr *dirname = _common_init();
   Eina_Stringshare *filename;
   Eina_Stringshare *filename2;

   filename = eina_stringshare_printf("%s/eio_test_monitor_two_files_in_same_directory_1", dirname);
   filename2 = eina_stringshare_printf("%s/eio_test_monitor_two_files_in_same_directory_2", dirname);
   _create_file((void*)filename);
   _create_file((void*)filename2);

   //monitor file
   eio_monitor_add(filename);
   eio_monitor_add(filename2);
   ecore_event_handler_add(EIO_MONITOR_FILE_MODIFIED, (Ecore_Event_Handler_Cb)_file_modified_cb, filename);

   ecore_timer_add(TEST_OPERATION_DELAY, _modify_file, filename);

   ecore_main_loop_begin();

   //cleanup
   _common_shutdown(dirname);
}
EFL_END_TEST


EFL_START_TEST(eio_test_monitor_two_files_in_same_directory_one_removed)
{
   Eina_Tmpstr *dirname = _common_init();
   Eina_Stringshare *filename;
   Eina_Stringshare *filename2;
   Eio_Monitor *monitor;

   filename = eina_stringshare_printf("%s/eio_test_monitor_two_files_in_same_directory_one_removed_1", dirname);
   filename2 = eina_stringshare_printf("%s/eio_test_monitor_two_files_in_same_directory_one_removed_2", dirname);
   _create_file((void*)filename);
   _create_file((void*)filename2);

   //monitor file
   monitor = eio_monitor_add(filename);
   eio_monitor_add(filename2);
   eio_monitor_del(monitor);

   ecore_event_handler_add(EIO_MONITOR_FILE_MODIFIED, (Ecore_Event_Handler_Cb)_unexpected_event_cb, filename);

   ecore_timer_add(TEST_OPERATION_DELAY, _modify_file, filename);
   ecore_timer_add(0.2, _test_timeout_expected, NULL);

   ecore_main_loop_begin();

   //cleanup
   _common_shutdown(dirname);
}
EFL_END_TEST


void eio_test_monitor(TCase *tc)
{
   tcase_add_test(tc, eio_test_monitor_add_and_remove);
   tcase_add_test(tc, eio_test_monitor_add_remove_add);
   tcase_add_test(tc, eio_test_monitor_add_add_remove_remove);

   tcase_add_test(tc, eio_test_monitor_directory_file_created_notify);
   tcase_add_test(tc, eio_test_monitor_directory_multi_file_created_notify);
   tcase_add_test(tc, eio_test_monitor_directory_file_deleted_notify);
   tcase_add_test(tc, eio_test_monitor_directory_file_modified_notify);
#if !defined(_WIN32) && !defined(__MACH__)
   tcase_add_test(tc, eio_test_monitor_directory_file_closed_notify);
#endif
   tcase_add_test(tc, eio_test_monitor_directory_directory_created_notify);
   tcase_add_test(tc, eio_test_monitor_directory_directory_deleted_notify);
   tcase_add_test(tc, eio_test_monitor_directory_directory_modified_notify);
#ifndef __MACH__
   tcase_add_test(tc, eio_test_monitor_directory_directory_self_deleted_notify);
#endif

   tcase_add_test(tc, eio_test_monitor_file_file_modified_notify);
   tcase_add_test(tc, eio_test_monitor_file_file_attrib_modified_notify);
#if !defined(_WIN32) && !defined(__MACH__)
   tcase_add_test(tc, eio_test_monitor_file_file_closed_notify);
#endif
#ifndef __MACH__
   tcase_add_test(tc, eio_test_monitor_file_file_self_deleted_notify);
#endif

   tcase_add_test(tc, eio_test_monitor_two_monitors_one_event);
   tcase_add_test(tc, eio_test_monitor_two_monitors_one_removed_one_event);
   tcase_add_test(tc, eio_test_monitor_two_monitors_one_removed_no_event);
   tcase_add_test(tc, eio_test_monitor_two_files_in_same_directory);
   tcase_add_test(tc, eio_test_monitor_two_files_in_same_directory_one_removed);
}
