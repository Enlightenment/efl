/* EINA - EFL data type library
 * Copyright (C) 2008 Cedric Bail
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library;
 * if not, see <http://www.gnu.org/licenses/>.
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#include "eina_suite.h"
#include "Eina.h"
#include "eina_safety_checks.h"

static int default_dir_rights = 0777;
const int file_min_offset = 1;

#ifdef EINA_SAFETY_CHECKS
struct log_ctx {
   const char *msg;
   const char *fnc;
   Eina_Bool did;
};

/* tests should not output on success, just uncomment this for debugging */
//#define SHOW_LOG 1

static void
_eina_test_safety_print_cb(const Eina_Log_Domain *d, Eina_Log_Level level, const char *file, const char *fnc, int line, const char *fmt, void *data, va_list args EINA_UNUSED)
{
   struct log_ctx *ctx = data;
   va_list cp_args;
   const char *str;

   va_copy(cp_args, args);
   str = va_arg(cp_args, const char *);
   va_end(cp_args);

   ck_assert_int_eq(level, EINA_LOG_LEVEL_ERR);
   ck_assert_str_eq(fmt, "%s");
   ck_assert_str_eq(ctx->msg, str);
   ck_assert_str_eq(ctx->fnc, fnc);
   ctx->did = EINA_TRUE;

#ifdef SHOW_LOG
   eina_log_print_cb_stderr(d, level, file, fnc, line, fmt, NULL, args);
#else
   (void)d;
   (void)file;
   (void)line;
#endif
}
#endif


START_TEST(eina_file_split_simple)
{
   Eina_Array *ea;

   eina_init();

#ifdef EINA_SAFETY_CHECKS
#ifdef SHOW_LOG
   fprintf(stderr, "you should have a safety check failure below:\n");
#endif
   struct log_ctx ctx;

#define TEST_MAGIC_SAFETY(fn, _msg)             \
   ctx.msg = _msg;                              \
   ctx.fnc = fn;                                \
   ctx.did = EINA_FALSE

   eina_log_print_cb_set(_eina_test_safety_print_cb, &ctx);

   TEST_MAGIC_SAFETY("eina_file_split", "safety check failed: path == NULL");
   ea = eina_file_split(NULL);
   fail_if(ea);
   fail_unless(ctx.did);

   eina_log_print_cb_set(eina_log_print_cb_stderr, NULL);
#undef TEST_MAGIC_SAFETY
#endif

#ifdef _WIN32
   ea = eina_file_split(strdup("\\this\\is\\a\\small\\test"));
#else
   ea = eina_file_split(strdup("/this/is/a/small/test"));
#endif

   fail_if(!ea);
   fail_if(eina_array_count(ea) != 5);
   fail_if(strcmp(eina_array_data_get(ea, 0), "this"));
   fail_if(strcmp(eina_array_data_get(ea, 1), "is"));
   fail_if(strcmp(eina_array_data_get(ea, 2), "a"));
   fail_if(strcmp(eina_array_data_get(ea, 3), "small"));
   fail_if(strcmp(eina_array_data_get(ea, 4), "test"));

   eina_array_free(ea);

#ifdef _WIN32
   ea =
      eina_file_split(strdup(
                         "this\\\\is\\\\\\a \\more\\complex\\\\\\case\\\\\\"));
#else
   ea = eina_file_split(strdup("this//is///a /more/complex///case///"));
#endif

   fail_if(!ea);
   fail_if(eina_array_count(ea) != 6);
   fail_if(strcmp(eina_array_data_get(ea, 0), "this"));
   fail_if(strcmp(eina_array_data_get(ea, 1), "is"));
   fail_if(strcmp(eina_array_data_get(ea, 2), "a "));
   fail_if(strcmp(eina_array_data_get(ea, 3), "more"));
   fail_if(strcmp(eina_array_data_get(ea, 4), "complex"));
   fail_if(strcmp(eina_array_data_get(ea, 5), "case"));

   eina_array_free(ea);

   eina_shutdown();
}
END_TEST

