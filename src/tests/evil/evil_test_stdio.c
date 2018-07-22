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
#include <stdio.h>
#include <direct.h>

#include <Evil.h>

#include "evil_suite.h"

static int
_evil_test_stdio_file_new(const char *n, const char *t)
{
  FILE *f;

  f = fopen(n, "wb");
  if (!f)
    return 0;
  if (fwrite(t, 1, strlen(t), f) != strlen(t))
    {
      fclose(f);
      _unlink(n);
      return 0;
    }

  fclose(f);

  return 1;
}

static FILE *
_evil_test_stdio_file_new_and_opened(const char *n, const char *t)
{
  FILE *f;

  f = fopen(n, "wb");
  if (!f)
    return NULL;
  if (fwrite(t, 1, strlen(t), f) != strlen(t))
    {
      fclose(f);
      _unlink(n);
      return NULL;
    }

  return f;
}

EFL_START_TEST(evil_stdio_rename_src_file_none)
{
   int res;

   res = rename(NULL, NULL);
   fail_if(res != -1);

   res = rename("evil_foo.txt", NULL);
   fail_if(res != -1);
}
EFL_END_TEST

EFL_START_TEST(evil_stdio_rename_dst_file_none)
{
   int res1;
   int res2;
   int res3;
   int res4;

   res1 = _evil_test_stdio_file_new("foo.txt", "test file none");
   fail_if(res1 == 0);

   res1 = rename("foo.txt", NULL);
   res2 = rename("foo.txt", "bar.txt");
   res3 = _unlink("bar.txt");
   res4 = _unlink("foo.txt");

   fail_if(res1 == 0);
   fail_if(res2 == -1);
   fail_if(res3 == -1);
   fail_if(res4 == 0);
}
EFL_END_TEST

EFL_START_TEST(evil_stdio_rename_dst_file_exists)
{
   int res1;
   int res2;
   int res3;

   res1 = _evil_test_stdio_file_new("foo.txt", "test file exists foo");
   fail_if(res1 == 0);

   res2 = _evil_test_stdio_file_new("bar.txt", "test file exists bar");
   if (res2 == 0)
     _unlink("foo.txt");

   fail_if(res2 == 0);

   res1 = rename("foo.txt", "bar.txt");
   res2 = _unlink("bar.txt");
   res3 = _unlink("foo.txt");

   fail_if(res1 == -1);
   fail_if(res2 == -1);
   fail_if(res3 == 0);
}
EFL_END_TEST

EFL_START_TEST(evil_stdio_rename_dst_file_used)
{
   FILE *f;
   int res1;
   int res2;
   int res3;

   f = _evil_test_stdio_file_new_and_opened("foo.txt", "test file used foo");
   fail_if(f == NULL);

   res1 = _evil_test_stdio_file_new("bar.txt", "test file used bar");
   if (res1 == 0)
     {
        fclose(f);
        _unlink("foo.txt");
     }

   fail_if(res1 == 0);

   res1 = rename("foo.txt", "bar.txt");
   res2 = _unlink("bar.txt");
   fclose(f);
   res3 = _unlink("foo.txt");

   fail_if(res1 == 0);
   fail_if(res2 == -1);
   fail_if(res3 == -1);
}
EFL_END_TEST

EFL_START_TEST(evil_stdio_rename_dst_file_move_to_dir)
{
   int res1;
   int res2;
   int res3;
   int res4;

   res1 = _evil_test_stdio_file_new("foo.txt", "test file move foo");
   fail_if(res1 == 0);

   res2 = _mkdir("foo_dir");
   fail_if(res2 == -1);

   res1 = rename("foo.txt", "foo_dir/bar.txt");
   res2 = _unlink("foo_dir/bar.txt");
   res3 = _rmdir("foo_dir");
   res4 = _unlink("foo.txt");

   fail_if(res1 == -1);
   fail_if(res2 == -1);
   fail_if(res3 == -1);
   fail_if(res4 == 0);
}
EFL_END_TEST

EFL_START_TEST(evil_stdio_rename_dst_dir_none)
{
   int res1;
   int res2;
   int res3;

   res1 = _mkdir("foo_dir");
   fail_if(res1 == -1);

   res1 = rename("foo_dir", "bar_dir");
   res2 = _rmdir("bar_dir");
   res3 = _rmdir("foo_dir");

   fail_if(res1 == -1);
   fail_if(res2 == -1);
   fail_if(res3 == 0);
}
EFL_END_TEST

EFL_START_TEST(evil_stdio_rename_dst_dir_exists)
{
   int res1;
   int res2;
   int res3;

   res1 = _mkdir("foo_dir");
   fail_if(res1 == -1);

   res1 = _mkdir("bar_dir");
   if (res1 == -1)
     _rmdir("foo_dir");

   fail_if(res1 == -1);

   res1 = rename("foo_dir", "bar_dir");
   res2 = _rmdir("bar_dir");
   res3 = _rmdir("foo_dir");

   fail_if(res1 == -1);
   fail_if(res2 == -1);
   fail_if(res3 == 0);
}
EFL_END_TEST

void evil_test_stdio(TCase *tc)
{
   tcase_add_test(tc, evil_stdio_rename_src_file_none);

   tcase_add_test(tc, evil_stdio_rename_dst_file_none);
   tcase_add_test(tc, evil_stdio_rename_dst_file_exists);
   tcase_add_test(tc, evil_stdio_rename_dst_file_used);
   tcase_add_test(tc, evil_stdio_rename_dst_file_move_to_dir);

   tcase_add_test(tc, evil_stdio_rename_dst_dir_none);
   tcase_add_test(tc, evil_stdio_rename_dst_dir_exists);
}
