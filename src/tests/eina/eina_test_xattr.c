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

#include "eina_suite.h"
#include "Eina.h"

Eina_Tmpstr*
get_file_path(const char* tmpdirname, const char* filename)
{
    char file_path[PATH_MAX] = "";
    eina_str_join(file_path, sizeof(file_path), '/', tmpdirname, filename);
    return eina_tmpstr_add(file_path);
}

#ifdef XATTR_TEST_DIR
START_TEST(eina_test_xattr_set)
{
   char *filename = "tmpfile";
   char *attribute1 = "user.comment1";
   char *data1 = "This is comment 1";
   char *attribute2 = "user.comment2";
   char *data2 = "This is comment 2";
   char *ret_str;
   int fd, len;
   Eina_Bool ret;
   Eina_Tmpstr *test_file_path;

   eina_init();

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

   ret = eina_xattr_set(test_file_path, attribute2, data2, strlen(data2), EINA_XATTR_REPLACE);
   fail_if(ret != EINA_FALSE);

   ret = eina_xattr_del(test_file_path, attribute1);
   fail_if(ret != EINA_TRUE);

   ret = eina_xattr_del(test_file_path, attribute2);
   fail_if(ret != EINA_FALSE);

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

   eina_shutdown();
}
END_TEST
#endif

void
eina_test_xattr(TCase *tc)
{
#ifdef XATTR_TEST_DIR
   tcase_add_test(tc, eina_test_xattr_set);
#endif
}
