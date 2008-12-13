

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif /* HAVE_CONFIG_H */

#include <stdio.h>
#include <string.h>

#include <Evil.h>

#include "evil_suite.h"

static int
test_env_tests_run(suite *s)
{
   char *val;
   int   res;

   res = setenv("EVIL_TEST_ENV", "val1", 1);
   if (res < 0)
     return 0;

   val = getenv("EVIL_TEST_ENV");
   if (strcmp(val, "val1"))
     return 0;

   return 1;
}

int
test_environment(suite *s)
{

   return test_env_tests_run(s);
}
