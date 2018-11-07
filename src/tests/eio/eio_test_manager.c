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

static uint64_t test_count = 0;
static Eina_Bool direct = EINA_FALSE;

#define DONE_CALLED 0xdeadbeef

static void
_access_cb(void *data, Eina_Array *paths)
{
   uint64_t *number_of_listed_files = data;

   *number_of_listed_files += eina_array_count(paths);
}

static void
_progress_cb(void *data, Eina_Array *entries)
{
   uint64_t *number_of_listed_files = data;

   (*number_of_listed_files) += eina_array_count(entries);
}

static Eina_Value
_future_cb(void *data,
           const Eina_Value file,
           const Eina_Future *dead EINA_UNUSED)
{
   if (file.type == EINA_VALUE_TYPE_ERROR)
     {
        Eina_Error err;

        eina_value_get(&file, &err);
        fprintf(stderr, "Something has gone wrong: %s\n", eina_error_msg_get(err));
        abort();
     }
   if (file.type == EINA_VALUE_TYPE_UINT64)
     {
        uint64_t *number_of_listed_files = data;
        uint64_t value;

        eina_value_get(&file, &value);

        fail_if((*number_of_listed_files) != test_count);
        fail_if(value != test_count);
        *number_of_listed_files = DONE_CALLED;
     }

   ecore_main_loop_quit();

   return file;
}

static Eina_Value
_done_cb(void *data,
         const Eina_Value file,
         const Eina_Future *dead EINA_UNUSED)
{
   uint64_t *number_of_listed_files = data;

   if (eina_value_type_get(&file) == EINA_VALUE_TYPE_ERROR)
     {
        Eina_Error err = 0;
        const char *msg;

        eina_value_error_get(&file, &err);
        msg = eina_error_msg_get(err);

        EINA_LOG_ERR("error: %s", msg);
        abort();
     }
   else
     {
        Eina_Value convert = EINA_VALUE_EMPTY;
        uint64_t files_count = 0;

        eina_value_setup(&convert, EINA_VALUE_TYPE_ULONG);
        eina_value_convert(&file, &convert);
        eina_value_ulong_get(&convert, &files_count);

        fail_if((*number_of_listed_files) != test_count);
        fail_if(files_count != test_count);

        *number_of_listed_files = DONE_CALLED;
     }

   ecore_main_loop_quit();

   return file;
}

static Eina_Value
_open_done_cb(void *data,
              const Eina_Value file,
              const Eina_Future *dead EINA_UNUSED)
{
   if (file.type == EINA_VALUE_TYPE_ERROR)
     {
        Eina_Error err;

        eina_value_get(&file, &err);
        fprintf(stderr, "Something has gone wrong: %s\n", eina_error_msg_get(err));
        abort();
     }
   if (file.type == EINA_VALUE_TYPE_FILE)
     {
        Eina_Bool *opened = (Eina_Bool *)data;

        *opened = EINA_TRUE;
     }
   ecore_main_loop_quit();

   return file;
}

static Eina_Value
_stat_done_cb(void *data,
              const Eina_Value st,
              const Eina_Future *dead EINA_UNUSED)
{
   Eina_Bool *is_dir = data;
   unsigned int rights;

   if (st.type == EINA_VALUE_TYPE_ERROR)
     {
        Eina_Error err;
        eina_value_get(&st, &err);
        fprintf(stderr, "Something has gone wrong: %s\n", eina_error_msg_get(err));
        abort();
     }

   if (st.type == EINA_VALUE_TYPE_STRUCT)
     {
        unsigned int mode = 0;

        fail_if(!eina_value_struct_get(&st, "mode", &mode));
        fail_if(S_ISDIR(mode) != *is_dir);
        fail_if(S_ISLNK(mode));

        rights = mode & (S_IRWXU | S_IRWXG | S_IRWXO);
        fail_if(rights != default_rights);
     }

   ecore_main_loop_quit();

   return st;
}

static void
_test_ls(Eina_Future *(*func)(const Eo *obj, const char *path, Eina_Bool recursive,
                              void *info_data, EflIoDirectInfo info, Eina_Free_Cb info_free_cb),
         uint64_t expected_test_count,
         const char* test_dirname)
{
   Efl_Io_Manager *job = efl_add(EFL_IO_MANAGER_CLASS, efl_main_loop_get());
   Eina_Future *f = NULL;
   uint64_t main_files = 0;

   fail_if(!job);

   f = func(job, test_dirname, EINA_FALSE, &main_files, _progress_cb, NULL);
   fail_if(!f);
   test_count = expected_test_count;
   eina_future_then(f, &_done_cb, &main_files, NULL);

   ecore_main_loop_begin();

   fail_if(main_files != DONE_CALLED);
   main_files = 0;

   f = func(job, test_dirname, EINA_TRUE, &main_files, _progress_cb, NULL);
   test_count = expected_test_count + 4;
   eina_future_then(f, &_done_cb, &main_files, NULL);

   ecore_main_loop_begin();

   fail_if(main_files != DONE_CALLED);

   efl_del(job);
}

