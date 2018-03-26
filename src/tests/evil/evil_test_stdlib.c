/* EVIL - EFL library for Windows port
 * Copyright (C) 2017 Vincent Torri
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

#include <Evil.h>

#include "evil_suite.h"

EFL_START_TEST(evil_stdlib_setenv_NULL)
{
   char *val;
   int   res;

   res = setenv("EVIL_TEST_ENV", NULL, 1);
   fail_if(res < 0);

   val = getenv("EVIL_TEST_ENV");
   fail_if(val != 0);
}
EFL_END_TEST

EFL_START_TEST(evil_stdlib_setenv_NULL_after_set)
{
   char *val;
   int   res;

   res = setenv("EVIL_TEST_ENV", "val", 1);
   fail_if(res < 0);

   val = getenv("EVIL_TEST_ENV");
   fail_if(val == 0);

   fail_if(strcmp(val, "val") != 0);

   res = setenv("EVIL_TEST_ENV", NULL, 1);
   fail_if(res < 0);

   val = getenv("EVIL_TEST_ENV");
   fail_if(val != 0);
}
EFL_END_TEST

EFL_START_TEST(evil_stdlib_getenv_one)
{
   char *val;
   int   res;

   res = setenv("EVIL_TEST_ENV", "val", 1);
   fail_if(res < 0);

   val = getenv("EVIL_TEST_ENV");
   fail_if(val == 0);

   fail_if(strcmp(val, "val") != 0);

   res = setenv("EVIL_TEST_ENV", NULL, 1);
   fail_if(res < 0);

   val = getenv("EVIL_TEST_ENV");
   fail_if(val != 0);
}
EFL_END_TEST

EFL_START_TEST(evil_stdlib_getenv_two)
{
   char *val;
   int   res;

   res = setenv("EVIL_TEST_ENV1", "val1", 1);
   fail_if(res < 0);

   res = setenv("EVIL_TEST_ENV2", "val2", 1);
   fail_if(res < 0);

   val = getenv("EVIL_TEST_ENV1");
   fail_if(val == 0);

   fail_if(strcmp(val, "val1") != 0);

   val = getenv("EVIL_TEST_ENV2");
   fail_if(val == 0);

   fail_if(strcmp(val, "val2") != 0);

   res = setenv("EVIL_TEST_ENV1", NULL, 1);
   fail_if(res < 0);

   val = getenv("EVIL_TEST_ENV1");
   fail_if(val != 0);

   res = setenv("EVIL_TEST_ENV2", NULL, 1);
   fail_if(res < 0);

   val = getenv("EVIL_TEST_ENV2");
   fail_if(val != 0);
}
EFL_END_TEST

EFL_START_TEST(evil_stdlib_getenv_two_swapped)
{
   char *val;
   int   res;

   res = setenv("EVIL_TEST_ENV1", "val1", 1);
   fail_if(res < 0);

   res = setenv("EVIL_TEST_ENV2", "val2", 1);
   fail_if(res < 0);

   val = getenv("EVIL_TEST_ENV2");
   fail_if(val == 0);

   fail_if(strcmp(val, "val2") != 0);

   val = getenv("EVIL_TEST_ENV1");
   fail_if(val == 0);

   fail_if(strcmp(val, "val1") != 0);

   res = setenv("EVIL_TEST_ENV1", NULL, 1);
   fail_if(res < 0);

   val = getenv("EVIL_TEST_ENV1");
   fail_if(val != 0);

   res = setenv("EVIL_TEST_ENV2", NULL, 1);
   fail_if(res < 0);

   val = getenv("EVIL_TEST_ENV2");
   fail_if(val != 0);
}
EFL_END_TEST

EFL_START_TEST(evil_stdlib_unsetenv)
{
   char *val;
   int   res;

   res = setenv("EVIL_TEST_ENV", "val", 1);
   fail_if(res < 0);

   val = getenv("EVIL_TEST_ENV");
   fail_if(val == 0);

   fail_if(unsetenv("EVIL_TEST_ENV") != 0);

   val = getenv("EVIL_TEST_ENV");
   fail_if(val != 0);
}
EFL_END_TEST

EFL_START_TEST(evil_stdlib_mkdtemp)
{
   char template[] = "file_XXXXXX";
   char *res;

   res = mkdtemp(template);
   fail_if(res == NULL);

   fail_if(rmdir(res) < 0);
}
EFL_END_TEST

EFL_START_TEST(evil_stdlib_mkdtemp_fail)
{
   char template[] = "file_XXX";
   char *res;

   res = mkdtemp(template);
   fail_if(res != NULL);
}
EFL_END_TEST

EFL_START_TEST(evil_stdlib_mkstemp)
{
   char template[] = "file_XXXXXX";
   int fd;

   fd = mkstemp(template);
   fail_if(fd < 0);

   fail_if(close(fd) == -1);

   fail_if(unlink(template) == -1);
}
EFL_END_TEST

EFL_START_TEST(evil_stdlib_mkstemp_fail)
{
   char template[] = "file_XXX";
   int fd;

   fd = mkstemp(template);
   fail_if(fd >= 0);
}
EFL_END_TEST

EFL_START_TEST(evil_stdlib_mkstemps)
{
   char template[] = "file_XXXXXX.ext";
   int fd;

   fd = mkstemps(template, 4);
   fail_if(fd < 0);

   fail_if(close(fd) == -1);

   fail_if(unlink(template) == -1);
}
EFL_END_TEST

EFL_START_TEST(evil_stdlib_mkstemps_fail_1)
{
   char template[] = "file_XXX.ext";
   int fd;

   fd = mkstemps(template, 4);
   fail_if(fd >= 0);
}
EFL_END_TEST

EFL_START_TEST(evil_stdlib_mkstemps_fail_2)
{
   char template[] = "file_XXX";
   int fd;

   fd = mkstemps(template, 4);
   fail_if(fd >= 0);
}
EFL_END_TEST

EFL_START_TEST(evil_stdlib_realpath_1)
{
   char buf[PATH_MAX];
   char *filename = "C:\\Windows\\System32\\kernel32.dll";
   char *res;

   res = realpath(filename, buf);
   fail_if(res == NULL);
}
EFL_END_TEST

EFL_START_TEST(evil_stdlib_realpath_2)
{
   char buf[PATH_MAX];
   char *filename = "C:\\Windows\\System32\\.\\kernel32.dll";
   char *res;

   res = realpath(filename, buf);
   fail_if(res == NULL);
}
EFL_END_TEST

EFL_START_TEST(evil_stdlib_realpath_3)
{
   char buf[PATH_MAX];
   char *filename = "C:\\Windows\\System32\\..\\System32\\kernel32.dll";
   char *res;

   res = realpath(filename, buf);
   fail_if(res == NULL);
}
EFL_END_TEST

EFL_START_TEST(evil_stdlib_realpath_fail)
{
   char buf[PATH_MAX];
   char *filename = "C:\\Windows\\System32\\System32\\kernel.dll";
   char *res;

   res = realpath(filename, buf);
   fail_if(res != NULL);
}
EFL_END_TEST

void evil_test_stdlib(TCase *tc)
{
   tcase_add_test(tc, evil_stdlib_setenv_NULL);
   tcase_add_test(tc, evil_stdlib_setenv_NULL_after_set);
   tcase_add_test(tc, evil_stdlib_getenv_one);
   tcase_add_test(tc, evil_stdlib_getenv_two);
   tcase_add_test(tc, evil_stdlib_getenv_two_swapped);
   tcase_add_test(tc, evil_stdlib_unsetenv);

   tcase_add_test(tc, evil_stdlib_mkdtemp);
   tcase_add_test(tc, evil_stdlib_mkdtemp_fail);
   tcase_add_test(tc, evil_stdlib_mkstemp);
   tcase_add_test(tc, evil_stdlib_mkstemp_fail);
   tcase_add_test(tc, evil_stdlib_mkstemps);
   tcase_add_test(tc, evil_stdlib_mkstemps_fail_1);
   tcase_add_test(tc, evil_stdlib_mkstemps_fail_2);

   tcase_add_test(tc, evil_stdlib_realpath_1);
   tcase_add_test(tc, evil_stdlib_realpath_2);
   tcase_add_test(tc, evil_stdlib_realpath_3);
   tcase_add_test(tc, evil_stdlib_realpath_fail);
}
