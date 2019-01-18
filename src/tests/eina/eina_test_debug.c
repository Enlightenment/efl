#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <Eina.h>

#include "eina_suite.h"
#ifndef _WIN32
# include <signal.h>
#endif
#ifdef SIGPROF
EFL_START_TEST(eina_test_debug_sighandler)
{
   /* ensure this doesn't crash */
   raise(SIGPROF);
}
EFL_END_TEST
#endif

void
eina_test_debug(TCase *tc)
{
#ifdef SIGPROF
   if (!eina_streq(getenv("CK_FORK"), "no"))
     tcase_add_test_raise_signal(tc, eina_test_debug_sighandler, SIGPROF);
#endif
}