EFL_START_TEST(efl_io_manager_test_stat)
{
   Eina_Tmpstr *test_dirname;
   Eina_Tmpstr *nested_dirname;
   Eina_Tmpstr *nested_filename;
   Efl_Io_Manager *job;
   Eina_Future *f;
   Eina_Bool is_dir = EINA_TRUE;
   int ret;

   ret = ecore_file_init();
   fail_if(ret < 1);

   test_dirname = get_eio_test_file_tmp_dir();
   nested_dirname = create_test_dirs(test_dirname);
   nested_filename = get_full_path(test_dirname, files[3]);

   job = efl_add(EFL_IO_MANAGER_CLASS, efl_main_loop_get());
   fail_if(!job);

   // Start testing
   f = efl_io_manager_stat(job, nested_dirname);
   eina_future_then(f, _stat_done_cb, &is_dir, NULL);
   ecore_main_loop_begin();

   is_dir = EINA_FALSE;
   f = efl_io_manager_stat(job, nested_filename);
   eina_future_then(f, _stat_done_cb, &is_dir, NULL);
   ecore_main_loop_begin();

   // Cleanup
   efl_del(job);
   fail_if(!ecore_file_recursive_rm(test_dirname));

   eina_tmpstr_del(nested_dirname);
   eina_tmpstr_del(test_dirname);
   eina_tmpstr_del(nested_filename);
   ecore_file_shutdown();
}
EFL_END_TEST

EFL_START_TEST(efl_io_manager_test_ls)
{
   Eina_Tmpstr *test_dirname;
   Eina_Tmpstr *nested_dirname;
   Eina_Tmpstr *nested_filename;
   Efl_Io_Manager *job;
   Eina_Future *f;
   uint64_t main_files = 0;
   int ret;

   ret = ecore_file_init();
   fail_if(ret < 1);

   test_dirname = get_eio_test_file_tmp_dir();
   nested_dirname = create_test_dirs(test_dirname);
   nested_filename = get_full_path(test_dirname, files[3]);

   // Start testing
   job = efl_add(EFL_IO_MANAGER_CLASS, efl_main_loop_get());
   fail_if(!job);

   f = efl_io_manager_ls(job, test_dirname, &main_files, _access_cb, NULL);
   test_count = 6;
   eina_future_then(f, _future_cb, &main_files, NULL);

   ecore_main_loop_begin();

   fail_if(main_files != DONE_CALLED);

   // No recursion for efl_io_manager_ls, should I fix that ?
   /* _test_ls(&efl_io_manager_ls, 5, test_dirname); */
   direct = EINA_TRUE;
   _test_ls(&efl_io_manager_stat_ls, 6, test_dirname);
   _test_ls(&efl_io_manager_direct_ls, 6, test_dirname);

   // Cleanup
   efl_del(job);
   fail_if(!ecore_file_recursive_rm(test_dirname));

   eina_tmpstr_del(nested_dirname);
   eina_tmpstr_del(test_dirname);
   eina_tmpstr_del(nested_filename);
   ecore_file_shutdown();
}
EFL_END_TEST

EFL_START_TEST(efl_io_manager_test_open)
{
   Eina_Tmpstr *test_dirname;
   Eina_Tmpstr *nested_dirname;
   Eina_Tmpstr *nested_filename;
   Efl_Io_Manager *job;
   Eina_Future *f;
   Eina_Bool opened_file = EINA_FALSE;
   int ret;

   ret = ecore_file_init();
   fail_if(ret < 1);

   test_dirname = get_eio_test_file_tmp_dir();
   nested_dirname = create_test_dirs(test_dirname);
   nested_filename = get_full_path(test_dirname, files[3]);

   job = efl_add(EFL_IO_MANAGER_CLASS, efl_main_loop_get());

   f = efl_io_manager_open(job, nested_filename, EINA_FALSE);
   eina_future_then(f, _open_done_cb, &opened_file, NULL);
   ecore_main_loop_begin();

   fail_if(!opened_file);

   // Cleanup
   efl_del(job);
   fail_if(!ecore_file_recursive_rm(test_dirname));

   eina_tmpstr_del(nested_dirname);
   eina_tmpstr_del(test_dirname);

   eina_tmpstr_del(nested_filename);
   ecore_file_shutdown();
}
EFL_END_TEST

EFL_START_TEST(efl_io_test_instantiated)
{
   Efl_Io_Manager *manager;

   ck_assert_int_eq(eio_shutdown(), 0);
   ecore_init();
   fail_if(efl_provider_find(efl_main_loop_get(), EFL_IO_MANAGER_CLASS) != NULL);
   ecore_shutdown();
   ck_assert_int_eq(eio_init(), 1);

   manager = efl_provider_find(efl_main_loop_get(), EFL_IO_MANAGER_CLASS);
   fail_if(manager == NULL);
   fail_if(!efl_isa(manager, EFL_IO_MANAGER_CLASS));
}
EFL_END_TEST

void
eio_test_job(TCase *tc)
{
    tcase_add_test(tc, efl_io_manager_test_ls);
    tcase_add_test(tc, efl_io_manager_test_stat);
    tcase_add_test(tc, efl_io_manager_test_open);
    tcase_add_test(tc, efl_io_test_instantiated);
}
