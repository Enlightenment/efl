#ifdef HAVE_CONFIG_H
# include "config.h"
#endif /* HAVE_CONFIG_H */

#include <string.h>

#include <Evil.h>

#include "evil_suite.h"

int test_mkstemp_test(void)
{
   char  _template[PATH_MAX];
   char  cwd[PATH_MAX];
   int   fd;

#ifdef _WIN32_WCE
   if (!getcwd(cwd, PATH_MAX))
     return 0;
   _snprintf(_template, PATH_MAX, "%s\\%s", cwd, "file_XXXXXX");
#else
   _snprintf(_template, PATH_MAX, "%s", "file_XXXXXX");
#endif

   fd = mkstemp(_template);

   if (fd < 0)
     return 0;

   return 1;
}

static int
test_mkstemp_run(suite *s)
{
   int res;

   res = test_mkstemp_test();

   return res;
}

int
test_mkstemp(suite *s)
{

   return test_mkstemp_run(s);
}
