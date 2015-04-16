#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <Eio.h>
#include <Ecore.h>

#include "eio_suite.h"

static int default_rights = 0777;
int test_count = 0;

static Eina_Bool
_filter_cb(void *data EINA_UNUSED, Eio_File *handler EINA_UNUSED, const char *file)
{
   char *last_slash = strrchr(file, '/');

   //Check if it is a hidden file
   if (last_slash != NULL && strlen(last_slash) > 1 && last_slash[1] == '.')
     return EINA_FALSE;

   return EINA_TRUE;
}

static Eina_Bool
_delete_filter_cb(void *data EINA_UNUSED, Eio_File *handler EINA_UNUSED,
                  const Eina_File_Direct_Info *info EINA_UNUSED)
{
   return EINA_TRUE;
}

static void
_main_cb(void *data, Eio_File *handler EINA_UNUSED, const char *file)
{
   int *number_of_listed_files = (int *)data;

   fprintf(stderr, "Processing file:%s\n", file);
   (*number_of_listed_files)++;
}

static Eina_Bool
_direct_filter_cb(void *data EINA_UNUSED, Eio_File *handler EINA_UNUSED,
                  const Eina_File_Direct_Info *info)
{
   char *last_slash = strrchr(info->path, '/');

   //Check if it is a hidden file
   if (last_slash != NULL && strlen(last_slash) > 1 && last_slash[1] == '.')
     return EINA_FALSE;

   return EINA_TRUE;
}

static void
_direct_main_cb(void *data, Eio_File *handler EINA_UNUSED, const Eina_File_Direct_Info *info)
{
   int *number_of_listed_files = (int *)data;

   fprintf(stderr, "Processing file:%s\n", info->path);
   (*number_of_listed_files)++;
}

static void
_progress_cb(void *data, Eio_File *handler EINA_UNUSED, const Eio_Progress *info)
{
   int *number_of_listed_files = (int *)data;

   fprintf(stderr, "Processing file:%s\n", info->source);
   (*number_of_listed_files)++;
}

static void
_done_cb(void *data, Eio_File *handler EINA_UNUSED)
{
   int *number_of_listed_files = (int *)data;
   fail_if((*number_of_listed_files) != test_count);
   ecore_main_loop_quit();
}

static void
_error_cb(void *data EINA_UNUSED, Eio_File *handler EINA_UNUSED, int error)
{
   fprintf(stderr, "Something wrong has happend:%s\n", strerror(error));
   ecore_main_loop_quit();
}

Eina_Tmpstr*
get_full_path(const char* tmpdirname, const char* filename)
{
    char full_path[PATH_MAX] = "";
    eina_str_join(full_path, sizeof(full_path), '/', tmpdirname, filename);
    return eina_tmpstr_add(full_path);
}

Eina_Tmpstr*
get_eio_test_file_tmp_dir()
{
   Eina_Tmpstr *tmp_dir;

   Eina_Bool created = eina_file_mkdtemp("EioFileTestXXXXXX", &tmp_dir);

   if (!created)
     {
        return NULL;
     }

   return tmp_dir;
}

Eina_Tmpstr*
create_test_dirs(Eina_Tmpstr *test_dirname)
{
   int i, fd;
   const char *good_dirs[] =
     {
        "eio_file_ls_simple_dir",
        "b."
     };
   const char *files[] =
     {
        ".hidden_file",
        "~$b@:-*$a!{}",
        "$b$a",
        "normal_file"
     };
   int count = sizeof(good_dirs) / sizeof(const char *);
   fail_if(test_dirname == NULL);

   for (i = 0; i != count; ++i)
     {
        Eina_Tmpstr *dirname = get_full_path(test_dirname, good_dirs[i]);
        fail_if(mkdir(dirname, default_rights) != 0);
        eina_tmpstr_del(dirname);
     }
   count = sizeof(files) / sizeof(const char *);
   for (i = 0; i != count; ++i)
     {
        Eina_Tmpstr *filename = get_full_path(test_dirname, files[i]);
        fd = open(filename, O_RDWR|O_CREAT, default_rights);
        fail_if(fd < 0);
        fail_if(close(fd) != 0);
        eina_tmpstr_del(filename);
     }
   Eina_Tmpstr *nested_dirname = get_full_path(test_dirname, good_dirs[0]);
   for (i = 0; i != count; ++i)
     {
        Eina_Tmpstr *filename = get_full_path(nested_dirname, files[i]);
        fd = open(filename, O_RDWR|O_CREAT, default_rights);
        fail_if(fd < 0);
        fail_if(close(fd) != 0);
        eina_tmpstr_del(filename);
     }
   return nested_dirname;
}

START_TEST(eio_file_test_ls)
{
   int number_of_listed_files = 0, ret;

   ret = ecore_init();
   fail_if(ret < 1);
   ret = eio_init();
   fail_if(ret < 1);
   ret = eina_init();
   fail_if(ret < 1);

   Eina_Tmpstr *test_dirname = get_eio_test_file_tmp_dir();
   Eina_Tmpstr *nested_dirname = create_test_dirs(test_dirname);

   eio_file_ls(test_dirname, _filter_cb, _main_cb, _done_cb, _error_cb,
               &number_of_listed_files);
   test_count = 5;
   fail_if(number_of_listed_files != 0); //check asynchronous
   ecore_main_loop_begin();

   number_of_listed_files = 0;
#ifdef _WIN32
   eio_file_direct_ls(test_dirname, _direct_filter_cb, _direct_main_cb, _done_cb,
                   _error_cb, &number_of_listed_files);
#else
   eio_file_stat_ls(test_dirname, _direct_filter_cb, _direct_main_cb, _done_cb,
                   _error_cb, &number_of_listed_files);
#endif
   test_count = 5;
   fail_if(number_of_listed_files != 0); //check asynchronous
   ecore_main_loop_begin();

   number_of_listed_files = 0;
#ifdef _WIN32
   eio_dir_direct_ls(test_dirname, _direct_filter_cb, _direct_main_cb, _done_cb,
                   _error_cb, &number_of_listed_files);
#else
   eio_dir_stat_ls(test_dirname, _direct_filter_cb, _direct_main_cb, _done_cb,
                   _error_cb, &number_of_listed_files);
#endif
   test_count = 8;
   fail_if(number_of_listed_files != 0); //check asynchronous
   ecore_main_loop_begin();

   number_of_listed_files = 0;
   eio_dir_unlink(nested_dirname, _delete_filter_cb, _progress_cb, _done_cb,
                   _error_cb, &number_of_listed_files);
   test_count = 5; // 4 internal files + 1 nested_dir
   fail_if(number_of_listed_files != 0); // check asynchronous
   ecore_main_loop_begin();

   number_of_listed_files = 0;
   eio_dir_unlink(test_dirname, _delete_filter_cb, _progress_cb, _done_cb,
                   _error_cb, &number_of_listed_files);
   test_count = 6; // 4 internal files + 1 dir + 1 test_dir
   fail_if(number_of_listed_files != 0); // check asynchronous
   ecore_main_loop_begin();

   eina_tmpstr_del(nested_dirname);
   eina_tmpstr_del(test_dirname);
   eina_shutdown();
   eio_shutdown();
   ecore_shutdown();
}
END_TEST

void
eio_test_file(TCase *tc)
{
    tcase_add_test(tc, eio_file_test_ls);
}

