#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <libgen.h>

#include <Eina.h>
#include <Ecore.h>
#include <Ecore_File.h>

#include "ecore_suite.h"

#ifndef O_BINARY
# define O_BINARY 0
#endif

#define MAXSIZE 256

void
_writeToFile(const char *filePath, char *text)
{
   FILE *f = fopen(filePath, "rb+");
   if (f == NULL)
     f = fopen(filePath, "wb");
   fail_if(f == NULL);
   fprintf(f, "%s\n", text);
   fclose(f);
}

static char *
get_tmp_dir(void)
{
   Eina_Tmpstr *tmp_dir;
   char *realpath;

   Eina_Bool created = eina_file_mkdtemp("EcoreFileTestXXXXXX", &tmp_dir);
   if (!created) return NULL;

   realpath = ecore_file_realpath(tmp_dir);
   eina_tmpstr_del(tmp_dir);
   return realpath;
}

static char *
get_tmp_file(void)
{
   Eina_Tmpstr *tmp_file;
   char *realpath;

   int fd = eina_file_mkstemp("EcoreFileTestXXXXXX", &tmp_file);
   if (fd < 0) return NULL;
   close(fd);

   realpath = ecore_file_realpath(tmp_file);
   eina_tmpstr_del(tmp_file);
   return realpath;
}

static void
file_monitor_cb(void *data EINA_UNUSED, Ecore_File_Monitor *em EINA_UNUSED,
                Ecore_File_Event event, const char *path)
{
   switch (event)
     {
        case ECORE_FILE_EVENT_NONE:
        case ECORE_FILE_EVENT_CREATED_FILE:
          fprintf(stderr, "File created in %s", path);
          break;
        case ECORE_FILE_EVENT_DELETED_FILE:
          fprintf(stderr, "File deleted in %s", path);
          break;
        case ECORE_FILE_EVENT_MODIFIED:
          fprintf(stderr, "File modified in %s", path);
          break;
        case ECORE_FILE_EVENT_CLOSED:
          fprintf(stderr, "File closed in %s", path);
          break;
        case ECORE_FILE_EVENT_DELETED_DIRECTORY:
          fprintf(stderr, "Directory deleted in %s", path);
          break;
        case ECORE_FILE_EVENT_CREATED_DIRECTORY:
          fprintf(stderr, "Directory created in %s", path);
          break;
        case ECORE_FILE_EVENT_DELETED_SELF:
          fprintf(stderr, "Path %s deleted", path);
          break;
     }
}

void
completion_cb(void *data EINA_UNUSED, const char *file EINA_UNUSED, int status)
{
   fprintf(stderr, "Done (status: %d)\n", status);
   ecore_main_loop_quit();
}

void
err_completion_cb(void *data EINA_UNUSED, const char *file EINA_UNUSED, int status)
{
   fail_if(status != 1);
   ecore_main_loop_quit();
}

int
progress_cb(void *data EINA_UNUSED, const char *file EINA_UNUSED,
            long int dltotal, long int dlnow,
            long int ultotal EINA_UNUSED, long int ulnow EINA_UNUSED)
{
   fprintf(stderr, "Progress: %ld/%ld\n", dlnow, dltotal);
   return ECORE_FILE_PROGRESS_CONTINUE;
}

EFL_START_TEST(ecore_test_ecore_file_init)
{
   int ret;

   ret = ecore_file_init();
   fail_if(ret != 1);

   ret = ecore_file_shutdown();
   fail_if(ret != 0);
}
EFL_END_TEST

