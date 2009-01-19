#ifdef HAVE_CONFIG_H
# include "config.h"
#endif /* HAVE_CONFIG_H */

#include <string.h>

#include <Evil.h>

#include "evil_suite.h"

int test_realpath_test(void)
{
   char  buf[PATH_MAX];
   char *filename;
   char *result;

   filename = "evil_suite.exe";

   if (!(result = realpath(filename, buf)))
     return 0;

   printf ("res : %s\n", buf);

   return 1;
}

static int
test_realpath_run(suite *s)
{
   int res;

   res = test_realpath_test();

   return res;
}

int
test_realpath(suite *s)
{

   return test_realpath_run(s);
}
