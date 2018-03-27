/* EINA - EFL data type library
 * Copyright (C) 2015 Vivek Ellur
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

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

#include <Eina.h>

#include "eina_suite.h"

Eina_Tmpstr*
get_file_path(const char* tmpdirname, const char* filename)
{
    char file_path[PATH_MAX] = "";
    eina_str_join(file_path, sizeof(file_path), '/', tmpdirname, filename);
    return eina_tmpstr_add(file_path);
}

#ifdef XATTR_TEST_DIR
EFL_START_TEST(eina_test_xattr_set)
{
   char *filename = "tmpfile";
   char *attribute1 = "user.comment1";
   char *data1 = "This is comment 1";
   char *data2 = "This is comment 2";
   char *ret_str;
   int fd;
   ssize_t len;
   Eina_Bool ret;
   Eina_Tmpstr *test_file_path;


   test_file_path = get_file_path(XATTR_TEST_DIR, filename);
   fd = open(test_file_path, O_WRONLY | O_CREAT | O_TRUNC, S_IRWXU | S_IRWXG | S_IRWXO);
   fail_if(fd == 0);

   ret = eina_xattr_set(test_file_path, attribute1, data1, strlen(data1), EINA_XATTR_INSERT);
   fail_if(ret != EINA_TRUE);
   ret_str = eina_xattr_get(test_file_path, attribute1, &len);
   fail_if(ret_str == NULL);
   fail_if(len == 0);
   fail_if(strcmp(ret_str, data1) != 0);
   free(ret_str);

   ret = eina_xattr_set(test_file_path, attribute1, data1, strlen(data1), EINA_XATTR_CREATED);
   fail_if(ret != EINA_FALSE);

   ret = eina_xattr_set(test_file_path, attribute1, data2, strlen(data2), EINA_XATTR_REPLACE);
   fail_if(ret != EINA_TRUE);
   ret_str = eina_xattr_get(test_file_path, attribute1, &len);
   fail_if(ret_str == NULL);
   fail_if(len == 0);
   fail_if(strcmp(ret_str, data2) != 0);
   free(ret_str);

   ret = eina_xattr_del(test_file_path, attribute1);
   fail_if(ret != EINA_TRUE);

   ret = eina_xattr_fd_set(fd, attribute1, data1, strlen(data1), EINA_XATTR_CREATED);
   fail_if(ret != EINA_TRUE);
   ret_str = eina_xattr_fd_get(fd, attribute1, &len);
   fail_if(ret_str == NULL);
   fail_if(len == 0);
   fail_if(strcmp(ret_str, data1) != 0);
   free(ret_str);

   ret = eina_xattr_fd_del(fd, attribute1);
   fail_if(ret != EINA_TRUE);

   close(fd);
   unlink(test_file_path);
   eina_tmpstr_del(test_file_path);

}
EFL_END_TEST

EFL_START_TEST(eina_test_xattr_list)
{
   char *filename = "tmpfile";
   char *filename_cp = "tmpfile_cp";
   const char *attribute[] =
     {
        "user.comment1",
        "user.comment2",
        "user.comment3"
     };
   const char *data[] =
     {
        "This is a test file",
        "This line is a comment",
        "This file has extra attributes"
     };
   char *ret_str;
   int fd, fd1, count=0;
   unsigned int i;
   Eina_Bool ret;
   Eina_Tmpstr *test_file_path, *cp_file_path;
   Eina_Iterator *it;
   Eina_Xattr *xattr;


   test_file_path = get_file_path(XATTR_TEST_DIR, filename);
   cp_file_path = get_file_path(XATTR_TEST_DIR, filename_cp);

   fd = open(test_file_path, O_WRONLY | O_CREAT | O_TRUNC, S_IRWXU | S_IRWXG | S_IRWXO);
   fail_if(fd == 0);
   fd1 = open(cp_file_path, O_WRONLY | O_CREAT | O_TRUNC, S_IRWXU | S_IRWXG | S_IRWXO);
   fail_if(fd1 == 0);

   for (i = 0; i < sizeof(attribute) / sizeof(attribute[0]); ++i)
     {
        ret = eina_xattr_set(test_file_path, attribute[i], data[i], strlen(data[i]), EINA_XATTR_INSERT);
        fail_if(ret != EINA_TRUE);
     }

   it = eina_xattr_ls(test_file_path);
   EINA_ITERATOR_FOREACH(it, ret_str)
     {
        for (i = 0; i < sizeof (attribute) / sizeof (attribute[0]); i++)
          if (strcmp(attribute[i], ret_str) == 0)
            {
               count++;
               break ;
            }
     }
   fail_if(count != sizeof (attribute) / sizeof (attribute[0]));
   eina_iterator_free(it);

   count = 0;
   it = eina_xattr_value_ls(test_file_path);
   EINA_ITERATOR_FOREACH(it, xattr)
     {
        for (i = 0; i < sizeof (data) / sizeof (data[0]); i++)
          if (strcmp(attribute[i], xattr->name) == 0 &&
              strcmp(data[i], xattr->value) == 0)
            {
               count++;
               break ;
            }
     }
   fail_if(count != sizeof (data) / sizeof (data[0]));
   eina_iterator_free(it);

   count = 0;
   it = eina_xattr_fd_ls(fd);
   EINA_ITERATOR_FOREACH(it, ret_str)
     {
        for (i = 0; i < sizeof (attribute) / sizeof (attribute[0]); i++)
          if (strcmp(attribute[i], ret_str) == 0)
            {
               count++;
               break ;
            }
     }
   fail_if(count != sizeof (attribute) / sizeof (attribute[0]));
   eina_iterator_free(it);

   count = 0;
   it = eina_xattr_value_fd_ls(fd);
   EINA_ITERATOR_FOREACH(it, xattr)
     {
        for (i = 0; i < sizeof (data) / sizeof (data[0]); i++)
          if (strcmp(attribute[i], xattr->name) == 0 &&
              strcmp(data[i], xattr->value) == 0)
            {
               count++;
               break ;
            }
     }
   fail_if(count != sizeof (data) / sizeof (data[0]));
   eina_iterator_free(it);

   /* Test case for eina_xattr_copy and eina_xattr_fd_copy */
   ret = eina_xattr_copy(test_file_path, cp_file_path);
   fail_if(ret != EINA_TRUE);

   count = 0;
   it = eina_xattr_value_ls(cp_file_path);
   EINA_ITERATOR_FOREACH(it, xattr)
     {
        for (i = 0; i < sizeof (data) / sizeof (data[0]); i++)
          if (strcmp(attribute[i], xattr->name) == 0 &&
              strcmp(data[i], xattr->value) == 0)
            {
               count++;
               break ;
            }
     }
   fail_if(count != sizeof (data) / sizeof (data[0]));
   eina_iterator_free(it);

   for (i = 0; i < sizeof(attribute) / sizeof(attribute[0]); ++i)
     {
        ret = eina_xattr_del(cp_file_path, attribute[i]);
        fail_if(ret != EINA_TRUE);
     }

   ret = eina_xattr_fd_copy(fd, fd1);
   fail_if(ret != EINA_TRUE);

   count = 0;
   it = eina_xattr_value_fd_ls(fd1);
   EINA_ITERATOR_FOREACH(it, xattr)
     {
        for (i = 0; i < sizeof (data) / sizeof (data[0]); i++)
          if (strcmp(attribute[i], xattr->name) == 0 &&
              strcmp(data[i], xattr->value) == 0)
            {
               count++;
               break ;
            }
     }
   fail_if(count != sizeof (data) / sizeof (data[0]));
   eina_iterator_free(it);

   close(fd);
   close(fd1);
   unlink(test_file_path);
   unlink(cp_file_path);
   eina_tmpstr_del(test_file_path);
   eina_tmpstr_del(cp_file_path);
}
EFL_END_TEST