EFL_START_TEST(ecore_test_ecore_file_operations)
{
   const char* dirs[] = {"b", "b/c", "b/c/d", "d", 0};
   char *dirs2[] = {"a2", "b2", "c2", 0};
   const char *src_dir, *src_file, *dest_file;
   const char *not_exist_file;
   const char *tmpdir = NULL;
   char *random_text = "This is random test String";
   char *escaped_text = "This\\ is\\ random\\ test\\ String";
   char *exe_cmd = "test.sh --opt1=a --opt2=b";
   char *exe = "test.sh";
   char dir[MAXSIZE] = {'\0'};
   unsigned int ret;
   int fd;
   int i;
   Eina_Bool res;
   Eina_List *list;

   tmpdir = ecore_file_realpath(eina_environment_tmp_get());

   ret = ecore_file_init();
   fail_if(ret != 1);

   src_dir = get_tmp_dir();
   fail_if(!src_dir);

   src_file = get_tmp_file();
   fail_if(!src_file);

   res = ecore_file_exists(src_file);
   fail_if(res != EINA_TRUE);
   res = ecore_file_is_dir(src_file);
   fail_if(res != EINA_FALSE);
   res = ecore_file_remove(src_file);
   fail_if(res != EINA_TRUE);
   res = ecore_file_exists(src_file);
   fail_if(res != EINA_FALSE);

   res = ecore_file_is_dir(src_dir);
   fail_if(res != EINA_TRUE);
   ret = ecore_file_dir_is_empty(src_dir);
   fail_if(ret != 1);
   res = ecore_file_mkdir(src_dir);
   fail_if(res != EINA_FALSE);
   res = ecore_file_remove(src_dir);
   fail_if(res != EINA_TRUE);
   res = ecore_file_is_dir(src_dir);
   fail_if(res != EINA_FALSE);

   src_dir = get_tmp_dir();
   fail_if(!src_dir);
   ret = ecore_file_mksubdirs(src_dir, dirs);
   fail_if(ret != 4);
   list = ecore_file_ls(src_dir);
   fail_if(!list);
   fail_if(eina_list_count(list) != 2);
   res = ecore_file_recursive_rm(src_dir);
   fail_if(res != EINA_TRUE);

   src_dir = get_tmp_dir();
   fail_if(!src_dir);
   strcat(dir, src_dir);
   strcat(dir, "/");
   strcat(dir, dirs[2]);
   res = ecore_file_mkpath(dir);
   fail_if(res != EINA_TRUE);
   res = ecore_file_recursive_rm(src_dir);
   fail_if(res != EINA_TRUE);

   src_file = get_tmp_file();
   fail_if(!src_file);
   _writeToFile(src_file, random_text);

   ret = ecore_file_size(src_file);
   fail_if(ret != strlen(random_text)+1);
   ret = ecore_file_is_dir(src_file);
   fail_if(ret != EINA_FALSE);

   dest_file = get_tmp_file();
   fail_if(!dest_file);
   res = ecore_file_cp(src_file, dest_file);
   fail_if(res != EINA_TRUE);
   res = ecore_file_exists(dest_file);
   fail_if(res != EINA_TRUE);
   fail_if(ecore_file_size(src_file) != ecore_file_size(dest_file));
   res = ecore_file_remove(dest_file);
   fail_if(res != EINA_TRUE);

   /* On Windows, symlink/readlink are not supported */
#ifndef _WIN32
   res = ecore_file_symlink(src_file, dest_file);
   fail_if(res != EINA_TRUE);
   ck_assert_str_eq(ecore_file_readlink(dest_file), src_file);
   ck_assert_str_eq(ecore_file_realpath(dest_file), src_file);
   res = ecore_file_unlink(dest_file);
   fail_if(res != EINA_TRUE);
#endif

   dest_file = get_tmp_file();
   fail_if(!dest_file);
   res = ecore_file_mv(src_file, dest_file);
   fail_if(res != EINA_TRUE);
   res = ecore_file_exists(dest_file);
   fail_if(res != EINA_TRUE);
   res = ecore_file_exists(src_file);
   fail_if(res != EINA_FALSE);

   ck_assert_str_eq(ecore_file_dir_get(dest_file), tmpdir);
   ck_assert_str_eq(ecore_file_realpath(dest_file), dest_file);
   fail_if(ecore_file_mod_time(dest_file) == 0);
   fail_if(ecore_file_can_read(dest_file) != EINA_TRUE);
   fail_if(ecore_file_can_write(dest_file) != EINA_TRUE);
   fail_if(ecore_file_can_exec(dest_file) != EINA_FALSE);
   fail_if(ecore_file_remove(dest_file) != EINA_TRUE);

   ck_assert_str_eq(ecore_file_app_exe_get(exe_cmd), exe);
   ck_assert_str_eq(ecore_file_escape_name(random_text), escaped_text);
   ck_assert_str_eq(ecore_file_strip_ext(exe), "test");
   res = ecore_file_app_installed(random_text);
   fail_if(res != EINA_FALSE);
   src_file = get_tmp_file();
   fail_if(!src_file);
   fail_if(ecore_file_remove(src_file) != EINA_TRUE);
   fd = open(src_file, O_RDWR | O_BINARY | O_CREAT, 0700);
   fail_if(fd < 0);
   fail_if(close(fd) != 0);
   fail_if(ecore_file_can_exec(src_file) != EINA_TRUE);

   src_dir = get_tmp_dir();
   fail_if(!src_dir);
   strcpy(dir, src_dir);
   strcat(dir, "/");
   strcat(dir, dirs[0]);
   fail_if(ecore_file_mkdir(dir) != EINA_TRUE);

   fail_if(ecore_file_mkdirs(NULL) != -1);
   for (i = 0; i < 3; i++)
     {
        char tmp[PATH_MAX];
        strcpy(tmp, src_dir);
        strcat(tmp, "/");
        strcat(tmp, dirs2[i]);
        dirs2[i] = strdup(tmp);
     }
   fail_if(ecore_file_mkdirs((const char **)dirs2) != 3);
   for (i = 0; i < 3; i++)
     free(dirs2[i]);

   fail_if(ecore_file_mksubdirs(src_dir, NULL) != -1);
   fail_if(ecore_file_mksubdirs(NULL, dirs) != -1);
   fail_if(ecore_file_mksubdirs("", dirs) != -1);
   fail_if(ecore_file_mksubdirs(src_file, dirs) != 0);
   fail_if(ecore_file_remove(src_file) != EINA_TRUE);
   fail_if(ecore_file_recursive_rm(src_dir) != EINA_TRUE);

   src_dir = get_tmp_dir();
   fail_if(!src_dir);
   fail_if(ecore_file_rmdir(src_dir) != EINA_TRUE);
   fail_if(ecore_file_dir_is_empty(src_dir) != -1);
   fail_if(ecore_file_ls(src_dir) != NULL);

   not_exist_file = get_tmp_file();
   fail_if(!not_exist_file);
   fail_if(ecore_file_remove(not_exist_file) != EINA_TRUE);
   fail_if(ecore_file_exists(not_exist_file) != EINA_FALSE);
   fail_if(ecore_file_mod_time(not_exist_file) != 0);
   fail_if(ecore_file_size(not_exist_file) != 0);

   ck_assert_str_eq(ecore_file_realpath(NULL), "");
   ck_assert_str_eq(ecore_file_realpath(not_exist_file), "");

   src_file = get_tmp_file();
   fail_if(!src_file);
   fail_if(ecore_file_remove(src_file) != EINA_TRUE);
   fd = open(src_file, O_RDWR | O_BINARY | O_CREAT, 0400);
   fail_if(fd < 0);
   fail_if(close(fd) != 0);
   fail_if(ecore_file_can_read(src_file) != EINA_TRUE);
#if defined(HAVE_GETUID) && defined(HAVE_GETEUID)
   if (getuid() || geteuid())
#endif
     {
        fail_if(ecore_file_can_write(src_file) != EINA_FALSE);
        fail_if(ecore_file_can_exec(src_file) != EINA_FALSE);
     }
   fail_if(ecore_file_cp(src_file, src_file) != EINA_FALSE);
   fail_if(ecore_file_remove(src_file) != EINA_TRUE);

   src_file = get_tmp_file();
   fail_if(!src_file);
   fail_if(ecore_file_remove(src_file) != EINA_TRUE);
   fd = open(src_file, O_RDWR | O_BINARY | O_CREAT, 0200);
   fail_if(fd < 0);
   fail_if(close(fd) != 0);
#if defined(HAVE_GETUID) && defined(HAVE_GETEUID)
   if (getuid() || geteuid())
#endif
     {
        fail_if(ecore_file_can_read(src_file) != EINA_FALSE);
     }
   fail_if(ecore_file_can_exec(src_file) != EINA_FALSE);
   fail_if(ecore_file_can_write(src_file) != EINA_TRUE);
   fail_if(ecore_file_remove(src_file) != EINA_TRUE);

   src_file = get_tmp_file();
   fail_if(!src_file);
   fail_if(ecore_file_remove(src_file) != EINA_TRUE);
   fd = open(src_file, O_RDWR | O_BINARY | O_CREAT, 0100);
   fail_if(fd < 0);
   fail_if(close(fd) != 0);
#if defined(HAVE_GETUID) && defined(HAVE_GETEUID)
   if (getuid() || geteuid())
#endif
     {
        fail_if(ecore_file_can_read(src_file) != EINA_FALSE);
        fail_if(ecore_file_can_write(src_file) != EINA_FALSE);
     }
   fail_if(ecore_file_can_exec(src_file) != EINA_TRUE);
   fail_if(ecore_file_remove(src_file) != EINA_TRUE);

   fail_if(ecore_file_unlink(not_exist_file) != EINA_FALSE);
   fail_if(ecore_file_remove(not_exist_file) != EINA_FALSE);
   fail_if(ecore_file_cp(not_exist_file, "test_file") != EINA_FALSE);
   fail_if(ecore_file_mv(not_exist_file, "test_file") != EINA_FALSE);

   ck_assert_int_eq(chdir(eina_environment_tmp_get()), 0);
   fail_if(ecore_file_mkpath(src_dir) != EINA_TRUE);
   fail_if(ecore_file_rmdir(src_dir) != EINA_TRUE);
   fail_if(ecore_file_mkpath(NULL) != EINA_FALSE);
   fail_if(ecore_file_mkpaths(dirs) != 4);
   for (i = 0; dirs[i]; i++)
     if (ecore_file_is_dir(dirs[i]))
       fail_if(ecore_file_recursive_rm(dirs[i]) != EINA_TRUE);
   fail_if(ecore_file_mkpaths(NULL) != -1);

   fail_if(ecore_file_dir_get(NULL) != NULL);
   fail_if(ecore_file_strip_ext(NULL) != NULL);
   fail_if(ecore_file_escape_name(NULL) != NULL);

   ret = ecore_file_shutdown();
   fail_if(ret != 0);

}
EFL_END_TEST

