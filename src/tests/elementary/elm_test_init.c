#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include <Elementary.h>
#include "elm_suite.h"

EFL_START_TEST(elm_main)
{
}
EFL_END_TEST

void elm_test_init(TCase *tc)
{
   tcase_add_test(tc, elm_main);
}
