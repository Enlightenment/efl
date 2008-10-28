

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif /* HAVE_CONFIG_H */

#include <stdio.h>
#include <string.h>

#include <Evil.h>

#include "evil_suite.h"

static int
test_link_test_file_create(const char *name, const char *data)
{
   FILE       *f;
   size_t      length;
   size_t      res;

   f = fopen(name, "wb");
   if (!f)
     return 0;

   length = strlen(data) + 1;
   res = fwrite(data, 1, length, f);
   if (res < length)
     {
        fclose(f);
        return 0;
     }

   fclose(f);

   return 1;
}

static int
test_link_test_symlink(void)
{
   int res;

   if (!test_link_test_file_create("evil_test_link.dat",
                                   "evil_test_link symlink data\n"))
     return 0;

   if (symlink("evil_test_link.dat", "evil_test_link") < 0)
     return 0;

   if (unlink("evil_test_link.dat") < 0)
     return 0;

   return 1;
}

static int
test_link_tests_run(suite *s)
{
   return test_link_test_symlink();
}

int
test_link(suite *s)
{

   return test_link_tests_run(s);
}
