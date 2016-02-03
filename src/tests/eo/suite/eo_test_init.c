#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <stdio.h>

#include "Eo.h"
#include "eo_suite.h"

START_TEST(eo_simple)
{
   fail_if(!eo_init()); /* one init by test suite */
   fail_if(!eo_init());
   fail_if(!eo_shutdown());
   fail_if(eo_shutdown());
}
END_TEST

START_TEST(eo_init_shutdown)
{
   fail_if(!eo_init());
   ck_assert_str_eq("Eo_Base", eo_class_name_get(EO_BASE_CLASS));
   fail_if(eo_shutdown());

   fail_if(!eo_init());
   ck_assert_str_eq("Eo_Base", eo_class_name_get(EO_BASE_CLASS));
   fail_if(eo_shutdown());
}
END_TEST

void eo_test_init(TCase *tc)
{
   tcase_add_test(tc, eo_simple);
   tcase_add_test(tc, eo_init_shutdown);
}
