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
_writeToFile(char *filePath, char *text)
{
   FILE *f = fopen(filePath, "w");
   fail_if(!f);
   fprintf(f, "%s\n", text);
   fclose(f);
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
   const char *srcDir = "/tmp/rootDir";
   const char* dirs[] = {"b", "b/c", "b/c/d", "d"}; 
   char *srcFile = "/tmp/EcoreFileSrc.txt";
   const char *destFile = "/tmp/EcoreFileDest.txt";
   char *randomText = "This is random test String";
   char dir[MAXSIZE] = {'\0'};
   int ret;
   Eina_Bool res;
   Eina_List *list;

   char unqFile[MAXSIZE], unqDir[MAXSIZE];
   char *ptr;

   ret = ecore_file_init();
   fail_if(ret != 1);
   
   ptr = tmpnam(unqFile);
   fail_if(!ptr);

   ptr = tmpnam(unqDir);
   fail_if(!ptr);

   res = ecore_file_exists(unqFile);
   fail_if(res != EINA_FALSE);

   res = ecore_file_is_dir(unqFile);
   fail_if(res != EINA_FALSE);
   res = ecore_file_is_dir(unqDir);
   fail_if(res != EINA_FALSE);
   
   res = ecore_file_mkdir(srcDir);
   fail_if(res != EINA_TRUE);
   res = ecore_file_is_dir(srcDir);
   fail_if(res != EINA_TRUE);
   ret = ecore_file_dir_is_empty(srcDir);
   fail_if(res != 1);
   res = ecore_file_remove(srcDir);
   fail_if(res != EINA_TRUE);

   ret = ecore_file_mksubdirs(srcDir, dirs);
   fail_if(ret != 4);
   list = ecore_file_ls(srcDir);
   fail_if(!list);
   fail_if(eina_list_count(list) != 2);
   res = ecore_file_recursive_rm(srcDir);
   fail_if(res != EINA_TRUE);

   strcat(dir, srcDir);
   strcat(dir, "/");
   strcat(dir, dirs[2]);
   res = ecore_file_mkpath(dir);
   fail_if(res != EINA_TRUE);
   res = ecore_file_recursive_rm(srcDir);
   fail_if(res != EINA_TRUE);

   _writeToFile(srcFile, randomText);

   ret = ecore_file_size(srcFile);
   fail_if(ret != strlen(randomText)+1);

   res = ecore_file_cp(srcFile, destFile);
   fail_if(res != EINA_TRUE);
   res = ecore_file_exists(destFile);
   fail_if(res != EINA_TRUE);
   fail_if(ecore_file_size(srcFile) != ecore_file_size(destFile));
   res = ecore_file_remove(destFile);
   fail_if(res != EINA_TRUE);
  
   res = ecore_file_symlink(srcFile, destFile);
   fail_if(res != EINA_TRUE);
   ck_assert_str_eq(ecore_file_readlink(destFile), srcFile);
   res = ecore_file_unlink(destFile);
   fail_if(res != EINA_TRUE);

   res = ecore_file_mv(srcFile, destFile);
   fail_if(res != EINA_TRUE);
   res = ecore_file_exists(destFile);
   fail_if(res != EINA_TRUE);
   res = ecore_file_exists(srcFile);
   fail_if(res != EINA_FALSE);

   ck_assert_str_eq(ecore_file_file_get(destFile), "EcoreFileDest.txt");
   ck_assert_str_eq(ecore_file_dir_get(destFile), "/tmp");
   ck_assert_str_eq(ecore_file_realpath(destFile), destFile);
   fail_if(ecore_file_mod_time(destFile) == 0);
   fail_if(ecore_file_can_read(destFile) != EINA_TRUE);
   fail_if(ecore_file_can_write(destFile) != EINA_TRUE);
   fail_if(ecore_file_can_exec(destFile) != EINA_FALSE);
   res = ecore_file_remove(destFile);
   fail_if(res != EINA_TRUE);
 
   ret = ecore_file_shutdown();
   fail_if(ret != 0);

}
END_TEST

void ecore_test_ecore_file(TCase *tc)
{
   tcase_add_test(tc, ecore_test_ecore_file_init);
   tcase_add_test(tc, ecore_test_ecore_file_operations);
}
