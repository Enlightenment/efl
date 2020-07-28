#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <stdlib.h>
#include <string.h>

#include <evil_private.h>

#include "evil_suite.h"

EFL_START_TEST(evil_libgen_basename_simple)
{
   char path[] = "/bin/foo/bar.h";
   char expected[] = "bar.h";
   char *res = evil_basename(path);

   fail_if(strcmp(res, expected) != 0);
}
EFL_END_TEST

EFL_START_TEST(evil_libgen_basename_short)
{
   char path[] = "a";
   char expected[] = "a";
   char *res = evil_basename(path);

   fail_if(strcmp(res, expected) != 0);
}
EFL_END_TEST

EFL_START_TEST(evil_libgen_basename_root)
{
   char path[] = "/";
   char expected[] = "/";
   char *res = evil_basename(path);

   fail_if(strcmp(res, expected) != 0);
}
EFL_END_TEST

EFL_START_TEST(evil_libgen_basename_trailing)
{
   char path[] = "/bin/foo/";
   char expected[] = "foo";

   char *res = evil_basename(path);

   fail_if(strcmp(res, expected) != 0);
}
EFL_END_TEST

EFL_START_TEST(evil_libgen_basename_empty)
{
   char path[] = "";
   char expected[] = ".";
   char *res = evil_basename(path);

   fail_if(strcmp(res, expected) != 0);
}
EFL_END_TEST

EFL_START_TEST(evil_libgen_basename_null)
{
   char expected[] = ".";
   char *res = evil_basename(NULL);

   fail_if(strcmp(res, expected) != 0);
}
EFL_END_TEST

EFL_START_TEST(evil_libgen_basename_filename)
{
   char path[] = "bar.h";
   char expected[] = "bar.h";
   char *res = evil_basename(path);

   fail_if(strcmp(res, expected) != 0);
}
EFL_END_TEST

EFL_START_TEST(evil_libgen_basename_windows)
{
   char path[] = "C:\\foo\\bar.h";
   char expected[] = "bar.h";
   char *res = evil_basename(path);

   fail_if(strcmp(res, expected) != 0);
}
EFL_END_TEST

EFL_START_TEST(evil_libgen_dirname_simple)
{
   char path[] = "/bin/foo/bar.h";
   char expected[] = "/bin/foo";
   char *res = evil_dirname(path);

   fail_if(strcmp(res, expected) != 0);
}
EFL_END_TEST

EFL_START_TEST(evil_libgen_dirname_short)
{
   char path[] = "a";
   char expected[] = "";
   char *res = evil_dirname(path);

   fail_if(strcmp(res, expected) != 0);
}
EFL_END_TEST

EFL_START_TEST(evil_libgen_dirname_root)
{
   char path[] = "/";
   char expected[] = "";
   char *res = evil_dirname(path);

   fail_if(strcmp(res, expected) != 0);
}
EFL_END_TEST

EFL_START_TEST(evil_libgen_dirname_trailing)
{
   char path[] = "/bin/foo/";
   char expected[] = "/bin/foo";

   char *res = evil_dirname(path);

   fail_if(strcmp(res, expected) != 0);
}
EFL_END_TEST

EFL_START_TEST(evil_libgen_dirname_empty)
{
   char path[] = "";
   char expected[] = ".";
   char *res = evil_dirname(path);

   fail_if(strcmp(res, expected) != 0);
}
EFL_END_TEST

EFL_START_TEST(evil_libgen_dirname_null)
{
   char expected[] = ".";
   char *res = evil_dirname(NULL);

   fail_if(strcmp(res, expected) != 0);
}
EFL_END_TEST

EFL_START_TEST(evil_libgen_dirname_filename)
{
   char path[] = "bar.h";
   char expected[] = "";
   char *res = evil_dirname(path);

   fail_if(strcmp(res, expected) != 0);
}
EFL_END_TEST

EFL_START_TEST(evil_libgen_dirname_windows)
{
   char path[] = "C:\\foo\\bar.h";
   char expected[] = "C:\\foo";
   char *res = evil_dirname(path);

   fail_if(strcmp(res, expected) != 0);
}
EFL_END_TEST

void evil_test_libgen(TCase *tc)
{
   tcase_add_test(tc, evil_libgen_basename_simple);
   tcase_add_test(tc, evil_libgen_basename_short);
   tcase_add_test(tc, evil_libgen_basename_root);
   tcase_add_test(tc, evil_libgen_basename_trailing);
   tcase_add_test(tc, evil_libgen_basename_empty);
   tcase_add_test(tc, evil_libgen_basename_null);
   tcase_add_test(tc, evil_libgen_basename_filename);
   tcase_add_test(tc, evil_libgen_basename_windows);

   tcase_add_test(tc, evil_libgen_dirname_simple);
   tcase_add_test(tc, evil_libgen_dirname_short);
   tcase_add_test(tc, evil_libgen_dirname_root);
   tcase_add_test(tc, evil_libgen_dirname_trailing);
   tcase_add_test(tc, evil_libgen_dirname_empty);
   tcase_add_test(tc, evil_libgen_dirname_null);
   tcase_add_test(tc, evil_libgen_dirname_filename);
   tcase_add_test(tc, evil_libgen_dirname_windows);
}
