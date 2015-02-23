#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include <Eio.h>
#include <Ecore.h>
#include <Ecore_File.h>

#include "eio_suite.h"

/////////////////timeout function

#define TEST_TIMEOUT_SEC 10
#define TEST_OPERATION_DELAY 0.5

static Ecore_Timer * test_timeout_timer;

static Eina_Bool _test_timeout_cb(void* data EINA_UNUSED)
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

///////////////// file and directory operations

typedef struct {
   const char* src;
   const char* dst;
} RenameOperation;

static Eina_Bool _delete_directory(void* data)
{
   const char* dirname = (const char*)data;
   if (ecore_file_is_dir(dirname))
     {
        ecore_file_recursive_rm(dirname);
     }
   return ECORE_CALLBACK_CANCEL;
}

static Eina_Bool _create_directory(void* data)
{
   const char* dirname = (const char*)data;
   ecore_file_mkpath(dirname);
   return ECORE_CALLBACK_CANCEL;
}


static Eina_Bool _create_file(void* data)
{
   FILE* fd = fopen((const char*)data, "w+");
   ck_assert_ptr_ne(fd, NULL);
   fprintf(fd, "test test");
   fclose(fd);
   return ECORE_CALLBACK_CANCEL;
}

static Eina_Bool _delete_file(void* data)
{
   Eina_Bool file_removed = ecore_file_remove((const char*)data);
   ck_assert(file_removed);
   return ECORE_CALLBACK_CANCEL;
}

static Eina_Bool _modify_file(void* data)
{
   FILE* fd = fopen((const char*)data, "a");
   ck_assert_ptr_ne(fd, NULL);
   fprintf(fd, "appened");
   fclose(fd);
   return ECORE_CALLBACK_CANCEL;
}

static Eina_Bool _modify_attrib_file(void* data)
{
   int ret = chmod((const char*)data, 0666);
   ck_assert_int_eq(ret, 0);
   return ECORE_CALLBACK_CANCEL;
}

/////// helper functions

static void _check_event_path(void* data, void* event)
{
   const char* expected_path = ecore_file_realpath((const char*)data);
   const char* actual_path = ecore_file_realpath(((Eio_Monitor_Event*)event)->filename);
   ck_assert_str_eq(actual_path, expected_path);
}

static Eina_Tmpstr* _common_init()
{
   Eina_Tmpstr* dirname;
   fail_if(eio_init() != 1);
   ecore_file_init();

   //test timeout
   test_timeout_timer = ecore_timer_add(TEST_TIMEOUT_SEC, _test_timeout_cb, NULL);

   eina_file_mkdtemp("checkFileCreationXXXXXX", &dirname);
   return dirname;
}

static void _common_shutdown(Eina_Tmpstr* dirname)
{
   _delete_directory((void*)dirname);
   ecore_file_shutdown();
   fail_if(eio_shutdown() != 0);
   eina_tmpstr_del(dirname);
}

/////// tests monitoring a directory

static void _file_created_cb(void* data, int type, void* event)
{
   ck_assert_int_eq(type, (int)EIO_MONITOR_FILE_CREATED);
   _check_event_path(data, event);
   _cancel_timeout();
   ecore_main_loop_quit();
}


START_TEST(eio_test_monitor_directory_file_created_notify)
{
   Eina_Tmpstr* dirname = _common_init();
   Eina_Stringshare* filename;

   filename = eina_stringshare_printf("%s/filecreated", dirname);

   //monitor directory
   eio_monitor_add(dirname);
   ecore_event_handler_add(EIO_MONITOR_FILE_CREATED, (Ecore_Event_Handler_Cb)_file_created_cb, filename);

   ecore_timer_add(TEST_OPERATION_DELAY, _create_file, filename);

   ecore_main_loop_begin();

   _common_shutdown(dirname);
}
END_TEST

static void _file_deleted_cb(void* data, int type, void* event)
{
   ck_assert_int_eq(type, (int)EIO_MONITOR_FILE_DELETED);
   _check_event_path(data, event);
   _cancel_timeout();
   ecore_main_loop_quit();
}

START_TEST(eio_test_monitor_directory_file_deleted_notify)
{
   Eina_Tmpstr* dirname = _common_init();
   Eina_Stringshare* filename;

   filename = eina_stringshare_printf("%s/filecreated", dirname);
   _create_file((void*)filename);

   //monitor directory
   eio_monitor_add(dirname);
   ecore_event_handler_add(EIO_MONITOR_FILE_DELETED, (Ecore_Event_Handler_Cb)_file_deleted_cb, filename);

   ecore_timer_add(TEST_OPERATION_DELAY, _delete_file, filename);

   ecore_main_loop_begin();

   _common_shutdown(dirname);
}
END_TEST

