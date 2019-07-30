#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include <Elementary.h>
#include "elm_suite.h"

EFL_START_TEST(elm_config_profiles)
{
   const char *dir, *profile;

   profile = elm_config_profile_get();
   fail_if(!profile);
   dir = elm_config_profile_dir_get(profile, EINA_TRUE);
   ck_assert(dir);
   elm_config_profile_dir_free(dir);

   dir = elm_config_profile_dir_get(profile, EINA_FALSE);
   ck_assert(dir);
   elm_config_profile_dir_free(dir);
}
EFL_END_TEST

void elm_test_config(TCase *tc)
{
   tcase_add_test(tc, elm_config_profiles);
}
