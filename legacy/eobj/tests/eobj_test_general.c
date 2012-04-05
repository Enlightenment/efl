#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <stdio.h>

#include "eobj_suite.h"
#include "eobj.h"

START_TEST(eobj_simple)
{
}
END_TEST

void eobj_test_general(TCase *tc)
{
   tcase_add_test(tc, eobj_simple);
}
