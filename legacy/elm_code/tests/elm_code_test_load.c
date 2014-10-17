#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "elm_code_suite.h"

START_TEST (elm_code_load)
{
   ck_assert(1);
}
END_TEST

void elm_code_test_load(TCase *tc)
{
   tcase_add_test(tc, elm_code_load);
}