static void _file_modified_cb(void* data, int type, void* event)
{
   ck_assert_int_eq(type, (int)EIO_MONITOR_FILE_MODIFIED);
   _check_event_path(data, event);
   _cancel_timeout();
   ecore_main_loop_quit();
}

START_TEST(eio_test_monitor_directory_file_modified_notify)
{
   Eina_Tmpstr* dirname = _common_init();
   Eina_Stringshare* filename;

   filename = eina_stringshare_printf("%s/filecreated", dirname);
   _create_file((void*)filename);

   //monitor directory
   eio_monitor_add(dirname);
   ecore_event_handler_add(EIO_MONITOR_FILE_MODIFIED, (Ecore_Event_Handler_Cb)_file_modified_cb, filename);

   ecore_timer_add(TEST_OPERATION_DELAY, _modify_file, filename);

   ecore_main_loop_begin();

   //cleanup
   _common_shutdown(dirname);
}
END_TEST

static void _file_closed_cb(void* data, int type, void* event)
{
   ck_assert_int_eq(type, (int)EIO_MONITOR_FILE_CLOSED);
   _check_event_path(data, event);
   _cancel_timeout();
   ecore_main_loop_quit();
}

START_TEST(eio_test_monitor_directory_file_closed_notify)
{
   Eina_Tmpstr* dirname = _common_init();
   Eina_Stringshare* filename;

   filename = eina_stringshare_printf("%s/filecreated", dirname);
   _create_file((void*)filename);

   //monitor directory
   eio_monitor_add(dirname);
   ecore_event_handler_add(EIO_MONITOR_FILE_CLOSED, (Ecore_Event_Handler_Cb)_file_closed_cb, filename);
   ecore_timer_add(TEST_OPERATION_DELAY, _modify_file, filename);

   ecore_main_loop_begin();

   _common_shutdown(dirname);
}
END_TEST

static void _directory_created_cb(void* data, int type, void* event)
{
   ck_assert_int_eq(type, (int)EIO_MONITOR_DIRECTORY_CREATED);
   _check_event_path(data, event);
   _cancel_timeout();
   ecore_main_loop_quit();
}

START_TEST(eio_test_monitor_directory_directory_created_notify)
{
   Eina_Tmpstr* dirname = _common_init();
   Eina_Stringshare* filename;

   filename = eina_stringshare_printf("%s/dircreated", dirname);

   //monitor directory
   eio_monitor_add(dirname);
   ecore_event_handler_add(EIO_MONITOR_DIRECTORY_CREATED, (Ecore_Event_Handler_Cb)_directory_created_cb, filename);

   ecore_timer_add(TEST_OPERATION_DELAY, _create_directory, filename);

   ecore_main_loop_begin();

   _common_shutdown(dirname);
}
END_TEST

static void _directory_deleted_cb(void* data, int type, void* event)
{
   ck_assert_int_eq(type, (int)EIO_MONITOR_DIRECTORY_DELETED);
   _check_event_path(data, event);
   _cancel_timeout();
   ecore_main_loop_quit();
}

START_TEST(eio_test_monitor_directory_directory_deleted_notify)
{
   Eina_Tmpstr* dirname = _common_init();
   Eina_Stringshare* filename;

   filename = eina_stringshare_printf("%s/dircreated", dirname);
   _create_directory((void*)filename);

   //monitor directory
   eio_monitor_add(dirname);
   ecore_event_handler_add(EIO_MONITOR_DIRECTORY_DELETED, (Ecore_Event_Handler_Cb)_directory_deleted_cb, filename);

   ecore_timer_add(TEST_OPERATION_DELAY, _delete_directory, filename);

   ecore_main_loop_begin();

   _common_shutdown(dirname);
}
END_TEST

static void _directory_modified_cb(void* data, int type, void* event)
{
   ck_assert_int_eq(type, (int)EIO_MONITOR_DIRECTORY_MODIFIED);
   _check_event_path(data, event);
   _cancel_timeout();
   ecore_main_loop_quit();
}