EFL_START_TEST(eina_test_xattr_types)
{
   char *filename = "tmpfile";
   char *str_attr = "user.name", *ret_str;
   char *str_data = "Vivek Ellur";
   char *int_attr = "user.id";
   int  int_data = 1234, int_ret;
   char *double_attr = "user.size";
   double double_data = 123.456, double_ret;
   int fd;
   Eina_Bool ret;
   Eina_Tmpstr *test_file_path;


   test_file_path = get_file_path(XATTR_TEST_DIR, filename);
   fd = open(test_file_path, O_WRONLY | O_CREAT | O_TRUNC, S_IRWXU | S_IRWXG | S_IRWXO);
   fail_if(fd == 0);

   ret = eina_xattr_string_set(test_file_path, str_attr, str_data, EINA_XATTR_INSERT);
   fail_if(ret != EINA_TRUE);
   ret_str = eina_xattr_string_get(test_file_path, str_attr);
   fail_if(ret_str == NULL);
   fail_if(strcmp(ret_str, str_data) != 0);
   free(ret_str);

   ret = eina_xattr_int_set(test_file_path, int_attr, int_data, EINA_XATTR_INSERT);
   fail_if(ret != EINA_TRUE);
   ret = eina_xattr_int_get(test_file_path, int_attr, &int_ret);
   fail_if(ret != EINA_TRUE);
   fail_if(int_data != int_ret);

   ret = eina_xattr_double_set(test_file_path, double_attr, double_data, EINA_XATTR_INSERT);
   fail_if(ret != EINA_TRUE);
   ret = eina_xattr_double_get(test_file_path, double_attr, &double_ret);
   fail_if(ret != EINA_TRUE);
   fail_if(!EINA_DBL_EQ(double_data, double_ret));

   close(fd);
   unlink(test_file_path);
   eina_tmpstr_del(test_file_path);

}
EFL_END_TEST
#endif

void
eina_test_xattr(TCase *tc)
{
#ifdef XATTR_TEST_DIR
   tcase_add_test(tc, eina_test_xattr_set);
   tcase_add_test(tc, eina_test_xattr_list);
   tcase_add_test(tc, eina_test_xattr_types);
#else
   (void)tc;
#endif
}
