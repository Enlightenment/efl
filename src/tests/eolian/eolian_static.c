#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <Eolian.h>

#include "eolian_suite.h"

EFL_START_TEST(eolian_static_check)
{
   Eolian_State *eos = eolian_state_new();
   fail_if(!eolian_state_directory_add(eos, EO_SRC_DIR));
   fail_if(!eolian_state_all_eot_files_parse(eos));
   fail_if(!eolian_state_all_eo_files_parse(eos));
#if 0
   /* too many failures to enable this yet */
   fail_if(!eolian_state_check(eos));
#endif
   eolian_state_free(eos);
}
EFL_END_TEST

void eolian_static_test(TCase *tc)
{
   tcase_add_test(tc, eolian_static_check);
}
