#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include <Elementary.h>
#include "elm_suite.h"

START_TEST (elm_main)
{
    ck_assert(elm_init(1, NULL) == 1);
    ck_assert(elm_shutdown() == 0);
}
END_TEST

void elm_test_init(TCase *tc)
{
   tcase_add_test(tc, elm_main);
}
