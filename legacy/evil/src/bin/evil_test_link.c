

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
   if (!test_link_test_file_create("evil_test_link.dat",
                                   "evil_test_link symlink data\n"))
     return 0;

   if (symlink("evil_test_link.dat", "evil_test_link.lnk") < 0)
     {
        unlink("evil_test_link.dat");
        return 0;
     }

   if (unlink("evil_test_link.lnk") < 0)
     {
        unlink("evil_test_link.dat");
        return 0;
     }

   if (unlink("evil_test_link.dat") < 0)
     return 0;

   return 1;
}

static int
test_link_test_readlink(void)
{
   char    buf[1024];
   char   *data;
   FILE   *f;
   ssize_t s1;
   size_t  s2;
   int     l;

   data = "evil_test_link symlink data\n";

   if (!test_link_test_file_create("evil_test_link.dat", data))
     return 0;

   if (symlink("evil_test_link.dat", "evil_test_link.lnk") < 0)
     return 0;

   if ((s1 = readlink("evil_test_link.lnk", buf, 1023)) < 0)
     {
        unlink("evil_test_link.dat");
        unlink("evil_test_link.lnk");
        return 0;
     }

   buf[s1] = '\0';

   f = fopen(buf, "rb");
   if (!f)
     {
        unlink("evil_test_link.dat");
        unlink("evil_test_link.lnk");
        return 0;
     }

   l = strlen(data);
   s2 = fread(buf, 1, l + 1, f);

   if ((int)s2 != (l + 1))
     {
        fclose(f);
        unlink("evil_test_link.dat");
        unlink("evil_test_link.lnk");
        return 0;
     }

   if (strcmp(buf, data))
     {
        fclose(f);
        unlink("evil_test_link.dat");
        unlink("evil_test_link.lnk");
        return 0;
     }

   fclose(f);

   if (unlink("evil_test_link.lnk") < 0)
     {
        unlink("evil_test_link.dat");
        return 0;
     }

   if (unlink("evil_test_link.dat") < 0)
     return 0;

   return 1;
}

static int
test_link_tests_run(suite *s)
{
   int res;

   res = test_link_test_symlink();
   res &= test_link_test_readlink();

   return res;
}

int
test_link(suite *s)
{

   return test_link_tests_run(s);
}
