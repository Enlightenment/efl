#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <Ecore.h>
#include <Ecore_File.h>
#include <Eio.h>

#include "eio_suite.h"
#include "eio_test_common.h"

#ifndef O_BINARY
# define O_BINARY 0
#endif

static int test_count = 0;

static int DONE_CALLED = 0xdeadbeef;

static void
_filter_direct_cb(void *data EINA_UNUSED, const Eo_Event *event)
{
   Eio_Filter_Direct_Data *event_info = event->info;
   char *last_slash = strrchr(event_info->info->path, '/');

   //Check if it is a hidden file
   if (last_slash != NULL && strlen(last_slash) > 1 && last_slash[1] == '.')
     event_info->filter = EINA_FALSE;
   else
     event_info->filter = EINA_TRUE;
}

static void
_main_direct_cb(void *data, const Eina_File_Direct_Info *info)
{
   int *number_of_listed_files = (int *)data;

   fprintf(stderr, "Processing file:%s\n", info->path);
   (*number_of_listed_files)++;
}

static void
_filter_cb(void *data EINA_UNUSED, const Eo_Event *event)
{
   Eio_Filter_Name_Data *event_info = event->info;
   char *last_slash = strrchr(event_info->file, '/');

   //Check if it is a hidden file
   if (last_slash != NULL && strlen(last_slash) > 1 && last_slash[1] == '.')
     event_info->filter = EINA_FALSE;
   else
     event_info->filter = EINA_TRUE;
}

static void
_main_cb(void *data, const char *file)
{
   int *number_of_listed_files = (int *)data;

   fprintf(stderr, "Processing file:%s\n", file);
   (*number_of_listed_files)++;
}

static void
_done_cb(void *data, void* value EINA_UNUSED)
{
   int *number_of_listed_files = (int *)data;
   fail_if((*number_of_listed_files) != test_count);
   *number_of_listed_files = DONE_CALLED;
   ecore_main_loop_quit();
}

static void
_error_cb(void *data EINA_UNUSED, Eina_Error error)
{
   const char *msg = eina_error_msg_get(error);
   EINA_LOG_ERR("error: %s", msg);
   ecore_main_loop_quit();
}

static void
_open_done_cb(void *data, void *file_value)
{
   Eina_Bool *opened = (Eina_Bool *)data;
   *opened = EINA_TRUE;
   Eina_File* file = eina_file_dup(file_value);
   eina_file_close(file);
   ecore_main_loop_quit();
}

static void
_stat_done_cb(void *data, void *value)
{
   Eina_Stat const* stat = value;
   Eina_Bool *is_dir = data;
   unsigned int rights;
   fail_if(eio_file_is_dir(stat) != *is_dir);
   fail_if(eio_file_is_lnk(stat));
   rights = stat->mode & (S_IRWXU | S_IRWXG | S_IRWXO);
   fail_if(rights != default_rights);
   ecore_main_loop_quit();
}

typedef Eina_Promise* (*Efl_Io_Manager_Test_Stat_Ls_Func)(Eo *job, const char *path);

static void
_do_ls_test(Efl_Io_Manager_Test_Stat_Ls_Func ls_func,
      const Eo_Event_Description *event,
      Eo_Event_Cb filter_cb,
      Eina_Promise_Progress_Cb progress_cb,
      int expected_test_count,
      const char* test_dirname)
{
   int main_files = 0;

   Efl_Io_Manager *job = eo_add(EFL_IO_MANAGER_CLASS, NULL);
   Eina_Promise *promise = NULL;

   eo_event_callback_add(job, event, filter_cb, NULL);
   promise = ls_func(job, test_dirname);
   test_count = expected_test_count;
   eina_promise_progress_cb_add(promise, progress_cb, &main_files, NULL);
   eina_promise_then(promise, &_done_cb, &_error_cb, &main_files);

   ecore_main_loop_begin();

   fail_if(main_files != DONE_CALLED);

   eo_unref(job);
}

static void
_do_direct_ls_test(Efl_Io_Manager_Test_Stat_Ls_Func ls_func,
      int expected_test_count,
      const char *test_dirname)
{
   _do_ls_test(ls_func,
               EFL_IO_MANAGER_EVENT_FILTER_DIRECT,
               (Eo_Event_Cb)&_filter_direct_cb,
               (Eina_Promise_Progress_Cb)&_main_direct_cb,
               expected_test_count,
               test_dirname);
}