EFL_START_TEST(ecore_test_ecore_file_path)
{
   const char *src_dir, *src_file, *dest_file;
   char *dup_dir, *path;
   unsigned int ret;
   int fd;
   Eina_Bool res;
   Eina_List *list, *l;

   src_file = get_tmp_file();
   fail_if(!src_file);
   fail_if(ecore_file_remove(src_file) != EINA_TRUE);
   fd = open(src_file, O_RDWR | O_BINARY | O_CREAT, 0700);
   fail_if(fd < 0);
   fail_if(close(fd) != 0);
   fail_if(ecore_file_can_exec(src_file) != EINA_TRUE);
   dup_dir = strdup(src_file);
   fail_if(!dup_dir);
   dest_file = basename(dup_dir);
   dup_dir = strdup(src_file);

   src_dir = getenv("PATH");
   fail_if(!src_dir);
   path = malloc(strlen(src_dir) + strlen(dup_dir) + 1);
   *path = '\0';
   strcat(path, src_dir);
   strcat(path, ":");
   strcat(path, dirname(dup_dir));
   ret = setenv("PATH", path, 1);
   fail_if(ret != 0);
   free(dup_dir);
   free(path);

   ret = ecore_file_init();

   res = ecore_file_app_installed(dest_file);
   fail_if(res != EINA_TRUE);
   res = ecore_file_app_installed(src_file);
   fail_if(res != EINA_TRUE);
   list = NULL;
   list = ecore_file_app_list();
   fd = 0;
   EINA_LIST_FOREACH(list, l, path)
     {
        if (strcmp(path, src_file) == 0)
          {
             fd = 1;
             break;
          }
     }
   fail_if(fd == 0);
   EINA_LIST_FREE(list, dup_dir)
     free(dup_dir);
   ret = setenv("PATH", src_dir, 1);
   fail_if(ret != 0);

   fail_if(ecore_file_remove(src_file) != EINA_TRUE);

   ret = ecore_file_shutdown();
   fail_if(ret != 0);
}
EFL_END_TEST

