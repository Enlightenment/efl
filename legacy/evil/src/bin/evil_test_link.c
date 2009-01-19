#ifdef HAVE_CONFIG_H
# include "config.h"
#endif /* HAVE_CONFIG_H */

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
#ifdef _WIN32_WCE
   const char *old_name = "\\efl\\evil_test_link.dat";
   const char *new_name = "\\efl\\evil_test_link.lnk";
#else
   const char *old_name = "evil_test_link.dat";
   const char *new_name = "evil_test_link.lnk";
#endif

   if (!test_link_test_file_create(old_name,
                                   "evil_test_link symlink data\n"))
     return 0;

   if (symlink(old_name, new_name) < 0)
     {
        unlink(old_name);
        return 0;
     }

   if (unlink(new_name) < 0)
     {
        unlink(old_name);
        return 0;
     }

   if (unlink(old_name) < 0)
     return 0;

   return 1;
}

static int
test_link_test_readlink(void)
{
   char        buf[1024];
#ifdef _WIN32_WCE
   const char *old_name = "\\efl\\evil_test_link.dat";
   const char *new_name = "\\efl\\evil_test_link.lnk";
#else
   const char *old_name = "evil_test_link.dat";
   const char *new_name = "evil_test_link.lnk";
#endif
   const char *data = "evil_test_link symlink data\n";
   FILE       *f;
   ssize_t     s1;
   size_t      s2;
   int         l;

   if (!test_link_test_file_create(old_name, data))
     return 0;

   if (symlink(old_name, new_name) < 0)
     return 0;

   if ((s1 = readlink(new_name, buf, 1023)) < 0)
     {
        unlink(old_name);
        unlink(new_name);
        return 0;
     }

   buf[s1] = '\0';

   f = fopen(buf, "rb");
   if (!f)
     {
        unlink(old_name);
        unlink(new_name);
        return 0;
     }

   l = strlen(data);
   s2 = fread(buf, 1, l + 1, f);

   if ((int)s2 != (l + 1))
     {
        fclose(f);
        unlink(old_name);
        unlink(new_name);
        return 0;
     }

   if (strcmp(buf, data))
     {
        fclose(f);
        unlink(old_name);
        unlink(new_name);
        return 0;
     }

   fclose(f);

   if (unlink(new_name) < 0)
     {
        unlink(old_name);
        return 0;
     }

   if (unlink(old_name) < 0)
     return 0;

   return 1;
}

static int
test_link_tests_run(suite *s)
{
   int res;

   res  = test_link_test_symlink();
   res &= test_link_test_readlink();

   return res;
}

int
test_link(suite *s)
{

   return test_link_tests_run(s);
}