START_TEST(efl_io_manager_test_file_direct_stat)
{
   Eina_Bool is_dir;
   int ret;

   ret = ecore_init();
   fail_if(ret < 1);
   ret = eio_init();
   fail_if(ret < 1);
   ret = eina_init();
   fail_if(ret < 1);
   ret = ecore_file_init();
   fail_if(ret < 1);

   Eina_Tmpstr *test_dirname = get_eio_test_file_tmp_dir();
   Eina_Tmpstr *nested_dirname = create_test_dirs(test_dirname);
   Eina_Tmpstr *nested_filename = get_full_path(test_dirname, files[3]);

   Efl_Io_Manager *job = eo_add(EFL_IO_MANAGER_CLASS, NULL);
   Eina_Promise *promise = NULL;
   // Start testing
   is_dir = EINA_TRUE;

   promise = efl_io_manager_file_direct_stat(job, nested_dirname);
   eina_promise_then(promise, &_stat_done_cb, &_error_cb, &is_dir);
   ecore_main_loop_begin();

   is_dir = EINA_FALSE;
   promise = efl_io_manager_file_direct_stat(job, nested_filename);
   eina_promise_then(promise, &_stat_done_cb, &_error_cb, &is_dir);
   ecore_main_loop_begin();
   eo_unref(job);

   // Cleanup
   fail_if(!ecore_file_recursive_rm(test_dirname));

   eina_tmpstr_del(nested_dirname);
   eina_tmpstr_del(test_dirname);
   eina_tmpstr_del(nested_filename);
   ecore_file_shutdown();
   eina_shutdown();
   eio_shutdown();
   ecore_shutdown();
}
END_TEST

START_TEST(efl_io_manager_test_ls_funcs)
{
   int ret;

   ret = ecore_init();
   fail_if(ret < 1);
   ret = eio_init();
   fail_if(ret < 1);
   ret = eina_init();
   fail_if(ret < 1);
   ret = ecore_file_init();
   fail_if(ret < 1);

   Eina_Tmpstr *test_dirname = get_eio_test_file_tmp_dir();
   Eina_Tmpstr *nested_dirname = create_test_dirs(test_dirname);
   Eina_Tmpstr *nested_filename = get_full_path(test_dirname, files[3]);

   // Start testing

   _do_ls_test(&efl_io_manager_file_ls,
         EFL_IO_MANAGER_EVENT_FILTER_NAME,
         (Eo_Event_Cb)&_filter_cb,
         (Eina_Promise_Progress_Cb)&_main_cb,
         5,
         test_dirname);

   _do_direct_ls_test(&efl_io_manager_file_stat_ls, 5, test_dirname);

   _do_direct_ls_test(&efl_io_manager_file_direct_ls, 5, test_dirname);

   _do_direct_ls_test(&efl_io_manager_dir_stat_ls, 8, test_dirname);

   _do_direct_ls_test(&efl_io_manager_dir_direct_ls, 8, test_dirname);

   // Cleanup
   fail_if(!ecore_file_recursive_rm(test_dirname));

   eina_tmpstr_del(nested_dirname);
   eina_tmpstr_del(test_dirname);
   eina_tmpstr_del(nested_filename);
   ecore_file_shutdown();
   eina_shutdown();
   eio_shutdown();
   ecore_shutdown();
}
END_TEST

START_TEST(efl_io_manager_test_open)
{
   Eina_Bool opened_file;
   int ret;

   ret = ecore_init();
   fail_if(ret < 1);
   ret = eina_init();
   fail_if(ret < 1);
   ret = ecore_file_init();
   fail_if(ret < 1);
   ret = eio_init();
   fail_if(ret < 1);

   Eina_Tmpstr *test_dirname = get_eio_test_file_tmp_dir();
   Eina_Tmpstr *nested_dirname = create_test_dirs(test_dirname);
   Eina_Tmpstr *nested_filename = get_full_path(test_dirname, files[3]);

   opened_file = EINA_FALSE;
   Efl_Io_Manager *job = eo_add(EFL_IO_MANAGER_CLASS, NULL);
   Eina_Promise *promise = NULL;

   promise = efl_io_manager_file_open(job, nested_filename, EINA_FALSE);
   eina_promise_then(promise, &_open_done_cb, &_error_cb, &opened_file);
   ecore_main_loop_begin();
   eo_unref(job);
   fail_if(!opened_file);

   // Cleanup
   fail_if(!ecore_file_recursive_rm(test_dirname));

   eina_tmpstr_del(nested_dirname);
   eina_tmpstr_del(test_dirname);

   eio_shutdown();
   eina_tmpstr_del(nested_filename);
   ecore_file_shutdown();
   eina_shutdown();
   ecore_shutdown();
}
END_TEST

START_TEST(efl_io_instantiated)
{
   Efl_Io_Manager *manager;

   ecore_init();

   fail_if(eo_provider_find(ecore_main_loop_get(), EFL_IO_MANAGER_CLASS) != NULL);

   eio_init();

   manager = eo_provider_find(ecore_main_loop_get(), EFL_IO_MANAGER_CLASS);
   fail_if(manager == NULL);
   fail_if(!eo_isa(manager, EFL_IO_MANAGER_CLASS));

   eio_shutdown();

   ecore_shutdown();
}
END_TEST

void
eio_test_job(TCase *tc)
{
    tcase_add_test(tc, efl_io_manager_test_ls_funcs);
    tcase_add_test(tc, efl_io_manager_test_file_direct_stat);
    tcase_add_test(tc, efl_io_manager_test_open);
    tcase_add_test(tc, efl_io_instantiated);
}