Eina_Tmpstr*
get_full_path(const char* tmpdirname, const char* filename)
{
    char full_path[PATH_MAX] = "";
    eina_str_join(full_path, sizeof(full_path), '/', tmpdirname, filename);
    return eina_tmpstr_add(full_path);
}

Eina_Tmpstr*
get_eina_test_file_tmp_dir()
{
   Eina_Tmpstr *tmp_dir;

   Eina_Bool created = eina_file_mkdtemp("EinaFileTestXXXXXX", &tmp_dir);

   if (!created)
     {
        return NULL;
     }

   return tmp_dir;
}

START_TEST(eina_file_direct_ls_simple)
{
   eina_init();

   const char *good_dirs[] =
     {
        "eina_file_direct_ls_simple_dir",
        "a.",
        "$a$b",
        "~$a@:-*$b!{}"
     };
   const int good_dirs_count = sizeof(good_dirs) / sizeof(const char *);
   Eina_Tmpstr *test_dirname = get_eina_test_file_tmp_dir();
   fail_if(test_dirname == NULL);

   for (int i = 0; i != good_dirs_count; ++i)
     {
        Eina_Tmpstr *dirname = get_full_path(test_dirname, good_dirs[i]);
        // clean old test directories
        rmdir(dirname);
        fail_if(mkdir(dirname, default_dir_rights) != 0);

        Eina_File_Direct_Info *dir_info;
        Eina_Iterator *it = eina_file_direct_ls(test_dirname);
        Eina_Bool found_dir = EINA_FALSE;

        fail_if(!eina_iterator_container_get(it));
        EINA_ITERATOR_FOREACH(it, dir_info)
          {
             if (!strcmp(dir_info->path, dirname))
               {
                  found_dir = EINA_TRUE;
               }
          }

        eina_iterator_free(it);

        fail_if(!found_dir);
        fail_if(rmdir(dirname) != 0);

        eina_tmpstr_del(dirname);
     }
   fail_if(rmdir(test_dirname) != 0);
   eina_tmpstr_del(test_dirname);
   eina_shutdown();
}
END_TEST

START_TEST(eina_file_ls_simple)
{
   eina_init();

   const char *good_dirs[] =
     {
        "eina_file_ls_simple_dir",
        "b.",
        "$b$a",
        "~$b@:-*$a!{}"
     };
   const int good_dirs_count = sizeof(good_dirs) / sizeof(const char *);
   Eina_Tmpstr *test_dirname = get_eina_test_file_tmp_dir();
   fail_if(test_dirname == NULL);

   for (int i = 0; i != good_dirs_count; ++i)
     {
        Eina_Tmpstr *dirname = get_full_path(test_dirname, good_dirs[i]);
        // clean old test directories
        rmdir(dirname);
        fail_if(mkdir(dirname, default_dir_rights) != 0);

        char *filename;
        Eina_Iterator *it = eina_file_ls(test_dirname);
        Eina_Bool found_dir = EINA_FALSE;

        fail_if(!eina_iterator_container_get(it));
        EINA_ITERATOR_FOREACH(it, filename)
          {
             if (!strcmp(filename, dirname))
               {
                  found_dir = EINA_TRUE;
               }
          }

        eina_iterator_free(it);

        fail_if(!found_dir);
        fail_if(rmdir(dirname) != 0);

        eina_tmpstr_del(dirname);
     }
   fail_if(rmdir(test_dirname) != 0);
   eina_tmpstr_del(test_dirname);
   eina_shutdown();
}
END_TEST