START_TEST(eio_test_monitor_directory_directory_modified_notify)
{
   Eina_Tmpstr* dirname = _common_init();
   Eina_Stringshare* filename;

   filename = eina_stringshare_printf("%s/filecreated", dirname);
   _create_directory((void*)filename);

   //monitor directory
   eio_monitor_add(dirname);
   ecore_event_handler_add(EIO_MONITOR_DIRECTORY_MODIFIED, (Ecore_Event_Handler_Cb)_directory_modified_cb, filename);

   ecore_timer_add(TEST_OPERATION_DELAY, _modify_attrib_file, filename);

   ecore_main_loop_begin();

   _common_shutdown(dirname);
}
END_TEST


static void _directory_self_deleted_cb(void* data, int type, void* event)
{
   ck_assert_int_eq(type, (int)EIO_MONITOR_SELF_DELETED);
   _check_event_path(data, event);
   _cancel_timeout();
   ecore_main_loop_quit();
}


START_TEST(eio_test_monitor_directory_directory_self_deleted_notify)
{
   Eina_Tmpstr* dirname = _common_init();

   //monitor directory
   eio_monitor_add(dirname);
   ecore_event_handler_add(EIO_MONITOR_SELF_DELETED, (Ecore_Event_Handler_Cb)_directory_self_deleted_cb, dirname);

   ecore_timer_add(TEST_OPERATION_DELAY, _delete_directory, dirname);

   ecore_main_loop_begin();

   _common_shutdown(dirname);
}
END_TEST

//////// test monitoring a single file

START_TEST(eio_test_monitor_file_file_modified_notify)
{
   Eina_Tmpstr* dirname = _common_init();
   Eina_Stringshare* filename;

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
END_TEST

START_TEST(eio_test_monitor_file_file_attrib_modified_notify)
{
   Eina_Tmpstr* dirname = _common_init();
   Eina_Stringshare* filename;

   filename = eina_stringshare_printf("%s/filecreated", dirname);
   _create_file((void*)filename);

   //monitor file
   eio_monitor_add(filename);
   ecore_event_handler_add(EIO_MONITOR_FILE_MODIFIED, (Ecore_Event_Handler_Cb)_file_modified_cb, filename);

   ecore_timer_add(TEST_OPERATION_DELAY, _modify_attrib_file, filename);

   ecore_main_loop_begin();

   _common_shutdown(dirname);
}
END_TEST


START_TEST(eio_test_monitor_file_file_closed_notify)
{
   Eina_Tmpstr* dirname = _common_init();
   Eina_Stringshare* filename;

   filename = eina_stringshare_printf("%s/filecreated", dirname);
   _create_file((void*)filename);

   //monitor file
   eio_monitor_add(dirname);
   ecore_event_handler_add(EIO_MONITOR_FILE_CLOSED, (Ecore_Event_Handler_Cb)_file_closed_cb, filename);
   ecore_timer_add(TEST_OPERATION_DELAY, _modify_file, filename);

   ecore_main_loop_begin();

   _common_shutdown(dirname);
}
END_TEST

START_TEST(eio_test_monitor_file_file_self_deleted_notify)
{
   Eina_Tmpstr* dirname = _common_init();
   Eina_Stringshare* filename;

   filename = eina_stringshare_printf("%s/filecreated", dirname);
   _create_file((void*)filename);

   //monitor file
   eio_monitor_add(filename);
   ecore_event_handler_add(EIO_MONITOR_SELF_DELETED, (Ecore_Event_Handler_Cb)_directory_self_deleted_cb, filename);

   ecore_timer_add(TEST_OPERATION_DELAY, _delete_file, filename);

   ecore_main_loop_begin();

   _common_shutdown(dirname);
}
END_TEST

void eio_test_monitor(TCase *tc)
{
   tcase_add_test(tc, eio_test_monitor_directory_file_created_notify);
   tcase_add_test(tc, eio_test_monitor_directory_file_deleted_notify);
   tcase_add_test(tc, eio_test_monitor_directory_file_modified_notify);
#ifndef _WIN32
   tcase_add_test(tc, eio_test_monitor_directory_file_closed_notify);
#endif
   tcase_add_test(tc, eio_test_monitor_directory_directory_created_notify);
   tcase_add_test(tc, eio_test_monitor_directory_directory_deleted_notify);
   tcase_add_test(tc, eio_test_monitor_directory_directory_modified_notify);
   tcase_add_test(tc, eio_test_monitor_directory_directory_self_deleted_notify);

   tcase_add_test(tc, eio_test_monitor_file_file_modified_notify);
   tcase_add_test(tc, eio_test_monitor_file_file_attrib_modified_notify);
#ifndef _WIN32
   tcase_add_test(tc, eio_test_monitor_file_file_closed_notify);
#endif
   tcase_add_test(tc, eio_test_monitor_file_file_self_deleted_notify);

}
