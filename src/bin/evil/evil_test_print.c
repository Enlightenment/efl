#ifdef HAVE_CONFIG_H
# include "config.h"
#endif /* HAVE_CONFIG_H */

#include <stdlib.h>
#include <stdio.h>

#include <Evil.h>

#include "evil_suite.h"
#include "evil_test_print.h"

static int
test_print_test(void)
{
  char buf[16];
  int i1 = 1;
  size_t i2 = 123456;
  int res;

  res = printf("%02hhd\n", i1);
  if (res != 3)
    return 0;

  res = snprintf(buf, sizeof(buf), "%zu", i2);
  if (res != 6)
    return 0;

  return 1;
}

static int
test_print_run(suite *s __UNUSED__)
{
   int res;

   res = test_print_test();

   return res;
}

int
test_print(suite *s)
{
   return test_print_run(s);
}
