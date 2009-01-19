#ifdef HAVE_CONFIG_H
# include "config.h"
#endif /* HAVE_CONFIG_H */

#include <string.h>

#include <Evil.h>

#include "evil_suite.h"

static int
test_env_test_setenv_NULL(void)
{
   char *val;
   int   res;

   res = setenv("EVIL_TEST_ENV", NULL, 1);
   if (res < 0)
     return 0;

   val = getenv("EVIL_TEST_ENV");

   return val ? 0 : 1;
}

static int
test_env_test_setenv_NULL_after_set(void)
{
   char *val;
   int   res;

   res = setenv("EVIL_TEST_ENV", "val", 1);
   if (res < 0)
     return 0;

   val = getenv("EVIL_TEST_ENV");
   if (!val)
     return 0;

   if (strcmp(val, "val"))
     return 0;

   res = setenv("EVIL_TEST_ENV", NULL, 1);
   if (res < 0)
     return 0;

   val = getenv("EVIL_TEST_ENV");

   return val ? 0 : 1;
}

static int
test_env_test_getenv_one(void)
{
   char *val;
   int   res;

   res = setenv("EVIL_TEST_ENV", "val", 1);
   if (res < 0)
     return 0;

   val = getenv("EVIL_TEST_ENV");
   if (!val)
     return 0;

   if (strcmp(val, "val"))
     return 0;

   return 1;
}

static int
test_env_test_getenv_two(void)
{
   char *val;
   int   res;

   res = setenv("EVIL_TEST_ENV1", "val1", 1);
   if (res < 0)
     return 0;

   res = setenv("EVIL_TEST_ENV2", "val2", 1);
   if (res < 0)
     return 0;

   val = getenv("EVIL_TEST_ENV1");
   if (!val)
     return 0;
   if (strcmp(val, "val1"))
     return 0;

   val = getenv("EVIL_TEST_ENV2");
   if (!val)
     return 0;

   if (strcmp(val, "val2"))
     return 0;

   return 1;
}

static int
test_env_test_getenv_two_swapped(void)
{
   char *val;
   int   res;

   res = setenv("EVIL_TEST_ENV1", "val1", 1);
   if (res < 0)
     return 0;

   res = setenv("EVIL_TEST_ENV2", "val2", 1);
   if (res < 0)
     return 0;

   val = getenv("EVIL_TEST_ENV2");
   if (!val)
     return 0;
   if (strcmp(val, "val2"))
     return 0;

   val = getenv("EVIL_TEST_ENV1");
   if (!val)
     return 0;

   if (strcmp(val, "val1"))
     return 0;

   return 1;
}

static int
test_env_test_unsetenv(void)
{
   char *val;
   int   res;

   res = setenv("EVIL_TEST_ENV", "val", 1);
   if (res < 0)
     return 0;

   val = getenv("EVIL_TEST_ENV");
   if (!val)
     return 0;

   if (unsetenv("EVIL_TEST_ENV") != 0)
     return 0;

   val = getenv("EVIL_TEST_ENV");
   if (val)
     return 0;

   return 1;
}

static int
test_env_tests_run(suite *s)
{
   int res;

   res  = test_env_test_setenv_NULL();
   res &= test_env_test_setenv_NULL_after_set();
   res &= test_env_test_getenv_one();
   res &= test_env_test_getenv_two();
   res &= test_env_test_getenv_two_swapped();
   res &= test_env_test_unsetenv();

   return res;
}

int
test_environment(suite *s)
{

   return test_env_tests_run(s);
}
