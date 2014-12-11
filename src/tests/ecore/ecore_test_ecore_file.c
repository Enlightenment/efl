#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include <Ecore_File.h>
#include <Ecore.h>
#include <Eina.h>

#include "ecore_suite.h"

#define MAXSIZE 256

void
_writeToFile(const char *filePath, char *text)
{
   FILE *f = fopen(filePath, "r+");
   if (f == NULL)
     f = fopen(filePath, "w");
   fail_if(f == NULL);
   fprintf(f, "%s\n", text);
   fclose(f);
}

Eina_Tmpstr*
get_tmp_dir()
{
   Eina_Tmpstr *tmp_dir;

   Eina_Bool created = eina_file_mkdtemp("EcoreFileTestXXXXXX", &tmp_dir);

   if (!created)
     {
        return NULL;
     }

   return tmp_dir;
}

Eina_Tmpstr*
get_tmp_file()
{
   Eina_Tmpstr *tmp_file;

   Eina_Bool created = eina_file_mkstemp("EcoreFileTestXXXXXX", &tmp_file);

   if (!created)
     {
        return NULL;
     }

   return tmp_file;
}

int
_compare_path(const char *filePathA, const char *filePathB)
{
   char realFilePathA[MAXSIZE];
   char realFilePathB[MAXSIZE];
#ifdef _WIN32
   GetFullPathName(filePathA, MAXSIZE, realFilePathA, NULL);
   GetFullPathName(filePathB, MAXSIZE, realFilePathB, NULL);
#else
   realpath(filePathA, realFilePathA);
   realpath(filePathB, realFilePathB);
#endif
   return strcmp(realFilePathA, realFilePathB);
}

#define assert_path_eq(X, Y)                                            \
  do {                                                                  \
     const char* _ck_x = (X);                                           \
     const char* _ck_y = (Y);                                           \
     ck_assert_msg(0 == _compare_path(_ck_y, _ck_x),                    \
                   "Assertion '%s' failed: %s==\"%s\", %s==\"%s\"", #X"=="#Y, #X, _ck_x, #Y, _ck_y); \
  } while (0)

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

int
progress_cb(void *data EINA_UNUSED, const char *file EINA_UNUSED,
            long int dltotal, long int dlnow,
            long int ultotal EINA_UNUSED, long int ulnow EINA_UNUSED)
{
   fprintf(stderr, "Progress: %ld/%ld\n", dlnow, dltotal);
   return ECORE_FILE_PROGRESS_CONTINUE;
}

START_TEST(ecore_test_ecore_file_init)
{
   int ret;

   ret = ecore_file_init();
   fail_if(ret != 1);

   ret = ecore_file_shutdown();
   fail_if(ret != 0);
}
END_TEST

START_TEST(ecore_test_ecore_file_operations)
{
   const char* dirs[] = {"b", "b/c", "b/c/d", "d", 0};
   const char *src_dir, *src_file, *dest_file;
   const char *tmpdir = NULL;
   char *random_text = "This is random test String";
   char dir[MAXSIZE] = {'\0'};
   unsigned int ret;
   Eina_Bool res;
   Eina_List *list;

#ifndef HAVE_EVIL
#if defined(HAVE_GETUID) && defined(HAVE_GETEUID)
   if (getuid() == geteuid())
#endif
     {
        tmpdir = getenv("TMPDIR");
        if (!tmpdir) tmpdir = getenv("XDG_RUNTIME_DIR");
     }
   if (!tmpdir) tmpdir = "/tmp";
#else
   tmpdir = (char *)evil_tmpdir_get();
#endif /* ! HAVE_EVIL */

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

   res = ecore_file_is_dir(src_dir);
   fail_if(res != EINA_TRUE);
   ret = ecore_file_dir_is_empty(src_dir);
   fail_if(ret != 1);
   res = ecore_file_mkdir(src_dir);
   fail_if(res != EINA_FALSE);
   res = ecore_file_remove(src_dir);
   fail_if(res != EINA_TRUE);

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

   res = ecore_file_symlink(src_file, dest_file);
   fail_if(res != EINA_TRUE);
   assert_path_eq(ecore_file_readlink(dest_file), src_file);
   assert_path_eq(ecore_file_realpath(dest_file), src_file);
   res = ecore_file_unlink(dest_file);
   fail_if(res != EINA_TRUE);

   dest_file = get_tmp_file();
   fail_if(!dest_file);
   res = ecore_file_mv(src_file, dest_file);
   fail_if(res != EINA_TRUE);
   res = ecore_file_exists(dest_file);
   fail_if(res != EINA_TRUE);
   res = ecore_file_exists(src_file);
   fail_if(res != EINA_FALSE);

   assert_path_eq(ecore_file_dir_get(dest_file), tmpdir);
   assert_path_eq(ecore_file_realpath(dest_file), dest_file);
   fail_if(ecore_file_mod_time(dest_file) == 0);
   fail_if(ecore_file_can_read(dest_file) != EINA_TRUE);
   fail_if(ecore_file_can_write(dest_file) != EINA_TRUE);
   fail_if(ecore_file_can_exec(dest_file) != EINA_FALSE);
   res = ecore_file_remove(dest_file);
   fail_if(res != EINA_TRUE);

   ret = ecore_file_shutdown();
   fail_if(ret != 0);

}
END_TEST

START_TEST(ecore_test_ecore_file_monitor)
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

   assert_path_eq(ecore_file_monitor_path_get(mon), realp);

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
END_TEST

START_TEST(ecore_test_ecore_file_download)
{
   const char *download_dir;
   const char *download_file;
   const char *download_url = "http://check.sourceforge.net/xml/check_unittest.xslt";
   char dest_name[MAXSIZE] = {'\0'};
   Eina_Bool res;
   Eina_Hash *headers;
   int ret;

   ret = ecore_file_init();
   fail_if(ret != 1);

   download_dir = get_tmp_dir();
   fail_if(!download_dir);
   download_file = ecore_file_file_get(download_url);
   fail_if(!download_file);
   strcat(dest_name, download_dir);
   strcat(dest_name, "/");
   strcat(dest_name, download_file);
   res = ecore_file_download(download_url, dest_name, completion_cb,
                             progress_cb, NULL, NULL);
   fail_if(res != EINA_TRUE);
   ecore_main_loop_begin();
   fprintf(stderr, "Downloaded %lld bytes\n", ecore_file_size(dest_name));
   res = ecore_file_exists(dest_name);
   fail_if(res != EINA_TRUE);
   res = ecore_file_unlink(dest_name);
   fail_if(res != EINA_TRUE);

   headers = eina_hash_string_small_new(NULL);
   eina_hash_add(headers, "Content-type", "text/xml");

   res = ecore_file_download_full(download_url, dest_name, completion_cb,
                                  progress_cb, NULL, NULL, headers);
   fail_if(res != EINA_TRUE);
   ecore_main_loop_begin();
   fprintf(stderr, "Downloaded %lld bytes\n", ecore_file_size(dest_name));
   res = ecore_file_exists(dest_name);
   fail_if(res != EINA_TRUE);

   ret = ecore_file_shutdown();
   fail_if(ret != 0);
}
END_TEST

void ecore_test_ecore_file(TCase *tc)
{
   tcase_add_test(tc, ecore_test_ecore_file_init);
   tcase_add_test(tc, ecore_test_ecore_file_operations);
   tcase_add_test(tc, ecore_test_ecore_file_monitor);
   tcase_add_test(tc, ecore_test_ecore_file_download);
}