START_TEST(eina_file_map_new_test)
{
   const char *eina_map_test_string = "Hello. I'm the eina map test string."; 
   const char *test_file_name_part = "/example.txt";
   const char *test_file2_name_part = "/example_big.txt";
   char *test_file_path, *test_file2_path;
   char *big_buffer;
   const char *template = "abcdefghijklmnopqrstuvwxyz";
   int template_size = strlen(template);
   int memory_page_size = eina_cpu_page_size();
   const int big_buffer_size = memory_page_size * 1.5;
   const int iteration_number = big_buffer_size / template_size;
   int test_string_length = strlen(eina_map_test_string);
   int test_full_filename_size;
   int test_full_filename2_size;
   Eina_File *e_file, *e_file2;
   void *file_map, *file2_map;  
   int fd, correct_file_open_check, correct_map_check, map_offset, map_length, file_length, file2_length;
   int test_file_name_part_size = strlen(test_file_name_part);
   int test_file2_name_part_size = strlen(test_file2_name_part);
   int test_dirname_size;
   int start_point_final, last_chunk_size;

   eina_init();

   Eina_Tmpstr *test_dirname = get_eina_test_file_tmp_dir();
   fail_if(test_dirname == NULL);
   test_dirname_size = strlen((char *)test_dirname);
   
   // memory allocation
   test_full_filename_size = test_file_name_part_size + test_dirname_size + 1;
   test_file_path = (char *)malloc(test_full_filename_size);

   test_full_filename2_size = test_file2_name_part_size + test_dirname_size + 1;
   test_file2_path = (char *)malloc(test_full_filename2_size);

   // Creating big buffer 1.5 * (memory page size)
   big_buffer = (char *)malloc(big_buffer_size);
   // Set first symbol == '/0' to strcat corret work
   big_buffer[0] = '\0';
   // iteration_number - quantity of templates that can be fully put in big_buff
   for (int i = 0; i < iteration_number; i++)
	   strcat (big_buffer, template);
   // calculating last chunk of data that < template_size
   last_chunk_size = big_buffer_size - iteration_number * template_size - file_min_offset;
   //calculating start point for final iteration_number
   start_point_final = iteration_number * template_size;
   strncpy ((big_buffer + start_point_final), template, last_chunk_size);
   // set last element of big_buffer in '\0' - end of string
   big_buffer[big_buffer_size - file_min_offset] = '\0';
   // check big_buffer valid length
   fail_if((int)strlen(big_buffer) != (big_buffer_size - file_min_offset));

   // generating file 1 full name
   strcpy(test_file_path, (char *)test_dirname);
   strcat(test_file_path, test_file_name_part);
   // generating file 2 full name
   strcpy(test_file2_path, (char *)test_dirname);
   strcat(test_file2_path, test_file2_name_part);
   
   fd = open(test_file_path, O_WRONLY | O_CREAT | O_TRUNC, S_IWUSR | S_IRUSR);
   fail_if(fd == 0);   
   fail_if(write(fd, eina_map_test_string, strlen(eina_map_test_string)) != (ssize_t) strlen(eina_map_test_string));
   close(fd);

   fd = open(test_file2_path, O_WRONLY | O_CREAT | O_TRUNC, S_IWUSR | S_IRUSR);
   fail_if(fd == 0);
   fail_if(write(fd, big_buffer, big_buffer_size - file_min_offset) != big_buffer_size - file_min_offset);
   close(fd);

   e_file = eina_file_open(test_file_path, EINA_FALSE);
   fail_if(!e_file);
   file_length = eina_file_size_get(e_file);
   correct_file_open_check = file_length - test_string_length; 
   // check size of eina_map_test_string == size of file
   fail_if(correct_file_open_check != 0);

   fail_if(eina_file_refresh(e_file));

   e_file2 = eina_file_open(test_file2_path, EINA_FALSE);
   fail_if(!e_file);
   file2_length = eina_file_size_get(e_file2);
   correct_file_open_check = file2_length - (big_buffer_size - file_min_offset); 
   // check size of big_buffer == size of file
   fail_if(correct_file_open_check != 0); 

   // test : offset > file -> length  => return NULL
   map_offset = test_string_length + file_min_offset;
   map_length = file_min_offset;
   file_map = eina_file_map_new(e_file, EINA_FILE_WILLNEED, map_offset, map_length); 
   fail_if(file_map);

   // test : offset + length > file -> length => return NULL
   map_offset = file_min_offset;
   map_length = test_string_length;
   file_map = eina_file_map_new(e_file, EINA_FILE_WILLNEED, map_offset, map_length); 
   fail_if(file_map);
   fail_if(eina_file_map_faulted(e_file, file_map));

   // test : offset = 0 AND length = file->length - use eina_file_map_all
   map_offset = 0;
   map_length = test_string_length;
   file_map = eina_file_map_new(e_file, EINA_FILE_WILLNEED, map_offset, map_length); 
   fail_if(!file_map);
   correct_map_check= strcmp((char*) file_map, eina_map_test_string); 
   fail_if(correct_map_check != 0);  

   eina_file_map_free(e_file, file_map);

   // test : offset = memory_page_size AND length = file->length - memory_page_size => correct partly map
   map_offset = memory_page_size;
   map_length = big_buffer_size - memory_page_size - file_min_offset;
   file2_map = eina_file_map_new(e_file2, EINA_FILE_WILLNEED, map_offset, map_length); 
   fail_if(!file2_map);
   correct_map_check = strcmp((char*)file2_map, big_buffer + memory_page_size); 
   fail_if(correct_map_check != 0);  
  
   unlink(test_file_path);
   unlink(test_file2_path);
   free(test_file_path);
   free(test_file2_path);
   eina_file_map_free(e_file, file_map);
   eina_file_map_free(e_file2, file2_map);   
   eina_file_close(e_file); 
   eina_file_close(e_file2);
   eina_tmpstr_del(test_dirname);

   eina_shutdown();
}
END_TEST

