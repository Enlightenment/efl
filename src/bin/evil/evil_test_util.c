#ifdef HAVE_CONFIG_H
# include "config.h"
#endif /* HAVE_CONFIG_H */

#include <string.h>

#include <Evil.h>

#include "evil_suite.h"
#include "evil_test_util.h"


static int test_path_absolute_test_1(void)
{
   char *path;
   int   result;

   path = NULL;
   result = evil_path_is_absolute(path);
   if (result != 0)
     return 0;

   return 1;
}

static int test_path_absolute_test_2(void)
{
   char *path;
   int   result;

   path = "1";
   result = evil_path_is_absolute(path);
   if (result != 0)
     return 0;

   return 1;
}

static int test_path_absolute_test_3(void)
{
   char *path;
   int   result;

   path = "1:\\";
   result = evil_path_is_absolute(path);
   if (result != 0)
     return 0;

   return 1;
}

static int test_path_absolute_test_4(void)
{
   char *path;
   int   result;

   path = "1/\\";
   result = evil_path_is_absolute(path);
   if (result != 0)
     return 0;

   return 1;
}

static int test_path_absolute_test_5(void)
{
   char *path;
   int   result;

   path = "F:/foo";
   result = evil_path_is_absolute(path);
   if (result == 0)
     return 0;

   return 1;
}

static int test_path_absolute_test_6(void)
{
   char *path;
   int   result;

   path = "C:\\foo";
   result = evil_path_is_absolute(path);
   if (result == 0)
     return 0;

   return 1;
}

static int
test_path_absolute_run(suite *s __UNUSED__)
{
   int res;

   res  = test_path_absolute_test_1();
   res &= test_path_absolute_test_2();
   res &= test_path_absolute_test_3();
   res &= test_path_absolute_test_4();
   res &= test_path_absolute_test_5();
   res &= test_path_absolute_test_6();

   return res;
}

int
test_util(suite *s)
{
   return test_path_absolute_run(s);
}
