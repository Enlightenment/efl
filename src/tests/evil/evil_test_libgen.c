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
#include <string.h>
#include <libgen.h>

#include "evil_suite.h"

EFL_START_TEST(evil_libgen_null)
{
   char *dname;
   char *bname;

   dname = dirname(NULL);
   fail_if(strcmp(dname, ".") != 0);
   bname = basename(NULL);
   fail_if(strcmp(bname, ".") != 0);
}
EFL_END_TEST

EFL_START_TEST(evil_libgen_empty)
{
   const char *path = "";
   char *dirc;
   char *basec;
   char *dname;
   char *bname;

   dirc = strdup(path);
   fail_if(dirc == NULL);
   basec = strdup(path);
   fail_if(basec == NULL);

   dname = dirname(dirc);
   fail_if(strcmp(dname, ".") != 0);
   bname = basename(basec);
   fail_if(strcmp(bname, ".") != 0);

   free(dirc);
   free(basec);
}
EFL_END_TEST

EFL_START_TEST(evil_libgen_path)
{
   const char *path = "/usr/lib";
   char *dirc;
   char *basec;
   char *dname;
   char *bname;

   dirc = strdup(path);
   fail_if(dirc == NULL);
   basec = strdup(path);
   fail_if(basec == NULL);

   dname = dirname(dirc);
   fail_if(strcmp(dname, "/usr") != 0);
   bname = basename(basec);
   fail_if(strcmp(bname, "lib") != 0);

   free(dirc);
   free(basec);
}
EFL_END_TEST

EFL_START_TEST(evil_libgen_path_with_slash)
{
   const char *path = "/usr/";
   char *dirc;
   char *basec;
   char *dname;
   char *bname;

   dirc = strdup(path);
   fail_if(dirc == NULL);
   basec = strdup(path);
   fail_if(basec == NULL);

   dname = dirname(dirc);
   fail_if(strcmp(dname, "/") != 0);
   bname = basename(basec);
   fail_if(strcmp(bname, "usr") != 0);

   free(dirc);
   free(basec);
}
EFL_END_TEST

EFL_START_TEST(evil_libgen_folder)
{
   const char *path = "usr";
   char *dirc;
   char *basec;
   char *dname;
   char *bname;

   dirc = strdup(path);
   fail_if(dirc == NULL);
   basec = strdup(path);
   fail_if(basec == NULL);

   dname = dirname(dirc);
   fail_if(strcmp(dname, ".") != 0);
   bname = basename(basec);
   fail_if(strcmp(bname, "usr") != 0);

   free(dirc);
   free(basec);
}
EFL_END_TEST

EFL_START_TEST(evil_libgen_slash)
{
   const char *path = "/";
   char *dirc;
   char *basec;
   char *dname;
   char *bname;

   dirc = strdup(path);
   fail_if(dirc == NULL);
   basec = strdup(path);
   fail_if(basec == NULL);

   dname = dirname(dirc);
   fail_if(strcmp(dname, "/") != 0);
   bname = basename(basec);
   fail_if(strcmp(bname, "/") != 0);

   free(dirc);
   free(basec);
}
EFL_END_TEST

EFL_START_TEST(evil_libgen_dot)
{
   const char *path = ".";
   char *dirc;
   char *basec;
   char *dname;
   char *bname;

   dirc = strdup(path);
   fail_if(dirc == NULL);
   basec = strdup(path);
   fail_if(basec == NULL);

   dname = dirname(dirc);
   fail_if(strcmp(dname, ".") != 0);
   bname = basename(basec);
   fail_if(strcmp(bname, ".") != 0);

   free(dirc);
   free(basec);
}
EFL_END_TEST

EFL_START_TEST(evil_libgen_dot_dot)
{
   const char *path = "..";
   char *dirc;
   char *basec;
   char *dname;
   char *bname;

   dirc = strdup(path);
   fail_if(dirc == NULL);
   basec = strdup(path);
   fail_if(basec == NULL);

   dname = dirname(dirc);
   fail_if(strcmp(dname, ".") != 0);
   bname = basename(basec);
   fail_if(strcmp(bname, "..") != 0);

   free(dirc);
   free(basec);
}
EFL_END_TEST

EFL_START_TEST(evil_libgen_win_dir_1)
{
   const char *path = "c:/foo/bar";
   char *dirc;
   char *basec;
   char *dname;
   char *bname;

   dirc = strdup(path);
   fail_if(dirc == NULL);
   basec = strdup(path);
   fail_if(basec == NULL);

   dname = dirname(dirc);
   fail_if(strcmp(dname, "c:/foo") != 0);
   bname = basename(basec);
fail_if(strcmp(bname, "bar") != 0);

   free(dirc);
   free(basec);
}
EFL_END_TEST

EFL_START_TEST(evil_libgen_win_dir_2)
{
   const char *path = "c:/foo\\bar";
   char *dirc;
   char *basec;
   char *dname;
   char *bname;

   dirc = strdup(path);
   fail_if(dirc == NULL);
   basec = strdup(path);
   fail_if(basec == NULL);

   dname = dirname(dirc);
   fail_if(strcmp(dname, "c:/foo") != 0);
   bname = basename(basec);
   fail_if(strcmp(bname, "bar") != 0);

   free(dirc);
   free(basec);
}
EFL_END_TEST

EFL_START_TEST(evil_libgen_win_dir_3)
{
   const char *path = "c:\\foo/bar";
   char *dirc;
   char *basec;
   char *dname;
   char *bname;

   dirc = strdup(path);
   fail_if(dirc == NULL);
   basec = strdup(path);
   fail_if(basec == NULL);

   dname = dirname(dirc);
   fail_if(strcmp(dname, "c:\\foo") != 0);
   bname = basename(basec);
   fail_if(strcmp(bname, "bar") != 0);

   free(dirc);
   free(basec);
}
EFL_END_TEST

EFL_START_TEST(evil_libgen_win_dir_4)
{
   const char *path = "c:\\foo\\bar";
   char *dirc;
   char *basec;
   char *dname;
   char *bname;

   dirc = strdup(path);
   fail_if(dirc == NULL);
   basec = strdup(path);
   fail_if(basec == NULL);

   dname = dirname(dirc);
   fail_if(strcmp(dname, "c:\\foo") != 0);
   bname = basename(basec);
   fail_if(strcmp(bname, "bar") != 0);

   free(dirc);
   free(basec);
}
EFL_END_TEST

void evil_test_libgen(TCase *tc)
{
   tcase_add_test(tc, evil_libgen_null);
   tcase_add_test(tc, evil_libgen_empty);
   tcase_add_test(tc, evil_libgen_path);
   tcase_add_test(tc, evil_libgen_path_with_slash);
   tcase_add_test(tc, evil_libgen_folder);
   tcase_add_test(tc, evil_libgen_slash);
   tcase_add_test(tc, evil_libgen_dot);
   tcase_add_test(tc, evil_libgen_dot_dot);
   tcase_add_test(tc, evil_libgen_win_dir_1);
   tcase_add_test(tc, evil_libgen_win_dir_2);
   tcase_add_test(tc, evil_libgen_win_dir_3);
   tcase_add_test(tc, evil_libgen_win_dir_4);
}