EFL_START_TEST(ecore_test_ecore_file_monitor)
{
   Ecore_File_Monitor *mon;
   const char *src_dir;
   const char *file = "EcoreFileDest";
   const char *sub_dir[] = {"subdir1", 0};
   char dir_name[MAXSIZE] = {'\0'};
   char file_name[MAXSIZE] = {'\0'};
   char *random_text = "This is random test String";
   char *realp;
   Eina_Bool res;
   int ret;

   ret = ecore_file_init();
   fail_if(ret != 1);

   src_dir = get_tmp_dir();
   fail_if(!src_dir);

   realp = ecore_file_realpath(src_dir);
   fail_if(!realp);
   mon = ecore_file_monitor_add(realp, file_monitor_cb, NULL);
   fail_if(mon == NULL);

   strcat(file_name, src_dir);
   strcat(file_name, "/");
   strcat(file_name, file);
   _writeToFile(file_name, random_text);
   _writeToFile(file_name, random_text);

   ck_assert_str_eq(ecore_file_monitor_path_get(mon), realp);
   free(realp);

   ret = ecore_file_mksubdirs(src_dir, sub_dir);
   fail_if(ret != 1);

   res = ecore_file_remove(file_name);
   fail_if(res != EINA_TRUE);

   strcat(dir_name, src_dir);
   strcat(dir_name, "/");
   strcat(dir_name, sub_dir[0]);
   res = ecore_file_rmdir(dir_name);
   fail_if(res != EINA_TRUE);

   res = ecore_file_recursive_rm(src_dir);
   fail_if(res != EINA_TRUE);

   ecore_file_monitor_del(mon);

   ret = ecore_file_shutdown();
   fail_if(ret != 0);
}
EFL_END_TEST

