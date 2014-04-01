#ifdef HAVE_CONFIG_H
# include "config.h"
#endif /* HAVE_CONFIG_H */

#include <string.h>

#include <Evil.h>

#include "evil_suite.h"
#include "evil_test_mkstemp.h"


static int
test_mkstemp_test(void)
{
   char  _template[PATH_MAX];
#ifdef _WIN32_WCE
   char  cwd[PATH_MAX];
#endif
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
test_mkstemps_test(void)
{
   char  _template[PATH_MAX];
#ifdef _WIN32_WCE
   char  cwd[PATH_MAX];
#endif
   int   fd;

#ifdef _WIN32_WCE
   if (!getcwd(cwd, PATH_MAX))
     return 0;
   _snprintf(_template, PATH_MAX, "%s\\%s", cwd, "file_XXXXXX.ext");
#else
   _snprintf(_template, PATH_MAX, "%s", "file_XXXXXX.ext");
#endif

   fd = mkstemps(_template, 4);

   if (fd < 0)
     return 0;

   return 1;
}

static int
test_mkstemp_run(suite *s)
{
   int res;
   (void) s;

   res = test_mkstemp_test();

   return res;
}

static int
test_mkstemps_run(suite *s)
{
   int res;
   (void) s;

   res = test_mkstemps_test();

   return res;
}

int
test_mkstemp(suite *s)
{
   int res;

   res  = test_mkstemp_run(s);
   res &= test_mkstemps_run(s);

   return res;
}