static const char *virtual_file_data = "this\n"
  "is a test for the sake of testing\n"
  "it should detect all the line of this\n"
  "\n"
  "\r\n"
  "file !\n"
  "without any issue !";

START_TEST(eina_test_file_virtualize)
{
   Eina_File *f;
   Eina_File *tmp;
   Eina_Iterator *it;
   Eina_File_Line *ln;
   void *map;
   const unsigned int check[] = { 1, 2, 3, 6, 7 };
   int i = 0;

   eina_init();

   f = eina_file_virtualize("gloubi", virtual_file_data, strlen(virtual_file_data), EINA_FALSE);
   fail_if(!f);

   fail_if(!eina_file_virtual(f));

   tmp = eina_file_dup(f);
   fail_if(!tmp);
   eina_file_close(tmp);

   fail_if(strcmp("gloubi", eina_file_filename_get(f)));

   map = eina_file_map_new(f, EINA_FILE_WILLNEED, 7, 7);
   fail_if(map != (virtual_file_data + 7));
   eina_file_map_free(f, map);

   it = eina_file_map_lines(f);
   EINA_ITERATOR_FOREACH(it, ln)
     {
        fail_if(ln->index != check[i]);
        i++;
     }
   fail_if(eina_iterator_container_get(it) != f);
   eina_iterator_free(it);

   fail_if(i != 5);

   eina_file_close(f);

   eina_shutdown();
}
END_TEST

static void *
_eina_test_file_thread(void *data EINA_UNUSED, Eina_Thread t EINA_UNUSED)
{
   Eina_File *f;
   unsigned int i;

   for (i = 0; i < 10000; ++i)
     {
        f = eina_file_open("/bin/sh", EINA_FALSE);
        fail_if(!f);
        eina_file_close(f);
     }

   return NULL;
}

START_TEST(eina_test_file_thread)
{
   Eina_Thread th[4];
   unsigned int i;

   fail_if(!eina_init());

   for (i = 0; i < 4; i++)
     fail_if(!(eina_thread_create(&th[i], EINA_THREAD_NORMAL, 0, _eina_test_file_thread, NULL)));

   for (i = 0; i < 4; i++)
     fail_if(eina_thread_join(th[i]) != NULL);

   eina_shutdown();
}
END_TEST

void
eina_test_file(TCase *tc)
{
   tcase_add_test(tc, eina_file_split_simple);
   tcase_add_test(tc, eina_file_direct_ls_simple);
   tcase_add_test(tc, eina_file_ls_simple);
   tcase_add_test(tc, eina_file_map_new_test);
   tcase_add_test(tc, eina_test_file_virtualize);
   tcase_add_test(tc, eina_test_file_thread);
}