EFL_START_TEST(ecore_test_ecore_file_download)
{
   const char *download_dir;
   const char *download_file;
   const char *download_url = "http://example.com";
   char dest_name[MAXSIZE] = {'\0'};
   Eina_Bool res;
   Eina_Hash *headers;
   Ecore_File_Download_Job *job = NULL;
   int ret;

   ret = ecore_file_init();
   fail_if(ret != 1);

   download_dir = get_tmp_dir();
   fail_if(!download_dir);
   download_file = ecore_file_file_get(download_url); //example.com
   fail_if(!download_file);
   fail_if(!ecore_file_download_protocol_available("http://"));
   strcat(dest_name, download_dir);
   strcat(dest_name, "/");
   strcat(dest_name, download_file);

   res = ecore_file_download("xxyyzz", dest_name, completion_cb,
                             progress_cb, NULL, &job);
   fail_if(res != EINA_FALSE);

   res = ecore_file_download(download_url, dest_name, err_completion_cb,
                             progress_cb, NULL, &job);
   fail_if(res != EINA_TRUE);
   fail_if(!job);
   ecore_file_download_abort(job);
   ecore_main_loop_begin();
   if (timeout_reached) goto end;
   res = ecore_file_remove(dest_name);
   fail_if(res != EINA_TRUE);

   headers = eina_hash_string_small_new(NULL);
   eina_hash_add(headers, "Content-type", "text/html");

   res = ecore_file_download_full(download_url, dest_name, completion_cb,
                                  progress_cb, NULL, &job, headers);
   fail_if(res != EINA_TRUE);
   fail_if(!job);
   ecore_main_loop_begin();
   if (timeout_reached) goto end;
   fprintf(stderr, "Downloaded %lld bytes\n", ecore_file_size(dest_name));
   res = ecore_file_exists(dest_name);
   fail_if(res != EINA_TRUE);
end:
   res = ecore_file_recursive_rm(download_dir);
   fail_if(res != EINA_TRUE);

   ret = ecore_file_shutdown();
   fail_if(ret != 0);
}
EFL_END_TEST

void ecore_test_ecore_file(TCase *tc)
{
   tcase_add_test(tc, ecore_test_ecore_file_init);
   tcase_add_test(tc, ecore_test_ecore_file_operations);
   tcase_add_test(tc, ecore_test_ecore_file_monitor);
   tcase_add_test(tc, ecore_test_ecore_file_download);
   tcase_add_test(tc, ecore_test_ecore_file_path);
}
